#pragma once

#include "VEGameJobSystem.h"
#include "benchmark.h"

using namespace vgjs;

namespace workFunc {

	void work(const int);
	void benchmarkWorkWithFixedSize(const int, const int);
	void benchmarkWorkWithFixedTime(const int, const int, const int);
}

void BM_Work(benchmark::State&);