#include <iostream>

#include "VEGameJobSystem.h"
#include "VECoro.h"
#include "benchmark.h"

using namespace vgjs;

// Tests
namespace workFunc {
	void test(int, int);
	void work(int);
}
namespace workCoro {
	void test(int, int);
}
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
static void BM_Work(benchmark::State&);
static void BM_MandelbrotFunc(benchmark::State&);
static void BM_MandelbrotCoro(benchmark::State&);

// Single Manual Benchmarks
void BM_WorkFunc();
void BM_WorkCoro();

// Multiple Timed Benchmarks
void runTimedWorkFunc(std::chrono::time_point<std::chrono::system_clock>);
void runTimedWorkCoro(std::chrono::time_point<std::chrono::system_clock>);

// Data Storage for benchmark results
int											work_func_calls = 0;
int											work_coro_calls = 0;
std::chrono::duration<double, std::milli>	total_work_func_runtime = {};
std::chrono::duration<double, std::milli>	total_work_coro_runtime = {};



// General Settings
static const int		NUM_JOBS = std::thread::hardware_concurrency();
static constexpr int	NUM_THREADS = 8;
static constexpr int	NUM_LOOPS = 725000;



// Start a selected timed Test
void startTimedBenchmark(int num_sec) {
	std::chrono::time_point<std::chrono::system_clock> end;
	end = std::chrono::system_clock::now() + std::chrono::seconds(num_sec);

	schedule([=]() {runTimedWorkFunc(end); });
	//schedule([=]() {runTimedWorkCoro(end); });
}



int main(int argc, char** argv) {

	// Benchmark normal C++ Functions
	auto work_benchmark = [](benchmark::State& state) { BM_Work(state); };

	benchmark::RegisterBenchmark("Work", work_benchmark)->MeasureProcessCPUTime()->Unit(benchmark::kMillisecond);
	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();



	std::cout << std::endl;



	// Benchmark Jobs
	JobSystem::instance(NUM_THREADS);

	std::cout << std::endl <<	"Number of Threads used in VGJS: " << NUM_THREADS << std::endl;
	std::cout <<				"Number of work() Jobs:          " << NUM_JOBS << std::endl << std::endl;

	schedule([]() {startTimedBenchmark(20); });

	wait_for_termination();
}









// (Google) Benchmark wrapper for standard job in Func and Coro Tests
static void BM_Work(benchmark::State& state) {
	for (auto _ : state) {
		// This code gets timed
		workFunc::work(NUM_LOOPS);
	}
}



// (Google) Benchmark wrapper for Mandelbrot Func Test
static void BM_MandelbrotFunc(benchmark::State& state) {
	using namespace vgjs;
	JobSystem::instance(NUM_THREADS);
	//enable_logging();

	for (auto _ : state) {
		// This code gets timed
		schedule([=]() {mandelbrotFunc::test(); });
		wait_for_termination();
	}
}

// (Google) Benchmark wrapper for Mandelbrot Coro Test
static void BM_MandelbrotCoro(benchmark::State& state) {
	using namespace vgjs;
	JobSystem::instance(NUM_THREADS);
	//enable_logging();

	for (auto _ : state) {
		// This code gets timed
		schedule([=]() {mandelbrotCoro::test(); });
		wait_for_termination();
	}
}



// Benchmark wrapper for Func Test
void BM_WorkFunc() {
	auto job_start = std::chrono::high_resolution_clock::now();
	schedule([]() {workFunc::test(NUM_JOBS, NUM_LOOPS); });
	continuation([=]() {
		auto job_end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed_milliseconds = job_end - job_start;
		//std::cout << "BM_WorkFunc: " << elapsed_milliseconds.count() << " ms" << std::endl;

		total_work_func_runtime += elapsed_milliseconds;
		work_func_calls++;
	});
}

// Benchmark wrapper for Coro Test
void BM_WorkCoro() {
	auto job_start = std::chrono::high_resolution_clock::now();
	schedule([]() {workCoro::test(NUM_JOBS, NUM_LOOPS); });
	continuation([=]() {
		auto job_end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed_milliseconds = job_end - job_start;
		//std::cout << "BM_WorkCoro: " << elapsed_milliseconds.count() << " ms" << std::endl;

		total_work_coro_runtime += elapsed_milliseconds;
		work_coro_calls++;
	});
}



// Benchmark multiple runs of Func recursively until end point is reached
void runTimedWorkFunc(std::chrono::time_point<std::chrono::system_clock> end) {
	schedule([]() {BM_WorkFunc(); });

	if (std::chrono::system_clock::now() < end) {
		continuation([=]() {runTimedWorkFunc(end); });
	}
	else {
		continuation([]() {
			std::cout << std::endl <<	"   Test: workFunc"					<< std::endl;
			std::cout << std::endl <<	"   Number of calls:           "	<< work_func_calls << std::endl;
			std::cout <<				"   Mean job execution time:   "	<< total_work_func_runtime.count() / work_func_calls << " ms" << std::endl;
			vgjs::terminate();
		});
	}
}

// Benchmark multiple runs of Coro recursively until end point is reached
void runTimedWorkCoro(std::chrono::time_point<std::chrono::system_clock> end) {
	schedule([]() {BM_WorkCoro(); });

	if (std::chrono::system_clock::now() < end) {
		continuation([=]() {runTimedWorkCoro(end); });
	}
	else {
		continuation([]() {
			std::cout << std::endl <<	"   Test: workCoro"					<< std::endl;
			std::cout << std::endl <<	"   Number of calls:           "	<< work_coro_calls << std::endl;
			std::cout <<				"   Mean job execution time:   "	<< total_work_coro_runtime.count() / work_coro_calls << " ms" << std::endl;
			vgjs::terminate();
		});
	}
}













// Legacy
void run(int n) {
	//vgjs::schedule(std::bind(workFunc::test));
	//vgjs::schedule(std::bind(workCoro::test));
	//vgjs::schedule(std::bind(mandelbrotFunc::test));
	//vgjs::schedule(std::bind(mandelbrotCoro::test));
	//vgjs::schedule(std::bind(mctsFunc::test));
	//vgjs::schedule(std::bind(mctsCoro::test));

	if (n <= 1)
		vgjs::continuation([]() {vgjs::terminate(); });
	else
		vgjs::continuation([=]() {run(n - 1); });
}




static void BM_All(benchmark::State& state) {
	for (auto _ : state) {
		// This code gets timed

		using namespace vgjs;
		JobSystem::instance(NUM_THREADS);
		//enable_logging();

		schedule(std::bind(run, state.range(0)));
		wait_for_termination();

		//std::cout << "Exit BM_All\n";
	}
}



