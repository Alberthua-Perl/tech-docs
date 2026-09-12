#!/usr/bin/env bash
set -euo pipefail
if ! command -v perf >/dev/null; then
  echo "perf not found. Debian/Ubuntu: sudo apt install linux-tools-generic linux-tools-$(uname -r); or use your distro perf package."
  exit 0
fi
make -s app/false_sharing app/atomic_bench app/malloc_bench
echo "== perf stat"
perf stat -e cycles,instructions,branch-misses,cache-misses,cache-references,context-switches,cpu-migrations ./app/atomic_bench g || true
echo "== perf record/report (flame graph input)"
perf record -F 99 -g --call-graph dwarf -o perf.data ./app/malloc_bench
perf report --stdio --no-children 2>/dev/null | head -80 || perf report --stdio | head -80
echo "FlameGraph: git clone https://github.com/brendangregg/FlameGraph; perf script | FlameGraph/stackcollapse-perf.pl | FlameGraph/flamegraph.pl > out.svg"
