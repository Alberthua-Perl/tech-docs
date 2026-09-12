# Linux CPU 性能优化实战：从应用、GCC、调度器到 CPU 绑定与内存分配器

## 文档说明

CPU 很少只是“不够快”，更多时候是缓存失效、锁竞争、上下文切换、NUMA 远程访问和内存分配开销在吞噬有效算力。本文按收益从大到小梳理 Linux CPU 优化的五个关键层级：应用与算法、GCC 编译选项、CFS 调度与 NUMA 行为、CPU affinity/cpuset 隔离，以及 ptmalloc/tcmalloc/jemalloc 内存分配器选型与调优。

核心原则是 **Profile First, Optimize Later**：先用 `perf`、`bpftrace`、`Flame Graph` 等工具定位热点，再通过基准测试验证收益，避免盲目调参。

> 适用读者：后端/基础架构/性能工程/SRE；建议先具备 `top`、`pidstat`、`perf`、cgroup 与 NUMA 基本概念。

## 文档目录

- [1. Application 调优：收益最大的优化](#1-application-调优收益最大的优化)
- [2. GCC 编译器优化](#2-gcc-编译器优化)
- [3. Linux Scheduler 调优](#3-linux-scheduler-调优)
- [4. CPU Binding vs CPU Set](#4-cpu-binding-vs-cpu-set)
- [5. Memory Allocator 优化](#5-memory-allocator-优化)
- [总结与推荐检查清单](#总结与推荐检查清单)

## 1. Application 调优：收益最大的优化

无论硬件多么先进，算法和代码质量始终决定性能上限。应用层优化通常包括：

- 选择更优的数据结构和算法；例如将热点从 `O(n^2)` 降至 `O(n log n)` 的收益，往往超过任何 CPU 参数调整。
- 减少锁竞争（Lock Contention）；必要时使用读写锁、分片锁、无锁（Lock-free）结构或 RCU。
- 减少系统调用（System Call）；能用批量处理就别高频单次调用。
- 批量处理（Batch Processing）与异步化，降低陷入内核态和上下文切换的频率。
- Cache Friendly 数据布局：关注结构体字段对齐、伪共享（False Sharing）、访问局部性与内存连续性。
- 减少不必要的数据拷贝：优先评估 zero-copy 方案，如 `sendfile`、`splice`、`mmap`、环形队列与描述符传递。

执行原则：

- 先用 `perf record/report`、`perf top`、`bpftrace`、`flamegraph`、`gprof` 定位热点。
- 对改动做 A/B 基准测试，固定输入、版本、NUMA/绑核与采样条件。
- 记录指标：CPU 利用率、IPC、cache miss、上下文切换、运行队列、P99/P999 延迟、RSS 与分配速率。

## 2. GCC 编译器优化

现代 GCC 已能生成高质量机器代码，常见选项如下：

| 选项 | 作用 | 建议 |
| --- | --- | --- |
| `-O2` | 默认推荐，兼顾性能、体积与编译稳定性 | 生产构建常用起点 |
| `-O3` | 更强循环优化、自动向量化等 | 计算密集型可 benchmark 后启用 |
| `-march=native` | 使用本机指令集与调度模型 | 同机部署可用；跨机发布谨慎 |
| `-flto` | 链接期优化，利于跨模块内联 | 关注编译时间、调试性与符号 |
| `-fprofile-generate/-fprofile-use` | PGO，用真实负载训练分支/布局 | 训练集需代表线上流量 |

实践建议：不要默认堆选项。以 `-O2 -march=native` 为基线，再逐项验证 `-O3`、LTO、PGO 对吞吐、延迟、启动时间、二进制体积和可调试性的影响。

## 3. Linux Scheduler 调优

Linux 调度器决定线程运行在哪个 CPU、何时运行。现代 Linux 默认采用 Completely Fair Scheduler（CFS）；普通服务一般保持 `SCHED_OTHER`，用 `nice`/`renice` 只能微调时间片倾向，不能替代容量规划。

对普通应用，调度器已能较好完成 Load Balance、NUMA Balance、CPU Migration 与 Fair Scheduling。但在高性能场景中，频繁迁移可能带来：

- L1/L2/L3 Cache Miss 上升
- TLB Miss 与页表遍历增加
- NUMA Remote Access 与跨节点内存带宽压力
- Context Switch 与运行队列抖动

常见调优方向：

| 方向 | 示例 | 注意 |
| --- | --- | --- |
| NUMA balancing | `kernel.numa_balancing=0` | 仅适合已绑核、延迟敏感或拓扑明确场景；不建议全局默认关闭 |
| 调度粒度 | `sched_min_granularity_ns`、`sched_wakeup_granularity_ns` | 需按内核版本与负载实测，过小/过大都可能劣化 |
| 实时策略 | `SCHED_FIFO`、`SCHED_RR` | 需要相应权限，存在饿死风险；普通服务保持 `SCHED_OTHER` |
| 隔离与优先级 | cgroup cpu/cpuacct、cpuset、`nice` | 优先级不是资源保障；关键路径用 cpuset/cgroup 做硬边界 |

高吞吐系统通常优先降低 **Context Switch** 与 **CPU Migration**，同时用 `pidstat -w`、`vmstat`、`perf sched`、`runqlat/runqlen` 观察证据。

## 4. CPU Binding vs CPU Set

很多人容易混淆 CPU Affinity（绑定）和 CPU Set（cpuset）。

### CPU Binding（CPU Affinity）

- 针对单个进程或线程指定可运行 CPU，例如 `taskset -c 0-3 ./myapp` 或 `sched_setaffinity()`。
- 优点：减少 Cache Miss、降低 CPU Migration、提升 L1/L2 与 TLB 局部性、稳定延迟。
- 常用于：数据库、高性能网络、高频交易、延迟敏感服务。

### CPU Set（cpuset）

- cpuset 是 Linux cgroup 提供的资源隔离机制。
- 示例：`cpuset.cpus=0-7`、`cpuset.mems=0`。
- 它不仅限制 CPU，还限制 Memory Node/NUMA Node；常与 cpu、memory 控制器配合。
- 常用于：Kubernetes Guaranteed Pod、Container、NUMA 隔离、混部降噪。

结论：CPU Binding 更关注线程级调度优化与局部性；CPU Set 更关注系统级资源隔离与硬边界。二者可结合：用 cpuset 划出资源域，再在域内做精细 affinity。

## 5. Memory Allocator 优化

很多 CPU 时间实际消耗在 `malloc/free` 与内存管理上。多线程、小对象、高分配速率场景尤其值得评估。

| 分配器 | 来源/使用 | 特点 | 潜在缺点 | 典型场景 |
| --- | --- | --- | --- | --- |
| ptmalloc | glibc 默认 | 通用、稳定、兼容性好 | 多线程锁竞争与碎片相对更明显 | Linux 默认 |
| tcmalloc | Google | Thread Cache、小对象快、锁竞争低 | 需替换 glibc malloc 并回归验证 | RPC、Web 服务、部分高并发组件 |
| jemalloc | Facebook/FreeBSD 等广泛使用 | 碎片低、Arena 设计、NUMA 支持较好 | 配置复杂时需 profiling 验证 | 数据库与基础设施常用，如 Redis、ClickHouse、TiKV/TiDB 部分组件等 |

切换 allocator 通常不需要重编译业务代码，可用动态库预加载：

```bash
LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 ./myapp
```

调用链可理解为：

```text
app malloc()
  -> PLT/GOT 解析
  -> LD_PRELOAD 拦截
  -> jemalloc/tcmalloc malloc()
```

对于大量小对象分配，替换 allocator 常见收益为吞吐提升、CPU 降低与碎片减少，但必须用相同负载验证，并同时看 RSS、P99 延迟、CPU、分配速率与 `malloc_usable_size`/profiling 数据。

jemalloc 常用 `MALLOC_CONF`：

| 参数 | 默认值 | 作用 | 建议 |
| --- | --- | --- | --- |
| `background_thread` | 依版本 | 后台异步回收 | 多数服务建议开启并压测 |
| `dirty_decay_ms` | 约 10000ms，依版本 | dirty page 回收时间 | 降 RSS 可下调，延迟敏感需验证 |
| `muzzy_decay_ms` | 约 10000ms，依版本 | muzzy page 回收时间 | 与 dirty decay 联动验证 |
| `narenas` | 常约 4×CPU | Arena 数量 | 控制碎片与竞争，按核心数/线程数调 |
| `tcache` | true | Thread Cache | 通常保持开启 |
| `lg_tcache_max` | 15 | Thread Cache 最大对象 log2 | 控制缓存上限与命中 |
| `oversize_threshold` | 8MB | 大对象独立 arena | 大对象应用可调 |
| `metadata_thp` | auto | metadata 使用 THP | 结合 HugePage 策略验证 |
| `stats_print` | false | 打印统计 | 调试用 |
| `prof` | false | Heap Profiling | 内存分析按需开启 |

示例：

```bash
export MALLOC_CONF="background_thread:true,dirty_decay_ms:1000,muzzy_decay_ms:1000"
./myapp
```

注意：jemalloc 默认值随版本变化较大，生产调整应以所用版本 `jemalloc --version`/文档为准，并用 `MALLOC_CONF=stats_print:true` 或 `jeprof` 验证。

## 总结与推荐检查清单

Linux CPU 调优不是简单提高 CPU 使用率，而是减少无效计算、调度开销和资源竞争。推荐优先级：

1. **Application**：优化算法、数据结构、锁竞争与拷贝；收益最大。
2. **Compiler**：合理使用 `-O3`、`-march=native`、LTO、PGO，充分发挥硬件能力。
3. **Scheduler**：理解 CFS/NUMA balance，减少线程迁移和上下文切换，提高 cache/TLB 命中率。
4. **CPU Affinity/cpuset**：按业务做线程绑定和资源隔离，改善 Cache 与 NUMA 局部性。
5. **Memory Allocator**：针对多线程、高并发或大内存场景选择合适分配器，降低锁竞争和碎片。

上线前检查清单：

- [ ] 有火焰图/perf/bpftrace 证据，而非凭经验改参。
- [ ] 固定硬件拓扑、内核版本、glibc/allocator 版本与负载模型。
- [ ] 同时记录吞吐、P99/P999 延迟、CPU、IPC、cache miss、context switch、runqueue、RSS。
- [ ] 明确回滚方案与灰度范围。
- [ ] 对 `numa_balancing`、实时调度、allocator 替换等高风险项单独实验。

只有建立在性能分析基础上的调优，才能真正提升系统吞吐量、降低延迟，并充分释放 CPU 的计算能力。