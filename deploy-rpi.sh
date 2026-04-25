#!/bin/bash

# deploy-rpi.sh — Copy the latest myapp binary to a running Raspberry Pi
# over SSH.
#
# Usage:
#   ./deploy-rpi.sh [--host <ip-or-hostname>]
#
# The target host defaults to the RPI_HOST environment variable, which itself
# defaults to 192.168.50.11.

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-rpi"
SSH_USER="root"
REMOTE_BIN="/usr/bin/myapp"

TARGET_HOST="${RPI_HOST:-192.168.50.11}"

while [[ $# -gt 0 ]]; do
  case "$1" in
  --host)
    [[ -n "${2:-}" ]] || {
      echo "ERROR: --host requires an argument" >&2
      exit 1
    }
    TARGET_HOST="$2"
    shift 2
    ;;
  -h | --help)
    sed -n '3,10p' "${BASH_SOURCE[0]}" | sed 's/^# \?//'
    exit 0
    ;;
  *)
    echo "ERROR: Unknown argument: $1" >&2
    echo "Usage: $0 [--host <ip-or-hostname>]" >&2
    exit 1
    ;;
  esac
done

ARTIFACT="${BUILD_DIR}/myapp"
# check if artifact exists and is executable
if [[ ! -x "${ARTIFACT}" ]]; then
  echo "ERROR: No executable myapp found at ${ARTIFACT}" >&2
  echo "       Run 'build.sh' to build the app first." >&2
  exit 1
fi

SSH_OPTS=(-o ConnectTimeout=10)
if ! ssh "${SSH_OPTS[@]}" "${SSH_USER}@${TARGET_HOST}" true 2>/dev/null; then
  echo "ERROR: Cannot reach ${SSH_USER}@${TARGET_HOST} over SSH." >&2
  exit 1
fi

echo "Copying myapp to ${SSH_USER}@${TARGET_HOST}:${REMOTE_BIN} ..."
scp "${SSH_OPTS[@]}" "${ARTIFACT}" "${SSH_USER}@${TARGET_HOST}:${REMOTE_BIN}"

echo "Done. myapp deployed on ${TARGET_HOST}."
