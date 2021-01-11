#include "vgjs.h"

#include "benchmark.h"

#include "workFunc.h"
#include "workCoro.h"
#include "mandelbrotFunc.h"
#include "mandelbrotCoro.h"
#include "mctsFunc.h"
#include "mctsCoro.h"

#include "lockfreeQueueTest.h"

 
// Amount of loops determines length of work() function
//static constexpr uint32_t g_num_loops	= 365000;	// 10ms
//static constexpr uint32_t g_num_loops	= 185000;	// 5ms
//static constexpr uint32_t g_num_loops	= 36500;	// 1ms
//static constexpr uint32_t g_num_loops	= 3650;		// 100us
//static constexpr uint32_t g_num_loops	= 365;		// 10us 
//static constexpr uint32_t g_num_loops	= 253;		// 7us
//static constexpr uint32_t g_num_loops	= 185;		// 5us
static constexpr uint32_t g_num_loops	= 146;		// 4us		// Threshold for C++ function jobs (Win) 8c16t
//static constexpr uint32_t g_num_loops	= 110;		// 3us
//static constexpr uint32_t g_num_loops	= 73;		// 2us		// Threshold for Coro jobs (Win) 8c16t
//static constexpr uint32_t g_num_loops	= 60;		// 2us		// Laptop
//static constexpr uint32_t g_num_loops	= 37;		// 1us


// General Settings
static constexpr uint32_t g_num_threads = 16;		// Number of threads to use in the VGJS
static constexpr uint32_t g_num_seconds = 5;		// Number of seconds to run a fixed-time benchmark
static constexpr uint32_t g_num_jobs	= 25000;	// Number of work jobs to create when doing fixed-size benchmarks



// Start selected fixed-size Benchmarks
Coro<> startFixedSizeBenchmarks(const uint32_t num_loops, const uint32_t num_jobs) {
	
	co_await [=]() {workFunc::benchmarkWorkWithFixedSize(num_loops, num_jobs);};
	co_await 		workCoro::benchmarkWorkWithFixedSize(num_loops, num_jobs);
	
	co_await [ ]() {mandelbrotFunc::test();};
	co_await 		mandelbrotCoro::test();
	co_await [ ]() {mctsFunc::test();};
	co_await 		mctsCoro::test();

	co_return;
}

// Start selected fixed-time Benchmarks
Coro<> startFixedTimeBenchmarks(const uint32_t num_loops, const uint32_t num_jobs, const uint32_t num_sec, const uint32_t num_threads) {

	//co_await[=]() { workFunc::benchmarkWorkWithFixedTime(num_loops, num_jobs, num_sec, num_threads); };
	//co_await	    workCoro::benchmarkWorkWithFixedTime(num_loops, num_jobs, num_sec, num_threads /*, std::allocator_arg, &g_global_mem */);

	co_await[=]() { mandelbrotFunc::benchmarkWithFixedTime(num_sec, num_threads); };
	//co_await	    mandelbrotCoro::benchmarkWithFixedTime(num_jobs, num_sec, num_threads);

	co_return;
}

Coro<> startJobSystemBenchmarks(const uint32_t num_loops, const uint32_t num_jobs, const uint32_t num_seconds, const uint32_t num_threads) {

	//co_await startFixedSizeBenchmarks(num_loops, num_jobs);
	co_await startFixedTimeBenchmarks(num_loops, num_jobs, num_seconds, num_threads);

	std::cout << std::endl
			  << "Threads used in VGJS: " << g_num_threads << std::endl;
	std::cout << "Loops in work():      " << num_loops << std::endl;
	std::cout << std::endl;

	// Test Lock-free queue
	//schedule([]() {lock_free::test(); });

	vgjs::terminate();
	co_return;
}

// Register C++ functions as Benchmarks and run them
void startGoogleBenchmarks(const uint32_t num_loops) {

	// Dummy arguments for benchmark::Initialize()
	int argc = 0;
	char** argv = {};

	// Benchmarks as variables
	auto work_benchmark = [](benchmark::State& state) { BM_Work(state); };

	benchmark::RegisterBenchmark("work()", work_benchmark)->MeasureProcessCPUTime()->Unit(benchmark::kMicrosecond)->Arg(num_loops);		// Benchmark work function for speedup tests
	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();
	std::cout << std::endl;
}

int main() {

	// Benchmark Functions
	//startGoogleBenchmarks(g_num_loops);

	JobSystem::instance(thread_count(g_num_threads), thread_index(0) /*, &g_global_mem */);
	//enable_logging();

	// Benchmark JobSystem Tests
	schedule(startJobSystemBenchmarks(g_num_loops, g_num_jobs, g_num_seconds, g_num_threads));

	wait_for_termination();
}