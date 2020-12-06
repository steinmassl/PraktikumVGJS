#include <iostream>

#include "VEGameJobSystem.h"
#include "benchmark.h"

namespace func {
	void test(int,int);
	void work(int);
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
static void BM_Func(benchmark::State& state) {
	using namespace vgjs;
	JobSystem::instance(NUM_THREADS);
	//enable_logging();

	for (auto _ : state) {
		// This code gets timed
		schedule([=]() {func::test(NUM_JOBS, NUM_LOOPS); });
		wait_for_termination();
	}
}

// Benchmark wrapper for entire Coro Test
static void BM_Coro(benchmark::State& state) {
	using namespace vgjs;
	JobSystem::instance(NUM_THREADS);
	//enable_logging();

	for (auto _ : state) {
		// This code gets timed
		schedule([=]() {coro::test(NUM_JOBS, NUM_LOOPS); });
		wait_for_termination();
	}
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
static void BM_MandelbrotFunc(benchmark::State& state) {
	using namespace vgjs;
	JobSystem::instance(NUM_THREADS);
	//enable_logging();

	for (auto _ : state) {
		// This code gets timed
		schedule([=]() {mandelbrotCoro::test(); });
		wait_for_termination();
	}
}

//BENCHMARK(BM_All)->Iterations({ 1 })->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Arg(n);		//Milli
//BENCHMARK(BM_All)->Iterations({ 1 })->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->Arg(n);		//Micro
//BENCHMARK(BM_All)->Iterations({ 1 })->Unit(benchmark::kNanosecond)->MeasureProcessCPUTime()->Arg(n);		//Nano

// First benchmark work itself, then benchmark multiple calls with Functions and Coros
BENCHMARK(BM_Work)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime();
BENCHMARK(BM_Func)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Iterations({ 1 });
//BENCHMARK(BM_Coro)->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Iterations({ 1 });





