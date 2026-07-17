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

# --- Determine architecture ---
arch_x64='0'
arch_arm64='0'
ARCH=$(uname -m)
if [ "$ARCH" = "x86_64" ]; then
  arch_x64='1'
elif [ "$ARCH" = "aarch64" ] || [ "$ARCH" = "arm64" ]; then
  arch_arm64='1'
else
  echo "[ERROR] Unsupported architecture: $ARCH"
  exit 1
fi

cc_sanitize=""
if [[ "${asan:-0}" == "1" ]]; then
  echo "[asan enabled]"
  cc_sanitize="-fsanitize=address"
fi

# --- Compile/Link Line Definitions -------------------------------------------
cc_cflags_gcc=""
cc_cflags_clang=${cc_sanitize}" -fdiagnostics-absolute-paths -Wno-initializer-overrides -Wno-incompatible-pointer-types-discards-qualifiers"
cc_common_all_arch="-std=c11 -I../src/ -I../local/ -D_GNU_SOURCE -g -Wall -Wno-unused-function -Wno-missing-braces -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable"
cc_common_x64="-mcx16"
cc_common_arm64=""
if [[ "${arch_x64:-0}" == "1" ]]; then
  cc_common="${cc_common_all_arch} ${cc_common_x64}"
elif [[ "${arch_arm64:-0}" == "1" ]]; then
  cc_common="${cc_common_all_arch} ${cc_common_arm64}"
fi
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

# --- Build & Run Metaprogram -------------------------------------------------
if [[ "${no_meta:-0}" == "0" ]]; then
  echo "[building metagen]"
  cd build
  $compiler $cc_debug ../src/metagen/metagen_main.c $cc_link -o metagen
  ./metagen
  cd ..
fi

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
