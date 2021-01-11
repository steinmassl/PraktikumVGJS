#pragma once

#include "vgjs.h"

using namespace vgjs;

namespace workCoro {

	Coro<> work(const uint32_t /*, std::allocator_arg_t, n_pmr::memory_resource* */);
	Coro<> benchmarkWorkWithFixedSize(const uint32_t, const uint32_t);
	Coro<> benchmarkWorkWithFixedTime(const uint32_t, const uint32_t, const uint32_t, const uint32_t /*, std::allocator_arg_t, n_pmr::memory_resource* */);
}