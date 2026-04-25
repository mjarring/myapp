#!/bin/bash

# Common stuff for building the application, shared between x86 and arm64 builds
# Paths are relative to docker container, not host

export GEN_C=xdg-shell-protocol.c
export SRC=/work/source/myapp.cpp
export CFLAGS=(-I.)
export LDFLAGS=(-lwayland-client -lxkbcommon -lrt)

generate_wayland_source() {
  XDG_SHELL_XML=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml

  if [ ! -f "$XDG_SHELL_XML" ]; then
    echo "ERROR: $XDG_SHELL_XML not found. Check your wayland-protocols package."
    exit 1
  fi

  # Generate wayland protocol code
  wayland-scanner client-header "$XDG_SHELL_XML" xdg-shell-client-protocol.h
  wayland-scanner private-code "$XDG_SHELL_XML" "$GEN_C"
}
