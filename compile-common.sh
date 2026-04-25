#!/bin/bash

# Common stuff for building the application, shared between x86 and arm64 builds
# Paths are relative to docker container, not host

MYAPP_GEN_C=${MYAPP_GEN_C:-"xdg-shell-protocol.c"}
# shellcheck disable=SC2034 # Unused here but used in scripts that source this
MYAPP_SRC=${MYAPP_SRC:-"/work/source/myapp.cpp"}
# shellcheck disable=SC2034 # Unused here but used in scripts that source this
MYAPP_CFLAGS=(-I.)
# shellcheck disable=SC2034 # Unused here but used in scripts that source this
MYAPP_LDFLAGS=(-lwayland-client -lxkbcommon -lrt)

MYAPP_XDG_SHELL_XML=${MYAPP_XDG_SHELL_XML:-"/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml"}

generate_wayland_source() {

  if [ ! -f "$MYAPP_XDG_SHELL_XML" ]; then
    echo "ERROR: $MYAPP_XDG_SHELL_XML not found. Check your wayland-protocols package."
    exit 1
  fi

  # Generate wayland protocol code
  wayland-scanner client-header "$MYAPP_XDG_SHELL_XML" xdg-shell-client-protocol.h
  wayland-scanner private-code "$MYAPP_XDG_SHELL_XML" "$MYAPP_GEN_C"
}
