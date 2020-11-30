#include <chrono>
#include <iostream>
#include "VEGameJobSystem.h"

using namespace vgjs;

namespace func {

    volatile std::atomic<uint32_t> cnt = 0;

    void call(int n);

    unsigned long work(int n) {
        //do work
        volatile unsigned long x = 0;
        for(int i = 0; i < 1; i++) {
            x = x + std::chrono::system_clock::now().time_since_epoch().count();
            //std::cout << x << std::endl;
        }
        cnt++;
        if(n > 0) {
            n_pmr::vector<std::function<void(void)>> vec;
            vec.push_back([=]() {call(n - 1);});
            vec.push_back([=]() {call(n - 1);});
            schedule(vec);
        }
        return x;
    }

    void call(int n) {
        schedule([=]() {work(n);});
    }

    void test() {
        cnt = 0;
        int n = 13;

        schedule([=]() {call(n);});

        //continuation([=]() {std::cout << cnt << std::endl;});
    }
}