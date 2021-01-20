#pragma once

#include "vgjs.h"
//#include "benchmark.h"

using namespace vgjs;

namespace work {
	Coro<> test();
	//Coro<> benchmarkWithFixedTime(const uint32_t, const uint32_t, const uint32_t);
}

namespace mandelbrot {
	Coro<> test();
}

namespace mctsFunc {
	void test();
}

namespace mctsCoro {
	Coro<> test();
}

namespace lock_free {
	Coro<> test();
}

namespace test {
	Coro<> start_test();
}

//void BM_Work(benchmark::State&);