#include <iostream>

#include "include.h"
#include "workFunc.h"
#include "workCoro.h"

// Tests
namespace mandelbrotFunc {
	void test();
}
namespace mandelbrotCoro {
	void test();
}
namespace mctsFunc {
	void test();
}
namespace mctsCoro {
	void test();
}

// Google Benchmarks
static void BM_MandelbrotFunc(benchmark::State&);
static void BM_MandelbrotCoro(benchmark::State&);


// General Settings
const int g_num_jobs	= std::thread::hardware_concurrency();
const int g_num_threads = 16;
const int g_num_seconds = 20;

const int g_num_loops	= 725000;



// Start a selected timed Test
void startTimedBenchmark(const int num_loops, const int num_jobs, const int num_sec) {

	std::cout << std::endl <<	"Number of Threads used in VGJS: " << g_num_threads << std::endl;
	std::cout <<				"Number of work() Jobs:          " << g_num_jobs << std::endl << std::endl;

	std::chrono::time_point<std::chrono::system_clock> end;
	end = std::chrono::system_clock::now() + std::chrono::seconds(num_sec);

	//schedule([=]() {workFunc::benchmarkTimedWork(num_loops, num_jobs, end); });
	schedule(workCoro::benchmarkTimedWork(num_loops, num_jobs, end));

	//recursive call with switch case to work through each schedule one after another? maybe just vector?
}

void startGoogleBenchmarks(int argc, char** argv, const int num_loops, const int num_jobs) {
	
	auto work_benchmark = [](benchmark::State& state) { BM_Work(state); };
	auto workFunc_benchmark = [](benchmark::State& state) { BM_WorkFunc(state); };
	auto workCoro_benchmark = [](benchmark::State& state) { BM_WorkCoro(state); };

	// C++ Functions
	benchmark::RegisterBenchmark("work()", work_benchmark)->MeasureProcessCPUTime()->Unit(benchmark::kMillisecond)->Arg(g_num_loops);

	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();

	benchmark::ClearRegisteredBenchmarks();

	// Using JobSystem

	/*
	JobSystem::instance(g_num_threads);

	benchmark::RegisterBenchmark("workFunc", workFunc_benchmark)	->MeasureProcessCPUTime()->Unit(benchmark::kMillisecond)->Args({ g_num_loops, g_num_jobs });
	//benchmark::RegisterBenchmark("workCoro", workCoro_benchmark)	->MeasureProcessCPUTime()->Unit(benchmark::kMillisecond)->Args({ g_num_loops, g_num_jobs });

	
	benchmark::RunSpecifiedBenchmarks();
	*/
	std::cout << std::endl;
}



int main(int argc, char** argv) {

	// Google Benchmarks
	startGoogleBenchmarks(argc, argv, g_num_loops, g_num_jobs);


	// Manual Benchmarks

	schedule([]() {startTimedBenchmark(g_num_loops, g_num_jobs, g_num_seconds); });

	vgjs::wait_for_termination();
}













// (Google) Benchmark wrapper for Mandelbrot Func Test
static void BM_MandelbrotFunc(benchmark::State& state) {

	for (auto _ : state) {
		// This code gets timed
		schedule([=]() {mandelbrotFunc::test(); });
		wait_for_termination();
	}
}

// (Google) Benchmark wrapper for Mandelbrot Coro Test
static void BM_MandelbrotCoro(benchmark::State& state) {

	for (auto _ : state) {
		// This code gets timed
		schedule([=]() {mandelbrotCoro::test(); });
		wait_for_termination();
	}
}