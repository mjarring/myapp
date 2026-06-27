// Headers
#include "linux-dmabuf-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <fcntl.h>
#include <gbm.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

// Impls
#include "linux-dmabuf-unstable-v1-protocol.c"
#include "xdg-shell-protocol.c"

#define NUM_BUFFERS 2

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
  struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf;
  // Objects
  struct wl_surface *wl_surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  // State
  int width;
  int height;
  bool closed;
  bool configured;
  // OpenGL Buffers
  struct client_gl_buffer client_gl_buffers[NUM_BUFFERS];
};

static void registry_global(void *data, struct wl_registry *wl_registry,
                            uint32_t name, const char *interface,
                            uint32_t version);
static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name);

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                                  uint32_t serial);

static void
zwp_linux_dmabuf_format(void *data,
                        struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1,
                        uint32_t format);

static void zwp_linux_dmabuf_modifier(
    void *data, struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1,
    uint32_t format, uint32_t modifier_hi, uint32_t modifier_lo);

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_topleve,
                                   int32_t width, int32_t height,
                                   struct wl_array *states);

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel);

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial);

static void wl_surface_frame_done(void *data, struct wl_callback *cb,
                                  uint32_t time);

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer);

static struct wl_buffer *draw_frame(struct client_state *state);

static struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static const struct wl_callback_listener wl_surface_frame_listener = {
    .done = wl_surface_frame_done,
};

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
};

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static const struct zwp_linux_dmabuf_v1_listener zwp_linux_dmabuf_v1_listener =
    {
        .format = zwp_linux_dmabuf_format,
        .modifier = zwp_linux_dmabuf_modifier,
};

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

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                                  uint32_t serial) {
  struct client_state *state = data;
  xdg_surface_ack_configure(xdg_surface, serial);

  if (state->configured) {
    // Already configured, don't need to redraw
    return;
  }

  // Get the first free buffer and draw frame
  struct wl_buffer *buffer = draw_frame(state);
  wl_surface_attach(state->wl_surface, buffer, 0, 0);
  wl_surface_commit(state->wl_surface);
  state->configured = true;
}

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_topleve,
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

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
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

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer) {
  struct client_gl_buffer *client_gl_buffer = data;
  client_gl_buffer->busy = false;
}

static struct wl_buffer *draw_frame(struct client_state *state) {
  struct client_gl_buffer *free_buffer = NULL;

  // 1. Find a buffer that the compositor isn't using
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

  // 2. Mark it as busy and bind its FBO for OpenGL rendering
  free_buffer->busy = true;
  glBindFramebuffer(GL_FRAMEBUFFER, free_buffer->fbo);

  // 3. Draw your animated frame
  glViewport(0, 0, state->width, state->height);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glFinish(); // Ensure GPU is done

  return free_buffer->wl_buf;
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
  wl_registry_add_listener(state.wl_registry, &registry_listener, &state);
  wl_display_roundtrip(state.wl_display);

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
  PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR =
      (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
  PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES =
      (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress(
          "glEGLImageTargetTexture2DOES");

  // Allocate a buffer on the GPU for rendering
  for (int i = 0; i < NUM_BUFFERS; ++i) {
    state.client_gl_buffers[i].bo =
        gbm_bo_create(gbm, state.width, state.height, GBM_FORMAT_XRGB8888,
                      GBM_BO_USE_RENDERING);

    struct gbm_bo *bo = state.client_gl_buffers[i].bo;
    int dmabuf_fd = gbm_bo_get_fd(bo);
    uint32_t stride = gbm_bo_get_stride(bo);
    uint64_t modifier = gbm_bo_get_modifier(bo);

    struct zwp_linux_buffer_params_v1 *params =
        zwp_linux_dmabuf_v1_create_params(state.zwp_linux_dmabuf);

    zwp_linux_buffer_params_v1_add(params, dmabuf_fd, 0, 0, stride,
                                   modifier >> 32, modifier & 0xFFFFFFFF);

    state.client_gl_buffers[i].wl_buf = zwp_linux_buffer_params_v1_create_immed(
        params, state.width, state.height, GBM_FORMAT_XRGB8888, 0);

    wl_buffer_add_listener(state.client_gl_buffers[i].wl_buf,
                           &wl_buffer_listener, &state.client_gl_buffers[i]);

    // Wrap the raw dmabuf FD into an EGL image
    EGLint image_attribs[] = {EGL_WIDTH,
                              state.width,
                              EGL_HEIGHT,
                              state.height,
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

    glGenFramebuffers(1, &state.client_gl_buffers[i].fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, state.client_gl_buffers[i].fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture, 0);
  }

  state.wl_surface = wl_compositor_create_surface(state.wl_compositor);
  state.xdg_surface =
      xdg_wm_base_get_xdg_surface(state.xdg_wm_base, state.wl_surface);
  xdg_surface_add_listener(state.xdg_surface, &xdg_surface_listener, &state);
  state.xdg_toplevel = xdg_surface_get_toplevel(state.xdg_surface);
  xdg_toplevel_add_listener(state.xdg_toplevel, &xdg_toplevel_listener, &state);
  xdg_toplevel_set_title(state.xdg_toplevel, "OpenGL Test");
  wl_surface_commit(state.wl_surface);

  struct wl_callback *cb = wl_surface_frame(state.wl_surface);
  wl_callback_add_listener(cb, &wl_surface_frame_listener, &state);

  while (wl_display_dispatch(state.wl_display) != -1) {
    // Intentionally blank
  }

  return 0;
}
