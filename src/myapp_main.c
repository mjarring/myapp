// Headers
#include "myapp_main.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/glx.h>
#include <GLES2/gl2ext.h>
#include <assert.h>
#include <fcntl.h>
#include <gbm.h>
#include <limits.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

// Impls
#include "linux-dmabuf-unstable-v1-protocol.c"
#include "xdg-shell-protocol.c"

// Shaders
const GLchar *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "  // Invert the Y axis to account for Wayland/FBO coorinate mismatch\n"
    "  gl_Position = vec4(aPos.x, -aPos.y, aPos.z, 1.0);\n"
    "}\0";

const GLchar *fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

// Global OpenGL function definitions
PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = NULL;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;

// Listener structs
static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

static const struct wl_callback_listener wl_surface_frame_listener = {
    .done = wl_surface_frame_done,
};

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
};

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static const struct wl_pointer_listener wl_pointer_listener = {
    .enter = wl_pointer_enter,
    .leave = wl_pointer_leave,
    .motion = wl_pointer_motion,
    .button = wl_pointer_button,
    .axis = wl_pointer_axis,
    .frame = wl_pointer_frame,
    .axis_source = wl_pointer_axis_source,
    .axis_stop = wl_pointer_axis_stop,
    .axis_discrete = wl_pointer_axis_discrete,
};

static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .key = wl_keyboard_key,
    .modifiers = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info,
};

static const struct wl_touch_listener wl_touch_listener = {
    .down = wl_touch_down,
    .up = wl_touch_up,
    .motion = wl_touch_motion,
    .frame = wl_touch_frame,
    .cancel = wl_touch_cancel,
    .shape = wl_touch_shape,
    .orientation = wl_touch_orientation,
};

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name,
};

static const struct zwp_linux_dmabuf_v1_listener zwp_linux_dmabuf_v1_listener =
    {
        .format = zwp_linux_dmabuf_format,
        .modifier = zwp_linux_dmabuf_modifier,
};

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

// Implementations
static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer) {
  struct client_gl_buffer *client_gl_buffer = data;
  client_gl_buffer->busy = false;
}

static struct wl_buffer *draw_frame(struct client_state *state) {
  struct client_gl_buffer *free_buffer = NULL;

  // Find a buffer that the compositor isn't using
  for (int i = 0; i < NUM_BUFFERS; i++) {
    if (!state->client_gl_buffers[i].busy) {
      free_buffer = &state->client_gl_buffers[i];
      break;
    }
  }

  if (!free_buffer) {
    fprintf(stderr, "Dropping frame, no buffers available!\n");
    return NULL;
  }

  // Mark buffer as busy and bind its FBO for OpenGL rendering
  free_buffer->busy = true;
  glBindFramebuffer(GL_FRAMEBUFFER, free_buffer->fbo);

  // Draw frame
  glViewport(0, 0, state->width, state->height);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(state->shaderProgram);
  glBindVertexArray(state->triangleVao);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glFinish();

  return free_buffer->wl_buf;
}

static void xdg_toplevel_configure(void *data,
                                   struct xdg_toplevel *xdg_toplevel,
                                   int32_t width, int32_t height,
                                   struct wl_array *states) {
  struct client_state *state = data;
  if (width == 0 || height == 0) {
    // Compositor is deferring to us
    return;
  }
  state->width = width;
  state->height = height;
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel) {
  struct client_state *state = data;
  state->closed = true;
}

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                                  uint32_t serial) {
  struct client_state *state = data;
  xdg_surface_ack_configure(xdg_surface, serial);

  if (state->configured) {
    return;
  }

  struct wl_buffer *buffer = draw_frame(state);
  wl_surface_attach(state->wl_surface, buffer, 0, 0);
  wl_surface_commit(state->wl_surface);
  state->configured = true;
}

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static void wl_surface_frame_done(void *data, struct wl_callback *cb,
                                  uint32_t time) {
  // Destroy this callback
  wl_callback_destroy(cb);

  // Request another frame
  struct client_state *state = data;
  cb = wl_surface_frame(state->wl_surface);
  wl_callback_add_listener(cb, &wl_surface_frame_listener, state);

  // Submit a frame for this event
  struct wl_buffer *buffer = draw_frame(state);
  wl_surface_attach(state->wl_surface, buffer, 0, 0);
  wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_commit(state->wl_surface);
}

// Pointer function impls
static void wl_pointer_enter(void *data, struct wl_pointer *wl_wl_pointer,
                             uint32_t serial, struct wl_surface *surface,
                             wl_fixed_t surface_x, wl_fixed_t surface_y) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_ENTER;
  client_state->pointer_event.serial = serial;
  client_state->pointer_event.surface_x = surface_x;
  client_state->pointer_event.surface_y = surface_y;
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                             uint32_t serial, struct wl_surface *surface) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_LEAVE;
  client_state->pointer_event.serial = serial;
}

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer,
                              uint32_t time, wl_fixed_t surface_x,
                              wl_fixed_t surface_y) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_MOTION;
  client_state->pointer_event.time = time;
  client_state->pointer_event.surface_x = surface_x;
  client_state->pointer_event.surface_y = surface_y;
}

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
                              uint32_t serial, uint32_t time, uint32_t button,
                              uint32_t state) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_BUTTON;
  client_state->pointer_event.serial = serial;
  client_state->pointer_event.time = time;
  client_state->pointer_event.button = button;
  client_state->pointer_event.state = state;
}

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                            uint32_t time, uint32_t axis, wl_fixed_t value) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS;
  client_state->pointer_event.time = time;
  client_state->pointer_event.axes[axis].valid = true;
  client_state->pointer_event.axes[axis].value = value;
}

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                                   uint32_t axis_source) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_SOURCE;
  client_state->pointer_event.axis_source = axis_source;
}

static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t time, uint32_t axis) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_STOP;
  client_state->pointer_event.time = time;
  client_state->pointer_event.axes[axis].valid = true;
}

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
                                     uint32_t axis, int32_t discrete) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_DISCRETE;
  client_state->pointer_event.axes[axis].valid = true;
  client_state->pointer_event.axes[axis].discrete = discrete;
}

static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer) {
  struct client_state *client_state = data;
  struct pointer_event *event = &client_state->pointer_event;
  fprintf(stderr, "pointer frame @ %d: ", event->time);

  if (event->event_mask & POINTER_EVENT_ENTER) {
    fprintf(stderr, "entered %f, %f ", wl_fixed_to_double(event->surface_x),
            wl_fixed_to_double(event->surface_y));
    client_state->pointer_x = wl_fixed_to_int(event->surface_x);
    client_state->pointer_y = wl_fixed_to_int(event->surface_y);
  }

  if (event->event_mask & POINTER_EVENT_LEAVE) {
    fprintf(stderr, "leave");
  }

  if (event->event_mask & POINTER_EVENT_MOTION) {
    fprintf(stderr, "motion %f, %f ", wl_fixed_to_double(event->surface_x),
            wl_fixed_to_double(event->surface_y));
    client_state->pointer_x = wl_fixed_to_int(event->surface_x);
    client_state->pointer_y = wl_fixed_to_int(event->surface_y);
  }

  if (event->event_mask & POINTER_EVENT_BUTTON) {
    const char *state = event->state == WL_POINTER_BUTTON_STATE_RELEASED
                            ? "released"
                            : "pressed";
    fprintf(stderr, "button %d %s ", event->button, state);

    // Move the window if clicked near the top of frame
    if (event->button == BTN_LEFT &&
        event->state == WL_POINTER_BUTTON_STATE_PRESSED &&
        client_state->pointer_y < 25) {
      xdg_toplevel_move(client_state->xdg_toplevel, client_state->wl_seat,
                        client_state->pointer_event.serial);
    }
  }

  uint32_t axis_events = POINTER_EVENT_AXIS | POINTER_EVENT_AXIS_SOURCE |
                         POINTER_EVENT_AXIS_STOP | POINTER_EVENT_AXIS_DISCRETE;
  const char *axis_name[2] = {
      [WL_POINTER_AXIS_VERTICAL_SCROLL] = "vertical",
      [WL_POINTER_AXIS_HORIZONTAL_SCROLL] = "horizontal",
  };
  const char *axis_source[4] = {
      [WL_POINTER_AXIS_SOURCE_WHEEL] = "wheel",
      [WL_POINTER_AXIS_SOURCE_FINGER] = "fingy",
      [WL_POINTER_AXIS_SOURCE_CONTINUOUS] = "continuous",
      [WL_POINTER_AXIS_SOURCE_WHEEL_TILT] = "wheel tilt",
  };
  if (event->event_mask & axis_events) {
    for (size_t i = 0; i < 2; ++i) {
      if (!event->axes[i].valid) {
        continue;
      }
      fprintf(stderr, "%s axis ", axis_name[i]);
      if (event->event_mask & POINTER_EVENT_AXIS) {
        fprintf(stderr, "value %f ", wl_fixed_to_double(event->axes[i].value));
      }
      if (event->event_mask & POINTER_EVENT_AXIS_DISCRETE) {
        fprintf(stderr, "discrete %d ", event->axes[i].discrete);
      }
      if (event->event_mask & POINTER_EVENT_AXIS_SOURCE) {
        fprintf(stderr, "via %s ", axis_source[event->axis_source]);
      }
      if (event->event_mask & POINTER_EVENT_AXIS_STOP) {
        fprintf(stderr, "(stopped) ");
      }
    }
  }

  fprintf(stderr, "\n");
  memset(event, 0, sizeof(*event));
}

// Keyboard function impls
static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                               uint32_t format, int32_t fd, uint32_t size) {
  struct client_state *client_state = data;
  assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

  char *map_shm = (char *)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  assert(map_shm != MAP_FAILED);

  struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
      client_state->xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
      XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map_shm, size);
  close(fd);

  struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
  xkb_keymap_unref(client_state->xkb_keymap);
  xkb_state_unref(client_state->xkb_state);
  client_state->xkb_keymap = xkb_keymap;
  client_state->xkb_state = xkb_state;
}

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                              uint32_t serial, struct wl_surface *surface,
                              struct wl_array *keys) {
  struct client_state *client_state = data;
  fprintf(stderr, "keyboard enter; keys pressed are:\n");
  uint32_t *key = (uint32_t *)keys->data;
  uint32_t *key_end = key + (keys->size / sizeof(*key));
  for (; key < key_end; ++key) {
    char buf[128];
    xkb_keysym_t sym =
        xkb_state_key_get_one_sym(client_state->xkb_state, *key + 8);
    xkb_keysym_get_name(sym, buf, sizeof(buf));
    fprintf(stderr, "sym: %-12s (%d), ", buf, sym);
    xkb_state_key_get_utf8(client_state->xkb_state, *key + 8, buf, sizeof(buf));
    fprintf(stderr, "utf8: '%s'\n", buf);
  }
}

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                            uint32_t serial, uint32_t time, uint32_t key,
                            uint32_t state) {
  struct client_state *client_state = data;
  char buf[128];
  uint32_t keycode = key + 8;
  xkb_keysym_t sym =
      xkb_state_key_get_one_sym(client_state->xkb_state, keycode);
  xkb_keysym_get_name(sym, buf, sizeof(buf));
  const char *action =
      state == WL_KEYBOARD_KEY_STATE_PRESSED ? "press" : "release";
  fprintf(stderr, "key %s: sym: %-12s (%d), ", action, buf, sym);
  xkb_state_key_get_utf8(client_state->xkb_state, keycode, buf, sizeof(buf));
  fprintf(stderr, "utf8: '%s'\n", buf);
}

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                              uint32_t serial, struct wl_surface *surface) {
  fprintf(stderr, "keyboard leave\n");
}

static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                                  uint32_t serial, uint32_t mods_depressed,
                                  uint32_t mods_latched, uint32_t mods_locked,
                                  uint32_t group) {
  struct client_state *client_state = data;
  xkb_state_update_mask(client_state->xkb_state, mods_depressed, mods_latched,
                        mods_locked, 0, 0, group);
}

static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                                    int32_t rate, int32_t delay) {
  // TODO: Handle as needed
}

static struct touch_point *get_touch_point(struct client_state *client_state,
                                           int32_t id) {
  struct touch_event *touch = &client_state->touch_event;
  const size_t num_members = sizeof(touch->points) / sizeof(struct touch_point);
  int invalid = -1;
  for (size_t i = 0; i < num_members; ++i) {
    if (touch->points[i].id == id) {
      return &touch->points[i];
    }
    if (invalid == -1 && !touch->points[i].valid) {
      invalid = i;
    }
  }
  if (invalid == -1) {
    return NULL;
  }
  touch->points[invalid].valid = true;
  touch->points[invalid].id = id;
  return &touch->points[invalid];
}

static void wl_touch_down(void *data, struct wl_touch *wl_touch,
                          uint32_t serial, uint32_t time,
                          struct wl_surface *surface, int32_t id, wl_fixed_t x,
                          wl_fixed_t y) {
  struct client_state *client_state = data;
  struct touch_point *point = get_touch_point(client_state, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_DOWN;
  point->surface_x = wl_fixed_to_double(x);
  point->surface_y = wl_fixed_to_double(y);
  client_state->touch_event.time = time;
  client_state->touch_event.serial = serial;
}

static void wl_touch_up(void *data, struct wl_touch *wl_touch, uint32_t serial,
                        uint32_t time, int32_t id) {
  struct client_state *client_state = data;
  struct touch_point *point = get_touch_point(client_state, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_UP;
}

static void wl_touch_motion(void *data, struct wl_touch *wl_touch,
                            uint32_t time, int32_t id, wl_fixed_t x,
                            wl_fixed_t y) {
  struct client_state *client_state = data;
  struct touch_point *point = get_touch_point(client_state, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_MOTION;
  point->surface_x = x;
  point->surface_y = y;
  client_state->touch_event.time = time;
}

static void wl_touch_cancel(void *data, struct wl_touch *wl_touch) {
  struct client_state *client_state = data;
  client_state->touch_event.event_mask |= TOUCH_EVENT_CANCEL;
}

static void wl_touch_shape(void *data, struct wl_touch *wl_touch, int32_t id,
                           wl_fixed_t major, wl_fixed_t minor) {
  struct client_state *client_state = data;
  struct touch_point *point = get_touch_point(client_state, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_SHAPE;
  point->major = major;
  point->minor = minor;
}

static void wl_touch_orientation(void *data, struct wl_touch *wl_touch,
                                 int32_t id, wl_fixed_t orientation) {
  struct client_state *client_state = data;
  struct touch_point *point = get_touch_point(client_state, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_ORIENTATION;
  point->orientation = orientation;
}

static void wl_touch_frame(void *data, struct wl_touch *wl_touch) {
  struct client_state *client_state = data;
  struct touch_event *touch = &client_state->touch_event;
  const size_t num_members = sizeof(touch->points) / sizeof(struct touch_point);
  fprintf(stderr, "touch event @ %d:\n", touch->time);

  for (size_t i = 0; i < num_members; ++i) {
    struct touch_point *point = &touch->points[i];
    if (!point->valid) {
      continue;
    }
    fprintf(stderr, "point %d: ", point->id);

    if (point->event_mask & TOUCH_EVENT_DOWN) {
      fprintf(stderr, "down %f,%f ", wl_fixed_to_double(point->surface_x),
              wl_fixed_to_double(point->surface_y));
    }

    if (point->event_mask & TOUCH_EVENT_UP) {
      fprintf(stderr, "up ");
    }

    if (point->event_mask & TOUCH_EVENT_MOTION) {
      fprintf(stderr, "motion %f,%f ", wl_fixed_to_double(point->surface_x),
              wl_fixed_to_double(point->surface_y));
    }

    if (point->event_mask & TOUCH_EVENT_SHAPE) {
      fprintf(stderr, "shape %f,%f ", wl_fixed_to_double(point->major),
              wl_fixed_to_double(point->minor));
    }

    if (point->event_mask & TOUCH_EVENT_ORIENTATION) {
      fprintf(stderr, "orientation %f ",
              wl_fixed_to_double(point->orientation));
    }

    point->valid = false;
    fprintf(stderr, "\n");
  }
}

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
                                 uint32_t capabilities) {
  struct client_state *state = data;

  bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

  if (have_pointer && state->wl_pointer == NULL) {
    state->wl_pointer = wl_seat_get_pointer(state->wl_seat);
    wl_pointer_add_listener(state->wl_pointer, &wl_pointer_listener, state);
  } else if (!have_pointer && state->wl_pointer != NULL) {
    wl_pointer_release(state->wl_pointer);
    state->wl_pointer = NULL;
  }

  bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

  if (have_keyboard && state->wl_keyboard == NULL) {
    state->wl_keyboard = wl_seat_get_keyboard(state->wl_seat);
    wl_keyboard_add_listener(state->wl_keyboard, &wl_keyboard_listener, state);
  } else if (!have_keyboard && state->wl_keyboard != NULL) {
    wl_keyboard_release(state->wl_keyboard);
    state->wl_keyboard = NULL;
  }

  bool have_touch = capabilities & WL_SEAT_CAPABILITY_TOUCH;

  if (have_touch && state->wl_touch == NULL) {
    state->wl_touch = wl_seat_get_touch(state->wl_seat);
    wl_touch_add_listener(state->wl_touch, &wl_touch_listener, state);
  } else if (!have_touch && state->wl_touch != NULL) {
    wl_touch_release(state->wl_touch);
    state->wl_touch = NULL;
  }
}

static void wl_seat_name(void *data, struct wl_seat *wl_seat,
                         const char *name) {
  fprintf(stderr, "seat name: %s\n", name);
}

static void
zwp_linux_dmabuf_format(void *data,
                        struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1,
                        uint32_t format) {
  // Deprecated
}

static void zwp_linux_dmabuf_modifier(
    void *data, struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1,
    uint32_t format, uint32_t modifier_hi, uint32_t modifier_lo) {
  // Deprecated
}

static void registry_global(void *data, struct wl_registry *wl_registry,
                            uint32_t name, const char *interface,
                            uint32_t version) {
  struct client_state *state = data;
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    state->wl_compositor =
        wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    state->xdg_wm_base =
        wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, state);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    state->wl_seat = wl_registry_bind(wl_registry, name, &wl_seat_interface, 7);
    wl_seat_add_listener(state->wl_seat, &wl_seat_listener, state);
  } else if (strcmp(interface, zwp_linux_dmabuf_v1_interface.name) == 0) {
    state->zwp_linux_dmabuf =
        wl_registry_bind(wl_registry, name, &zwp_linux_dmabuf_v1_interface, 5);
    zwp_linux_dmabuf_v1_add_listener(state->zwp_linux_dmabuf,
                                     &zwp_linux_dmabuf_v1_listener, state);
  }
}

static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name) {
  // Deliberately left blank
}

static void init_opengl(struct client_state *state) {
  // Create OpenGL buffers
  int drm_fd = open("/dev/dri/renderD128", O_RDWR);
  struct gbm_device *gbm = gbm_create_device(drm_fd);

  // Configure EGL
  EGLDisplay egl_dpy = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm, NULL);
  eglInitialize(egl_dpy, NULL, NULL);
  eglBindAPI(EGL_OPENGL_ES_API);

  EGLint config_attribs[] = {EGL_SURFACE_TYPE, EGL_DONT_CARE,
                             EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE};
  EGLConfig egl_cfg;
  EGLint num_configs;
  eglChooseConfig(egl_dpy, config_attribs, &egl_cfg, 1, &num_configs);

  EGLContext egl_ctx = eglCreateContext(egl_dpy, egl_cfg, EGL_NO_CONTEXT, NULL);

  // Make current with NO surface (Headless)
  eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_ctx);

  // Create an EGL extension function pointer (requires casting in real code)
  eglCreateImageKHR =
      (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
  glEGLImageTargetTexture2DOES =
      (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress(
          "glEGLImageTargetTexture2DOES");

  // Create function pointer for glGenVertexArrays
  glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glXGetProcAddress(
      (const GLubyte *)"glGenVertexArrays");

  // Create function pointer for glBindVertexArray
  glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glXGetProcAddress(
      (const GLubyte *)"glBindVertexArray");

  glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)glXGetProcAddress(
      (const GLubyte *)"glDeleteVertexArrays");

  // TODO: (morgan) check for error

  // Allocate a buffer on the GPU for rendering
  for (int i = 0; i < NUM_BUFFERS; ++i) {
    state->client_gl_buffers[i].bo =
        gbm_bo_create(gbm, state->width, state->height, GBM_FORMAT_XRGB8888,
                      GBM_BO_USE_RENDERING);

    struct gbm_bo *bo = state->client_gl_buffers[i].bo;
    int dmabuf_fd = gbm_bo_get_fd(bo);
    uint32_t stride = gbm_bo_get_stride(bo);
    uint64_t modifier = gbm_bo_get_modifier(bo);

    struct zwp_linux_buffer_params_v1 *params =
        zwp_linux_dmabuf_v1_create_params(state->zwp_linux_dmabuf);

    zwp_linux_buffer_params_v1_add(params, dmabuf_fd, 0, 0, stride,
                                   modifier >> 32, modifier & 0xFFFFFFFF);

    state->client_gl_buffers[i].wl_buf =
        zwp_linux_buffer_params_v1_create_immed(
            params, state->width, state->height, GBM_FORMAT_XRGB8888, 0);

    wl_buffer_add_listener(state->client_gl_buffers[i].wl_buf,
                           &wl_buffer_listener, &state->client_gl_buffers[i]);

    // Wrap the raw dmabuf FD into an EGL image
    EGLint image_attribs[] = {EGL_WIDTH,
                              state->width,
                              EGL_HEIGHT,
                              state->height,
                              EGL_LINUX_DRM_FOURCC_EXT,
                              GBM_FORMAT_XRGB8888,
                              EGL_DMA_BUF_PLANE0_FD_EXT,
                              dmabuf_fd,
                              EGL_DMA_BUF_PLANE0_OFFSET_EXT,
                              0,
                              EGL_DMA_BUF_PLANE0_PITCH_EXT,
                              stride,
                              EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT,
                              modifier & 0xFFFFFFFF,
                              EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT,
                              modifier >> 32,
                              EGL_NONE};

    EGLImageKHR image = eglCreateImageKHR(
        egl_dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, image_attribs);

    // Map the EGL Image to a GL Texture, then bind to a Framebuffer
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    glGenFramebuffers(1, &state->client_gl_buffers[i].fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, state->client_gl_buffers[i].fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture, 0);
  }

  // Compile shaders
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLint success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s", infoLog);
  }

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n %s",
            infoLog);
  }

  state->shaderProgram = glCreateProgram();
  glAttachShader(state->shaderProgram, vertexShader);
  glAttachShader(state->shaderProgram, fragmentShader);
  glLinkProgram(state->shaderProgram);

  glGetProgramiv(state->shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(state->shaderProgram, 512, NULL, infoLog);
    fprintf(stderr, "ERROR::SHADER::LINK_FAILED\n %s", infoLog);
  }

  glUseProgram(state->shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // Set up vertex data and VAO
  // clang-format off
  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f
  };
  // clang-format on

  glGenVertexArrays(1, &state->triangleVao);

  glBindVertexArray(state->triangleVao);

  glGenBuffers(1, &state->triangleVbo);
  glBindBuffer(GL_ARRAY_BUFFER, state->triangleVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

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

  glDeleteVertexArrays(1, &state.triangleVao);
  glDeleteBuffers(1, &state.triangleVbo);
  glDeleteProgram(state.shaderProgram);

  return 0;
}
