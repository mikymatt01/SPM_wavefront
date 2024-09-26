#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <ff/ff.hpp>
#include <ff/farm.hpp>

using namespace ff;

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

void printMatrix(std::vector<double> M, uint64_t n)
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
divide_upper_matrix_into_triangles(std::vector<double> M, int n, int nw)
{
    std::vector<std::vector<triangle *>> triangles;
    std::vector<triangle *> triangles_straight;
    std::vector<triangle *> triangles_reversed;

    for (int i = 0; i < n;)
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

void iterate_on_matrix_by_triangle(std::vector<double> &M, triangle t, int n)
{
    if (t.is_diag)
    {
        for (int i = 0; i < t.size_side; i++)
        {
            int end_cicle = (t.size_side * n + t.size_side) + t.start_index - (n * i);
            for (int j = t.start_index + i; j < end_cicle; j += n + 1)
            {
                int row = std::floor((float)j / n);
                int col = j % n;
                int start_row = n * row + row;
                int start_col = n * (j - start_row) + j;
                if (start_row != j)
                {
                    double res = 0.0;
                    for (; start_row < j && start_col > j; ++start_row, --start_col)
                        res += M[start_row] * M[start_col];
                    res = cbrt(res);

                    M[j] = res;
                    M[col * n + row] = res;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < t.size_side; i++)
        {
            for (int j = t.start_index - (i * n); j <= t.start_index + i && j < std::ceil((float)t.start_index / n) * n; j += n + 1)
            {
                int row = std::floor((float)j / n);
                int col = j % n;
                int start_row = n * row + row;
                int start_col = n * (j - start_row) + j;
                if (start_row != j)
                {
                    double res = 0.0;
                    for (; start_row < j && start_col > j; ++start_row, --start_col)
                        res += M[start_row] * M[start_col];
                    res = cbrt(res);

                    M[j] = res;
                    M[col * n + row] = res;
                }
            }
        }
    }
}

using task = std::pair<int, int>;

struct Emitter : ff_monode_t<int, task>
{

    Emitter(const std::vector<std::vector<triangle *>> &_triangles, std::vector<double> &_M, int _n) : triangles(_triangles), M(_M), n(_n) {}

    task *svc(int *feedback)
    {
        if (feedback != nullptr)
        {
            tasks_received++;
            delete feedback;
        }

        if (i < (int)triangles.size())
        {
            if (tasks_received == tasks_sent)
            {
                tasks_received = 0;
                tasks_sent = 0;
                for (int j = 0; j < (int)triangles[i].size(); j++)
                {
                    ff_send_out(new task(i, j));
                    tasks_sent++;
                }
                i++;
            }
        }

        if (i == triangles.size())
            return EOS;
        return GO_ON;
    }

    const std::vector<std::vector<triangle *>> &triangles;
    std::vector<double> &M;
    int n;
    int tasks_received = 0;
    int tasks_sent = 0;
    int i = 0;
};

struct Worker : ff::ff_node_t<task, int>
{
    Worker(int _n, std::vector<double> &_M, const std::vector<std::vector<triangle *>> &_triangles) : n(_n), M(_M), triangles(_triangles) {}

    int *svc(task *task)
    {
        int i = task->first;
        int j = task->second;
        iterate_on_matrix_by_triangle(M, *triangles[i][j], n);
        delete task;
        ff_send_out(new int(1));
        return GO_ON;
    }

    int n;
    std::vector<double> &M;
    const std::vector<std::vector<triangle *>> &triangles;
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
    std::vector<double> M(n * n, 1);
    ssize_t nworkers = atoi(argv[2]); // ff_numCores();

    auto start_compute = std::chrono::high_resolution_clock::now();

    const std::vector<std::vector<triangle *>> triangles = divide_upper_matrix_into_triangles(M, n, nworkers);

    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<double>(m + 1) / n;

    Emitter emitter(triangles, M, n);
    std::vector<std::unique_ptr<ff::ff_node>> workers;
    for (int i = 0; i < nworkers; ++i)
        workers.push_back(std::make_unique<Worker>(n, M, triangles));
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

    std::cout << "time compute: " << duration_compute.count() << std::endl;
    std::cout << "end execution" << std::endl;
    std::cout << M[n - 1] << std::endl;
    std::cout << std::endl;
    return 0;
}
