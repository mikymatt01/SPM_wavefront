#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

struct task
{
    int start;
    int count;
    int k;
};
using namespace ff;
void printMatrix(std::vector<float> M, int n)
{
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; j++)
            std::cout << std::ceil(M[i * n + j] * 100) / 100 << "\t";
        std::cout << std::endl;
    }
}

struct Worker : ff::ff_node_t<task, int>
{
    Worker(int _n, std::vector<float> &_M) : n(_n), M(_M) {}

    int *svc(task *task)
    {
        int start = task->start;
        int count = task->count;
        int k = task->k;
        for (int i = start; i < start + count; i++)
        {
            double res = 0.0;
            for (int t = 0; t < k; ++t)
                res += M[i * n + i + t] * M[(i + k) * n + (i + k) - t];
            res = cbrt(res);

            M[i * n + i + k] = res;
            M[(i + k) * n + i] = res;
        }

        delete task;

        ff_send_out(new int(1));
        return GO_ON;
    }

    int n;
    std::vector<float> &M;
};
struct Emitter : ff_monode_t<int, task>
{

    Emitter(int _n, int _nw, std::vector<ff_node *> &w, std::vector<float> &_M)
        : n(_n), nw(_nw), workers(w), M(_M) {}

    task *svc(int *feedback)
    {
        if (feedback != nullptr)
        {
            tasks_received++;
            delete feedback;
        }
        if (n - k < nw && n - k != 0)
        {
            nw = n - k;
            for (int i = 0; i < nw - (n - k); i++)
                workers[i]->svc_end();
        }

        if (k < n && tasks_received == tasks_sent)
        {
            tasks_sent = 0;
            tasks_received = 0;
            int quotient = std::floor(float((n - k) / nw));
            int remainder = (n - k) % nw;
            int start = 0;

            std::vector<int> displs(nw, 0);
            std::vector<int> counts(nw, 0);
            for (int i = 0; i < nw; ++i)
                counts[i] = quotient;

            for (int i = 0; i < remainder; ++i)
                counts[i] += 1;

            for (int i = 1; i < nw; ++i)
            {
                start += counts[i - 1];
                displs[i] = start;
            }

            for (int i = 0; i < nw; i++)
            {
                ff_send_out(new task(displs[i], counts[i], k));
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
    int nw = 0;
    int tasks_received = 0;
    int tasks_sent = 0;
    std::vector<float> &M;
    ff_farm *farm;
    std::vector<ff_node *> &workers;
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

    for (int m = 0; m < n; m++)
        M[m * n + m] = static_cast<float>(m + 1) / n;

    ff_farm farm;
    std::vector<ff_node *> workers;
    for (int i = 0; i < n_workers; ++i)
    {
        workers.push_back(new Worker(n, M));
    }
    Emitter emitter(n, n_workers, workers, M);
    farm.add_emitter(emitter);
    farm.add_workers(workers);

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
