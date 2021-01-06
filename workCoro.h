#pragma once

#include "VEGameJobSystem.h"
#include "VECoro.h"

using namespace vgjs;

namespace workCoro {

	Coro<> work(const int);
	Coro<> benchmarkWorkWithFixedSize(const int, const int);
	Coro<> benchmarkWorkWithFixedTime(const int, const int, const int, const int);
}