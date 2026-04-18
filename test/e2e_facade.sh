#!/bin/sh
set -e

. ./common.sh

fail() {
  echo "$1"
  clean_and_exit 1
}

assert_exists_on_replicas() {
  kind="$1"
  rel="$2"

  case "$kind" in
    -d)
      [ -d "t1/$rel" ] && [ -d "t2/$rel" ] || fail "Replica dir mismatch for $rel"
      ;;
    -f)
      [ -f "t1/$rel" ] && [ -f "t2/$rel" ] || fail "Replica file mismatch for $rel"
      ;;
    -p)
      [ -p "t1/$rel" ] && [ -p "t2/$rel" ] || fail "Replica fifo mismatch for $rel"
      ;;
    *)
      fail "Unsupported assert kind: $kind"
      ;;
  esac
}

assert_content_on_replicas() {
  rel="$1"
  expected="$2"

  if [ "$(cat "t1/$rel")" != "$expected" ]; then
    fail "Replica 1 content mismatch for $rel"
  fi
  if [ "$(cat "t2/$rel")" != "$expected" ]; then
    fail "Replica 2 content mismatch for $rel"
  fi
}

create_test_directories
start_chiron

mkdir t3/e2e
mkdir t3/e2e/subdir
assert_exists_on_replicas -d e2e/subdir

printf "abcdef" > t3/e2e/file
if [ "$(cat t3/e2e/file)" != "abcdef" ]; then
  fail "Facade read/write failed"
fi
assert_exists_on_replicas -f e2e/file
assert_content_on_replicas e2e/file abcdef

# Exercise readdir/access paths via common user operations.
ls -la t3/e2e >/dev/null
if [ ! -r t3/e2e/file ] || [ ! -w t3/e2e/file ]; then
  fail "Facade access checks failed"
fi

chmod 640 t3/e2e/file
if [ "$(stat -c %a t3/e2e/file)" != "640" ]; then
  fail "Facade chmod failed"
fi
if [ "$(stat -c %a t1/e2e/file)" != "640" ] || [ "$(stat -c %a t2/e2e/file)" != "640" ]; then
  fail "Replica chmod mismatch"
fi

touch -t 202401010101 t3/e2e/file
facade_mtime="$(stat -c %Y t3/e2e/file)"
if [ "$(stat -c %Y t1/e2e/file)" != "$facade_mtime" ] || [ "$(stat -c %Y t2/e2e/file)" != "$facade_mtime" ]; then
  fail "Replica mtime mismatch after touch"
fi

truncate -s 3 t3/e2e/file
if [ "$(cat t3/e2e/file)" != "abc" ]; then
  fail "Facade truncate failed"
fi
assert_content_on_replicas e2e/file abc

ln t3/e2e/file t3/e2e/file.hard
assert_exists_on_replicas -f e2e/file.hard

ln -s file.hard t3/e2e/file.sym
if [ "$(readlink t3/e2e/file.sym)" != "file.hard" ]; then
  fail "Facade symlink failed"
fi
if [ "$(readlink t1/e2e/file.sym)" != "file.hard" ] || [ "$(readlink t2/e2e/file.sym)" != "file.hard" ]; then
  fail "Replica symlink mismatch"
fi

mv t3/e2e/file t3/e2e/file.renamed
assert_exists_on_replicas -f e2e/file.renamed
if [ -e t1/e2e/file ] || [ -e t2/e2e/file ]; then
  fail "Replica rename cleanup failed"
fi

mkfifo t3/e2e/fifo
assert_exists_on_replicas -p e2e/fifo

rm t3/e2e/file.sym
rm t3/e2e/file.hard
rm t3/e2e/file.renamed
rm t3/e2e/fifo
if [ -e t1/e2e/file.renamed ] || [ -e t2/e2e/file.renamed ]; then
  fail "Replica unlink failed"
fi

rmdir t3/e2e/subdir
rmdir t3/e2e
if [ -e t1/e2e ] || [ -e t2/e2e ]; then
  fail "Replica rmdir failed"
fi

clean_and_exit 0


