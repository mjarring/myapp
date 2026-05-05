FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
  gcc \
  g++ \
  gcc-aarch64-linux-gnu \
  g++-aarch64-linux-gnu \
  libwayland-bin \
  wayland-protocols \
  && dpkg --add-architecture arm64 \
  && apt-get update \
  && apt-get install -y --no-install-recommends \
  libwayland-dev \
  libxkbcommon-dev \
  libwayland-dev:arm64 \
  libxkbcommon-dev:arm64 \
  python3 \
  && rm -rf /var/lib/apt/lists/*

COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/
