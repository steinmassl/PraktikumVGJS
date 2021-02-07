#include <iostream>
#include <chrono>
#include <functional>
#include <atomic>
#include <iomanip>
#include <memory_resource>

#include "threadpool.hpp"

using namespace std::chrono;

namespace mjs {

	using namespace vks;

    class MinimalJobSystem {
        ThreadPool m_pool;
        uint32_t m_thread_count = 0;

    public:
        MinimalJobSystem(uint32_t& thread_count) {
            m_pool.setThreadCount(thread_count);
            m_thread_count = thread_count;
        }

        void schedule(std::function<void()>&& job) {
            m_pool.threads[rand() % m_thread_count]->addJob(std::move(job));
        }

        void schedule(std::pmr::vector<std::function<void()>>&& jobs) {
            for (int i = 0; i < jobs.size(); i++)
                m_pool.threads[i % m_thread_count]->addJob(std::move(jobs[i]));    // Spread jobs over all threads evenly
            
            /*for (auto& job : jobs)    // Alternatively schedule each job to a random thread
                schedule(std::move(job));
            */
        }

        void wait() {
            m_pool.wait();
        }

        uint32_t get_thread_count() {
            return m_thread_count;
        }
    };

	void func_perf(int micro, int i = 1) {
		volatile unsigned int counter = 1;
		volatile double root = 0.0;

		auto start = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);

		while (duration.count() < micro) {
			for (int i = 0; i < 10; ++i) {
				counter += counter;
				root = sqrt((float)counter);
			}
			duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
		}
		//std::cout << duration.count() << std::endl;
	}

    template<bool WITHALLOCATE = false, typename FT = std::function<void(void)>>
    std::tuple<double, double> performance_function(MinimalJobSystem& mjs, bool print = true, bool wrtfunc = true, int num = 1000, int micro = 1, unsigned int loops = 20, std::pmr::memory_resource* mr = std::pmr::new_delete_resource()) {

        double duration0_sum = 0;
        double duration2_sum = 0;

        for (uint32_t i = 0; i < loops; i++) {      // run multiple times to even out fluctuations
            // no JS
            auto start0 = high_resolution_clock::now();
            for (int i = 0; i < num; ++i) func_perf(micro);
            auto duration0 = duration_cast<microseconds>(high_resolution_clock::now() - start0);
            duration0_sum += duration0.count();

            // allocation
            std::pmr::vector<FT> perfv{ mr };
            if constexpr (!WITHALLOCATE) {
                perfv.resize(num, std::function<void(void)>{[&]() { func_perf(micro); }});
            }

            // Set total number of jobs to be executed
            //g_job_count = num;

            // multithreaded
            //g_start = high_resolution_clock::now(); // use timers in threadpool.hpp
            auto start2 = high_resolution_clock::now();
            // time allocation as well
            if constexpr (WITHALLOCATE) {
                perfv.resize(num, std::function<void(void)>{ [&]() { func_perf(micro); }});
            }

            mjs.schedule(std::move(perfv));    // start jobs in mjs

            mjs.wait(); // sync
            //g_duration = duration_cast<microseconds>(high_resolution_clock::now() - g_start);
            auto duration2 = duration_cast<microseconds>(high_resolution_clock::now() - start2);
            duration2_sum += duration2.count();
        }

        // calculate + output
        double speedup0 = duration0_sum / duration2_sum;
        double efficiency0 = speedup0 / mjs.get_thread_count();
		if (print /* && efficiency0 > 0.85 */) {
			std::cout << "Wrt function calls: Work/job " << std::right << std::setw(3) << micro << " us Speedup " << std::left << std::setw(8) << speedup0 << " Efficiency " << std::setw(8) << efficiency0 << std::endl;
		}
        return std::make_tuple(speedup0, efficiency0);
    }

    template<bool WITHALLOCATE = false, typename FT>
    void performance_driver(MinimalJobSystem& mjs, std::string text, std::pmr::memory_resource* mr = std::pmr::new_delete_resource(), int runtime = 400000) {
        int num = runtime;
        const int st = 0;
        const int mt = 100;
        const int dt1 = 1;
        const int dt2 = 1;
        const int dt3 = 1;
        const int dt4 = 10;
        int mdt = dt1;
        bool wrt_function = true; //speedup wrt to sequential function calls w/o JS

        std::cout << "\nPerformance for " << text << " on " << mjs.get_thread_count() << " threads\n\n";
        performance_function<WITHALLOCATE, FT>(mjs, false, wrt_function, (int)(num), 0); //heat up, allocate enough jobs
        for (int us = st; us <= mt; us += mdt) {
            int loops = (us == 0 ? num : (runtime / us));
            auto [speedup, eff] = performance_function<WITHALLOCATE, FT>(mjs, true, wrt_function, loops, us);
            if (/* eff > 0.95 && */ us >= 10) return;
            if (us >= 15) mdt = dt2;
            if (us >= 20) mdt = dt3;
            if (us >= 50) mdt = dt4;
        }
    }


	void test(uint32_t num_threads) {

        MinimalJobSystem mjs(num_threads);

        std::cout << "\n\nPerformance: min work (in microsconds) per job so that efficiency is >0.85 or >0.95\n";

        performance_driver<false, std::function<void(void)>>(mjs, "std::function calls (w / o allocate)");
	}
















}