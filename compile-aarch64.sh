#!/bin/bash

# Compiles the application for aarch64.

source build-common.sh

CC=aarch64-linux-gnu-gcc
AARCH64_LIBDIR=/usr/lib/aarch64-linux-gnu
BUILD_DIR="/work/build-aarch64"

mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR} || exit 1

generate_wayland_source

$CC "${CFLAGS[@]}" -o myapp "$SRC" "$GEN_C" "${LDFLAGS[@]}" -L"$AARCH64_LIBDIR"

popd || exit 1

echo "Done."
