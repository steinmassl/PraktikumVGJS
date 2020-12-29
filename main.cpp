#include "VEGameJobSystem.h"
#include "VECoro.h"
#include "benchmark.h"

#include "workFunc.h"
#include "workCoro.h"
#include "mandelbrotFunc.h"
#include "mandelbrotCoro.h"
#include "mctsFunc.h"
#include "mctsCoro.h"

#include "Lock-free_queue_test.h"


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
const int g_num_loops	= 73;		// 2us		// Threshold for Coro jobs (Win) 8c16t
//const int g_num_loops	= 37;		// 1us


// General Settings
const int g_num_threads = 4;		// Number of threads to use in the VGJS
const int g_num_jobs	= 100000;	// Number of work jobs to create when testing work()
const int g_num_seconds = 20;		// Number of seconds to run a timed Benchmark



// Start selected single Benchmarks
Coro<> startSingleBenchmarks(const int num_loops, const int num_jobs) {
	
	co_await [=]() {workFunc::benchmarkWork(num_loops, num_jobs);};
	co_await 		workCoro::benchmarkWork(num_loops, num_jobs);
	
	co_await [ ]() {mandelbrotFunc::test();};
	co_await 		mandelbrotCoro::test();
	co_await [ ]() {mctsFunc::test();};
	co_await 		mctsCoro::test();

	std::cout << std::endl <<	"Number of Threads used in VGJS: " << g_num_threads << std::endl;
	vgjs::terminate();
	co_return;
}

// Start a selected timed Benchmark
Coro<> startTimedBenchmark(const int num_loops, const int num_jobs, const int num_sec) {

	// Create time point N seconds from now
	std::chrono::time_point<std::chrono::system_clock> end;
	end = std::chrono::system_clock::now() + std::chrono::seconds(num_sec);

	// workFunc and workCoro for Speedup Testing - don't exist right now
	//schedule([=]() {workFunc::benchmarkTimedWork(num_loops, num_jobs, end); });
	//schedule(workCoro::benchmarkTimedWork(num_loops, num_jobs, end));

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

	
	benchmark::RegisterBenchmark("work()", work_benchmark)->MeasureProcessCPUTime()->Unit(benchmark::kMicrosecond)->Arg(num_loops);
	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();
}

int main() {

	// Google Benchmarks
	//startGoogleBenchmarks(g_num_loops);

	JobSystem::instance(g_num_threads);
	//enable_logging();

	// Timed Benchmarks - don't work right now
	//schedule([]() {startTimedBenchmark(g_num_loops, g_num_jobs, g_num_seconds); });

	// Single Benchmarks
	schedule(startSingleBenchmarks(g_num_loops, g_num_jobs));

	// Test Lock-free queue
	//schedule([]() {lock_free_queue::test(); });

	wait_for_termination();
}