#!/bin/bash

# Compiles the application for x86

source compile-common.sh

MYAPP_BUILD_DIR=${MYAPP_BUILD_DIR:-"/work/build-x86"}

mkdir -p "${MYAPP_BUILD_DIR}"

pushd "${MYAPP_BUILD_DIR}" || exit 1

generate_wayland_source

gcc "${CFLAGS[@]}" -o myapp "$MYAPP_SRC" "$MYAPP_GEN_C" "${LDFLAGS[@]}"

popd || exit 1

echo "Done."
