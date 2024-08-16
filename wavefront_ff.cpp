#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include "ff/ff.hpp"

using namespace ff;
using task_k=uint64_t;
using task_ki=std::pair<uint64_t, uint64_t>;

struct FactoryK: ff_monode_t<task_k> {  // it must be multi-output
    FactoryK(uint64_t _N) {
        N = _N;
    }
    
    task_k* svc(task_k*) {
		for(uint64_t k = 1; k < N; k++)
            ff_send_out(new task_k(k));
        return EOS;
    }
	
    uint64_t N;
};

struct FactoryI: ff_monode_t<task_k, task_ki> {  // it must be multi-output
    FactoryI(uint64_t _N) {
        N = _N;
    }
    
    task_ki* svc(task_k* task_k) {
        size_t k = *task_k;
        for(uint64_t i = 0; i + k < N; i++)
            ff_send_out(new task_ki(k, i));
        delete task_k;
        return GO_ON;
    }
	
    uint64_t N;
};

struct WorkerM: ff_monode_t<task_ki, float> {  // it must be multi-output
    WorkerM(std::vector<double>& _M, uint64_t _N) {
        M = _M;
        N = _N;
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

    void printMatrix(std::vector<double> M, uint64_t N) {
        for(uint64_t i = 0; i < N; ++i) {
            for(uint64_t j = 0; j < N; j++)
                std::cout << std::ceil(M[i * N + j] * 100) / 100 << "\t";
            std::cout << std::endl;
        }
    }

    float* svc(task_ki* task_ki) {
        uint64_t k = task_ki->first;
        uint64_t i = task_ki->second;
        M[i * N + i + k] = wavefrontElement(M, i, i + k, k, N);
        delete task_ki;
        return GO_ON;
    }
	
    void svc_end() { 
        printMatrix(M, N);
    }

    std::vector<double> M;
    uint64_t N;
};

int main(int argc, char *argv[]) {
    if(argc < 2) return 1;
    std::cout << "start execution" << std::endl;

	uint64_t N = atoi(argv[1]);
    std::vector<double> M(N*N, 1);

    for(uint64_t i = 0; i < N; i++)
        M[i * N + i] = static_cast<double>(i + 1) / N;

    FactoryK first(N);
    FactoryI second(N);
    WorkerM third(M, N);

    ff_Pipe pipe(first, second, third);

    if (pipe.run_and_wait_end()<0) {
        error("running pipe");
        return -1;
    }
    std::cout << "Time: " << pipe.ffTime() << "\n";

    std::cout << "end execution" << std::endl;
    return 0;
}