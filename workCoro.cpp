#include "workCoro.h"

namespace workCoro {

    int											g_work_calls = 0;
    std::chrono::duration<double, std::micro>	g_total_work_runtime = {};

    // Do some work not optimized by the compiler
    Coro<> work(const int& num_loops) {
        volatile unsigned long x = 0;
        for (int i = 0; i < num_loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
        }
        co_return;
    }

    // Start multiple work coros
    Coro<> test(const int& num_loops, const int& num_jobs, const bool single_benchmark) {
        n_pmr::vector<Coro<>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back(work(num_loops));
        }
        auto start = std::chrono::high_resolution_clock::now();
        co_await vec;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> elapsed_microseconds = end - start;
        g_total_work_runtime += elapsed_microseconds;
        if (single_benchmark) {
            std::cout << "   Test: workCoro" << std::endl << "   Execution time: " << elapsed_microseconds.count() << " us" << std::endl;
        }
        co_return;
    }

    Coro<> timedTest(const int& num_loops, const int& num_jobs) {
        n_pmr::vector<Coro<>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back(work(num_loops));
        }
        co_await vec;
        co_return;
    }

    // Benchmark wrapper for workFunc Test
    Coro<> benchmarkWork(const int num_loops, const int num_jobs) {
        co_await test(num_loops, num_jobs, true);
        co_return;
    }

    // Benchmark multiple runs of workCoro until end point is reached
    Coro<> benchmarkTimedWork(const int num_loops, const int num_jobs, std::chrono::time_point<std::chrono::system_clock> end) {
        while (std::chrono::system_clock::now() < end) {
            co_await timedTest(num_loops, num_jobs);
            g_work_calls++;
        }
        std::cout << std::endl <<   "     Test: workCoro" << std::endl;
        std::cout << std::endl <<   "     Number of calls:           " << g_work_calls << std::endl;
        std::cout <<                "     Mean job execution time:   " << 20000000.0 / g_work_calls << " us" << std::endl;
        std::cout <<                "     Mean job execution time new:   " << g_total_work_runtime.count() / g_work_calls << " us" << std::endl;
    }
}
