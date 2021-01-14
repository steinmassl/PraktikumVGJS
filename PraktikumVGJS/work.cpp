#include "tests.h"
#include <iomanip>

namespace work {

    // Do some work not optimized by the compiler - use this to test speedup
    void workFunc(const double us) {
        volatile uint64_t x = 0;

        auto start = std::chrono::high_resolution_clock::now();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);

        while (duration.count() < us) {
            x = x + (uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();
            stop = std::chrono::high_resolution_clock::now();
            duration = duration_cast<std::chrono::microseconds>(stop - start);
        }
    }

    Coro<> workCoro(const double us) {
        volatile uint64_t x = 0;

        auto start = std::chrono::high_resolution_clock::now();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);

        while (duration.count() < us) {
            x = x + (uint64_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();
            stop = std::chrono::high_resolution_clock::now();
            duration = duration_cast<std::chrono::microseconds>(stop - start);
        }
        co_return;
    }

    enum measure_type {
        FUNC_ONLY,
        SINGLE_THREAD,
        MULTI_THREAD
    };

    // Recursively benchmark batches of work until time runs out
    Coro<std::vector<double>*> measure(const bool use_coro, measure_type measure_type, const double us, const uint32_t num_jobs, const uint32_t num_sec) {

        std::vector<double>* durations = new std::vector<double>();

        std::chrono::time_point<std::chrono::high_resolution_clock> end_of_benchmark;
        end_of_benchmark = std::chrono::high_resolution_clock::now() + std::chrono::seconds(num_sec);

        while (std::chrono::high_resolution_clock::now() < end_of_benchmark) {

            auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();

            n_pmr::vector<Function> vec_func{};
            n_pmr::vector<Coro<>> vec_coro{};

            switch (measure_type) {
            case FUNC_ONLY:
                start = std::chrono::high_resolution_clock::now();
                for (uint32_t i = 0; i < num_jobs; i++) {
                    workFunc(us);
                }
                end = std::chrono::high_resolution_clock::now();
                break;
            

            case SINGLE_THREAD: 
                for (uint32_t i = 0; i < num_jobs; i++) {
                    vec_func.emplace_back(Function{ [=]() {workFunc(us); }, thread_index{0} });
                }
                start = std::chrono::high_resolution_clock::now();
                co_await vec_func;
                end = std::chrono::high_resolution_clock::now();
                break;
            

            case MULTI_THREAD: 
                for (uint32_t i = 0; i < num_jobs; i++) {
                    if (use_coro)
                        vec_coro.emplace_back(workCoro(us));
                    else 
                        vec_func.emplace_back(Function{ [=]() {workFunc(us); } });
                }
                start = std::chrono::high_resolution_clock::now();
                use_coro ? co_await vec_coro : co_await vec_func;
                end = std::chrono::high_resolution_clock::now();
                break;
            
            default:
                std::cout << "Default case" << std::endl;
            }

            std::chrono::duration<double, std::micro> elapsed_work_microseconds = end - start;
            durations->push_back(elapsed_work_microseconds.count());
        }
        co_return durations;
    }

    double calculateMedianExecutionTime(std::vector<double> durations) {
        std::sort(durations.begin(), durations.end());
        size_t size = durations.size();
        return size % 2 == 0 ? (durations.at(size / 2 - 1) + durations.at(size / 2)) / 2 : durations.at(size / 2);
    }

    double calculateMeanExecutionTime(std::vector<double> durations) {
        double sum = 0;
        for (const auto& time : durations) {
            sum += time;
        }
        return sum / durations.size();
    }

    // Benchmark work until time runs out
    Coro<> benchmarkWithFixedTime(const uint32_t num_jobs, const uint32_t num_sec, const uint32_t num_threads) {

        std::cout << std::endl;
        for (double num_microseconds = 1.0; num_microseconds <= 8.0; num_microseconds += 1.0) {
            auto res_func_only = co_await measure(false, measure_type::FUNC_ONLY, num_microseconds, num_jobs, num_sec);
            auto res_single_thread = co_await measure(false, measure_type::SINGLE_THREAD, num_microseconds, num_jobs, num_sec);
            auto res_all_threads = co_await measure(false, measure_type::MULTI_THREAD, num_microseconds, num_jobs, num_sec);
            auto res_all_threads_coro = co_await measure(true, measure_type::MULTI_THREAD, num_microseconds, num_jobs, num_sec);

            double speedup_func_only = calculateMedianExecutionTime(*res_func_only) / calculateMedianExecutionTime(*res_all_threads);
            double efficiency_func_only = speedup_func_only / num_threads;
            double speedup_func_only_coro = calculateMedianExecutionTime(*res_func_only) / calculateMedianExecutionTime(*res_all_threads_coro);
            double efficiency_func_only_coro = speedup_func_only_coro / num_threads;

            double speedup_single_thread = calculateMedianExecutionTime(*res_single_thread) / calculateMedianExecutionTime(*res_all_threads);
            double efficiency_single_thread = speedup_single_thread / num_threads;


            std::cout << "Duration: " << std::setw(2) << num_microseconds << " us" << std::endl
                << std::setw(25) << "wrt Functions FuncSpeedup: " << std::setw(7) << speedup_func_only
                << std::setw(15) << " FuncEfficiency: " << efficiency_func_only << std::endl

                << std::setw(25) << "wrt SingleThr FuncSpeedup: " << std::setw(7) << speedup_single_thread
                << std::setw(15) << " FuncEfficiency: " << efficiency_single_thread << std::endl

                << std::setw(25) << "wrt Functions CoroSpeedup: " << std::setw(7) << speedup_func_only_coro
                << std::setw(15) << " CoroEfficiency: " << efficiency_func_only_coro << std::endl
                << std::endl;
        }
        co_return;
    }
        /*
		std::cout << std::endl
			<< "    Test: workFunc (Fixed Time)" << std::endl;
		std::cout << "        Number of calls:                  " << g_call_count << std::endl;
		std::cout << "        Mean execution time (work):       " << work_mean << " us" << std::endl;
		std::cout << "        Median execution time (work):     " << work_median << " us" << std::endl;
		std::cout << "        Median batch execution time       " << batch_median / 1000.0 << "ms" << std::endl;
		std::cout << "        Jobs per batch:                   " << num_jobs << std::endl;

		// Output files
		std::string raw_file("PraktikumVGJS/results/rawWorkFunc" + std::to_string(num_threads) + ".txt");  // Raw data
		std::string summary_file("PraktikumVGJS/results/workFunc" + std::to_string(num_threads) + ".txt"); // Summaries

		std::ofstream outdata;

		// Write execution times of batches to file so they can be processed
		outdata.open(raw_file);
		if (outdata)
		{
			for (auto& batch_execution_time : g_runtime_vec)
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
				<< "Test: workFunc (Fixed Time)" << std::endl;
			outdata << "    Threads used in VJGS:             " << num_threads << std::endl;
			outdata << "    Number of calls:                  " << g_call_count << std::endl;
			outdata << "    Mean execution time (work):       " << work_mean << " us" << std::endl;
			outdata << "    Median execution time (work):     " << work_median << " us" << std::endl;
			outdata << "    Median batch execution time       " << batch_median / 1000.0 << "ms" << std::endl;
			outdata << "    Jobs per batch:                   " << num_jobs << std::endl;
			outdata << std::endl;
			outdata << "    Platform:                         " << g_platform << std::endl;
			outdata << "    CPU hardware threads:             " << g_cpu_hardware_threads << std::endl;
		}
		outdata.close();
		co_return;
	}
    */
}

/*
// (Google) Benchmark wrapper for work() to use in workFunc and workCoro Tests
void BM_Work(benchmark::State& state) {
    for (auto _ : state) {
        // This code gets timed
        workFunc::workFunc(1.0);
    }
}
*/



