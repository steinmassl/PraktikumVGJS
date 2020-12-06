#include <chrono>
#include <iostream>
#include "VEGameJobSystem.h"
#include "VECoro.h"
#include "benchmark.h"

using namespace vgjs;

namespace coro {

    // Do some work not optimized by the compiler
    Coro<> work(int& num_loops) {
        volatile unsigned long x = 0;
        for (int i = 0; i < num_loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
            //std::cout << x << std::endl;
        }
        co_return;
    }

    // Create work Coros and start them at the same time
    Coro<> call(int num_jobs, int num_loops) {
        // /*   uncomment to create empty job
        n_pmr::vector<Coro<>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back(work(num_loops));
        }
        co_await vec;
        vgjs::terminate();
        co_return;
        // */
        //vgjs::terminate();
    }

    void test(int num_jobs, int num_loops) {
        schedule(call(num_jobs, num_loops));
    }
}