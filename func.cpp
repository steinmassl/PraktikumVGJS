#include <chrono>
#include <iostream>
#include "VEGameJobSystem.h"
using namespace vgjs;

namespace func {

    volatile std::atomic<uint32_t> cnt = 0;

    unsigned long long work(int n) {
        //do work
        volatile unsigned long long x = 0;
        for(int i = 0; i < 100; i++) {
            x = x + std::chrono::system_clock::now().time_since_epoch().count();
            //std::cout << x << std::endl;
        }
        cnt++;
        if(n > 0) {
            n_pmr::vector<Function> vec;
            vec.push_back(Function{[=]() {work(n - 1);}});
            vec.push_back(Function{[=]() {work(n - 1);}});
            schedule(vec);
        }
        return x;
    }

    void call(int n) {
        auto v_workFunction = Function{[=]() {work(n);}};
        schedule(v_workFunction);
    }

    void test() {
        cnt = 0;
        //todo rekursiv Jobs aufrufen / keine optimisierung / random workload
        int n = 13;

        schedule([=]() {call(n);});

        //continuation([=]() {std::cout << cnt << std::endl;});
    }
}