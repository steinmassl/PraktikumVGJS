#include "workFunc.h"

namespace workFunc {

    int											g_work_calls = 0;
    std::chrono::duration<double, std::milli>	g_total_work_runtime = {};

    // Do some work not optimized by the compiler
    void work(const int num_loops) {
        volatile unsigned long x = 0;
        for (int i = 0; i < num_loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
            //std::cout << x << std::endl;
        }
    }

    // Start multiple work functions at the same time ( to mitigate differences to Coro variant )
    void test(const int num_loops, const int num_jobs, const bool google_benchmark) {
        n_pmr::vector<std::function<void(void)>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back([=]() {work(num_loops); });
        }
        schedule(vec);
        if (google_benchmark)
            continuation([]() {vgjs::terminate(); });
    }

    // Benchmark wrapper for workFunc Test
    void benchmarkWork(const int num_loops, const int num_jobs) {
        auto job_start = std::chrono::high_resolution_clock::now();
        schedule([=]() {test(num_loops, num_jobs, false); });
        continuation([=]() {
            auto job_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed_milliseconds = job_end - job_start;
            //std::cout << "BM_WorkFunc: " << elapsed_milliseconds.count() << " ms" << std::endl;

            g_total_work_runtime += elapsed_milliseconds;
            g_work_calls++;
        });
    }

    // Benchmark multiple runs of workFunc recursively until end point is reached
    void benchmarkTimedWork(const int num_loops, const int num_jobs, const std::chrono::time_point<std::chrono::system_clock> end) {
        schedule([=]() {benchmarkWork(num_loops, num_jobs); });

        if (std::chrono::system_clock::now() < end) {
            continuation([=]() {benchmarkTimedWork(num_loops, num_jobs, end); });
        }
        else {
            continuation([]() {
                std::cout << std::endl <<   "   Test: workFunc" << std::endl;
                std::cout << std::endl <<   "   Number of calls:           " << g_work_calls << std::endl;
                std::cout <<                "   Mean job execution time:   " << g_total_work_runtime.count() / g_work_calls << " ms" << std::endl;
                vgjs::terminate();
            });
        }
    }
}

// (Google) Benchmark wrapper for work() to use in workFunc and workCoro Tests
void BM_Work(benchmark::State& state) {
    for (auto _ : state) {
        // This code gets timed
        workFunc::work((int) state.range(0));
    }
}

// (Google) Benchmark wrapper for workFunc Test
void BM_WorkFunc(benchmark::State& state) {
    for (auto _ : state) {
        // This code gets timed
        schedule([=]() {workFunc::test((int) state.range(0),(int) state.range(1), true); });

        wait_for_termination();
    }
}



