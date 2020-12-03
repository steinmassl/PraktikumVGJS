#include <chrono>
#include <iostream>
#include "VEGameJobSystem.h"
#include "VECoro.h"
#include "benchmark.h"

using namespace vgjs;

namespace coro {

    //volatile std::atomic<uint32_t> cnt = 0;
    int loops = 10000000;

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

    Coro<> workCoro() {
        work();
        co_return;
    }


    Coro<> call(int num_jobs) {
        n_pmr::vector<Coro<>> vec;

        for (int i = 0; i < num_jobs; i++) {
            vec.emplace_back(workCoro());
        }
        co_await vec;
        vgjs::terminate();
        co_return;
    }

    /*
    void test() {
        cnt = 0;
        int depth = 13;

        schedule(call(depth));
    }
    */

    static void BM_Test(benchmark::State& state) {
        int num_threads = 16;
        int num_jobs = 16;

        JobSystem::instance(num_threads);
        //enable_logging();

        for (auto _ : state) {
            schedule(call(num_jobs));
            wait_for_termination();
        }
    }

    BENCHMARK(BM_Work)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
    BENCHMARK(BM_Test)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Iterations({ 1 });
}