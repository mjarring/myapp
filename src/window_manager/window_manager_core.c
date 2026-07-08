// File: window_manager_core.c
// ------
// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)
// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

#include "window_manager_core.h"

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include "base/base_core.h"
#include "base/base_strings.h"
#include "xdg-shell-client-protocol.h"

#include "xdg-shell-protocol.c"

void registry_global(void *data, struct wl_registry *wl_registry, uint32_t name,
                     const char *interface, uint32_t version) {
  struct wm_globals *wm_globals = (struct wm_globals *)data;

  String8 interface_str = str8_cstring((char *)interface);
  String8 wl_compositor_interface_name_str =
      str8_cstring((char *)wl_compositor_interface.name);
  String8 xdg_wm_base_interface_name_str =
      str8_cstring((char *)xdg_wm_base_interface.name);
  String8 wl_seat_interface_name_str =
      str8_cstring((char *)wl_seat_interface.name);

  if (str8_match(interface_str, wl_compositor_interface_name_str, 0)) {
    wm_globals->wl_compositor = (struct wl_compositor *)wl_registry_bind(
        wl_registry, name, &wl_compositor_interface, 4);
  } else if (str8_match(interface_str, xdg_wm_base_interface_name_str, 0)) {
    wm_globals->xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(
        wl_registry, name, &xdg_wm_base_interface, 1);
    /*
    xdg_wm_base_add_listener(wm_globals->xdg_wm_base, &xdg_wm_base_listener,
                             wm_globals);
                             */
  } else if (str8_match(interface_str, wl_seat_interface_name_str, 0)) {
    wm_globals->wl_seat = (struct wl_seat *)wl_registry_bind(
        wl_registry, name, &wl_seat_interface, 7);
    /*
    wl_seat_add_listener(wm_globals->wl_seat, &wl_seat_listener, wm_globals);
    */
  }
}

void registry_global_remove(void *data, struct wl_registry *registry,
                            uint32_t name) {
  // Deliberately left blank
}

B32 wm_init_globals(struct wm_globals *wm_globals) {
  wm_globals->wl_display = wl_display_connect(NULL);
  if (!wm_globals->wl_display) {
    fprintf(stderr, "Failed to connect to Wayland display.\n");
    return 0;
  }
  wm_globals->wl_registry = wl_display_get_registry(wm_globals->wl_display);
  wm_globals->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  wl_registry_add_listener(wm_globals->wl_registry, &registry_listener,
                           wm_globals);
  wl_display_roundtrip(wm_globals->wl_display);
  return 1;
}
