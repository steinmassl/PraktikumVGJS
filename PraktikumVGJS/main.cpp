#include "vgjs.h"
#include "tests.h"
//#include "benchmark.h"

// General Settings
static constexpr uint32_t g_num_seconds = 1;		// Number of seconds to run a fixed-time benchmark
static constexpr uint32_t g_num_jobs	= 50000;	// Number of work jobs to create when doing fixed-size benchmarks


Coro<> startJobSystemBenchmarks(const uint32_t num_jobs, const uint32_t num_seconds, const uint32_t num_threads) {

	//co_await work::test();
	//co_await mandelbrot::test();
	co_await mcts::test();

	// Basic test of Lock-free queue
	//co_await lock_free::test();

	vgjs::terminate();
	co_return;
}

int main(int argc, char* argv[]) {

	uint32_t num = argc > 1 ? std::stoi(argv[1]) : std::thread::hardware_concurrency();

	// Benchmark Functions
	//startGoogleBenchmarks();

	JobSystem::instance(thread_count(num));
	//enable_logging();

	// Benchmark JobSystem Tests
	schedule(startJobSystemBenchmarks(g_num_jobs, g_num_seconds, num));

	// Run test.cpp from Prof. Helmut Hlavacs 
	//schedule(test::start_test());

	wait_for_termination();
	//char in;
	//std::cin >> in;
}