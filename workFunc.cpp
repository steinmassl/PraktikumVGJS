#include "workFunc.h"

namespace workFunc {

    /*
    int											g_work_calls = 0;
    std::chrono::duration<double, std::micro>	g_total_work_runtime = {};
    */

    // Do some work not optimized by the compiler
    void work(const int num_loops) {
        volatile unsigned long x = 0;
        for (int i = 0; i < num_loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
        }
    }

    /*
    // Start multiple work functions at the same time ( to mitigate differences to Coro variant )
    void test(const int num_loops, const int num_jobs, const bool single_benchmark) {
        n_pmr::vector<std::function<void(void)>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back([=]() {work(num_loops); });
        }
        auto start = std::chrono::high_resolution_clock::now();
        schedule(vec);
        continuation([=]() {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed_microseconds = end - start;
            g_total_work_runtime += elapsed_microseconds;
            if (single_benchmark) {
                std::cout << "   Test: workFunc" << std::endl << "   Execution time: " << elapsed_microseconds.count() << " us" << std::endl;
            }
        });
    }

    
    void timedTest(const int num_loops, const int num_jobs) {
        n_pmr::vector<std::function<void(void)>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back([=]() {work(num_loops); });
        }
        schedule(vec);
    }

    // Benchmark wrapper for workFunc Test
    void benchmarkWork(const int num_loops, const int num_jobs) {
        schedule([=]() {workFunc::test(num_loops, num_jobs, true); });
    }
    */

    // Benchmark multiple runs of workFunc
    void benchmarkWork(const int num_loops, const int num_jobs/*, const std::chrono::time_point<std::chrono::system_clock> end_of_program*/) {

        n_pmr::vector<std::function<void(void)>> vec;
        vec.reserve(num_jobs);
        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back([=]() {work(num_loops); });
        }
        auto start = std::chrono::high_resolution_clock::now();

        schedule(vec);

        continuation([=]() {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;

            std::cout << std::endl <<   "   Test: workFunc"              << std::endl;
            //std::cout << std::endl <<   "   Number of calls:           " << g_work_calls << std::endl;
            std::cout <<                "   Execution time:   " << elapsed_milliseconds.count() << " ms" << std::endl;
            //std::cout <<                "   Mean job execution time new:   " << g_total_work_runtime.count() / g_work_calls << " us" << std::endl;

        });
    }
}

// (Google) Benchmark wrapper for work() to use in workFunc and workCoro Tests
void BM_Work(benchmark::State& state) {
    for (auto _ : state) {
        // This code gets timed
        workFunc::work((int) state.range(0));
    }
}



