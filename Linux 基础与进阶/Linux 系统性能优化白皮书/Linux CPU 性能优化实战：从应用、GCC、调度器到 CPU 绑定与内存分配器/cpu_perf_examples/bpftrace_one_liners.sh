#!/usr/bin/env bash
set -euo pipefail
if ! command -v bpftrace >/dev/null; then
  echo "bpftrace not found. Debian/Ubuntu: sudo apt install bpftrace; examples below."
  cat <<'EOF'
# CPU run queue latency histogram (needs BTF/tracepoints):
bpftrace -e 'tracepoint:sched:sched_wakeup { @start[args->pid] = nsecs; } tracepoint:sched:sched_switch { if (@start[args->prev_pid]) { @usecs = hist((nsecs - @start[args->prev_pid]) / 1000); delete(@start[args->prev_pid]); } }'
# syscall counts by process:
bpftrace -e 'tracepoint:raw_syscalls:sys_enter { @[comm, args->id] = count(); } interval:s:5 { exit(); }'
# context switches per second:
bpftrace -e 'tracepoint:sched:sched_switch { @cs = count(); } interval:s:1 { printf("cs/s: %d\n", @cs); clear(@cs); }'
EOF
  exit 0
fi
echo "bpftrace found; run a command and attach one-liners from the fallback text above."
