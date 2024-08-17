#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <ff/ff.hpp>

using namespace ff;
using task_ki=std::pair<uint64_t, uint64_t>;

struct FactoryI: ff_node_t<task_ki> {
    FactoryI(uint64_t _N) {
        N = _N;
        k = 1;
        parallel_tasks = 0;
        end = false;
    }
    
    task_ki* svc(task_ki* task) {
        if (parallel_tasks > 0 && task)
            parallel_tasks -= 1;
        if (parallel_tasks == 0 && N - k != 0) {
            parallel_tasks = N - k;
            for(uint64_t i = 0; i + k < N; i++)
                ff_send_out(new task_ki(k, i));
            ++k;
        }
        else if(N - k == 0) return EOS;
        return GO_ON;
    }
	
    uint64_t N;
    uint64_t k;
    uint64_t parallel_tasks;
    bool end;
};

struct WorkerM: ff_monode_t<task_ki, float> {
    WorkerM(std::vector<double>& _M, uint64_t _N): M(_M), N(_N) {}

    double wavefrontElement(uint64_t i, uint64_t j, uint64_t k) {
        double res = 0.0;
        for(uint64_t t = 0; t < k; ++t)
            res += M[i * N + i + t] * M[(j - t) * N + j];
        return res;
    }

    float* svc(task_ki* task) {
        uint64_t k = task->first;
        uint64_t i = task->second;
        M[i * N + i + k] = wavefrontElement(i, i + k, k);
        ff_send_out_to(new task_ki(k, i), 0);
        delete task;
        return GO_ON;
    }

    std::vector<double>& M;
    uint64_t N;
};

void printMatrix(std::vector<double> M, uint64_t N) {
    for(uint64_t i = 0; i < N; ++i) {
        for(uint64_t j = 0; j < N; j++)
            std::cout << std::ceil(M[i * N + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) return 1;
    std::cout << "start execution" << std::endl;

	uint64_t N = atoi(argv[1]);
    std::vector<double> M(N*N, 1);
    ssize_t nworkers = ff_numCores();

    for(uint64_t i = 0; i < N; i++)
        M[i * N + i] = static_cast<double>(i + 1) / N;

    FactoryI first(N);
    ff_Farm<task_ki> farm(
        [&]() {
            std::vector<std::unique_ptr<ff_node>> W;
            for(auto i=0;i<nworkers;++i)
                W.push_back(make_unique<WorkerM>(M, N));
            return W;
        } ()
    );
	farm.set_scheduling_ondemand();
	
    ff_Pipe pipe(first, farm);
    pipe.wrap_around();
    
    auto start = std::chrono::high_resolution_clock::now();
    if (pipe.run_and_wait_end()<0) {
        error("running pipe");
        return -1;
    }
    auto end = std::chrono::high_resolution_clock::now();

    // printMatrix(M, N);
    std::cout << "time: " << end - start << std::endl;
    std::cout << "end execution" << std::endl;
    return 0;
}