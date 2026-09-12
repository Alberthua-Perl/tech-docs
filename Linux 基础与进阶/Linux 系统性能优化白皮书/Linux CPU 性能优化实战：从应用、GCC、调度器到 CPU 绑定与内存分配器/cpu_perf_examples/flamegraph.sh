#!/usr/bin/env bash
set -euo pipefail
if ! command -v perf >/dev/null; then echo "perf not found"; exit 0; fi
make -s app/malloc_bench
perf record -F 99 -g --call-graph dwarf -o perf.data ./app/malloc_bench
perf script > perf.script
if [[ -d FlameGraph ]]; then
  ./FlameGraph/stackcollapse-perf.pl perf.script | ./FlameGraph/flamegraph.pl > flamegraph.svg
  echo "wrote flamegraph.svg"
else
  echo "wrote perf.script. Install FlameGraph to render: https://github.com/brendangregg/FlameGraph"
fi
