#!/bin/bash

# -----------------------------------------------------------------------------
# Script Name: build_mac.sh
# Description: Builds C programs from source in this repository.
# MacOS is the OS target.
# -----------------------------------------------------------------------------

# Exit on error and treat undefined vars as error
set -eu
# Change directory to script directory
cd "$(dirname "$0")"

usage() {
  cat <<EOF
Usage: $(basename "$0") [OPTIONS] [ARGS]

Build script for MacOS.

Options:
  -h, --help    Display this help message and exit.

Arguments:
  sample_macos         Builds sample macos program
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
if [[ "$#" == "0" ]]; then sample_macos='1'; fi

# --- Compile/Link Line Definitions -------------------------------------------
cc_cflags_gcc=""
cc_cflags_clang=""
cc_common="-std=c11 -I../src/ -Wall"
cc_debug="-g -O0 -DBUILD_DEBUG=1 ${cc_common}"
cc_release="-g -O2 -DBUILD_DEBUG=0 ${cc_common}"
cc_link=""

# --- External Libraries ------------------------------------------------------
# TODO

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
# TODO

# --- Build Everything (@build_targets) ---------------------------------------
cd build
if [[ "${sample_macos:-0}" == "1" ]]; then
  didbuild=1 && $compile ../src/sample_macos/sample_macos_main.c $cc_link -o sample_macos
fi
cd ..

# --- Warn On No Builds -------------------------------------------------------
if [[ "${didbuild:-0}" == "0" ]]; then
  echo "[WARNING] no valid build target specified; must use build target names as arguments to this script, like \`./build_mac.sh <target>\`."
  exit 1
fi
