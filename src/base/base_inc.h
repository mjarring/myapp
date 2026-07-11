// File: base_inc.h
// ------
// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)
// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

#ifndef BASE_INC_H
#define BASE_INC_H

////////////////////////////////
//~ rjf: Base Includes

#include "base_context_cracking.h"

#include "base_core.h"

#if OS_LINUX
#include "linux/base/linux_base.h"
#else
#error Operating system backend not found for base layer.
#endif

#endif
