// File: window_manager_inc.c
// ------
// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)
// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

#include "window_manager_core.c"
#if WM_STUB
#include "window_manager_stub.c"
#elif OS_LINUX
#include "linux/window_manager/linux_window_manager.c"
#else
#error Window manager layer not implemented for this operating system.
#endif
