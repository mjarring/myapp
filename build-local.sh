#!/bin/bash

# Builds the application and generates compile_commands.json for clangd.
# Detects the package manager and install missing dependencies.

# Dependency installation
DNF_PACKAGES=(wayland-devel wayland-protocols-devel libxkbcommon-devel pkgconf)
APT_PACKAGES=(libwayland-dev wayland-protocols libxkbcommon-dev pkg-config)
PACMAN_PACKAGES=(wayland wayland-protocols libxkbcommon pkgconf)

install_missing_dnf() {
  local missing=()
  for pkg in "${DNF_PACKAGES[@]}"; do
    rpm -q "$pkg" &>/dev/null || missing+=("$pkg")
  done
  if [ ${#missing[@]} -gt 0 ]; then
    echo "Installing missing packages via dnf: ${missing[*]}"
    sudo dnf install -y "${missing[@]}"
  fi
}

install_missing_apt() {
  local missing=()
  for pkg in "${APT_PACKAGES[@]}"; do
    dpkg -s "$pkg" &>/dev/null || missing+=("$pkg")
  done
  if [ ${#missing[@]} -gt 0 ]; then
    echo "Installing missing packages via apt: ${missing[*]}"
    sudo apt install -y "${missing[@]}"
  fi
}

install_missing_pacman() {
  local missing=()
  for pkg in "${PACMAN_PACKAGES[@]}"; do
    pacman -Q "$pkg" &>/dev/null || missing+=("$pkg")
  done
  if [ ${#missing[@]} -gt 0 ]; then
    echo "Installing missing packages via pacman: ${missing[*]}"
    sudo pacman -S --noconfirm "${missing[@]}"
  fi
}

if command -v dnf &>/dev/null; then
  install_missing_dnf
elif command -v apt &>/dev/null; then
  install_missing_apt
elif command -v pacman &>/dev/null; then
  install_missing_pacman
else
  echo "WARNING: No supported package manager found (dnf/apt/pacman). Install dependencies manually."
fi

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Change to that directory
cd "$SCRIPT_DIR" || exit 1

# Make sure build dir exists
mkdir -p "build"

pushd build || exit 1

# Code generation
XDG_SHELL_XML=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml

if [ ! -f "$XDG_SHELL_XML" ]; then
  echo "ERROR: $XDG_SHELL_XML not found even after install. Check your wayland-protocols package."
  exit 1
fi

# Generate wayland protocol code
GEN_C=xdg-shell-protocol.c
wayland-scanner client-header "$XDG_SHELL_XML" xdg-shell-client-protocol.h
wayland-scanner private-code "$XDG_SHELL_XML" xdg-shell-protocol.c

# Build the application
SRC=../source/myapp.cpp

# Flags
CFLAGS=(-I.)
LDFLAGS=(-lwayland-client -lxkbcommon -lrt)

# Generate compile_commands.json for clangd
cat >compile_commands.json <<EOF
[
  {
    "directory": "$(pwd)",
    "command": "gcc ${CFLAGS[*]} ${LDFLAGS[*]} -o myapp $SRC $GEN_C",
    "file": "$SRC"
  },
  {
    "directory": "$(pwd)",
    "command": "gcc -I. -c $GEN_C -o xdg-shell-protocol.o",
    "file": "$GEN_C"
  }
]
EOF

gcc "${CFLAGS[@]}" -o myapp "$SRC" "$GEN_C" "${LDFLAGS[@]}"

popd || exit 1
