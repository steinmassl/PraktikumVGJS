#include "workCoro.h"

namespace workCoro {

    // Do some work not optimized by the compiler
    Coro<> work(const int num_loops) {
        volatile unsigned long x = 0;
        for (int i = 0; i < num_loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
        }
        co_return;
    }

    // Benchmark multiple runs of workCoro
    Coro<> benchmarkWorkWithFixedSize(const int num_loops, const int num_jobs) {
        std::cout << "workCoro" << std::endl;
        n_pmr::vector<Coro<>> vec;
        vec.reserve(num_jobs);
        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back(work(num_loops));
        }

        auto start = std::chrono::high_resolution_clock::now();
        co_await vec;
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
        std::cout << std::endl << "   Test: workCoro (Fixed Size)" << std::endl;
        std::cout << "   Execution time:   " << elapsed_milliseconds.count() << " ms" << std::endl;
        co_return;
    }

    // Store fixed-time benchmark data
    int											g_call_count = 0;
    std::chrono::duration<double, std::milli>	g_total_work_runtime = {};

    // Benchmark work until time runs out 
    Coro<> timedRun(const int num_loops, const int num_jobs, const std::chrono::time_point<std::chrono::system_clock> end_of_benchmark) {
        while (std::chrono::system_clock::now() < end_of_benchmark) {
            g_call_count++;

            n_pmr::vector<Coro<>> vec;
            vec.reserve(num_jobs);
            for (int i = 0; i < num_jobs; i++) {
                vec.emplace_back(work(num_loops));
            }

            auto start = std::chrono::high_resolution_clock::now();
            co_await vec;
            auto end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
            g_total_work_runtime += elapsed_milliseconds;
        }
        co_return;
    }

    // Benchmark work until time runs out (similar design to workFunc variant to mitigate overhead differences)
    Coro<> benchmarkWorkWithFixedTime(const int num_loops, const int num_jobs, const int num_sec, const int num_threads) {
        std::chrono::time_point<std::chrono::system_clock> end;
        end = std::chrono::system_clock::now() + std::chrono::seconds(num_sec);

        co_await timedRun(num_loops, num_jobs, end);

        std::cout << std::endl <<   "   Test: workCoro (Fixed Time)"    << std::endl;
        std::cout <<                "   Number of calls:       "        << g_call_count << std::endl;
        std::cout <<                "   Mean execution time:   "        << g_total_work_runtime.count() / g_call_count << " ms" << std::endl;

        std::ofstream outdata;
        std::string file_name("results/workCoro" + std::to_string(num_threads) + ".txt");
        outdata.open(file_name, std::ios_base::app);
        if (outdata) {
            outdata << g_total_work_runtime.count() / g_call_count << "   (Number of loops: " << num_loops << ")" << std::endl;
        }
        outdata.close();
    }
}
