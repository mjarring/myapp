#!/bin/bash

# Cross-compiles myapp for arm64 / aarch64 using Docker.
# Requires Docker. Build environment is defined in Dockerfile.aarch64.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE=myapp-aarch64-builder

docker build -f "$SCRIPT_DIR/Dockerfile.aarch64" -t "$IMAGE" "$SCRIPT_DIR"

docker run --rm \
  --user "$(id -u):$(id -g)" \
  -v "$SCRIPT_DIR:/work" \
  "$IMAGE" \
  compile-aarch64.sh
