#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <mpi.h>

void divide_job_into_parts(int number, std::vector<int> &displs, std::vector<int> &counts, int n)
{
    if (n <= 0 || number < 0)
        return;

    int quotient = number / n;
    int remainder = number % n;
    int start = 0;

    for (int i = 0; i < n; ++i)
        counts[i] = quotient;

    for (int i = 0; i < remainder; ++i)
        counts[i] += 1;

    for (int i = 1; i < n; ++i)
    {
        start += counts[i - 1];
        displs[i] = start;
    }
}

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

inline int min(int a, int b)
{
    return a > b ? b : a;
}

inline int max(int a, int b)
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
        int remainder = d % nw;

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

std::vector<double> iterate_on_matrix_by_triangle(std::vector<double> &M, triangle t, int n)
{
    std::vector<double> values;
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
            values.push_back(res);
        }
    }
    return values;
}

std::vector<double> iterate_on_matrix_by_reversed_triangle(std::vector<double> &M, triangle t, int n)
{
    std::vector<double> values;
    for (int i = 0; i < t.size_side; i++)
    {
        int j = t.start_index - (i * n);
        for (int row = j / n, col = j % n; j < t.start_index + i + 1 && row < col; j += n + 1, row = j / n, col = j % n)
        {
            double res = 0.0;
            for (int start_row = n * row + row, start_col = (n * (j - start_row)) + j; start_row < j; ++start_row, --start_col)
                res += M[start_row] * M[start_col];
            res = cbrt(res);
            values.push_back(res);
        }
    }
    return values;
}

void printMatrix(std::vector<double> M, int n)
{
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; j++)
            std::cout << std::ceil(M[i * n + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "./" << argv[0] << " <n>" << std::endl;
        return -1;
    }
    std::cout << "start execution" << std::endl;

    MPI_Init(&argc, &argv);

    int n_processes;
    int myrank;

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);


    std::vector<int> counts(n_processes, 0);
    std::vector<int> displs(n_processes, 0);

    int n = atoi(argv[1]);
    int ntriangles = atoi(argv[2]);
    std::vector<double> M(n * n, 1);
    std::vector<std::vector<triangle *>> triangles = divide_upper_matrix_into_triangles(M, n, ntriangles);

    auto start = std::chrono::high_resolution_clock::now();

    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<double>(m + 1) / n;

    for (int i = 0; i < (int)triangles.size(); i++)
    {
        for (int j = 0; j < (int)triangles[i].size(); j++)
        {
            std::vector<double> values;
            if (*triangles[i][j]->is_diag)
                values = iterate_on_matrix_by_triangle(M, *triangles[i][j], n);
            else
                values = iterate_on_matrix_by_reversed_triangle(M, *triangles[i][j], n);
        }
    }

    for (int k = 1; k < n; k++)
    {
        std::vector<double> values;
        std::vector<double> global_values(n - k + 1);

        divide_job_into_parts(n - k, displs, counts, n_processes);

        for (int i = displs[myrank]; i < displs[myrank] + counts[myrank]; i++)
        {
            double value = 0.0;
            for (int t = 0; t < k; ++t)
                value += M[i * n + i + t] * M[(i + k) * n + (i + k) - t];
            value = cbrt(value);
            values.push_back(value);
        }

        MPI_Allgatherv(values.data(), values.size(), MPI_DOUBLE,
                       global_values.data(), counts.data(), displs.data(), MPI_DOUBLE,
                       MPI_COMM_WORLD);

        for (int i = 0; i < n - k; i += 1)
        {
            M[i * n + i + k] = global_values[i];
            M[(i + k) * n + i] = global_values[i];
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    if (!myrank)
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "time: " << duration.count() << std::endl;
        std::cout << "end execution" << std::endl;
    }

    MPI_Finalize();
    return 0;
}