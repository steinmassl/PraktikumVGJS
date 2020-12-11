#include "workCoro.h"

namespace workCoro {

    int											g_work_calls = 0;
    std::chrono::duration<double, std::milli>	g_total_work_runtime = {};

    // Do some work not optimized by the compiler
    Coro<> work(const int& num_loops) {
        volatile unsigned long x = 0;
        for (int i = 0; i < num_loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
            //std::cout << x << std::endl;
        }
        co_return;
    }

    // Start multiple work coros
    Coro<> test(const int& num_loops, const int& num_jobs, const bool google_benchmark) {
        n_pmr::vector<Coro<>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back(work(num_loops));
        }
        co_await vec;
        if (google_benchmark)
            vgjs::terminate();
        co_return;
    }

    // Benchmark wrapper for workCoro Test
    Coro<> benchmarkWork(const int& num_loops, const int& num_jobs) {
        auto job_start = std::chrono::high_resolution_clock::now();

        co_await workCoro::test(num_loops, num_jobs, false);

        auto job_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_milliseconds = job_end - job_start;
        //std::cout << "BM_WorkCoro: " << elapsed_milliseconds.count() << " ms" << std::endl;

        g_total_work_runtime += elapsed_milliseconds;
        g_work_calls++;
        co_return;
    }

    // Benchmark multiple runs of workCoro until end point is reached
    Coro<> benchmarkTimedWork(const int num_loops, const int num_jobs, std::chrono::time_point<std::chrono::system_clock> end) {
        while (std::chrono::system_clock::now() < end) {
            co_await benchmarkWork(num_loops, num_jobs);
        }
        std::cout << std::endl <<   "     Test: workCoro" << std::endl;
        std::cout << std::endl <<   "     Number of calls:           " << g_work_calls << std::endl;
        std::cout <<                "   Mean job execution time:   " << g_total_work_runtime.count() / g_work_calls << " ms" << std::endl;
        vgjs::terminate();
    }
}

// (Google) Benchmark wrapper for workCoro Test
void BM_WorkCoro(benchmark::State& state) {
    for (auto _ : state) {
        // This code gets timed
        schedule(workCoro::test((int)state.range(0), (int)state.range(1), true));

        wait_for_termination();
    }
}
