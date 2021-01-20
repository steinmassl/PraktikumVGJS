#include "vgjs.h"
#include "tests.h"
//#include "benchmark.h"

// General Settings
static constexpr uint32_t g_num_seconds = 1;		// Number of seconds to run a fixed-time benchmark
static constexpr uint32_t g_num_jobs	= 50000;	// Number of work jobs to create when doing fixed-size benchmarks

/*
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



	//co_await[=]() { mandelbrotFunc::benchmarkWithFixedTime(num_sec, num_threads); };
	//co_await	    mandelbrotCoro::benchmarkWithFixedTime(num_jobs, num_sec, num_threads);

	co_return;
}
*/
Coro<> startJobSystemBenchmarks(const uint32_t num_jobs, const uint32_t num_seconds, const uint32_t num_threads) {

	//co_await startFixedTimeBenchmarks(num_jobs, num_seconds, num_threads);
	//co_await startFixedSizeBenchmarks();

	//co_await work::test();
	co_await mandelbrot::test();
	// Test Lock-free queue
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