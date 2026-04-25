#!/bin/bash

# Cross-compiles myapp for Raspberry Pi 3 (arm64 / aarch64) using Docker.
# Requires Docker. Build environment is defined in Dockerfile.rpi.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE=myapp-rpi-builder

# Build the cross-compile image (cached after first run)
docker build -f "$SCRIPT_DIR/Dockerfile.rpi" -t "$IMAGE" "$SCRIPT_DIR"

# Run the cross-compile inside the container, output lands in build/ on the host
docker run --rm \
  --user "$(id -u):$(id -g)" \
  -v "$SCRIPT_DIR:/work" \
  "$IMAGE" \
  build-rpi-compile.sh
