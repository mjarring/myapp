// File: base_markup.h
// ------
// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)
// Copyright (c) 2026 Morgan Arrington
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef BASE_MARKUP_H
#define BASE_MARKUP_H

#define RADDBG_MARKUP_IMPLEMENTATION
#define RADDBG_MARKUP_VSNPRINTF raddbg_vsnprintf
#if OS_LINUX
#define RADDBG_MARKUP_STUBS
#endif
#include "third_party/raddbg_markup/raddbg_markup.h"

#define ThreadNameF(...)                                                       \
  (set_thread_namef(__VA_ARGS__), raddbg_thread_color_u32(LAYER_COLOR))

#endif // BASE_MARKUP_H
