#!/bin/bash

# Common stuff for building the application, shared between x86 and arm64 builds
# Paths are relative to docker container, not host

MYAPP_SRC_DIR=${MYAPP_SRC_DIR:-"/work/source"}
MYAPP_SHARE_DIR=${MYAPP_SHARE_DIR:-"/usr/share"}

MYAPP_GEN_C="xdg-shell-protocol.c"
# shellcheck disable=SC2034 # Unused here but used in scripts that source this
MYAPP_SRC="${MYAPP_SRC_DIR}/myapp.cpp"
# shellcheck disable=SC2034 # Unused here but used in scripts that source this
MYAPP_CFLAGS=(-I.)
if [[ -n "${CFLAGS+x}" ]]; then
  CFLAGS+=" ${MYAPP_CFLAGS[*]}"
else
  CFLAGS="${MYAPP_CFLAGS[*]}"
fi
# shellcheck disable=SC2034 # Unused here but used in scripts that source this
MYAPP_LDFLAGS=(-lwayland-client -lxkbcommon -lrt)
if [[ -n "${LDFLAGS+x}" ]]; then
  LDFLAGS+=" ${MYAPP_LDFLAGS[*]}"
else
  LDFLAGS="${MYAPP_LDFLAGS[*]}"
fi

MYAPP_XDG_SHELL_XML="${MYAPP_SHARE_DIR}/wayland-protocols/stable/xdg-shell/xdg-shell.xml"

generate_wayland_source() {

  if [ ! -f "$MYAPP_XDG_SHELL_XML" ]; then
    echo "ERROR: $MYAPP_XDG_SHELL_XML not found. Check your wayland-protocols package."
    exit 1
  fi

  # Generate wayland protocol code
  wayland-scanner client-header "$MYAPP_XDG_SHELL_XML" xdg-shell-client-protocol.h
  wayland-scanner private-code "$MYAPP_XDG_SHELL_XML" "$MYAPP_GEN_C"
}
