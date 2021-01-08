#pragma once

// Abstracted VGJS includes to toggle them easily

// Toggle Queue type here
#define LOCKLESS 0

#if (LOCKLESS)
	#include "VEGameJobSystemLockless.h"
	#include "VECoroLockless.h"
#else
	#include "VEGameJobSystem.h"
	#include "VECoro.h"
#endif
