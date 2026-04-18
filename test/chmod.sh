#!/bin/sh

. ./common.sh

create_test_directories
start_chiron

touch t3/hello
chmod 755 t3/hello
ls -alh t3
echo "1234" >> t3/hello
chmod 444 t3/hello
ls -alh t3

# Validate mode propagation first (facade + both replicas).
if [ "$(stat -c %a t3/hello)" != "444" ] || [ "$(stat -c %a t1/hello)" != "444" ] || [ "$(stat -c %a t2/hello)" != "444" ]; then
  echo "chmod mode propagation failed"
  clean_and_exit -1
fi

# When running tests as root, permission checks are bypassed. Use an unprivileged account.
if [ "$(id -u)" -eq 0 ]; then
  if command -v runuser >/dev/null 2>&1; then
    runuser -u nobody -- sh -c 'echo "5678" >> t3/hello' 2>/dev/null || true
  else
    su -s /bin/sh nobody -c 'echo "5678" >> t3/hello' 2>/dev/null || true
  fi
else
  echo "5678" >> t3/hello 2>/dev/null || true
fi

if [ "$(cat t3/hello)" != "1234" ]; then
  echo "chmod failed"
  clean_and_exit -1
else
  echo "chmod ok"
fi

chmod 777 t3/hello
rm t3/hello
clean_and_exit 0
