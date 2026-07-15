// File: base_inc.c
// ------
// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)
// Copyright (c) 2026 Morgan Arrington
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////
//~ rjf: Base Includes

#include "base_core.c"
#include "base_profile.c"
#include "base_memory.c"
#include "base_arena.c"
#include "base_math.c"
#include "base_strings.c"
#include "base_memory_map.c"
#include "base_hash.c"
#include "base_system.c"
#include "base_threads.c"
#include "base_ring.c"
#include "base_thread_context.c"
#include "base_files.c"
#include "base_shared_memory.c"
#include "base_processes.c"
#include "base_dynamic_libraries.c"
#include "base_command_line.c"
#include "base_markup.c"
#include "base_meta.c"
#include "base_log.c"
#include "base_test.c"
#include "base_entry_point.c"

#if (OS_LINUX)
#include "linux/base_linux.c"
#elif (OS_MAC)
#include "mac/base_mac.c"
#else
#error Operating system not supported by base layer.
#endif
