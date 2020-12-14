#pragma once

#include "include.h"

namespace workFunc {

	void work(const int);
	void test(const int, const int, const bool);

	void benchmarkWork(const int, const int);
	void benchmarkTimedWork(const int, const int, std::chrono::time_point<std::chrono::system_clock>);
}

void BM_Work(benchmark::State&);