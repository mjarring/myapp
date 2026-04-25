#!/bin/bash

# This should run in a docker container

source build-common.sh

CC=aarch64-linux-gnu-gcc
ARM64_LIBDIR=/usr/lib/aarch64-linux-gnu
BUILD_DIR="/work/build-rpi"

mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR} || exit 1

generate_wayland_source

$CC "${CFLAGS[@]}" -o myapp "$SRC" "$GEN_C" "${LDFLAGS[@]}" -L"$ARM64_LIBDIR"

popd || exit 1

echo "Done."
