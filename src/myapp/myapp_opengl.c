// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

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

  GLuint primaryFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(primaryFragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(primaryFragmentShader);
  glGetShaderiv(primaryFragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(primaryFragmentShader, 512, NULL, infoLog);
    fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n %s",
            infoLog);
  }

  GLuint yellowFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(yellowFragmentShader, 1, &yellowFragmentShaderSource, NULL);
  glCompileShader(yellowFragmentShader);
  glGetShaderiv(yellowFragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(yellowFragmentShader, 512, NULL, infoLog);
    fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n %s",
            infoLog);
  }

  state->shaderProgram = glCreateProgram();
  glAttachShader(state->shaderProgram, vertexShader);
  glAttachShader(state->shaderProgram, primaryFragmentShader);
  glLinkProgram(state->shaderProgram);

  glGetProgramiv(state->shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(state->shaderProgram, 512, NULL, infoLog);
    fprintf(stderr, "ERROR::SHADER::LINK_FAILED\n %s", infoLog);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(primaryFragmentShader);
  glDeleteShader(yellowFragmentShader);

  // Set up vertex data and VAO
  // clang-format off
  float vertices[] = {
    0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
   -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
  };
  /*
  unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
  };
  */
  // clang-format on

  glGenVertexArrays(1, &state->vao);

  glBindVertexArray(state->vao);

  glGenBuffers(1, &state->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  /*
  glGenBuffers(1, &state->ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  */

  glBindVertexArray(0);
}
