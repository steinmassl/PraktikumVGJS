#include <chrono>
#include <iostream>
#include "VEGameJobSystem.h"
#include "benchmark.h"

using namespace vgjs;

namespace func {

    //volatile std::atomic<uint32_t> cnt = 0;
    int loops = 200000;

    void work() {
        //do work
        volatile unsigned long x = 0;
        for (int i = 0; i < loops; i++) {
            x = x + (unsigned long)std::chrono::system_clock::now().time_since_epoch().count();
            //std::cout << x << std::endl;
        }
    }

    static void BM_Work(benchmark::State& state) {
        for (auto _ : state) {
            // This code gets timed
            work();
        }
    }

    /*
    void test() {
        int threads = 16;
        schedule([=]() {call(threads); });
        //continuation([=]() {std::cout << cnt << std::endl;});
    }
    */

    void call(int num_jobs) {
        n_pmr::vector<std::function<void(void)>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back([=]() {work(); });
        }
        schedule(vec);
        continuation([]() {vgjs::terminate(); });
    }

    static void BM_Test(benchmark::State& state) {
        int num_threads = 16;
        int num_jobs = 16;

        JobSystem::instance(num_threads);
        //enable_logging();
        
        for (auto _ : state) {
            schedule([=]() {call(num_jobs); });
            wait_for_termination();
        }
    }



    
    //BENCHMARK(BM_Work)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
    //BENCHMARK(BM_Test)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Iterations({ 1 });
}