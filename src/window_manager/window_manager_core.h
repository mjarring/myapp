// File: window_manager_core.h
// ------
// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)
// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

#ifndef WINDOW_MANAGER_CORE_H
#define WINDOW_MANAGER_CORE_H

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>

#include "base/base_core.h"
#include "xdg-shell-client-protocol.h"

struct wm_globals {
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;
  struct wl_compositor *wl_compositor;
  struct xdg_wm_base *xdg_wm_base;
  struct wl_seat *wl_seat;
  struct xkb_context *xkb_context;
};

void registry_global(void *data, struct wl_registry *wl_registry, uint32_t name,
                     const char *interface, uint32_t version);

void registry_global_remove(void *data, struct wl_registry *registry,
                            uint32_t name);

const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

B32 wm_init_globals(struct wm_globals *wm_globals);

#endif
