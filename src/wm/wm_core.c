// File: wm_core.c
// ------
// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

#include "wm_core.h"

wm_globals wm_init_globals() {
  wm_globals globals = {};
  globals.wl_display = wl_display_connect(NULL);
  if (!globals.wl_display) {
    fprintf(stderr, "Failed to connect to Wayland display.\n");
    return 1;
  }
  globals.wl_registry = wl_display_get_registry(state.wl_display);
  globals.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  wl_registry_add_listener(state.wl_registry, &registry_listener, &state);
  wl_display_roundtrip(state.wl_display);
  return globals;
}
