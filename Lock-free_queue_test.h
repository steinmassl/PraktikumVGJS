#pragma once

#include "VEGameJobSystem.h"

using namespace vgjs;

namespace lock_free_queue {
	struct pointer_t;
	struct node_t;
	
	class Queuable;
	class JobQueue;

	void test();
}