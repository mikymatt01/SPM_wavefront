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

    int n_processes, n_processes_group, myrank, my_rank_group;
    MPI_Group world_group;

    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

    std::vector<int> counts(n_processes, 0);
    std::vector<int> displs(n_processes, 0);

    int n = atoi(argv[1]);
    std::vector<double> M(n * n, 1);

    auto start = std::chrono::high_resolution_clock::now();

    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<double>(m + 1) / n;

    for (int k = 1; k < n; k++)
    {
        n_processes_group = n_processes;
        if (n - k < n_processes)
            n_processes_group = n - k;

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
        
        std::vector<double> values;
        std::vector<double> global_values(n - k + 1);

        divide_job_into_parts(n - k, displs, counts, n_processes_group);

        for (int i = displs[my_rank_group]; i < displs[my_rank_group] + counts[my_rank_group]; i++)
        {
            double value = 0.0;
            for (int t = 0; t < k; ++t)
                value += M[i * n + i + t] * M[(i + k) * n + (i + k) - t];
            value = cbrt(value);
            values.push_back(value);
        }

        MPI_Allgatherv(values.data(), values.size(), MPI_DOUBLE,
                       global_values.data(), counts.data(), displs.data(), MPI_DOUBLE,
                       new_comm);

        for (int i = 0; i < n - k; i += 1)
        {
            M[i * n + i + k] = global_values[i];
            M[(i + k) * n + i] = global_values[i];
        }

        if (new_comm != MPI_COMM_NULL)
        {
            MPI_Comm_free(&new_comm);
            MPI_Group_free(&new_group);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    if (!myrank)
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "time: " << duration.count() << std::endl;
        std::cout << "last: " << M[n - 1] << std::endl;
        std::cout << "end execution" << std::endl;
    }

    MPI_Finalize();
    return 0;
}
