#pragma once

// Abstracted VGJS includes to toggle them easily

// Toggle Queue type here
#define LOCKFREE 0

#if (LOCKFREE)
	#include "VGJSLockfree.h"
	#include "VGJSCoroLockfree.h"
#else
	#include "VGJS.h"
	#include "VGJSCoro.h"
#endif
