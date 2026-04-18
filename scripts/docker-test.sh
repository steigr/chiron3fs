#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE_NAME="chiron3fs-dev"

docker build -t "$IMAGE_NAME" "$PROJECT_ROOT"
docker run --rm \
  --privileged \
  --device /dev/fuse \
  --cap-add SYS_ADMIN \
  --security-opt apparmor:unconfined \
  -v "$PROJECT_ROOT:/work" \
  -w /work \
  "$IMAGE_NAME"

