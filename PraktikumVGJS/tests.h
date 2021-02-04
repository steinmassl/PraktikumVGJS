#pragma once

#include "VGJSHeaders.h"

using namespace vgjs;

namespace work {
	Coro<> test();
}

namespace mandelbrot {
	Coro<> test();
}

namespace mcts {
	Coro<> test();
}

namespace lock_free {
	void test();
}

namespace test {
	Coro<> start_test();
}

namespace mjs {
	void test(const uint32_t);
}