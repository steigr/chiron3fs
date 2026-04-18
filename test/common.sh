#!/bin/sh

chiron3fs_pid=0

clean_up() {
  if [ "$chiron3fs_pid" -ne 0 ]; then
    kill "$chiron3fs_pid" 2>/dev/null || true
  fi
  if command -v fusermount3 >/dev/null 2>&1; then
    fusermount3 -u t3 2>/dev/null || true
  fi
  sleep 1
  rm -rf t1 t2 t3
  [ -f test.log ] &&  rm test.log
}

clean_and_exit() {
  clean_up
  exit $1
}

create_test_directories() {
  mkdir t1 t2 t3
}

start_chiron() {
  ../src/chiron3fs -f --log ./test.log t1=t2 t3 &
  chiron3fs_pid=$!
  echo "Chiron3FS pid is $chiron3fs_pid"
  sleep 1
}

