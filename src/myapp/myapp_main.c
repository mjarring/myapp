// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

// H
#include "base/base_inc.h"
#include "myapp_inc.h"

// C
#include "base/base_inc.h"
#include "myapp_inc.c"

// Entry Point
int main(int argc, char *argv[]) {
  struct client_state state = {0};
  state.width = 640;
  state.height = 480;
  state.wl_display = wl_display_connect(NULL);
  if (!state.wl_display) {
    fprintf(stderr, "Failed to connect to Wayland display.\n");
    return 1;
  }
  state.wl_registry = wl_display_get_registry(state.wl_display);
  state.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  wl_registry_add_listener(state.wl_registry, &registry_listener, &state);
  wl_display_roundtrip(state.wl_display);

  init_opengl(&state);

  state.wl_surface = wl_compositor_create_surface(state.wl_compositor);
  state.xdg_surface =
      xdg_wm_base_get_xdg_surface(state.xdg_wm_base, state.wl_surface);
  xdg_surface_add_listener(state.xdg_surface, &xdg_surface_listener, &state);
  state.xdg_toplevel = xdg_surface_get_toplevel(state.xdg_surface);
  xdg_toplevel_add_listener(state.xdg_toplevel, &xdg_toplevel_listener, &state);
  xdg_toplevel_set_title(state.xdg_toplevel, "MyApp");
  wl_surface_commit(state.wl_surface);

  struct wl_callback *cb = wl_surface_frame(state.wl_surface);
  wl_callback_add_listener(cb, &wl_surface_frame_listener, &state);

  while (wl_display_dispatch(state.wl_display) != -1) {
    // Intentionally blank
  }

  glDeleteVertexArrays(1, &state.vao);
  glDeleteBuffers(1, &state.vbo);
  glDeleteProgram(state.shaderProgram);

  return 0;
}
