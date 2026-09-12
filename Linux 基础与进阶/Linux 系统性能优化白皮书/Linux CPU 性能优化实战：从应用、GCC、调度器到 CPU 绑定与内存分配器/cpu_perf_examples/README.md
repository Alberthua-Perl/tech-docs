# Linux CPU 性能优化：可运行示例

配套文档：`/mnt/agents/output/linux_cpu_performance_optimization_optimized.md`

## 0) 快速开始

```bash
cd /mnt/agents/output/cpu_perf_examples
make
./app/false_sharing b        # 同 cache line 伪共享
./app/false_sharing p        # 按 64B 对齐拆分，通常更快更稳
./app/atomic_bench g         # 全局 atomic 高竞争
./app/atomic_bench l         # 线程本地累加后一次性合并，通常更快
./app/syscall_batch          # 16MB 每字节一次 write：演示 syscall 放大
./app/syscall_batch b        # 8KB 批量 write
./app/zero_copy_sendfile     # read/write vs sendfile 对比
./build/gcc_bench_matrix.sh  # -O2/-O3/-march=native/LTO/PGO 对比
./sched/run_affinity_compare.sh
sudo ./cgroup/cpuset_cpu_demo.sh
./malloc/run_allocator_compare.sh
./profiling/perf_stat_record.sh
```

建议每次对比都固定：CPU 型号、 governor、SMT 开关、NUMA 拓扑、线程数、数据规模、warmup、运行次数；至少记录吞吐、P99/P999、CPU%、IPC、cache-miss、context-switch、cpu-migration、RSS。

## 1) Application 示例

| 文件 | 演示点 | 运行 |
| --- | --- | --- |
| `app/false_sharing.c` | False Sharing 与 `aligned(64)` 修复 | `./app/false_sharing b` vs `./app/false_sharing p` |
| `app/atomic_bench.c` | 全局 atomic 竞争 vs local accumulate | `./app/atomic_bench g` vs `./app/atomic_bench l` |
| `app/syscall_batch.c` | syscall 放大与 batching | `./app/syscall_batch` vs `./app/syscall_batch b` |
| `app/zero_copy_sendfile.c` | read/write vs `sendfile` | `./app/zero_copy_sendfile` |
| `app/matrix_mult.c` | Cache-friendly loop order；配合 GCC 选项压测 | `make app/matrix_mult && ./app/matrix_mult` |

观测建议：

```bash
perf stat -e cycles,instructions,cache-misses,cache-references,context-switches,cpu-migrations ./app/false_sharing b
perf stat -e cycles,instructions,cache-misses,cache-references,context-switches,cpu-migrations ./app/false_sharing p
perf record -F 99 -g --call-graph dwarf -o fs-b.data ./app/false_sharing b
perf record -F 99 -g --call-graph dwarf -o fs-p.data ./app/false_sharing p
```

## 2) GCC 编译器优化示例

```bash
./build/gcc_bench_matrix.sh
```

脚本会构建并计时：

- `mm_O2`: `-O2`
- `mm_O3`: `-O3`
- `mm_O3_native`: `-O3 -march=native`
- `mm_O3_native_lto`: `-O3 -march=native -flto`
- `mm_pgo`: `-fprofile-generate` 训练后 `-fprofile-use`（若工具链支持）

注意：`-march=native` 生成的二进制可能不适合跨代 CPU 发布；PGO 训练集要贴近真实负载，避免过拟合错误分支。

## 3) Linux Scheduler 示例

```bash
./sched/show_sched_state.sh
./sched/run_affinity_compare.sh
./sched/chrt_realtime_demo.sh
```

关键观察项：

- `pidstat -w 1` 看 `cswch/s`、`nvcswch/s`
- `perf stat -e context-switches,cpu-migrations,cache-misses,cycles,instructions <cmd>`
- `kernel.numa_balancing` 仅在已绑核/拓扑明确/延迟敏感场景评估关闭；不建议全局默认关闭。
- `SCHED_FIFO/RR` 需要 `CAP_SYS_NICE`，可能造成饿死；普通服务保持 `SCHED_OTHER/CFS`。

## 4) CPU Binding vs CPU Set 示例

线程级绑定：

```bash
taskset -c 0-3 ./app/atomic_bench l
taskset -pc <pid>
```

cgroup v2 cpuset/cpu 隔离：

```bash
sudo ./cgroup/cpuset_cpu_demo.sh
```

区别：`taskset/sched_setaffinity` 优化线程可运行集合与局部性；`cpuset.cpus/cpuset.mems` 做 cgroup 硬边界并限定内存节点。生产建议：cpuset 划资源域，域内再做细粒度 affinity；配合 `cpu.max` 做配额，配合 `cpuset.mems` 控制 NUMA 内存节点。

## 5) Memory Allocator 示例

```bash
./malloc/run_allocator_compare.sh
```

手动运行 jemalloc：

```bash
LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 ./app/malloc_bench
MALLOC_CONF="background_thread:true,dirty_decay_ms:1000,muzzy_decay_ms:1000" \
  LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 ./app/malloc_bench
MALLOC_CONF="stats_print:true" LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 ./app/malloc_bench
```

解释指标：吞吐、CPU%、RSS、page fault、分配速率、碎片、P99 延迟。不要只看运行时间；allocator 替换可能降 CPU 但升 RSS，或相反。

## Profiling 入口

```bash
./profiling/perf_stat_record.sh
./profiling/bpftrace_one_liners.sh
./profiling/flamegraph.sh
```

若环境缺少 `perf/bpftrace`，脚本会打印安装提示和等价 one-liner。火焰图需要 Brendangregg/FlameGraph。

## 安全与回滚

- 修改 `kernel.numa_balancing`、调度器参数、`SCHED_FIFO/RR`、cpuset、allocator 都属于实验性变更：先灰度、设回滚、保存 `/proc/cmdline`、`sysctl -a`、cgroup 配置与基准数据。
- 实时优先级与 cpuset 误用可能造成服务饿死、跨 NUMA 访问或内存节点不可达；先在隔离环境验证。
