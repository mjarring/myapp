#!/bin/bash

# Compiles the application for aarch64.

source compile-common.sh

MYAPP_CC=${MYAPP_CC:-"aarch64-linux-gnu-gcc"}
MYAPP_LIB_DIR=${MYAPP_LIB_DIR:-"/usr/lib"}
MYAPP_AARCH64_LIBDIR="${MYAPP_LIB_DIR}/${MYAPP_AARCH64_LIBDIR:-"aarch64-linux-gnu"}"
MYAPP_BUILD_DIR=${MYAPP_BUILD_DIR:-"/work/build-aarch64"}

mkdir -p "${MYAPP_BUILD_DIR}"
pushd "${MYAPP_BUILD_DIR}" || exit 1

generate_wayland_source

$MYAPP_CC "${MYAPP_CFLAGS[@]}" -o myapp "$MYAPP_SRC" "$MYAPP_GEN_C" "${MYAPP_LDFLAGS[@]}" -L"$MYAPP_AARCH64_LIBDIR"

popd || exit 1

echo "Done."
