#!/bin/bash

# Generate compile_commands.json for clangd

source build-common.sh

# Make sure build dir exists
mkdir -p "build"

pushd build || exit 1

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
