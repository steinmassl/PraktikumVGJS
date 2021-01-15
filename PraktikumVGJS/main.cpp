#include "vgjs.h"
#include "tests.h"
//#include "benchmark.h"


// General Settings
static constexpr uint32_t g_num_threads = 16;		// Number of threads to use in the VGJS
static constexpr uint32_t g_num_seconds = 5;		// Number of seconds to run a fixed-time benchmark
static constexpr uint32_t g_num_jobs	= 50000;	// Number of work jobs to create when doing fixed-size benchmarks

// Start selected fixed-size Benchmarks
Coro<> startFixedSizeBenchmarks() {
	
	co_await [ ]() {mandelbrotFunc::test();};
	co_await 		mandelbrotCoro::test();
	co_await [ ]() {mctsFunc::test();};
	co_await 		mctsCoro::test();

	co_return;
}

// Start selected fixed-time Benchmarks
Coro<> startFixedTimeBenchmarks(const uint32_t num_jobs, const uint32_t num_sec, const uint32_t num_threads) {

	co_await work::benchmarkWithFixedTime(num_jobs, num_sec, num_threads);

	//co_await[=]() { mandelbrotFunc::benchmarkWithFixedTime(num_sec, num_threads); };
	//co_await	    mandelbrotCoro::benchmarkWithFixedTime(num_jobs, num_sec, num_threads);

	co_return;
}

Coro<> startJobSystemBenchmarks(const uint32_t num_jobs, const uint32_t num_seconds, const uint32_t num_threads) {

	co_await startFixedTimeBenchmarks(num_jobs, num_seconds, num_threads);
	//co_await startFixedSizeBenchmarks();

	std::cout << std::endl
		<< "Threads used in VGJS: " << num_threads << std::endl;
	std::cout << std::endl;

	// Test Lock-free queue
	//co_await lock_free::test();

	vgjs::terminate();
	co_return;
}

int main() {

	// Benchmark Functions
	//startGoogleBenchmarks();

	JobSystem::instance(thread_count(g_num_threads));
	//enable_logging();

	// Benchmark JobSystem Tests
	schedule(startJobSystemBenchmarks(g_num_jobs, g_num_seconds, g_num_threads));

	wait_for_termination();
	std::cout << "Exit" << std::endl;
	char in;
	std::cin >> in;

}



/*
// Register C++ functions as Benchmarks and run them
void startGoogleBenchmarks() {

	// Dummy arguments for benchmark::Initialize()
	int argc = 0;
	char** argv = {};

	// Benchmarks as variables
	auto work_benchmark = [](benchmark::State& state) { BM_Work(state); };

	benchmark::RegisterBenchmark("work()", work_benchmark)->MeasureProcessCPUTime()->Unit(benchmark::kMicrosecond);		// Benchmark work function for speedup tests
	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();
	std::cout << std::endl;
}
*/