#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <ff/parallel_for.hpp>
#include <ff/map.hpp>

using namespace ff;

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

using task = triangle;

void printMatrix(std::vector<float> M, uint64_t n)
{
    for (uint64_t i = 0; i < n; ++i)
    {
        for (uint64_t j = 0; j < n; j++)
            std::cout << std::ceil(M[i * n + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

int min(int a, int b)
{
    return a > b ? b : a;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

std::vector<std::vector<triangle *>>
divide_upper_matrix_into_triangles(int n, int nw)
{
    std::vector<std::vector<triangle *>> triangles;
    std::vector<triangle *> triangles_straight;
    std::vector<triangle *> triangles_reversed;

    for (int i = 1; i < n;)
    {
        int d = n - i;
        int start_index = i;
        int n_straight_triangles = d > nw ? nw : d;
        int quotient = d / nw;
        int remainder = d - nw * quotient;

        for (int i_triangle = 0; i_triangle < n_straight_triangles; i_triangle++)
        {
            triangle *a = (triangle *)malloc(sizeof(triangle));
            a->start_index = start_index;
            a->size_side = remainder > 0 ? quotient + 1 : quotient;
            a->is_diag = true;

            remainder--;
            start_index = a->size_side * n + a->size_side + start_index;
            triangles_straight.push_back(a);

            if (i_triangle == 0)
                i += triangles_straight[0]->size_side;
            if (i_triangle == 1)
                i += triangles_straight[1]->size_side == triangles_straight[0]->size_side ? 1 : 0;
        }

        for (int i_triangle = 0; i_triangle < n_straight_triangles - 1; ++i_triangle)
        {
            triangle *a = (triangle *)malloc(sizeof(triangle));
            if (i_triangle == 0)
                a->size_side = min(triangles_straight[i_triangle]->size_side, triangles_straight[i_triangle + 1]->size_side);
            else
                a->size_side = triangles_reversed[i_triangle - 1]->size_side;

            a->start_index = triangles_straight[i_triangle + 1]->start_index - n;
            a->is_diag = false;

            triangles_reversed.push_back(a);
        }

        triangles.push_back(triangles_straight);
        triangles.push_back(triangles_reversed);

        triangles_straight.clear();
        triangles_reversed.clear();
    }

    return triangles;
}

struct Emitter : ff_monode_t<int, task>
{

    Emitter(const std::vector<std::vector<triangle *>> &_triangles, std::vector<float> &_M, int _n, int _nw)
        : triangles(_triangles), M(_M), n(_n), nw(_nw) {}

    void compute_triangle(triangle t)
    {
        for (int i = 0; i < t.size_side; i++)
        {
            int end_cicle = (t.size_side * n + t.size_side) + t.start_index - (n * i);
            int j = t.start_index + i;
            for (int row = j / n, col = j % n; j < end_cicle && row < col; j += n + 1, row = j / n, col = j % n)
            {
                double res = 0.0;
                for (int start_row = n * row + row, start_col = n * (j - start_row) + j; start_row < j; ++start_row, --start_col)
                    res += M[start_row] * M[start_col];
                res = cbrt(res);

                M[j] = res;
                M[col * n + row] = res;
            }
        }
    }

    std::vector<int> check_overlapping_indices(std::vector<triangle *> triangles, int n)
    {
        std::vector<int> indices;
        for (int i = 0; i < triangles.size() - 1; i++)
        {
            int upper_triangle_i = triangles[i]->start_index + triangles[i]->size_side;
            int lower_triangle_i = triangles[i + 1]->start_index + (triangles[i + 1]->size_side * n);
            if (upper_triangle_i == lower_triangle_i)
                indices.push_back(upper_triangle_i);
        }
        return indices;
    }

    void compute_reversed_triangle(triangle t)
    {
        for (int i = 0; i < t.size_side; i++)
        {
            int j = t.start_index - (i * n);
            for (int row = j / n, col = j % n; j < t.start_index + i + 1 && row < col; j += n + 1, row = j / n, col = j % n)
            {
                double res = 0.0;
                for (int start_row = n * row + row, start_col = (n * (j - start_row)) + j; start_row < j; ++start_row, --start_col)
                    res += M[start_row] * M[start_col];
                res = cbrt(res);

                M[j] = res;
                M[col * n + row] = res;
            }
        }
    }

    task *svc(int *feedback)
    {
        if (feedback != nullptr)
        {
            tasks_received++;
            delete feedback;
        }
        std::vector<int> overlapping_indices;

        if (i < (int)triangles.size())
        {
            if (tasks_received == tasks_sent)
            {
                tasks_received = 0;
                tasks_sent = 0;
                int j = 0;
                for (j = 0; j < (int)triangles[i].size() && j < nw; j++)
                {
                    ff_send_out(triangles[i][j]);
                    tasks_sent++;
                }

                if (triangles[i].size() > 0 && j < (int)triangles[i].size())
                {
                    if (triangles[i][j]->is_diag)
                        compute_triangle(*triangles[i][j]);
                    else
                        compute_reversed_triangle(*triangles[i][j]);
                }

                if ((int)triangles[i].size() > 0 && (int)triangles[i][0]->is_diag == false)
                    overlapping_indices = check_overlapping_indices(triangles[i], n);

                for (int j = 0; j < overlapping_indices.size(); j++)
                {
                    double res = 0.0;
                    int row = j / n, col = j % n;
                    for (int start_row = n * row + row, start_col = (n * (j - start_row)) + j; start_row < j; ++start_row, --start_col)
                        res += M[start_row] * M[start_col];
                    res = cbrt(res);
                    M[j] = res;
                    M[col * n + row] = res;
                }
                i++;
            }
        }

        if (i == triangles.size())
            return EOS;
        return GO_ON;
    }

    const std::vector<std::vector<triangle *>> &triangles;
    std::vector<float> &M;
    int n;
    int nw;
    int tasks_received = 0;
    int tasks_sent = 0;
    int i = 0;
};

struct Worker : ff_Map<task, int>
{
    using map = ff_Map<task, int>;

    Worker(int _n, std::vector<float> &_M)
        : n(_n), M(_M) {}

    int *svc(task *task)
    {
        triangle t = *task;
        if (t.is_diag)
        {
            for (int i = 0; i < t.size_side; i++)
            {
                int end_cicle = (t.size_side * n + t.size_side) + t.start_index - (n * i);
                int j = t.start_index + i;
                for (int row = j / n, col = j % n; j < end_cicle && row < col; j += n + 1, row = j / n, col = j % n)
                {
                    double res = 0.0;
                    for (int start_row = n * row + row, start_col = n * (j - start_row) + j; start_row < j; ++start_row, --start_col)
                        res += M[start_row] * M[start_col];
                    res = cbrt(res);

                    M[j] = res;
                    M[col * n + row] = res;
                }
            }
        }

        if (!t.is_diag)
        {
            for (int i = 0; i < t.size_side; i++)
            {
                int j = t.start_index - (i * n);
                for (int row = j / n, col = j % n; j < t.start_index + i + 1 && row < col; j += n + 1, row = j / n, col = j % n)
                {
                    double res = 0.0;
                    for (int start_row = n * row + row, start_col = (n * (j - start_row)) + j; start_row < j; ++start_row, --start_col)
                        res += M[start_row] * M[start_col];
                    res = cbrt(res);

                    M[j] = res;
                    M[col * n + row] = res;
                }
            }
        }

        delete task;
        ff_send_out(new int(1));
        return GO_ON;
    }

    int n;
    std::vector<float> &M;
};

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "./" << argv[0] << " <n> <nw>" << std::endl;
        return -1;
    }
    std::cout << "start execution" << std::endl;

    int n = atoi(argv[1]);
    ssize_t nworkers = atoi(argv[2]);
    std::vector<float> M(n * n, 1);

    auto start_compute = std::chrono::high_resolution_clock::now();

    const std::vector<std::vector<triangle *>> triangles = divide_upper_matrix_into_triangles(n, nworkers);

    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<float>(m + 1) / n;

    Emitter emitter(triangles, M, n, nworkers - 1);

    std::vector<std::unique_ptr<ff::ff_node>>
        workers;
    for (int i = 0; i < nworkers - 1; ++i)
        workers.push_back(std::make_unique<Worker>(n, M));
    ff::ff_Farm<int> farm(std::move(workers), emitter);

    farm.remove_collector();
    farm.wrap_around();
    farm.set_scheduling_ondemand();

    if (farm.run_and_wait_end() < 0)
    {
        std::cerr << "Error running the farm" << std::endl;
        return -1;
    }

    auto end_compute = std::chrono::high_resolution_clock::now();
    auto duration_compute = std::chrono::duration_cast<std::chrono::milliseconds>(end_compute - start_compute);
    std::cout << "time: " << duration_compute.count() << std::endl;
    std::cout << "end execution" << std::endl;
    std::cout << "last: " << M[n - 1] << std::endl;
    return 0;
}
