#include <iostream>
#include <vector>
#include <cmath>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <chrono>

using namespace ff;

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

    int N = atoi(argv[1]);
    // Initialize matrix with zeros
    std::vector<double> M(N * N, 1);

    // Initialize major diagonal elements
    for (int i = 0; i < N; i++)
        M[i * N + i] = static_cast<double>(i + 1) / N;

    // ParallelFor object from FastFlow
    ParallelFor pf;
    auto start = std::chrono::high_resolution_clock::now();
    for (int k = 1; k < N; k++)
        pf.parallel_for(0, N - k, 1, [&](int i)
                        {
            double res = 0.0;
            for (int t = 0; t < k; ++t)
                res += M[i * N + i + t] * M[(i + k) * N + (i + k) - t];
            res = cbrt(res);
            M[i * N + i + k] = res;
            M[(i + k) * N + i] = res; });
    auto end = std::chrono::high_resolution_clock::now();
    // Output the result matrix (can be removed for large N)
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "time: " << duration.count() << std::endl;
    std::cout << "end execution" << std::endl;

    return 0;
}
