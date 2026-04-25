#!/bin/bash

# Cross-compiles myapp for x86 using Docker.
# Requires Docker. Build environment is defined in Dockerfile.x86.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE=myapp-x86-builder

docker build -f "$SCRIPT_DIR/Dockerfile.x86" -t "$IMAGE" "$SCRIPT_DIR"

docker run --rm \
  --user "$(id -u):$(id -g)" \
  -v "$SCRIPT_DIR:/work" \
  "$IMAGE" \
  build-x86-compile.sh
