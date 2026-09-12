#!/usr/bin/env bash
set -euo pipefail
SRC=app/matrix_mult.c
BIN_DIR=build/bin
mkdir -p "$BIN_DIR"

build() {
  local name=$1; shift
  echo "== build $name: $*"
  gcc "$@" -DN=384 "$SRC" -o "$BIN_DIR/$name"
}

build mm_O2 -O2
build mm_O3 -O3
build mm_O3_native -O3 -march=native
build mm_O3_native_lto -O3 -march=native -flto

if gcc -O2 -fprofile-generate="$BIN_DIR/pgo" "$SRC" -o "$BIN_DIR/pgo_gen" 2>/dev/null; then
  echo "== run PGO training"
  "$BIN_DIR/pgo_gen" >/dev/null
  gcc -O3 -march=native -fprofile-use="$BIN_DIR/pgo" "$SRC" -o "$BIN_DIR/mm_pgo"
else
  echo "PGO not supported by this gcc/binutils, skip."
fi

echo "== run: record wall time, cpu time, context switches"
for b in "$BIN_DIR"/mm_*; do
  echo "--- $b"
  /usr/bin/time -v "$b" 2>&1 | egrep 'Elapsed|User time|System time|Voluntary|Involuntary|Maximum resident|Percent of CPU' || /usr/bin/time "$b"
done
