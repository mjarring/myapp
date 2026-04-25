#!/bin/bash

# This should run in a docker container

XDG_SHELL_XML=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
CC=aarch64-linux-gnu-gcc
ARM64_LIBDIR=/usr/lib/aarch64-linux-gnu

SRC=/workspace/source/myapp.cpp
GEN_C=xdg-shell-protocol.c

mkdir -p /workspace/build-rpi
pushd /workspace/build-rpi || exit 1

wayland-scanner client-header "$XDG_SHELL_XML" xdg-shell-client-protocol.h
wayland-scanner private-code "$XDG_SHELL_XML" "$GEN_C"

$CC -I. -L"$ARM64_LIBDIR" -o myapp "$SRC" "$GEN_C" \
  -lwayland-client -lxkbcommon -lrt

echo "Done. Binary: build-rpi/myapp (aarch64)"
