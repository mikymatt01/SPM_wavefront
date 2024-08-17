#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <ff/utils.hpp>
#include <mpi.h>

void custom_merge_op(void *invec, void *inoutvec, int *len, MPI_Datatype *datatype)
{
    std::pair<int, double> *in_vals = static_cast<std::pair<int, double> *>(invec);
    double *inout_vals = static_cast<double *>(inoutvec);

    for (int i = 0; i < *len; i++)
        inout_vals[in_vals[i].first] = in_vals[i].second;
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

double wavefrontElement(std::vector<double> M, uint64_t i, uint64_t j, uint64_t k, uint64_t N)
{
    double res = 0.0;
    for (uint64_t t = 0; t < k; ++t)
        res += M[i * N + i + t] * M[(j - t) * N + j];
    return res;
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
    MPI_Op custom_op;
    MPI_Op_create((MPI_User_function *)custom_merge_op, 1, &custom_op);

    int element;
    uint64_t N = atoi(argv[1]);
    std::vector<double> M(N * N, 1);
    std::vector<std::pair<int, double>> res;

    for (uint64_t i = 0; i < N; i++)
        M[i * N + i] = static_cast<double>(i + 1) / N;

    auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t k = 1; k < N; k++)
    {
        for (uint64_t i = 0; i + k < N; i++)
            if (i % n_processes == myrank)
                res.push_back(std::make_pair(i * N + i + k, wavefrontElement(M, i, i + k, k, N)));
        MPI_ReduceAll(res, M, res.size(), MPI_DOUBLE, custom_op, 0, MPI_COMM_WORLD);
    }
    auto end = std::chrono::high_resolution_clock::now();

    // printMatrix(M, N);
    std::cout << "time: " << end - start << std::endl;
    std::cout << "end execution" << std::endl;

    MPI_Finalize();
    return 0;
}

// M[i * N + i + k] = wavefrontElement(M, i, i + k, k, N);
