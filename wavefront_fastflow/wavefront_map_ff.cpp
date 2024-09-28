#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

using task = std::pair<int, int>;
using namespace ff;
// Emitter that sends tasks to workers and receives feedback from them
struct Emitter : ff_monode_t<int, task>
{

    Emitter(int _n) : n(_n) {}

    task *svc(int *feedback)
    {
        if (feedback != nullptr)
        {
            tasks_received++;
            delete feedback;
        }
        if (k < n && tasks_received == tasks_sent)
        {
            tasks_sent = 0;
            tasks_received = 0;
            for (int i = 0; i < n - k; i++)
            {
                ff_send_out(new task(k, i));
                tasks_sent++;
            }
            k++;
        }

        if (k == n)
            return EOS;
        return GO_ON;
    }

    int n;

    int k = 1;
    int i = 0;

    int tasks_received = 0;
    int tasks_sent = 0;
};

struct Worker : ff::ff_Map<task, int>
{
    using map = ff_Map<task, int>;

    Worker(int _n, std::vector<float> &_M) : n(_n), M(_M) {}

    int *svc(task *task)
    {
        int k = task->first;
        int i = task->second;
        double res = 0.0;
        for (int t = 0; t < k; ++t)
        {
            res += M[i * n + i + t] * M[(i + k) * n + (i + k) - t];
        }
        res = cbrt(res);

        M[i * n + i + k] = res;
        M[(i + k) * n + i] = res;
        delete task;

        ff_send_out(new int(1));
        return GO_ON;
    }

    int n;
    std::vector<float> &M;
};

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "./" << argv[0] << " <n>" << " <nw>" << std::endl;
        return -1;
    }

    std::cout << "start execution" << std::endl;

    int n = atoi(argv[1]);
    int n_workers = atoi(argv[2]);
    std::vector<float> M(n * n, 1);

    auto start = std::chrono::high_resolution_clock::now();

    Emitter emitter(n);
    std::vector<std::unique_ptr<ff::ff_node>> workers;
    for (int i = 0; i < n_workers; ++i)
    {
        workers.push_back(std::make_unique<Worker>(n, M));
    }
    ff::ff_Farm<int> farm(std::move(workers), emitter);

    farm.remove_collector();
    farm.wrap_around();
    farm.set_scheduling_ondemand();

    if (farm.run_and_wait_end() < 0)
    {
        std::cerr << "Error running the farm" << std::endl;
        return -1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "time: " << duration.count() << std::endl;
    std::cout << "end execution" << std::endl;
    std::cout << M[n - 1] << std::endl;

    return 0;
}
