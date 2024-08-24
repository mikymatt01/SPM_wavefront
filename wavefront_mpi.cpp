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

void printMatrix(std::vector<double> M, uint64_t N)
{
    for (uint64_t i = 0; i < N; ++i)
    {
        for (uint64_t j = 0; j < N; j++)
            std::cout << std::ceil(M[i * N + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    std::cout << "start execution" << std::endl;

    MPI_Init(&argc, &argv);

    int n_processes;
    int myrank;

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

    std::vector<int> counts(n_processes, 0);
    std::vector<int> displs(n_processes, 0);

    uint64_t N = atoi(argv[1]);
    std::vector<double> M(N * N, 1);

    for (uint64_t i = 0; i < N; i++)
        M[i * N + i] = static_cast<double>(i + 1) / N;

    auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t k = 1; k < N; k++)
    {
        std::vector<double> values;
        std::vector<double> global_values(N - k + 1);

        divide_job_into_parts(N - k, displs, counts, n_processes);

        for (uint64_t i = 0; i + k < N; i++)
            if (i >= displs[myrank] && i < displs[myrank] + counts[myrank])
            {
                double value = 0.0;
                for (uint64_t t = 0; t < k; ++t)
                    value += M[i * N + i + t] * M[(i + k) * N + (i + k) - t];
                value = cbrt(value);
                values.push_back(value);
            }

        MPI_Allgatherv(values.data(), values.size(), MPI_DOUBLE,
                       global_values.data(), counts.data(), displs.data(), MPI_DOUBLE,
                       MPI_COMM_WORLD);

        for (uint64_t i = 0; i + k < N; i += 1)
        {
            M[i * N + i + k] = global_values[i];
            M[(i + k) * N + i] = global_values[i];
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    if (!myrank)
    {
        printf("time: %d\n", end - start);
        std::cout << "end execution" << std::endl;
    }

    MPI_Finalize();
    return 0;
}
