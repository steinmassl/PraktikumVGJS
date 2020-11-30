#include <chrono>
#include <iostream>
#include "VEGameJobSystem.h"
#include "VECoro.h"

using namespace vgjs;

namespace coro {

    volatile std::atomic<uint32_t> cnt = 0;

    Coro<> call(int n);

    Coro<unsigned long> work(int n) {
        //do work
        volatile unsigned long x = 0;
        for(int i = 0; i < 1; i++) {
            x = x + std::chrono::system_clock::now().time_since_epoch().count();
            //std::cout << x << std::endl;
        }
        cnt++;
        if(n > 0) {
            n_pmr::vector<Coro<>> vec;
            vec.push_back(call(n - 1));
            vec.push_back(call(n - 1));
            co_await vec;

        }
        co_return x;
    }

    Coro<> call(int n) {
        //call Coro from Coro
        co_await work(n);                
        //std::cout << cnt << std::endl;
        co_return;
    }

    void test() {
        cnt = 0;
        int n = 13;

        schedule(call(n));
    }
}