#!/bin/bash

# Generate aarch64 compile_commands.json for clangd

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MYAPP_SRC_DIR=$SCRIPT_DIR/source

source compile-common.sh

# Make sure build dir exists
mkdir -p "build-clangd"

pushd build-clangd || exit 1

generate_wayland_source

cat >compile_commands.json <<EOF
[
  {
    "directory": "$(pwd)",
    "command": "gcc ${MYAPP_CFLAGS[*]} ${MYAPP_LDFLAGS[*]} -o myapp $MYAPP_SRC $MYAPP_GEN_C",
    "file": "$MYAPP_SRC"
  },
  {
    "directory": "$(pwd)",
    "command": "gcc -I. -c $MYAPP_GEN_C -o xdg-shell-protocol.o",
    "file": "$MYAPP_GEN_C"
  }
]
EOF

popd || exit 1

echo "Done."
