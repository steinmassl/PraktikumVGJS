#include "VEGameJobSystem.h"
#include "VECoro.h"
#include "benchmark.h"

#include "workFunc.h"
#include "workCoro.h"
#include "mandelbrotFunc.h"
#include "mandelbrotCoro.h"
#include "mctsFunc.h"
#include "mctsCoro.h"

//#include "Lock-free_queue_test.h"

 
// Amount of loops determines length of work() function
//const int g_num_loops	= 365000;	// 10ms
//const int g_num_loops	= 185000;	// 5ms
//const int g_num_loops	= 36500;	// 1ms
//const int g_num_loops	= 3650;		// 100us
//const int g_num_loops	= 365;		// 10us 
//const int g_num_loops	= 253;		// 7us
//const int g_num_loops	= 185;		// 5us
//const int g_num_loops	= 146;		// 4us		// Threshold for C++ function jobs (Win) 8c16t
//const int g_num_loops	= 110;		// 3us
//const int g_num_loops	= 73;		// 2us		// Threshold for Coro jobs (Win) 8c16t
const int g_num_loops	= 60;		// 2us		// Laptop
//const int g_num_loops	= 37;		// 1us


// General Settings
const int g_num_threads = 4;		// Number of threads to use in the VGJS
const int g_num_seconds = 10;		// Number of seconds to run a fixed-time benchmark
const int g_num_jobs	= 100000;	// Number of work jobs to create when doing fixed-size benchmarks



// Start selected fixed-size Benchmarks
Coro<> startFixedSizeBenchmarks(const int num_loops, const int num_jobs) {
	
	co_await [=]() {workFunc::benchmarkWorkWithFixedSize(num_loops, num_jobs);};
	co_await 		workCoro::benchmarkWorkWithFixedSize(num_loops, num_jobs);
	
	//co_await [ ]() {mandelbrotFunc::test();};
	//co_await 		mandelbrotCoro::test();
	//co_await [ ]() {mctsFunc::test();};
	//co_await 		mctsCoro::test();

	co_return;
}

// Start selected fixed-time Benchmarks
Coro<> startFixedTimeBenchmarks(const int num_loops, const int num_jobs, const int num_sec) {

	co_await [=]() {workFunc::benchmarkWorkWithFixedTime(num_loops, num_jobs, num_sec);};
	co_await	    workCoro::benchmarkWorkWithFixedTime(num_loops, num_jobs, num_sec);

	co_return;
}

Coro<> startJobSystemBenchmarks(const int num_loops, const int num_jobs, const int num_seconds) {

	co_await startFixedSizeBenchmarks(num_loops, num_jobs);
	co_await startFixedTimeBenchmarks(num_loops, num_jobs, num_seconds);

	std::cout << std::endl << "Number of Threads used in VGJS: " << g_num_threads << std::endl;

	// Test Lock-free queue
	//schedule([]() {lock_free_queue::test(); });

	vgjs::terminate();
	co_return;
}

// Register C++ functions as Benchmarks and run them
void startGoogleBenchmarks(const int num_loops) {

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
	startGoogleBenchmarks(g_num_loops);

	JobSystem::instance(g_num_threads);
	//enable_logging();

	// Benchmark JobSystem Tests
	schedule(startJobSystemBenchmarks(g_num_loops, g_num_jobs, g_num_seconds));

	wait_for_termination();
}