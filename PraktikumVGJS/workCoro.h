#pragma once

#include "vgjs.h"

using namespace vgjs;

namespace workCoro {

	Coro<> work(const int);
	Coro<> benchmarkWorkWithFixedSize(const int, const int);
	Coro<> benchmarkWorkWithFixedTime(const int, const int, const int, const int);
}