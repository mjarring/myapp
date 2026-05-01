#!/bin/bash

# Generate aarch64 compile_commands.json for clangd

source compile-common.sh

# Make sure build dir exists
mkdir -p "build-clangd"

pushd build-clangd || exit 1

generate_wayland_source

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

popd || exit 1

echo "Done."
