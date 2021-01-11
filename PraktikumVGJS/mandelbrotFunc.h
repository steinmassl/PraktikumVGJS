#pragma once

#include "vgjs.h"

using namespace vgjs;

namespace mandelbrotFunc {
	void draw();
	void calculatePixel(uint32_t, uint32_t);
	void mandelbrotRecursive(uint32_t);
	void test();
}
