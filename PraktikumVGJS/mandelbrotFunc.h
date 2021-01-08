#pragma once

#include "vgjs.h"

using namespace vgjs;

namespace mandelbrotFunc {
	void draw();
	void calculatePixel(int, int);
	void mandelbrotRecursive(int);
	void test();
}
