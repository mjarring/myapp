// File: wm_core.h
// ------
// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

#ifndef WM_CORE_H
#define WM_CORE_H

struct wm_globals {
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;
  struct wl_compositor *wl_compositor;
  struct xdg_wm_base *xdg_wm_base;
  struct wl_seat *wl_seat;
};

#endif
