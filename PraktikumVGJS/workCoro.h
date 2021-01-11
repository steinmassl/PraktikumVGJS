#pragma once

#include "vgjs.h"

using namespace vgjs;

namespace workCoro {

	Coro<> work(const int /*, std::allocator_arg_t, n_pmr::memory_resource* */);
	Coro<> benchmarkWorkWithFixedSize(const int, const int);
	Coro<> benchmarkWorkWithFixedTime(const int, const int, const int, const int /*, std::allocator_arg_t, n_pmr::memory_resource* */);
}