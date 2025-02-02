#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

void printMatrix(std::vector<double> M, uint64_t n)
{
    for (uint64_t i = 0; i < n; ++i)
    {
        for (uint64_t j = 0; j < n; j++)
            std::cout << std::ceil(M[i * n + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

void printArray(std::vector<float> x, int n)
{
    for (int i = 0; i < n; i++)
        std::cout << x[i] << " ";
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "./" << argv[0] << " <n>" << std::endl;
        return -1;
    }

    std::cout << "start execution" << std::endl;

    int n = atoi(argv[1]);
    std::vector<double> M(n * n, 1);

    auto start = std::chrono::high_resolution_clock::now();
    // initialize the major diagonal e^0_{m,m} with (m+1)/n
    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<double>(m + 1) / n;

    // for each diagonal k in the matrix
    for (int k = 1; k < n; k++)
    {
        // for each diagonal element i
        for (int i = 0; i < n - k; i++)
        {
            // compute cubic root of the dot product between two arrays
            double res = 0.0;
            for (int t = 0; t < k; ++t)
            {
                res += M[i * n + i + t] * M[(i + k) * n + (i + k) - t];
            }
            res = cbrt(res);

            // assign the result to the corresponding diagonal element e^k_{i,j}
            M[i * n + i + k] = res;

            // assign the result at the e^k_{j,i} to create the symmetric matrix for performance improvement
            M[(i + k) * n + i] = res;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "time: " << duration.count() << std::endl;
    std::cout << "end execution" << std::endl;
    std::cout << "last: " << M[n - 1] << std::endl;

    return 0;
}