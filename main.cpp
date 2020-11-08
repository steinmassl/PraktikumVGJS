#include <iostream>

#include "VEGameJobSystem.h"

namespace func {
    void test();
}

namespace coro {
    void test();
}

void run(int n) {
    vgjs::schedule(std::bind(func::test));
    vgjs::schedule(std::bind(coro::test));

    if(n <= 1)
        vgjs::continuation([]() {vgjs::terminate();});
    else
        vgjs::continuation([=]() {run(n - 1);});
}

using namespace vgjs;
int main() {

    int n = 1000;


    JobSystem::instance();

    //enable_logging();

    schedule(std::bind(run, n));
    

    wait_for_termination();
    std::cout << "Exit" << std::endl;

}