#pragma once

#include "include.h"

namespace mandelbrotCoro {
	Coro<> draw();
	Coro<> calculatePixel(int, int);
	Coro<> mandelbrotRecursive(int);
	Coro<> mandelbrotLoop();
	Coro<> test();
}