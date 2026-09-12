#!/usr/bin/env bash
set -euo pipefail
if [[ ${EUID:-$(id -u)} -ne 0 ]]; then
  echo "run as root: this script creates a cgroup v2 demo"; exit 1
fi
if [[ ! -f /sys/fs/cgroup/cgroup.controllers ]]; then
  echo "cgroup v2 not mounted at /sys/fs/cgroup"; exit 1
fi
make -s app/cpu_burner
CG=/sys/fs/cgroup/cpuopt_demo
# enable controllers in parent if permitted
echo +cpu +cpuset > /sys/fs/cgroup/cgroup.subtree_control 2>/dev/null || true
mkdir -p "$CG"
cpus=$(cat /sys/fs/cgroup/cpuset.cpus.effective 2>/dev/null || echo "0-$(( $(nproc)-1 ))")
mems=$(cat /sys/fs/cgroup/cpuset.mems.effective 2>/dev/null || echo 0)
echo "$cpus" > "$CG/cpuset.cpus"
echo "$mems" > "$CG/cpuset.mems"
echo 50000 100000 > "$CG/cpu.max"   # 0.5 CPU quota for 100ms period
./app/cpu_burner 5 & pid=$!
echo "$pid" > "$CG/cgroup.procs"
echo "moved pid=$pid to $CG; cpu.max=$(cat $CG/cpu.max); cpuset.cpus=$(cat $CG/cpuset.cpus)"
wait "$pid" || true
echo "== cpu.stat"
cat "$CG/cpu.stat"
rmdir "$CG" 2>/dev/null || true
