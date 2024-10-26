#include <iostream>
#include <vector>
#include <cmath>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <chrono>

using namespace ff;

void printMatrix(std::vector<double> M, uint64_t n)
{
    for (uint64_t i = 0; i < n; ++i)
    {
        for (uint64_t j = 0; j < n; j++)
            std::cout << std::ceil(M[i * n + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
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

    auto start = std::chrono::high_resolution_clock::now();

    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<double>(m + 1) / n;

    ParallelFor pf;
    for (int k = 1; k < n; k++)
        pf.parallel_for(0, n - k, 1, [&](int i)
                        {
            double res = 0.0;
            for (int t = 0; t < k; ++t)
                res += M[i * n + i + t] * M[(i + k) * n + (i + k) - t];
            res = cbrt(res);
            M[i * n + i + k] = res;
            M[(i + k) * n + i] = res; }, nworkers);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "time: " << duration.count() << std::endl;
    std::cout << "end execution" << std::endl;

    return 0;
}