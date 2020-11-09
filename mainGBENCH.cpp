
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <functional>
#include <string>
#include <algorithm>
#include <chrono>

#include "VEGameJobSystem.h"
#include "benchmark.h"

namespace coro {
	void test();
}

namespace func {
	void test();
}

namespace mixed {
	void test();
}

namespace docu {
	void test(int);
}


void driver( int n ) {

	vgjs::schedule( std::bind(coro::test) );
	vgjs::schedule (std::bind(func::test) );
	vgjs::schedule(std::bind(mixed::test) );

	if (n <= 1) {
		vgjs::continuation([]() { std::cout << "terminate()\n";  vgjs::terminate(); } );
	}
	else {
		vgjs::continuation([=]() { std::cout << "driver(" << n << ")\n";  driver(n - 1); } );
	}
}

static void BM_Run(benchmark::State& state) {			//Benchmark wrapper function
	using namespace vgjs;
	for (auto _ : state) {
		// This code gets timed
		
		JobSystem::instance();
		//enable_logging();

		schedule([=]() { driver(state.range(0)); });
		//schedule([=]() {docu::test(state.range(0)); });

		wait_for_termination();

		//std::cout << "Exit\n";
	}
}

int n = 50;		

BENCHMARK(BM_Run)->Iterations({ 1 })->Unit(benchmark::kMillisecond)->MeasureProcessCPUTime()->Arg(n);
//BENCHMARK(BM_Run)->Iterations({ 1 })->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->Arg(n);
//BENCHMARK(BM_Run)->Iterations({ 1 })->Unit(benchmark::kNanosecond)->MeasureProcessCPUTime()->Arg(n);
//BENCHMARK_MAIN();




