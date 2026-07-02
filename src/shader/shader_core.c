// Copyright (c) 2026 Morgan Arrington. All Rights Reserved.

void create_shader(const char *vertex_path, const char *fragment_path) {
  FILE *vert_shader_file = fopen(vertex_path, "r");
  if (!vert_shader_file) {
    fprintf(stderr, "Failed to open vertex shader file!");
    return;
  }

  FILE *frag_shader_file = fopen(vertex_path, "r");
  if (!frag_shader_file) {
    fprintf(stderr, "Failed to open vertex shader file!");
    return;
  }

  char *line = NULL; // Must be initialized to NULL
  size_t len = 0;    // Must be initialized to 0
  ssize_t read;

  // getline handles dynamic memory allocation for you
  while ((read = getline(&line, &len, fp)) != -1) {
    // 'read' contains the length of the string, 'line' contains the string
    printf("Read %zd characters: %s", read, line);
  }

  // Cleanup: getline allocates memory using malloc, so you MUST free it
  free(line);
  fclose(fp);
}
