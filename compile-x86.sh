#!/bin/bash

# Compiles the application for x86

source build-common.sh

BUILD_DIR="/work/build"

mkdir -p "${BUILD_DIR}"

pushd ${BUILD_DIR} || exit 1

generate_wayland_source

gcc "${CFLAGS[@]}" -o myapp "$SRC" "$GEN_C" "${LDFLAGS[@]}"

popd || exit 1

echo "Done."
