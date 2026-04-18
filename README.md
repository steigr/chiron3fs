# chiron3fs

`chiron3fs` is a FUSE3 port of the original `chironfs` replicated filesystem.

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

