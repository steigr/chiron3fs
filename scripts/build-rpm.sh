#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PKG_NAME="chiron3fs"
VERSION="$(sed -n 's/^AC_INIT(\[[^]]*\], \[\([^]]*\)\].*/\1/p' "$ROOT_DIR/configure.ac")"
OUTPUT_DIR="$ROOT_DIR/dist/packages/rpm"
SPEC_FILE="$ROOT_DIR/packaging/rpm/${PKG_NAME}.spec"
ARCH="${ARCH:-}"
RPM_TARGET="${RPM_TARGET:-}"

if [[ -z "$VERSION" ]]; then
  echo "Unable to determine version from configure.ac" >&2
  exit 1
fi

if ! command -v rpmbuild >/dev/null 2>&1; then
  echo "rpmbuild is required to build RPM packages" >&2
  exit 1
fi

mkdir -p "$OUTPUT_DIR"

(
  cd "$ROOT_DIR"
  autoreconf -fi
  ./configure
  make dist
)

TARBALL="$ROOT_DIR/${PKG_NAME}-${VERSION}.tar.gz"
if [[ ! -f "$TARBALL" ]]; then
  echo "Expected source tarball not found: $TARBALL" >&2
  exit 1
fi

RPM_TOPDIR="$(mktemp -d)"
trap 'rm -rf "$RPM_TOPDIR"' EXIT

mkdir -p "$RPM_TOPDIR"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
cp "$TARBALL" "$RPM_TOPDIR/SOURCES/"
cp "$SPEC_FILE" "$RPM_TOPDIR/SPECS/"

rpmbuild_cmd=(rpmbuild -ba --define "_topdir $RPM_TOPDIR")
if [[ -n "$RPM_TARGET" ]]; then
  rpmbuild_cmd+=(--target "$RPM_TARGET")
fi
rpmbuild_cmd+=("$RPM_TOPDIR/SPECS/${PKG_NAME}.spec")
"${rpmbuild_cmd[@]}"

if [[ -z "$ARCH" ]]; then
  ARCH="$(find "$RPM_TOPDIR/RPMS" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | head -n 1)"
fi
OUTPUT_DIR="$OUTPUT_DIR/$ARCH"
mkdir -p "$OUTPUT_DIR"

find "$RPM_TOPDIR/RPMS" "$RPM_TOPDIR/SRPMS" -type f \( -name '*.rpm' -o -name '*.src.rpm' \) -exec cp {} "$OUTPUT_DIR/" \;

echo "RPM artifacts written to: $OUTPUT_DIR"

