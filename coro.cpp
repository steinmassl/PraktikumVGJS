#include <chrono>
#include <iostream>
#include "VEGameJobSystem.h"
#include "VECoro.h"
using namespace vgjs;

namespace coro {

    volatile std::atomic<uint32_t> cnt = 0;

    Coro<unsigned long long> work(int n) {
        //do work
        volatile unsigned long long x = 0;
        for(int i = 0; i < 100; i++) {
            x = x + std::chrono::system_clock::now().time_since_epoch().count();
            //std::cout << x << std::endl;
        }
        cnt++;
        if(n > 0) {
            n_pmr::vector<Coro<unsigned long long>> vec;
            vec.push_back(work(n - 1));
            vec.push_back(work(n - 1));
            co_await vec;

        }
        co_return x;
    }

    Coro<> call(int n) {
        //call Coro from Coro
        auto v_workCoro = work(n);          //cannot send rValue, bug?
        co_await v_workCoro;                
        //std::cout << cnt << std::endl;
        co_return;
    }

    void test() {
        cnt = 0;
        //todo rekursiv Jobs aufrufen / keine optimisierung / random workload
        int n = 13;

        schedule(call(n));
    }
}