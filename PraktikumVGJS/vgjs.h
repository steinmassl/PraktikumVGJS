#pragma once

// Abstracted VGJS includes to toggle them easily

// Toggle Queue type here
#define LOCKFREE 0

#if (LOCKFREE)
	#include "VEGameJobSystemLockfree.h"
	#include "VECoroLockfree.h"
#else
	#include "VEGameJobSystem.h"
	#include "VECoro.h"
#endif

// Current Platform/Compiler for output
#if(defined(_MSC_VER))
	static const std::string g_platform = "Win";
#elif(defined(__GNUC__))
	static const std::string g_platform = "Linux";
#endif

// CPU hardware threads for output 
//static const uint32_t g_cpu_hardware_threads = std::thread::hardware_concurrency();

// Global memory resource
//static auto g_global_mem = n_pmr::synchronized_pool_resource({ .max_blocks_per_chunk = 10000, .largest_required_pool_block = 1 << 10 }, n_pmr::new_delete_resource());
