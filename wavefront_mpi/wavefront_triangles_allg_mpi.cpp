#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <mpi.h>

typedef struct
{
    int start_index;
    int size_side;
    bool is_diag;
} triangle;

void printTriangle(triangle t)
{
    std::cout << "start_index: " << t.start_index << std::endl;
    std::cout << "size_side: " << t.size_side << std::endl;
    std::cout << "is_diag: " << t.is_diag << std::endl;
    std::cout << std::endl;
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

void printDoubleArray(std::vector<double> v, int n)
{
    for (int i = 0; i < n; ++i)
        std::cout << std::ceil(v[i] * 100) / 100 << "\t";
}

void printIntArray(std::vector<int> v, int n)
{
    for (int i = 0; i < n; ++i)
        std::cout << v[i] << "\t";
}

inline int min(int a, int b)
{
    return a > b ? b : a;
}

inline int max(int a, int b)
{
    return a > b ? a : b;
}

std::vector<std::vector<triangle *>>
divide_upper_matrix_into_triangles(int n, int nw)
{
    std::vector<std::vector<triangle *>> triangles;
    std::vector<triangle *> triangles_straight;
    std::vector<triangle *> triangles_reversed;

    for (int i = 1; i < n; nw--)
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
            a->start_index = triangles_straight[i_triangle + 1]->start_index - n;
            if (i_triangle == 0)
                a->size_side = min(triangles_straight[i_triangle]->size_side, triangles_straight[i_triangle + 1]->size_side);
            else
                a->size_side = triangles_reversed[i_triangle - 1]->size_side;
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
            for (int start_row = n * row + row, start_col = n * (j - start_row) + j; start_row < j; ++start_row, --start_col) {
                res += M[start_row] * M[start_col];
            }
            res = cbrt(res);
            M[j] = res;
            M[col * n + row] = res;
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
            for (int start_row = n * row + row, start_col = (n * (j - start_row)) + j; start_row < j; ++start_row, --start_col) {
                res += M[start_row] * M[start_col];
            }
            res = cbrt(res);
            M[j] = res;
            M[col * n + row] = res;
            values.push_back(res);
        }
    }
    return values;
}

void update_matrix_with_triangle(std::vector<double> &M, std::vector<double> values, triangle t, int n)
{
    int values_index = 0;
    for (int i = 0; i < t.size_side; i++)
    {
        int end_cicle = (t.size_side * n + t.size_side) + t.start_index - (n * i);
        int j = t.start_index + i;
        for (int row = j / n, col = j % n; j < end_cicle && row < col; j += n + 1, row = j / n, col = j % n)
        {
            M[j] = values[values_index];
            M[col * n + row] = values[values_index];
            values_index++;
        }
    }
}

void update_matrix_with_reversed_triangle(std::vector<double> &M, std::vector<double> values, triangle t, int n)
{
    int values_index = 0;
    for (int i = 0; i < t.size_side; i++)
    {
        int j = t.start_index - (i * n);
        for (int row = j / n, col = j % n; j < t.start_index + i + 1 && row < col; j += n + 1, row = j / n, col = j % n)
        {
            M[j] = values[values_index];
            M[col * n + row] = values[values_index];
            values_index++;
        }
    }
}

int count_elements_in_triangle(triangle t, int n)
{
    int count = 0;
    for (int i = 0; i < t.size_side; i++)
    {
        int end_cicle = (t.size_side * n + t.size_side) + t.start_index - (n * i);
        int j = t.start_index + i;
        for (int row = j / n, col = j % n; j < end_cicle && row < col; j += n + 1, row = j / n, col = j % n)
            count++;
    }
    return count;
}

int count_elements_in_reversed_triangle(triangle t, int n)
{
    int count = 0;
    for (int i = 0; i < t.size_side; i++)
    {
        int j = t.start_index - (i * n);
        for (int row = j / n, col = j % n; j < t.start_index + i + 1 && row < col; j += n + 1, row = j / n, col = j % n)
            count++;
    }
    return count;
}

std::vector<double> get_subarray(std::vector<double> global_values, int start_index, int n_elements) {
    std::vector<double> values;
    for (int i = start_index; i < start_index + n_elements; i++)
        values.push_back(global_values[i]);
    return values;
}

std::vector<int> check_overlapping_indices(std::vector<triangle*> triangles, int n) {
    std::vector<int> indices;
    for (int i = 0; i < triangles.size() - 1; i++) {
        int upper_triangle_i = triangles[i]->start_index + triangles[i]->size_side;
        int lower_triangle_i = triangles[i + 1]->start_index + (triangles[i + 1]->size_side * n);
        if (upper_triangle_i == lower_triangle_i)
            indices.push_back(upper_triangle_i);
    }
    return indices;
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

    int n_processes, n_processes_group, myrank, my_rank_group, my_size_group;
    MPI_Group world_group;

    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

    int n = atoi(argv[1]);

    std::vector<int> counts(n_processes, 0);
    std::vector<int> displs(n_processes, 0);
    std::vector<double> M(n * n, 0);
    std::vector<double> global_values;
    std::vector<double> values;
    std::vector<int> overlapping_indices;
    int displ = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<triangle *>> triangles = divide_upper_matrix_into_triangles(n, n_processes);

    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<double>(m + 1) / n;

    int cycle = 0;
    for (int i = 0; i < (int)triangles.size(); i++, cycle++)
    {
        n_processes_group = n_processes;
        if (triangles[i].size() < n_processes)
            n_processes_group = triangles[i].size();

        std::vector<int> ranks_group;
        for (int i = 0; i < n_processes_group; i++)
            ranks_group.push_back(i);

        MPI_Group new_group;
        MPI_Comm new_comm;

        MPI_Group_incl(world_group, n_processes_group, ranks_group.data(), &new_group);
        MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm);
        MPI_Group_rank(new_group, &my_rank_group);
        MPI_Group_size(new_group, &n_processes_group);

        if (my_rank_group < 0)
            continue;

        counts.resize(n_processes_group);
        displs.resize(n_processes_group);

        displ = 0;
        values.clear();

        for (int j = 0; j < n_processes_group; j++)
        {
            counts[j] = 0;
            displs[j] = 0;
        }

        if ((int)triangles[i].size() > 0 && (int)triangles[i][0]->is_diag == false)
            overlapping_indices = check_overlapping_indices(triangles[i], n);

        for (int j = 0; j < (int)triangles[i].size(); j++)
        {
            if (my_rank_group == j)
            {
                if (triangles[i][j]->is_diag)
                    values = iterate_on_matrix_by_triangle(M, *triangles[i][j], n);
                else
                    values = iterate_on_matrix_by_reversed_triangle(M, *triangles[i][j], n);
            }

            if (triangles[i][j]->is_diag) {
                counts[j] = count_elements_in_triangle(*triangles[i][j], n);
                displs[j] = displ;
            } else {
                counts[j] = count_elements_in_reversed_triangle(*triangles[i][j], n);
                displs[j] = displ;
            }
            displ += counts[j];
        }

        global_values.resize(displ);

        MPI_Allgatherv(values.data(), values.size(), MPI_DOUBLE,
                       global_values.data(), counts.data(), displs.data(), MPI_DOUBLE,
                       new_comm);

        for (int j = 0; j < (int)triangles[i].size(); j++)
        {
            values = get_subarray(global_values, displs[j], counts[j]);
            if (triangles[i][j]->is_diag)
                update_matrix_with_triangle(M, values, *triangles[i][j], n);
            else
                update_matrix_with_reversed_triangle(M, values, *triangles[i][j], n);
        }

        for (int j = 0; j < overlapping_indices.size(); j++) {
            double res = 0.0;
            int row = j / n, col = j % n;
            for (int start_row = n * row + row, start_col = (n * (j - start_row)) + j; start_row < j; ++start_row, --start_col)
                res += M[start_row] * M[start_col];
            res = cbrt(res);
            M[j] = res;
            M[col * n + row] = res;
        }

        if (new_comm != MPI_COMM_NULL)
        {
            MPI_Comm_free(&new_comm);
            MPI_Group_free(&new_group);
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    if (!myrank)
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "time: " << duration.count() << std::endl;
        std::cout << "last: " << M[n - 1] << std::endl;
        std::cout << "end execution" << std::endl;
    }
    MPI_Finalize();
    return 0;
}