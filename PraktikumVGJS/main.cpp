#include <cstdint>
#include <thread>
#include <iostream>
#include <sstream>

#include "VGJSHeaders.h"
#include "tests.h"

using namespace vgjs;

int main(int argc, char* argv[]) {

	int num = argc > 1 ? std::stoi(argv[1]) : std::thread::hardware_concurrency();

	// Test minimal JobSystem to compare with VGJS
	//mjs::test(num);

	JobSystem js(thread_count_t{ num });
	//enable_logging();

	// Main Tests
	schedule(work::test());
	//schedule(mandelbrot::test());
	//schedule(mcts::test());

	// Basic lock-free queue test
	//schedule([]() {lock_free::test(); });

	// Run performance.cpp from Prof. Helmut Hlavacs 
	//schedule(test::start_test());

	wait_for_termination();
	//char in;
	//std::cin >> in;
}