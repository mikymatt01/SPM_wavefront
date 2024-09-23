#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

void printMatrix(std::vector<double> M, int n)
{
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; j++)
            std::cout << std::ceil(M[i * n + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

inline void init_matrix(std::vector<double> &M, int n)
{
    int value = 0;
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; j++)
        {
            M[i * n + j] = value;
            value++;
        }
    }
}

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

int min(int a, int b)
{
    return a > b ? b : a;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

inline std::vector<std::vector<triangle *>>
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

        for (int i_triangle = triangles_straight.size() - n_straight_triangles; i_triangle < triangles_straight.size() - 1; ++i_triangle)
        {
            triangle *a = (triangle *)malloc(sizeof(triangle));
            if (i_triangle == 0)
                a->size_side = min(triangles_straight[i_triangle]->size_side, triangles_straight[i_triangle + 1]->size_side);
            else
                a->size_side = triangles_straight[i_triangle - 1]->size_side;

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

void printTriangle(triangle t)
{
    std::cout << "start_row: " << t.start_index << std::endl;
    std::cout << "size_side: " << t.size_side << std::endl;
    std::cout << "is_diag: " << t.is_diag << std::endl;
}

void printArray(std::vector<float> x, int n)
{
    for (int i = 0; i < n; i++)
        std::cout << x[i] << " ";
    std::cout << std::endl;
}

inline void iterate_on_matrix_by_triangle(std::vector<double> &M, triangle t, int n)
{
    if (t.is_diag)
    {
        auto right_triangle_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < t.size_side; i++)
        {
            for (int j = t.start_index + i; j < (t.size_side * n + t.size_side) + t.start_index - (n * i); j += n + 1)
            {
                std::vector<float> x;
                std::vector<float> y;
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
        auto right_triangle_end = std::chrono::high_resolution_clock::now();
        auto duration_right_triangle = std::chrono::duration_cast<std::chrono::milliseconds>(right_triangle_end - right_triangle_start);
        // printTriangle(t);
        // std::cout << "time right triangle compute: " << duration_right_triangle.count() << std::endl;
    }
    else
    {
        auto reversed_triangle_start = std::chrono::high_resolution_clock::now();
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
        auto reversed_triangle_end = std::chrono::high_resolution_clock::now();
        auto duration_resersed_triangle = std::chrono::duration_cast<std::chrono::milliseconds>(reversed_triangle_end - reversed_triangle_start);
        // printTriangle(t);
        // std::cout << "time reversed triangle compute:: " << duration_resersed_triangle.count() << std::endl;
    }
    std::cout << std::endl;
}

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
    // ff::ParallelFor pf(nworkers);
    auto start_compute = std::chrono::high_resolution_clock::now();

    auto start_divide = std::chrono::high_resolution_clock::now();
    const std::vector<std::vector<triangle *>> triangles = divide_upper_matrix_into_triangles(M, n, nworkers);
    auto end_divide = std::chrono::high_resolution_clock::now();

    auto start_init = std::chrono::high_resolution_clock::now();
    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<double>(m + 1) / n;
    auto end_init = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < triangles[i].size(); j++)
        {
            iterate_on_matrix_by_triangle(M, *triangles[i][j], n);
        }
        /*pf.parallel_for(0, triangles[i].size(), 1, [&](const long j)
                        { iterate_on_matrix_by_triangle(M, *triangles[i][j], n); }, nworkers);*/
    }
    auto end_compute = std::chrono::high_resolution_clock::now();

    auto duration_divide = std::chrono::duration_cast<std::chrono::milliseconds>(end_divide - start_divide);
    auto duration_init = std::chrono::duration_cast<std::chrono::milliseconds>(end_init - start_init);
    auto duration_compute = std::chrono::duration_cast<std::chrono::milliseconds>(end_compute - start_compute);
    // std::cout << "time divide: " << duration_divide.count() << std::endl;
    // std::cout << "time init: " << duration_init.count() << std::endl;
    std::cout << "time compute: " << duration_compute.count() << std::endl;
    std::cout << "end execution" << std::endl;
    std::cout << M[n - 1] << std::endl;
    return 0;
}
