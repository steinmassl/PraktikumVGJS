#pragma once

#include "vgjs.h"

using namespace vgjs;

namespace mandelbrotCoro {
	Coro<> draw();
	Coro<> calculatePixel(int, int);
	Coro<> mandelbrotRecursive(int);
	Coro<> mandelbrotLoop();
	Coro<> test();
}