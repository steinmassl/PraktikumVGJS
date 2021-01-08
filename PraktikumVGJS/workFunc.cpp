#include "workFunc.h"

namespace workFunc {

    // Do some work not optimized by the compiler - use this to test speedup
    void work(const int num_loops) {
        volatile unsigned long x = 0;
        for (int i = 0; i < num_loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
        }
    }

    // Benchmark a certain number of work calls
    void benchmarkWorkWithFixedSize(const int num_loops, const int num_jobs) {
        n_pmr::vector<std::function<void(void)>> vec;
        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back([=]() {work(num_loops); });
        }
        auto start = std::chrono::high_resolution_clock::now();

        schedule(vec);

        continuation([=]() {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;

            std::cout << std::endl <<   "   Test: workFunc (Fixed Size)"    << std::endl;
            std::cout <<                "   Execution time:   "             << elapsed_milliseconds.count() << " ms" << std::endl;
        });
    }

    // Store fixed-time benchmark data
    int											g_call_count = 0;
    std::chrono::duration<double, std::milli>	g_total_work_runtime = {};

    // Recursively benchmark work until time runs out
    void timedRun(const int num_loops, const int num_jobs, const std::chrono::time_point<std::chrono::system_clock> end_of_benchmark) {
        g_call_count++;
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
            g_total_work_runtime += elapsed_milliseconds;
            if (std::chrono::system_clock::now() < end_of_benchmark) {
                timedRun(num_loops, num_jobs, end_of_benchmark);
            }
        });
    }

    // Benchmark work until time runs out
    void benchmarkWorkWithFixedTime(const int num_loops, const int num_jobs, const int num_sec, const int num_threads) {
        std::chrono::time_point<std::chrono::system_clock> end;
        end = std::chrono::system_clock::now() + std::chrono::seconds(num_sec);

        schedule([=]() {timedRun(num_loops, num_jobs, end); });

        continuation([=]() {
            std::cout << std::endl << "   Test: workFunc (Fixed Time)"  << std::endl;
            std::cout <<              "   Number of calls:       "      << g_call_count << std::endl;
            std::cout <<              "   Mean execution time:   "      << g_total_work_runtime.count() / g_call_count << " ms" << std::endl;

            std::ofstream outdata;
            std::string file_name("results/workFunc" + std::to_string(num_threads) + ".txt");
            outdata.open(file_name, std::ios_base::app);
            if (outdata) {
                outdata << g_total_work_runtime.count() / g_call_count << "   (Number of loops: " << num_loops << ")" << std::endl;
            }
            outdata.close();
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



