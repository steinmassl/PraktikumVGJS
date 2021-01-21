#pragma once

#include "vgjs.h"

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
	Coro<> test();
}

namespace test {
	Coro<> start_test();
}