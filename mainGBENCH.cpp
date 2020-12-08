#include <iostream>

#include "VEGameJobSystem.h"
#include "benchmark.h"

namespace func {
	void test(int,int);
	void work(int);
	void BM_Func(benchmark::State&);
}

namespace coro {
	void test(int,int);
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

// General Settings
static constexpr int NUM_ITERATIONS = 1;
static constexpr int NUM_THREADS = 4;

// Func and Coro Test Settings
static constexpr int NUM_JOBS = 4;
static constexpr int NUM_LOOPS = 100000;

//

// Legacy
void run(int n) {
	//vgjs::schedule(std::bind(func::test));
	//vgjs::schedule(std::bind(coro::test));
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




// Benchmark wrapper for standard job in Func and Coro Tests
static void BM_Work(benchmark::State& state) {
	for (auto _ : state) {
		// This code gets timed
		func::work(NUM_LOOPS);
	}
}

// Benchmark wrapper for entire Func Test
static void BM_Func() {
	using namespace vgjs;
	// This code gets timed
	auto start = std::chrono::high_resolution_clock::now();
	schedule([]() {func::test(NUM_THREADS, NUM_LOOPS); });
	continuation([=]() {
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed_milliseconds = end - start;
		std::cout << "BM_Func: " << elapsed_milliseconds.count() << " ms" << std::endl;
		vgjs::terminate();
	});
}

// Benchmark wrapper for entire Coro Test
static void BM_Coro(benchmark::State& state) {
	using namespace vgjs;
	JobSystem::instance(NUM_THREADS);
	//enable_logging();

	for (auto _ : state) {
		// This code gets timed
		coro::test(NUM_JOBS, NUM_LOOPS);
	}
	vgjs::wait_for_termination();
}

// Benchmark wrapper for Mandelbrot Func Test
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

// Benchmark wrapper for Mandelbrot Coro Test
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

void BM_Test(benchmark::State& state) {
	int microseconds = 1;
	std::chrono::duration<double, std::micro> sleep_duration{
	  static_cast<double>(microseconds)
	};
	for (auto _ : state) {
		auto start = std::chrono::high_resolution_clock::now();
		// Simulate some useful workload with a sleep
		std::this_thread::sleep_for(sleep_duration);
		auto end = std::chrono::high_resolution_clock::now();

		auto elapsed_seconds =
			std::chrono::duration_cast<std::chrono::duration<double>>(
				end - start);

		state.SetIterationTime(elapsed_seconds.count());
		//vgjs::terminate();
	}
}

int main(int argc, char** argv) {

	// Benchmark normal C++ Functions
	auto work_benchmark = [](benchmark::State& state) { BM_Work(state); };

	benchmark::RegisterBenchmark("Work", work_benchmark)->MeasureProcessCPUTime()->Unit(benchmark::kMillisecond);
	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();
	std::cout << std::endl;

	// Benchmark Jobs
	using namespace vgjs;
	JobSystem::instance(NUM_THREADS);
	//enable_logging();

	schedule([]() {BM_Func(); });

	wait_for_termination();
}

//BENCHMARK(BM_All)->Iterations({ 1 })->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Arg(n);		//Milli
//BENCHMARK(BM_All)->Iterations({ 1 })->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->Arg(n);		//Micro
//BENCHMARK(BM_All)->Iterations({ 1 })->Unit(benchmark::kNanosecond)->MeasureProcessCPUTime()->Arg(n);		//Nano

// First benchmark work itself, then benchmark multiple calls with Functions and Coros
//BENCHMARK(BM_Work)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
//BENCHMARK(BM_Func)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Iterations({ 1 });
//BENCHMARK(BM_Coro)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Iterations({ 1 });





