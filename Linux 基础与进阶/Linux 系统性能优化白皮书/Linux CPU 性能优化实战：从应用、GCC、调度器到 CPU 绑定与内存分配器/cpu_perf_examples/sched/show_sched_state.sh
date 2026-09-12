#!/usr/bin/env bash
set -euo pipefail
echo "== current shell"
echo "pid=$$"; taskset -pc $$ || true; chrt -p $$ || true
echo
echo "== selected sysctls"
sysctl kernel.numa_balancing sched_min_granularity_ns sched_wakeup_granularity_ns 2>/dev/null || true
echo
echo "== quick scheduler counters while running: app/atomic_bench g"
./app/atomic_bench g & pid=$!
if command -v pidstat >/dev/null; then
  pidstat -p "$pid" -u -w 1 3 || true
else
  echo "pidstat not found; install sysstat or use vmstat 1 / cat /proc/$pid/status"
fi
wait "$pid"
