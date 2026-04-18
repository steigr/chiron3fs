#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DOCKER_PLATFORM="${DOCKER_PLATFORM:-}"
ARCH="${ARCH:-}"
DEB_IMAGE="${DEB_IMAGE:-ubuntu:26.04}"
DEB_FLAVOR="${DEB_FLAVOR:-}"

docker_args=(run --rm)
if [[ -n "$DOCKER_PLATFORM" ]]; then
  docker_args+=(--platform "$DOCKER_PLATFORM")
fi

# Apt-based build container for reproducible DEB generation on Ubuntu or Debian.
docker "${docker_args[@]}" \
  -v "$ROOT_DIR:/work" \
  -w /work \
  "$DEB_IMAGE" \
  bash -lc '
    set -euo pipefail
    export DEBIAN_FRONTEND=noninteractive
    apt-get update
    apt-get install -y --no-install-recommends \
      autoconf \
      automake \
      build-essential \
      debhelper \
      dpkg-dev \
      fakeroot \
      libfuse3-dev \
      pkg-config
    ARCH="'$ARCH'" DEB_FLAVOR="'$DEB_FLAVOR'" ./scripts/build-deb.sh
  '
