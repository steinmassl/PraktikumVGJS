#include <iostream>
#include <iomanip>
#include <functional>
#include <string>
#include <chrono>
#include <math.h>

#include "tests.h"

using namespace std::chrono;

namespace work {

    const int num_blocks = 50000;
    const int block_size = 1 << 10;

    auto				g_global_mem = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());

    auto				g_global_mem_f = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());
    thread_local auto	g_local_mem_f = n_pmr::unsynchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());

    auto				g_global_mem_c = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());
    thread_local auto	g_local_mem_c = n_pmr::unsynchronized_pool_resource({ .max_blocks_per_chunk = num_blocks, .largest_required_pool_block = block_size }, n_pmr::new_delete_resource());

    thread_local auto	g_local_mem_m = n_pmr::monotonic_buffer_resource(1 << 20, n_pmr::new_delete_resource());


    // Do some work not optimized by the compiler - use this to test speedup
    void func_perf(int micro, int i = 1) {
        volatile unsigned int counter = 1;
        volatile double root = 0.0;

        if (i > 1) schedule([=]() { func_perf(micro, i - 1); });

        auto start = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);

        while (duration.count() < micro) {
            for (int i = 0; i < 10; ++i) {
                counter = counter + counter;
                root = sqrt((float)counter);
            }
            duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
        }
        //std::cout << duration.count() << std::endl;
    }

    Coro<> Coro_perf(std::allocator_arg_t, n_pmr::memory_resource* mr, int micro) {
        func_perf(micro, 1);
        co_return;
    }

    template<bool WITHALLOCATE = false, typename FT1 = Function, typename FT2 = std::function<void(void)>>
    Coro<std::tuple<double, double>> performance_function(bool print = true, bool wrtfunc = true, int num = 1000, int micro = 1, std::pmr::memory_resource* mr = std::pmr::new_delete_resource()) {
        auto& js = JobSystem::instance();

        // no JS
        auto start0 = high_resolution_clock::now();
        for (int i = 0; i < num; ++i) func_perf(micro);
        auto duration0 = duration_cast<microseconds>(high_resolution_clock::now() - start0);

        // allocation
        std::pmr::vector<FT2> perfv2{ mr };
        if constexpr (!WITHALLOCATE) {
            if constexpr (std::is_same_v<FT1, Function>) {
                perfv2.resize(num, std::function<void(void)>{[&]() { func_perf(micro); }});
            }
            else {
                perfv2.reserve(num);
                for (int i = 0; i < num; ++i) perfv2.emplace_back(Coro_perf(std::allocator_arg, std::pmr::new_delete_resource(), micro));
            }
        }

        // multithreaded
        auto start2 = high_resolution_clock::now();
        // time allocation as well
        if constexpr (WITHALLOCATE) {
            if constexpr (std::is_same_v<FT1, Function>) {
                perfv2.resize(num, std::function<void(void)>{ [&]() { func_perf(micro); }});
            }
            else {
                perfv2.reserve(num);
                for (int i = 0; i < num; ++i) perfv2.emplace_back(Coro_perf(std::allocator_arg, mr, micro));
            }
        }
        co_await perfv2;
        auto duration2 = duration_cast<microseconds>(high_resolution_clock::now() - start2);

        // calculate + output
        double speedup0 = (double)duration0.count() / (double)duration2.count();
        double efficiency0 = speedup0 / js.get_thread_count().value;
        if (wrtfunc) {
            if (print /* && efficiency0 > 0.85 */) {
                std::cout << "Wrt function calls: Work/job " << std::right << std::setw(3) << micro << " us Speedup " << std::left << std::setw(8) << speedup0 << " Efficiency " << std::setw(8) << efficiency0 << std::endl;
            }
            co_return std::make_tuple(speedup0, efficiency0);
        }
    }

    template<bool WITHALLOCATE = false, typename FT1, typename FT2>
    Coro<> performance_driver(std::string text, std::pmr::memory_resource* mr = std::pmr::new_delete_resource(), int runtime = 400000) {
        int num = runtime;
        const int st = 0;
        const int mt = 100;
        const int dt1 = 1;
        const int dt2 = 1;
        const int dt3 = 1;
        const int dt4 = 10;
        int mdt = dt1;
        bool wrt_function = true; //speedup wrt to sequential function calls w/o JS

        auto& js = JobSystem::instance();

        std::cout << "\nPerformance for " << text << " on " << js.get_thread_count().value << " threads\n\n";
        co_await performance_function<WITHALLOCATE, Function, std::function<void(void)>>(false, wrt_function, (int)(num), 0); //heat up, allocate enough jobs
        for (int us = st; us <= mt; us += mdt) {
            int loops = (us == 0 ? num : (runtime / us));
            auto [speedup, eff] = co_await performance_function<WITHALLOCATE, FT1, FT2>(true, wrt_function, loops, us, mr);
            if (eff > 0.95 && us >= 10) co_return;
            if (us >= 15) mdt = dt2;
            if (us >= 20) mdt = dt3;
            if (us >= 50) mdt = dt4;
        }
        co_return;
    }

    Coro<> test() {
        std::cout << "\n\nPerformance: min work (in microsconds) per job so that efficiency is >0.85 or >0.95\n";

        co_await performance_driver<false, Function, std::function<void(void)>>("std::function calls (w / o allocate)");
        co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate new/delete)", std::pmr::new_delete_resource());
        co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate synchronized)", &g_global_mem_f);
        co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate unsynchronized)", &g_local_mem_f);
        co_await performance_driver<true, Function, std::function<void(void)>>("std::function calls (with allocate monotonic)", &g_local_mem_m);
        g_local_mem_m.release();

        co_await performance_driver<false, Coro<>, Coro<>>("Coro<> calls (w / o allocate)");
        co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate new/delete)", std::pmr::new_delete_resource());
        co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate synchronized)", &g_global_mem_c);
        co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate unsynchronized)", &g_local_mem_c);
        co_await performance_driver<true, Coro<>, Coro<>>("Coro<> calls (with allocate monotonic)", &g_local_mem_m);

        vgjs::terminate();

        co_return;
    }
}