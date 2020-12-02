#include <iostream>

#include "VEGameJobSystem.h"
#include "benchmark.h"

namespace func {
	void test();
}

namespace coro {
	void test();
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

void run(int n) {
	//vgjs::schedule(std::bind(func::test));
	//vgjs::schedule(std::bind(coro::test));
	//vgjs::schedule(std::bind(mandelbrotFunc::test));
	//vgjs::schedule(std::bind(mandelbrotCoro::test));
	vgjs::schedule(std::bind(mctsFunc::test));
	//vgjs::schedule(std::bind(mctsCoro::test));

	if (n <= 1)
		vgjs::continuation([]() {vgjs::terminate(); });
	else
		vgjs::continuation([=]() {run(n - 1); });
}

static void BM_Run(benchmark::State& state) {			//Benchmark wrapper function
	using namespace vgjs;
	for (auto _ : state) {
		// This code gets timed
		
		JobSystem::instance();
		//enable_logging();

		schedule(std::bind(run, state.range(0)));

		wait_for_termination();

		std::cout << "Exit\n";
	}
}

int n = 1;		

BENCHMARK(BM_Run)->Iterations({ 1 })->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Arg(n);		//Milli
//BENCHMARK(BM_Run)->Iterations({ 1 })->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->Arg(n);		//Micro
//BENCHMARK(BM_Run)->Iterations({ 1 })->Unit(benchmark::kNanosecond)->MeasureProcessCPUTime()->Arg(n);		//Nano
//BENCHMARK_MAIN();




