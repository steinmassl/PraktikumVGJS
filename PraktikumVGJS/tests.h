#pragma once

#include "vgjs.h"
//#include "benchmark.h"

using namespace vgjs;

namespace work {
	Coro<> benchmarkWithFixedTime(const uint32_t, const uint32_t, const uint32_t);
}

//void BM_Work(benchmark::State&);

namespace mandelbrotFunc {

	void test();
	void benchmarkWithFixedTime(const uint32_t, const uint32_t);
}

namespace mandelbrotCoro {
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