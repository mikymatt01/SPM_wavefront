#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <ff/utils.hpp>

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
    return cbrt(res);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    std::cout << "start execution" << std::endl;

    uint64_t N = atoi(argv[1]);
    std::vector<double> M(N * N, 1);

    for (uint64_t i = 0; i < N; i++)
        M[i * N + i] = static_cast<double>(i + 1) / N;

    auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t k = 1; k < N; k++)
        for (uint64_t i = 0; i + k < N; i++)
            M[i * N + i + k] = wavefrontElement(M, i, i + k, k, N);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "time: " << end - start << std::endl;
    std::cout << "end execution" << std::endl;
    return 0;
}