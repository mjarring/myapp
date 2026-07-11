#!/bin/bash

# -----------------------------------------------------------------------------
# Script Name: build.sh
# Description: Builds C programs from source in this repository.
# -----------------------------------------------------------------------------

# Exit on error and tread undefined vars as error
set -eu
# Change directory to script directory
cd "$(dirname "$0")"

usage() {
  cat <<EOF
Usage: $(basename "$0") [OPTIONS] [ARGS]

Builds the myapp program.

Options:
  -h, --help    Display this help message and exit.

Arguments:
  myapp         Builds myapp program
  asan          Enable address sanitizing
EOF
}

# --- Unpack Arguments ---
for arg in "$@"; do
  if [[ "$arg" == "-h" || "$arg" == "--help" ]]; then
    usage
    exit 0
  fi
  declare "$arg"='1'
done
if [[ "$#" == "0" ]]; then myapp='1'; fi

cc_sanitize=""
if [[ "${asan:-0}" == "1" ]]; then
  echo "[asan enabled]"
  cc_sanitize="-fsanitize=address"
fi

# --- Compile/Link Line Definitions -------------------------------------------
cc_cflags_gcc=""
cc_cflags_clang=${cc_sanitize}" -fdiagnostics-absolute-paths"
cc_common="-std=c11 -mcx16 -I../src/ -I../local/ -g -Wall -Wno-unused-function"
cc_debug="-g -O0 -DBUILD_DEBUG=1 ${cc_common}"
cc_release="-g -O2 -DBUILD_DEBUG=0 ${cc_common}"
cc_link="-lrt -lm"

# --- External Libraries ------------------------------------------------------
# sudo dnf install -y wayland-devel wayland-protocols-devel libxkbcommon-devel mesa-libGL-devel mesa-libEGL-devel mesa-libgbm-devel
cc_wayland="-lwayland-client"
cc_xkbcommon="-lxkbcommon"
cc_render="-lGL -lEGL -lgbm"

# --- Choose Compile/Link Lines -----------------------------------------------
if [[ "${gcc:-0}" == "1" ]]; then
  compiler="${CC:-gcc}   $cc_cflags_gcc"
  echo "[gcc compile]"
elif [[ "${clang:-1}" == "1" ]]; then
  compiler="${CC:-clang} $cc_cflags_clang"
  echo "[clang compile]"
fi
if [[ "${release:-0}" == "1" ]]; then
  echo "[release mode]"
  compile="$compiler $cc_release"
elif [[ "${debug:-1}" == "1" ]]; then
  echo "[debug mode]"
  compile="$compiler $cc_debug"
fi

# --- Prep Directories --------------------------------------------------------
mkdir -p build local

# --- Generate Wayland protocol code ------------------------------------------
cc_xdg_shell_xml="/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml"
cc_linux_dmabuf_xml="/usr/share/wayland-protocols/unstable/linux-dmabuf/linux-dmabuf-unstable-v1.xml"
cd build
if [[ "${myapp:-0}" == "1" ]]; then
  wayland-scanner client-header "$cc_xdg_shell_xml" ../local/xdg-shell-client-protocol.h
  wayland-scanner private-code "$cc_xdg_shell_xml" ../local/xdg-shell-protocol.c
  wayland-scanner client-header "$cc_linux_dmabuf_xml" ../local/linux-dmabuf-unstable-v1-client-protocol.h
  wayland-scanner private-code "$cc_linux_dmabuf_xml" ../local/linux-dmabuf-unstable-v1-protocol.c
fi
cd ..

# --- Build Everything (@build_targets) ---------------------------------------
cd build
if [[ "${myapp:-0}" == "1" ]]; then
  didbuild=1 && $compile ../src/myapp/myapp_main.c $cc_link $cc_wayland $cc_xkbcommon $cc_render -o myapp
fi
cd ..

# --- Warn On No Builds -------------------------------------------------------
if [[ "${didbuild:-0}" == "0" ]]; then
  echo "[WARNING] no valid build target specified; must use build target names as arguments to this script, like \`./build.sh myapp\`."
  exit 1
fi
