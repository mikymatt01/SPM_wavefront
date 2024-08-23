#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <ff/ff.hpp>

using namespace ff;
using task_ki = std::pair<uint64_t, uint64_t>;

struct Data
{
    double value;
    bool computed;
};

struct FactoryI : ff_node_t<task_ki>
{
    FactoryI(std::vector<Data> &_M, uint64_t _N) : M(_M), N(_N), init(true) {}

    void printMatrix(std::vector<Data> M, uint64_t N)
    {
        for (uint64_t i = 0; i < N; ++i)
        {
            for (uint64_t j = 0; j < N; j++)
                std::cout << M[i * N + j].computed << "\t";
            std::cout << std::endl;
        }
    }

    task_ki *svc(task_ki *task)
    {
        if (M[N - 1].computed)
            return EOS;

        if (init)
        {
            for (uint64_t i = 0; i + 1 < N; i++)
                ff_send_out(new task_ki(1, i));
            init = false;
        }

        if (task)
        {
            int k = task->first;
            int i = task->second;

            int curr_diag_el = i * N + i + k;
            int prev_diag_el = (i - 1) * N + i - 1 + k;
            int next_diag_el = (i + 1) * N + i + 1 + k;

            int index_to_compute = (i - 1) * N + i + k;
            if (prev_diag_el >= 0 && !M[index_to_compute].computed)
                if (M[curr_diag_el].computed && M[prev_diag_el].computed)
                    ff_send_out(new task_ki(k + 1, i - 1));

            index_to_compute = i * N + i + k + 1;
            if (!M[index_to_compute].computed)
                if (M[curr_diag_el].computed && M[next_diag_el].computed)
                    ff_send_out(new task_ki(k + 1, i));
        }
        return GO_ON;
    }

    uint64_t N;
    bool init;
    std::vector<Data> &M;
};

struct WorkerM : ff_monode_t<task_ki, float>
{
    WorkerM(std::vector<Data> &_M, uint64_t _N) : M(_M), N(_N) {}

    double wavefrontElement(uint64_t i, uint64_t j, uint64_t k)
    {
        double res = 0.0;
        for (uint64_t t = 0; t < k; ++t)
            res += M[i * N + i + t].value * M[(j - t) * N + j].value;
        return cbrt(res);
    }

    float *svc(task_ki *task)
    {
        uint64_t k = task->first;
        uint64_t i = task->second;
        M[i * N + i + k].value = wavefrontElement(i, i + k, k);
        M[i * N + i + k].computed = true;
        ff_send_out_to(new task_ki(k, i), 0);
        delete task;
        return GO_ON;
    }

    std::vector<Data> &M;
    uint64_t N;
};

void printMatrix(std::vector<Data> M, uint64_t N)
{
    for (uint64_t i = 0; i < N; ++i)
    {
        for (uint64_t j = 0; j < N; j++)
            std::cout << std::ceil(M[i * N + j].value * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    std::cout << "start execution" << std::endl;

    uint64_t N = atoi(argv[1]);
    std::vector<Data> M(N * N, {1.0, false});
    ssize_t nworkers = ff_numCores();

    for (uint64_t i = 0; i < N; i++)
    {
        M[i * N + i].value = static_cast<double>(i + 1) / N;
        M[i * N + i].computed = true;
    }

    FactoryI first(M, N);
    ff_Farm<task_ki> farm(
        [&]()
        {
            std::vector<std::unique_ptr<ff_node>> W;
            for (auto i = 0; i < nworkers; ++i)
                W.push_back(make_unique<WorkerM>(M, N));
            return W;
        }());
    farm.set_scheduling_ondemand();

    ff_Pipe pipe(first, farm);
    pipe.wrap_around();

    auto start = std::chrono::high_resolution_clock::now();
    if (pipe.run_and_wait_end() < 0)
    {
        error("running pipe");
        return -1;
    }
    auto end = std::chrono::high_resolution_clock::now();

    // printMatrix(M, N);
    std::cout << "time: " << end - start << std::endl;
    std::cout << "end execution" << std::endl;
    return 0;
}