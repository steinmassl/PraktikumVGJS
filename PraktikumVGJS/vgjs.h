#pragma once

// Abstracted VGJS includes to toggle them easily

// Toggle Queue type here
#define LOCKFREE 1

#if (LOCKFREE)
	#include "VEGameJobSystemLockfree.h"
	#include "VECoroLockfree.h"
#else
	#include "VEGameJobSystem.h"
	#include "VECoro.h"
#endif
