#!/usr/bin/env bash
set -euo pipefail
make -s app/false_sharing app/atomic_bench app/malloc_bench
CPUS="0-$(( $(nproc) > 4 ? 3 : $(nproc)-1 ))"
echo "== baseline no explicit taskset"
./app/false_sharing p
./app/atomic_bench l
echo
echo "== taskset -c $CPUS (same process set; compare with perf/pidstat for migrations)"
taskset -c "$CPUS" ./app/false_sharing p
taskset -c "$CPUS" ./app/atomic_bench l
echo
echo "If you have perf: perf stat -e context-switches,cpu-migrations,cache-misses,cycles,instructions <cmd>"
