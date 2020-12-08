#include <chrono>
#include <iostream>
#include "VEGameJobSystem.h"
#include "benchmark.h"

using namespace vgjs;

namespace func {


    // Do some work not optimized by the compiler
    void work(int num_loops) {
        volatile unsigned long x = 0;
        for (int i = 0; i < num_loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
            //std::cout << x << std::endl;
        }
    }

    // Create work functions and start them at the same time to mitigate differences to Coro variant
    void call(int num_jobs, int num_loops) {
        n_pmr::vector<std::function<void(void)>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back([=]() {work(num_loops); });
        }
        schedule(vec);
    }

    void test(int num_jobs, int num_loops) {
        schedule([=]() {call(num_jobs, num_loops); });
    }
}

