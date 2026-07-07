// File: newapp_main.c
// ------
// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

#include "base/base_inc.h"
#include "window_manager/window_manager_inc.h"

#include "base/base_inc.c"
#include "window_manager/window_manager_inc.c"

int main(int argc, char **argv) {
  // Init wm
  struct wm_globals wm_globals = {};
  wm_init_globals(&wm_globals);
}
