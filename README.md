# chiron3fs

`chiron3fs` is a FUSE3 port of the original replicated filesystem project.

Maintainer: `steigr` (`chiron3fs@stei.gr`)

Project repository: `https://github.com/steigr/chiron3fs`

It mirrors writes to multiple replica directories and reads from healthy replicas,
with optional low-priority replicas prefixed by `:` in the replica list.

## Build (Linux)

```bash
autoreconf -fi
./configure
make -j"$(nproc)"
```

Binaries are generated in `src/`:
- `src/chiron3fs`
- `src/chiron3ctl`

## Run

```bash
mkdir -p /tmp/chiron/r1 /tmp/chiron/r2 /tmp/chiron/mnt
./src/chiron3fs -f /tmp/chiron/r1=/tmp/chiron/r2 /tmp/chiron/mnt
```

In another shell:

```bash
echo "hello" > /tmp/chiron/mnt/hello.txt
cat /tmp/chiron/r1/hello.txt
cat /tmp/chiron/r2/hello.txt
```

Unmount:

```bash
fusermount3 -u /tmp/chiron/mnt
```

## /etc/fstab Integration

Example `fstab` entry:

```bash
chiron3fs#/srv/chiron/r1=/srv/chiron/r2 /srv/chiron/mnt fuse defaults,allow_other,log=/var/log/chiron3fs.log 0 0
```

Mount it with:

```bash
sudo mount /srv/chiron/mnt
```

## systemd Integration

Create `/etc/systemd/system/chiron3fs.service`:

```ini
[Unit]
Description=chiron3fs replicated facade mount
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/chiron3fs -f /srv/chiron/r1=/srv/chiron/r2 /srv/chiron/mnt
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

Enable and start it:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now chiron3fs.service
```

## Linux Testing With Docker (from macOS)

Use the provided helper script (requires Docker runtime with `/dev/fuse` support):

```bash
./scripts/docker-test.sh
```

That command builds the project image, then runs:
- `autoreconf -fi`
- `./configure`
- `make`
- `make check`

If your Docker runtime does not expose `/dev/fuse`, run tests in a Linux VM or CI runner with FUSE enabled.

## Build RPM (Fedora)

Native host build:

```bash
./scripts/build-rpm.sh
```

Docker build from macOS:

```bash
./scripts/docker-build-rpm.sh
```

Artifacts are written to `dist/packages/rpm/`.

## Build DEB (Ubuntu/Debian)

Native host build:

```bash
./scripts/build-deb.sh
```

Docker build from macOS:

```bash
./scripts/docker-build-deb.sh

# Debian 12 builder
DEB_FLAVOR=debian DEB_IMAGE=debian:12 ./scripts/docker-build-deb.sh
```

Artifacts are written to `dist/packages/deb/`.

