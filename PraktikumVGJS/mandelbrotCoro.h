#pragma once

#include "vgjs.h"

using namespace vgjs;

namespace mandelbrotCoro {
	Coro<> draw();
	Coro<> calculatePixel(uint32_t, uint32_t);
	Coro<> mandelbrotRecursive(uint32_t);
	Coro<> mandelbrotLoop();
	Coro<> test();
}