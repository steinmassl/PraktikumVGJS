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

    // Store fixed-time benchmark data
    uint32_t									g_call_count = 0;               // Number of times timedRun has been called
    std::chrono::duration<double, std::micro>	g_total_timed_runtime = {};     // Sum of timedRun execution times (check overhead of surrounding code)
    std::vector<double>                         g_runtime_vec;                  // Vector of batch execution times for processing

    n_pmr::vector<Coro<>> g_vec;    // Reuse vector for work jobs

    // Benchmark work until time runs out 
    Coro<> timedRun(const int num_loops, const int num_jobs, const std::chrono::time_point<std::chrono::system_clock> end_of_benchmark) {
        while (std::chrono::system_clock::now() < end_of_benchmark) {
            auto timedRun_start = std::chrono::high_resolution_clock::now();
            
            g_call_count++;

            g_vec.clear();
            for (int i = 0; i < num_jobs; i++) {
                g_vec.emplace_back(work(num_loops));
            }

            auto start = std::chrono::high_resolution_clock::now();
            co_await g_vec;

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed_work_microseconds = end - start;
            g_runtime_vec.push_back(elapsed_work_microseconds.count());

            auto timedRun_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> elapsed_timedRun_microseconds = timedRun_end - timedRun_start;
            g_total_timed_runtime += elapsed_timedRun_microseconds;
        }
        co_return;
    }

    Coro<> measureCalls(const int num_loops, const int num_jobs, const std::chrono::time_point<std::chrono::system_clock> end_of_benchmark) {
        while (std::chrono::system_clock::now() < end_of_benchmark) {
            g_call_count++;
            g_vec.clear();
            for (int i = 0; i < num_jobs; i++) {
                g_vec.emplace_back(work(num_loops));
            }
            co_await g_vec;
        }
        co_return;
    }

    // Benchmark work until time runs out (similar design to workFunc variant to mitigate overhead differences)
    Coro<> benchmarkWorkWithFixedTime(const int num_loops, const int num_jobs, const int num_sec, const int num_threads) {
        std::chrono::time_point<std::chrono::system_clock> end;
        end = std::chrono::system_clock::now() + std::chrono::seconds(num_sec);

        co_await timedRun(num_loops, num_jobs, end);

        std::sort(g_runtime_vec.begin(), g_runtime_vec.end());  // To find median
        size_t size = g_runtime_vec.size();
        double batch_median = size % 2 == 0 ? (g_runtime_vec.at(size / 2 - 1) + g_runtime_vec.at(size / 2)) / 2 : g_runtime_vec.at(size / 2);

        // Sum of batch execution times
        double total_work_execution_time = 0;
            for(const auto& time : g_runtime_vec) {
                total_work_execution_time += time;
        }

        // Calculate values for a single job
        double work_mean = total_work_execution_time / (g_call_count * num_jobs);
        double work_median = batch_median / num_jobs;
        double timedRun_median = g_total_timed_runtime.count() / (g_call_count * num_jobs);

        std::cout << std::endl
                  << "    Test: workCoro (Fixed Time)" << std::endl;
        std::cout << "        Number of calls:                  " << g_call_count << std::endl;
        std::cout << "        Mean execution time (work):       " << work_mean << " us" << std::endl;
        std::cout << "        Median execution time (work):     " << work_median << " us" << std::endl;
        //std::cout << "        Mean execution time (timedRun):   " << timedRun_median << " us" << std::endl;
        std::cout << "        Median batch execution time       " << batch_median / 1000.0 << "ms" << std::endl;
        std::cout << "        Jobs per batch:                   " << num_jobs << std::endl;

        // Output files
        std::string raw_file("PraktikumVGJS/results/rawWorkCoro" + std::to_string(num_threads) + ".txt");  // Raw data
        std::string summary_file("PraktikumVGJS/results/workCoro" + std::to_string(num_threads) + ".txt"); // Summaries

        std::ofstream outdata;

        // Write execution times of batches to file so they can be processed
        outdata.open(raw_file);
        if (outdata)
        {
            for (auto &batch_execution_time : g_runtime_vec)
            {
                outdata << batch_execution_time / num_jobs << std::endl;
            }
        }
        outdata.close();
        // Append summary of benchmark to file
        outdata.open(summary_file, std::ios_base::app);
        if (outdata)
        {
            outdata << std::endl
                    << "Test: workCoro (Fixed Time)" << std::endl;
            outdata << "    Threads used in VJGS:             " << num_threads << std::endl;
            outdata << "    Number of calls:                  " << g_call_count << std::endl;
            outdata << "    Mean execution time (work):       " << work_mean << " us" << std::endl;
            outdata << "    Median execution time (work):     " << work_median << " us" << std::endl;
            outdata << "    Mean execution time (timedRun):   " << timedRun_median << " us" << std::endl;
            outdata << "    Median batch execution time       " << batch_median / 1000.0 << "ms" << std::endl;
            outdata << "    Jobs per batch:                   " << num_jobs << std::endl;
            outdata << std::endl
                    << "    Loops in work():                  " << num_loops << std::endl;
            outdata << "    Jobs per batch:                   " << num_jobs << std::endl;
            outdata << "    Platform:                         " << g_platform << std::endl;
            outdata << "    CPU hardware threads:             " << g_cpu_hardware_threads << std::endl;
        }
        outdata.close();
    }

    // Benchmark multiple runs of workCoro
    Coro<> benchmarkWorkWithFixedSize(const int num_loops, const int num_jobs) {
        n_pmr::vector<Coro<>> vec;
        vec.reserve(num_jobs);
        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back(work(num_loops));
        }

        auto start = std::chrono::high_resolution_clock::now();
        co_await vec;
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::micro> elapsed_microseconds = end - start;
        std::cout << std::endl <<   "    Test: workCoro (Fixed Size)" << std::endl;
        std::cout <<                "        Mean Execution time:              " << elapsed_microseconds.count() / num_jobs << " us" << std::endl;
        co_return;
    }
}
