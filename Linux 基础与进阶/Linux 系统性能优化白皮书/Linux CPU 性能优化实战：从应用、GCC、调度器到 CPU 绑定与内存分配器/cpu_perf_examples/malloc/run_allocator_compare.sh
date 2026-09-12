#!/usr/bin/env bash
set -euo pipefail
make -s app/malloc_bench
run() { echo "--- $1"; shift; "$@"; }
run "ptmalloc/glibc default" ./app/malloc_bench
for name in jemalloc tcmalloc; do
  case $name in
    jemalloc) lib=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 ;;
    tcmalloc) lib=/usr/lib/x86_64-linux-gnu/libtcmalloc.so.4 ;;
  esac
  if [[ -r "$lib" ]]; then
    run "$name via LD_PRELOAD=$lib" env "LD_PRELOAD=$lib" ./app/malloc_bench
  else
    echo "--- $name not installed; apt example: sudo apt install lib${name}-dev || sudo apt install google-perftools"
  fi
done
echo
echo "jemalloc config example:"
echo 'MALLOC_CONF="background_thread:true,dirty_decay_ms:1000,muzzy_decay_ms:1000" LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 ./app/malloc_bench'
echo 'stats: MALLOC_CONF="stats_print:true" LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 ./app/malloc_bench'
