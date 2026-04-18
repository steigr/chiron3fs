#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DOCKER_PLATFORM="${DOCKER_PLATFORM:-}"
ARCH="${ARCH:-}"
RPM_TARGET="${RPM_TARGET:-}"

docker_args=(run --rm)
if [[ -n "$DOCKER_PLATFORM" ]]; then
  docker_args+=(--platform "$DOCKER_PLATFORM")
fi

# Fedora build container for reproducible RPM generation from macOS.
docker "${docker_args[@]}" \
  -v "$ROOT_DIR:/work" \
  -w /work \
  fedora:40 \
  bash -lc '
    set -euo pipefail
    dnf -y install \
      autoconf \
      automake \
      gcc \
      make \
      rpm-build \
      redhat-rpm-config \
      fuse3-devel \
      pkgconf-pkg-config

    if [ "'$RPM_TARGET'" = "i686" ]; then
      dnf -y install \
        glibc-devel.i686 \
        libgcc.i686 \
        fuse3-devel.i686
    fi

    ARCH="'$ARCH'" RPM_TARGET="'$RPM_TARGET'" ./scripts/build-rpm.sh
  '

