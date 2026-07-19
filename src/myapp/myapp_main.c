// File: myapp_main.c
// ------
// Copyright (c) 2026 Morgan Arrington
// Licensed under the MIT license (https://opensource.org/license/mit/)

#include "base/base_inc.h"
#include "mdesk/mdesk.h"
#include "window_manager/window_manager_inc.h"

#include "base/base_inc.c"
#include "mdesk/mdesk.c"
#include "window_manager/window_manager_inc.c"

internal void entry_point(CmdLine *cmd_line)
{
  fprintf(stdout, "Hello from myapp\n");
}
