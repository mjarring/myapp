// File: window_manager_inc.h
// ------
// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)
// Copyright (c) 2026 Morgan Arrington
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef WINDOW_MANAGER_INC_H
#define WINDOW_MANAGER_INC_H

#include "window_manager.h"
#if WM_STUB
#include "window_manager_stub.h"
#elif OS_LINUX
#include "linux/linux_window_manager.h"
#else
#error Window manager layer not implemented for this operating system.
#endif

#endif // WINDOW_MANAGER_INC_H
