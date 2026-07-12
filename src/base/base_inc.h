// File: base_inc.h
// ------
// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)
// Copyright (c) 2026 Morgan Arrington
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef BASE_INC_H
#define BASE_INC_H

////////////////////////////////
//~ rjf: Base Includes

#include "base_context_cracking.h"

#include "base_core.h"
#include "base_profile.h"
#include "base_memory.h"
#include "base_arena.h"
#include "base_math.h"
#include "base_strings.h"
#include "base_memory_map.h"
#include "base_hash.h"
#include "base_system.h"
#include "base_threads.h"
#include "base_ring.h"
#include "base_thread_context.h"
#include "base_files.h"
#include "base_shared_memory.h"
#include "base_processes.h"
#include "base_dynamic_libraries.h"
#include "base_command_line.h"
#include "base_markup.h"
#include "base_meta.h"
#include "base_log.h"
#include "base_test.h"
#include "base_entry_point.h"

#if (OS_LINUX)
#include "linux/base_linux.h"
#else
#error "Base layer does not support this operating system"
#endif

#endif
