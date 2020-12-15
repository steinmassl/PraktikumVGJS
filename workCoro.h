#pragma once

#include "include.h"

namespace workCoro {

	Coro<> work(const int);
	//Coro<> test(const int&, const int&, const bool);

	Coro<> benchmarkWork(const int, const int);
	//Coro<> benchmarkTimedWork(const int, const int, std::chrono::time_point<std::chrono::system_clock>);
}