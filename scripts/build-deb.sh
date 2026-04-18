#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PKG_NAME="chiron3fs"
VERSION="$(sed -n 's/^AC_INIT(\[[^]]*\], \[\([^]]*\)\].*/\1/p' "$ROOT_DIR/configure.ac")"
OUTPUT_DIR="$ROOT_DIR/dist/packages/deb"
DEBIAN_TEMPLATE_DIR="$ROOT_DIR/packaging/debian/debian"
ARCH="${ARCH:-}"
DEB_FLAVOR="${DEB_FLAVOR:-}"

if [[ -z "$VERSION" ]]; then
  echo "Unable to determine version from configure.ac" >&2
  exit 1
fi

if ! command -v dpkg-buildpackage >/dev/null 2>&1; then
  echo "dpkg-buildpackage is required to build Debian packages" >&2
  exit 1
fi

mkdir -p "$OUTPUT_DIR"

(
  cd "$ROOT_DIR"
  autoreconf -fi
  ./configure
  make dist
)

UPSTREAM_TARBALL="$ROOT_DIR/${PKG_NAME}-${VERSION}.tar.gz"
if [[ ! -f "$UPSTREAM_TARBALL" ]]; then
  echo "Expected source tarball not found: $UPSTREAM_TARBALL" >&2
  exit 1
fi

WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

cp "$UPSTREAM_TARBALL" "$WORK_DIR/${PKG_NAME}_${VERSION}.orig.tar.gz"
(
  cd "$WORK_DIR"
  tar -xzf "${PKG_NAME}_${VERSION}.orig.tar.gz"
  cp -a "$DEBIAN_TEMPLATE_DIR" "${PKG_NAME}-${VERSION}/debian"
  chmod +x "${PKG_NAME}-${VERSION}/debian/rules"
  cd "${PKG_NAME}-${VERSION}"
  dpkg-buildpackage -us -uc -b
)


if [[ -z "$ARCH" ]]; then
  ARCH="$(dpkg --print-architecture)"
fi
if [[ -n "$DEB_FLAVOR" ]]; then
  ARCH_OUTPUT_DIR="$OUTPUT_DIR/$DEB_FLAVOR/$ARCH"
else
  ARCH_OUTPUT_DIR="$OUTPUT_DIR/$ARCH"
fi
mkdir -p "$ARCH_OUTPUT_DIR"
find "$WORK_DIR" -maxdepth 1 -type f \( -name '*.deb' -o -name '*.changes' -o -name '*.buildinfo' -o -name '*.ddeb' \) -exec cp {} "$ARCH_OUTPUT_DIR/" \;
echo "DEB artifacts written to: $ARCH_OUTPUT_DIR"

