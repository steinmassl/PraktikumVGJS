#pragma once

#include "vgjs.h"
#include "benchmark.h"

using namespace vgjs;

namespace workFunc {

	void work(const uint32_t);
	void benchmarkWorkWithFixedSize(const uint32_t, const uint32_t);
	void benchmarkWorkWithFixedTime(const uint32_t, const uint32_t, const uint32_t, const uint32_t);
}

void BM_Work(benchmark::State&);