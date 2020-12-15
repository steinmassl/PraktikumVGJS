#include "workCoro.h"

namespace workCoro {

    /*
    int             							g_work_calls = 0;
    std::chrono::duration<double, std::micro>	g_total_work_runtime = {};
    */

    // Do some work not optimized by the compiler
    Coro<> work(const int num_loops) {
        volatile unsigned long x = 0;
        for (int i = 0; i < num_loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
        }
        co_return;
    }

    /*
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

    Coro<> timedTest(const int& num_loops, const std::chrono::time_point<std::chrono::system_clock>& end) {
        for (int i = 0; i < 16384; i++) {

        }
        co_return;
    }
        

    // Benchmark wrapper for workFunc Test
    Coro<> benchmarkWork(const int num_loops, const int num_jobs) {
        co_await test(num_loops, num_jobs, true);
        co_return;
    }
    */


    // Benchmark multiple runs of workCoro
    Coro<> benchmarkWork(const int num_loops, const int num_jobs/*, std::chrono::time_point<std::chrono::system_clock> end_of_program*/) {
        n_pmr::vector<Coro<>> vec;
        vec.reserve(num_jobs);
        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back(work(num_loops));
        }

        auto start = std::chrono::high_resolution_clock::now();
        co_await vec;
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
        std::cout << std::endl <<   "     Test: workCoro" << std::endl;
        //std::cout << std::endl <<   "     Number of calls:           " << g_work_calls << std::endl;
        std::cout <<                "     Execution time:   " << elapsed_milliseconds.count() << " ms" << std::endl;
        //std::cout <<                "     Mean job execution time new:   " << g_total_work_runtime.count() / g_work_calls << " us" << std::endl;
        co_return;
    }
}
