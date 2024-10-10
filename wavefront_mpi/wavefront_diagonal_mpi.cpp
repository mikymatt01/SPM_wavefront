#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <mpi.h>

struct Dependence {
    std::vector<double> values;
    int k;
};

void printMatrix(std::vector<double> M, int n_rows, int n_cols)
{
    for (int i = 0; i < n_rows; ++i)
    {
        for (int j = 0; j < n_cols; j++)
            std::cout << std::ceil(M[i * n_rows + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

void printIntArray(std::vector<int> M, int n)
{
    for (int i = 0; i < n; ++i)
        std::cout << M[i]<< "\t";
    std::cout << std::endl;
}

void printDoubleArray(std::vector<double> M, int n)
{
    for (int i = 0; i < n; ++i)
        std::cout << M[i] << "\t";
    std::cout << std::endl;
}

inline void divide_job_into_parts(int number, std::vector<int> &displs, std::vector<int> &counts, int n)
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

inline std::vector<double> find_diagonal_dependences(
    std::vector<double> &M, 
    int n_matrix, 
    int start_index, 
    int diag_size
)
{
    int j = start_index;
    int row = j / n_matrix;
    int k = j - (row * n_matrix + row);
    std::vector<double> dependences(k * diag_size * 2);
    int row_el_index = 0;
    int col_el_index = k * diag_size;
    int col_el;
    for (int i = 0; i < diag_size; i++, j += n_matrix + 1)
    {
        row = j / n_matrix;
        col_el = n_matrix * k + j;
        for (int row_el = row * n_matrix + row; row_el < j; row_el++, col_el--)
        {
            dependences[row_el_index] = M[row_el];
            dependences[col_el_index] = M[col_el];
            row_el_index++;
            col_el_index++;
        }
    }
    return dependences;
}

inline std::vector<double> compute_diagonal_using_dependences(
    int n_matrix,
    int diag_size,
    std::vector<double> dependences
)
{
    std::vector<double> values;
    if (!diag_size)
        return values;

    int k = dependences.size() / (diag_size * 2);
    int row_count = 0;
    double res;
    int row_deps = 0;
    int col_deps = k * diag_size;
    for (int row_count = 0; row_count < diag_size; row_count += 1)
    {
        res = 0.0;
        for (; row_deps < (row_count + 1) * k; row_deps++, col_deps++)
            res += dependences[row_deps] * dependences[col_deps];
        res = cbrt(res);

        values.push_back(res);
    }
    return values;
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

    std::vector<int> send_counts(n_processes, 0);
    std::vector<int> send_displs(n_processes, 0);
    
    std::vector<int> counts(n_processes, 0);
    std::vector<int> displs(n_processes, 0);
    
    int n = atoi(argv[1]);
    std::vector<double> M;
    if (!myrank)
        M.resize(n * n);
    std::vector<double> values;

    int start_index = 0;

    auto start = std::chrono::high_resolution_clock::now();

    if (!myrank)
    {
        for (int m = 0; m < n; m++)
            M[m * n + m] = static_cast<double>(m + 1) / n;
    }

    for (int k = 1; k < n; k++)
    {
        start_index = k;
        std::vector<double> global_values(n - k + 1);
        std::vector<double> global_dependences;
        std::vector<double> partial;
        int send_displ = 0;

        divide_job_into_parts(n - k, displs, counts, n_processes);

        for (int rank = 0; rank < n_processes; rank++)
        {
            send_counts[rank] = counts[rank] * k * 2;
            send_displs[rank] = send_displ;
            send_displ += send_counts[rank];
        }

        if (!myrank)
            for (int rank = 0; rank < n_processes; rank++)
            {
                partial = find_diagonal_dependences(M, n, start_index, counts[rank]);
                start_index += (counts[rank] * n + counts[rank]);
                global_dependences.insert(global_dependences.end(), partial.begin(), partial.end());
            }

        std::vector<double> dependences(send_counts[myrank]);
        MPI_Scatterv(
            global_dependences.data(),
            send_counts.data(),
            send_displs.data(),
            MPI_DOUBLE,
            dependences.data(),
            send_counts[myrank],
            MPI_DOUBLE,
            0,
            MPI_COMM_WORLD);

        values = compute_diagonal_using_dependences(n, counts[myrank], dependences);

        MPI_Gatherv(
            values.data(),
            values.size(),
            MPI_DOUBLE,
            global_values.data(),
            counts.data(),
            displs.data(),
            MPI_DOUBLE,
            0,
            MPI_COMM_WORLD);

        if (!myrank)
        {
            for (int i = 0; i < n - k; i += 1)
            {
                M[i * n + i + k] = global_values[i];
                M[(i + k) * n + i] = global_values[i];
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    if (!myrank)
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "time: " << duration.count() << std::endl;
        std::cout << "end execution" << std::endl;
        std::cout << "last: " << M[n - 1] << std::endl;
    }

    MPI_Finalize();
    return 0;
    }