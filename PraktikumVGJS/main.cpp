#include <cstdint>
#include <thread>
#include <iostream>
#include <sstream>

#include "vgjs.h"
#include "tests.h"

/*
Coro<> startJobSystemBenchmarks(const uint32_t num_threads) {

	co_await work::test();
	//co_await mandelbrot::test();
	//co_await mcts::test();

	// Basic test of Lock-free queue
	//co_await lock_free::test();

	vgjs::terminate();
	co_return;
}
*/
int main(int argc, char* argv[]) {

	uint32_t num = argc > 1 ? std::stoi(argv[1]) : std::thread::hardware_concurrency();

	// Test minimal JobSystem
	mjs::test(num);

	//JobSystem::instance(thread_count(num));
	//enable_logging();

	// Benchmark JobSystem Tests
	//schedule(startJobSystemBenchmarks(num));

	//schedule(work::test());
	//schedule(mandelbrot::test());
	//schedule(mcts::test());

	// Run test.cpp from Prof. Helmut Hlavacs 
	//schedule(test::start_test());

	//wait_for_termination();
	//char in;
	//std::cin >> in;
}