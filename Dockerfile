FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
  gcc-x86-64-linux-gnu \
  g++-x86-64-linux-gnu \
  gcc-aarch64-linux-gnu \
  g++-aarch64-linux-gnu \
  libwayland-bin \
  wayland-protocols \
  && dpkg --add-architecture arm64 \
  && dpkg --add-architecture amd64 \
  && apt-get update \
  && apt-get install -y --no-install-recommends \
  libwayland-dev:amd64 \
  libxkbcommon-dev:amd64 \
  libwayland-dev:arm64 \
  libxkbcommon-dev:arm64 \
  python3 \
  && rm -rf /var/lib/apt/lists/*

COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/
