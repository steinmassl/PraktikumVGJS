#pragma once

#include "vgjs.h"

using namespace vgjs;

namespace lock_free {

	template<typename JOB = Job>
	struct pointer_t;
	template<typename JOB = Job>
	struct node_t;

	void test();
}