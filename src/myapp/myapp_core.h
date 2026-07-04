// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

#ifndef MYAPP_CORE_H
#define MYAPP_CORE_H

#include <stdint.h>
#include "linux-dmabuf-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include <GLES2/gl2.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon.h>

#define NUM_BUFFERS 2

// Enums
enum pointer_event_mask {
  POINTER_EVENT_ENTER = 1 << 0,
  POINTER_EVENT_LEAVE = 1 << 1,
  POINTER_EVENT_MOTION = 1 << 2,
  POINTER_EVENT_BUTTON = 1 << 3,
  POINTER_EVENT_AXIS = 1 << 4,
  POINTER_EVENT_AXIS_SOURCE = 1 << 5,
  POINTER_EVENT_AXIS_STOP = 1 << 6,
  POINTER_EVENT_AXIS_DISCRETE = 1 << 7,
};

enum touch_event_mask {
  TOUCH_EVENT_DOWN = 1 << 0,
  TOUCH_EVENT_UP = 1 << 1,
  TOUCH_EVENT_MOTION = 1 << 2,
  TOUCH_EVENT_CANCEL = 1 << 3,
  TOUCH_EVENT_SHAPE = 1 << 4,
  TOUCH_EVENT_ORIENTATION = 1 << 5,
};

// Structs
struct pointer_event {
  uint32_t event_mask;
  wl_fixed_t surface_x, surface_y;
  uint32_t button, state;
  uint32_t time;
  uint32_t serial;
  struct {
    bool valid;
    wl_fixed_t value;
    int32_t discrete;
  } axes[2];
  uint32_t axis_source;
};

struct touch_point {
  bool valid;
  int32_t id;
  uint32_t event_mask;
  wl_fixed_t surface_x, surface_y;
  wl_fixed_t major, minor;
  wl_fixed_t orientation;
};

struct touch_event {
  uint32_t event_mask;
  uint32_t time;
  uint32_t serial;
  struct touch_point points[10];
};

struct client_gl_buffer {
  struct gbm_bo *bo;
  struct wl_buffer *wl_buf;
  GLuint fbo;
  bool busy;
};

struct client_state {
  // Globals
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;
  struct wl_compositor *wl_compositor;
  struct xdg_wm_base *xdg_wm_base;
  struct wl_seat *wl_seat;
  struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf;
  // Objects
  struct wl_surface *wl_surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct wl_keyboard *wl_keyboard;
  struct wl_pointer *wl_pointer;
  struct wl_touch *wl_touch;
  // State
  int width;
  int height;
  bool closed;
  bool configured;
  struct client_gl_buffer client_gl_buffers[NUM_BUFFERS];
  struct pointer_event pointer_event;
  struct xkb_state *xkb_state;
  struct xkb_context *xkb_context;
  struct xkb_keymap *xkb_keymap;
  struct touch_event touch_event;
  // Last known pointer location
  int pointer_x;
  int pointer_y;
  // Shaders
  GLuint shaderProgram;
  GLuint vbo;
  GLuint vao;
  // Animate
  uint32_t last_frame_time;
  float elapsed;
};

// Function definitions
static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer);

static void xdg_toplevel_configure(void *data,
                                   struct xdg_toplevel *xdg_toplevel,
                                   int32_t width, int32_t height,
                                   struct wl_array *states);

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel);

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                                  uint32_t serial);

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial);

static void wl_surface_frame_done(void *data, struct wl_callback *cb,
                                  uint32_t time);

// Pointer function defs
static void wl_pointer_enter(void *data, struct wl_pointer *wl_wl_pointer,
                             uint32_t serial, struct wl_surface *surface,
                             wl_fixed_t surface_x, wl_fixed_t surface_y);

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                             uint32_t serial, struct wl_surface *surface);

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer,
                              uint32_t time, wl_fixed_t surface_x,
                              wl_fixed_t surface_y);

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
                              uint32_t serial, uint32_t time, uint32_t button,
                              uint32_t state);

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                            uint32_t time, uint32_t axis, wl_fixed_t value);

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                                   uint32_t axis_source);

static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t time, uint32_t axis);

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
                                     uint32_t axis, int32_t discrete);

static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer);

// Keyboard function defs
static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                               uint32_t format, int32_t fd, uint32_t size);

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                              uint32_t serial, struct wl_surface *surface,
                              struct wl_array *keys);

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                            uint32_t serial, uint32_t time, uint32_t key,
                            uint32_t state);

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                              uint32_t serial, struct wl_surface *surface);

static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                                  uint32_t serial, uint32_t mods_depressed,
                                  uint32_t mods_latched, uint32_t mods_locked,
                                  uint32_t group);

static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                                    int32_t rate, int32_t delay);

// Touch function defs
static void wl_touch_down(void *data, struct wl_touch *wl_touch,
                          uint32_t serial, uint32_t time,
                          struct wl_surface *surface, int32_t id, wl_fixed_t x,
                          wl_fixed_t y);

static void wl_touch_up(void *data, struct wl_touch *wl_touch, uint32_t serial,
                        uint32_t time, int32_t id);

static void wl_touch_motion(void *data, struct wl_touch *wl_touch,
                            uint32_t time, int32_t id, wl_fixed_t x,
                            wl_fixed_t y);

static void wl_touch_cancel(void *data, struct wl_touch *wl_touch);

static void wl_touch_shape(void *data, struct wl_touch *wl_touch, int32_t id,
                           wl_fixed_t major, wl_fixed_t minor);

static void wl_touch_orientation(void *data, struct wl_touch *wl_touch,
                                 int32_t id, wl_fixed_t orientation);

static void wl_touch_frame(void *data, struct wl_touch *wl_touch);

// Seat function defs
static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
                                 uint32_t capabilities);

static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name);

static void
zwp_linux_dmabuf_format(void *data,
                        struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1,
                        uint32_t format);

static void zwp_linux_dmabuf_modifier(
    void *data, struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1,
    uint32_t format, uint32_t modifier_hi, uint32_t modifier_lo);

static void registry_global(void *data, struct wl_registry *wl_registry,
                            uint32_t name, const char *interface,
                            uint32_t version);

static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name);
#endif
