#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

void printMatrix(std::vector<double> M, uint64_t N) {
    for(uint64_t i = 0; i < N; ++i) {
        for(uint64_t j = 0; j < N; j++)
            std::cout << std::ceil(M[i * N + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

void printVector(std::vector<double> a) {
    for(uint64_t i = 0; i < a.size(); i++)
        std::cout << std::ceil(a[i] * 100) / 100 << "\t";
    std::cout << std::endl;
}

double wavefrontElement(std::vector<double> M, uint64_t i, uint64_t j, uint64_t k, uint64_t N) {
    std::vector<double> a;
    std::vector<double> b;
    double res = 0.0;

    for(uint64_t t = 0; t < k; t++)
        a.push_back(M[i * N + i + t]);

    for(uint64_t t = 0; t < k; t++)
        b.push_back(M[(j - t) * N + j]);

    for(uint64_t i = 0; i < k; ++i)
        res += a[i] * b[i];

    return res;
}

int main(int argc, char *argv[]) {
    if(argc < 2) return 1;
    std::cout << "start execution" << std::endl;

	uint64_t N = atoi(argv[1]);
    std::vector<double> M(N*N, 1);

    for(uint64_t i = 0; i < N; i++)
        M[i * N + i] = static_cast<double>(i + 1) / N;

    auto start = std::chrono::high_resolution_clock::now();
    for(uint64_t k = 1; k < N; k++)
        for(uint64_t i = 0; i + k < N; i++)
            M[i * N + i + k] = wavefrontElement(M, i, i + k, k, N);
    auto end = std::chrono::high_resolution_clock::now();
    
    printMatrix(M, N);
    
    std::cout << "time: " << end - start << std::endl;
    std::cout << "end execution" << std::endl;
    return 0;
}