# Linux CPU 性能优化：可运行代码示例合集


> 目录实体文件见 `cpu_perf_examples/`；本合集把主要文件合并成一个 Markdown，便于复制。


## `README.md`

```
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

```


## `Makefile`

```makefile
CC ?= gcc
CFLAGS ?= -O2 -g -pthread -Wall -Wextra
BINS := app/false_sharing app/atomic_bench app/syscall_batch app/zero_copy_sendfile app/matrix_mult app/malloc_bench app/cpu_burner

all: $(BINS)

app/%: app/%.c
	$(CC) $(CFLAGS) $< -o $@

app/matrix_mult: app/matrix_mult.c
	$(CC) $(CFLAGS) -DN=384 $< -o $@

clean:
	rm -f $(BINS)

.PHONY: all clean

```


## `app/false_sharing.c`

```
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THREADS 4
#define ITERS 5000000ULL

typedef struct { uint64_t v; char pad[64 - sizeof(uint64_t)]; } padded64;
static padded64 good[THREADS];
static uint64_t bad[THREADS];

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void *worker_good(void *arg) {
    int id = *(int *)arg;
    for (uint64_t i = 0; i < ITERS; i++) { good[id].v++; __asm__ __volatile__("" ::: "memory"); }
    return NULL;
}

static void *worker_bad(void *arg) {
    int id = *(int *)arg;
    for (uint64_t i = 0; i < ITERS; i++) { bad[id]++; __asm__ __volatile__("" ::: "memory"); }
    return NULL;
}

int main(int argc, char **argv) {
    int use_padded = argc > 1 && argv[1][0] == 'p';
    pthread_t t[THREADS];
    int ids[THREADS];
    double st = now_sec();

    for (int i = 0; i < THREADS; i++) {
        ids[i] = i;
        if (pthread_create(&t[i], NULL, use_padded ? worker_good : worker_bad, &ids[i])) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);

    double en = now_sec();
    uint64_t sum = 0;
    for (int i = 0; i < THREADS; i++) sum += use_padded ? good[i].v : bad[i];
    printf("mode=%s threads=%d iters=%llu sum=%llu elapsed=%.3f s\n",
           use_padded ? "padded" : "shared", THREADS,
           (unsigned long long)ITERS, (unsigned long long)sum, en - st);
    return 0;
}

```


## `app/atomic_bench.c`

```
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define THREADS 4
#define ITERS 10000000ULL
static _Atomic uint64_t total = 0;

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void *worker_global(void *unused) {
    (void)unused;
    for (uint64_t i = 0; i < ITERS; i++)
        atomic_fetch_add_explicit(&total, 1, memory_order_relaxed);
    return NULL;
}

static void *worker_local(void *unused) {
    (void)unused;
    uint64_t local = 0;
    for (uint64_t i = 0; i < ITERS; i++) local++;
    atomic_fetch_add_explicit(&total, local, memory_order_relaxed);
    return NULL;
}

int main(int argc, char **argv) {
    int local_first = argc > 1 && argv[1][0] == 'l';
    pthread_t t[THREADS];
    double st = now_sec();
    for (int i = 0; i < THREADS; i++) {
        if (pthread_create(&t[i], NULL, local_first ? worker_local : worker_global, NULL)) {
            perror("pthread_create");
            return 1;
        }
    }
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);
    double en = now_sec();
    printf("mode=%s threads=%d iters_per_thread=%llu total=%llu elapsed=%.3f s\n",
           local_first ? "local_accumulate" : "global_atomic", THREADS,
           (unsigned long long)ITERS, (unsigned long long)total, en - st);
    return 0;
}

```


## `app/syscall_batch.c`

```
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BYTES (16U * 1024U * 1024U)
#define CHUNK 8192U

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    int batch = argc > 1 && argv[1][0] == 'b';
    int fd = open("/dev/null", O_WRONLY);
    if (fd < 0) { perror("open"); return 1; }

    char *buf = malloc(BYTES);
    if (!buf) { perror("malloc"); return 1; }
    memset(buf, 'x', BYTES);

    double st = now_sec();
    if (batch) {
        for (size_t off = 0; off < BYTES; off += CHUNK) {
            if (write(fd, buf + off, CHUNK) != (ssize_t)CHUNK) { perror("write"); return 1; }
        }
    } else {
        for (size_t off = 0; off < BYTES; off++) {
            if (write(fd, buf + off, 1) != 1) { perror("write"); return 1; }
        }
    }
    double en = now_sec();

    printf("mode=%s bytes=%u syscalls≈%u elapsed=%.3f s\n",
           batch ? "batch_8k" : "one_byte", BYTES,
           batch ? (unsigned)(BYTES / CHUNK) : (unsigned)BYTES, en - st);
    free(buf);
    close(fd);
    return 0;
}

```


## `app/zero_copy_sendfile.c`

```
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define SRC "/tmp/cpuopt_zero_copy_src.bin"
#define SRC_SIZE (64U * 1024U * 1024U)
#define CHUNK (1024U * 1024U)

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static int ensure_src(void) {
    int fd = open(SRC, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror("open src"); return -1; }
    char *buf = malloc(CHUNK);
    if (!buf) { perror("malloc"); return -1; }
    memset(buf, 'a', CHUNK);
    for (size_t off = 0; off < SRC_SIZE; off += CHUNK) {
        if (write(fd, buf, CHUNK) != (ssize_t)CHUNK) { perror("write src"); return -1; }
    }
    free(buf);
    close(fd);
    return 0;
}

static int copy_read_write(void) {
    int in = open(SRC, O_RDONLY);
    int out = open("/dev/null", O_WRONLY);
    if (in < 0 || out < 0) { perror("open"); return -1; }
    char *buf = malloc(CHUNK);
    if (!buf) { perror("malloc"); return -1; }
    double st = now_sec();
    ssize_t n;
    while ((n = read(in, buf, CHUNK)) > 0) {
        if (write(out, buf, (size_t)n) != n) { perror("write"); return -1; }
    }
    double en = now_sec();
    printf("read_write copy: %.3f s\n", en - st);
    free(buf); close(in); close(out);
    return n < 0 ? -1 : 0;
}

static int copy_sendfile(void) {
    int in = open(SRC, O_RDONLY);
    int out = open("/dev/null", O_WRONLY);
    if (in < 0 || out < 0) { perror("open"); return -1; }
    double st = now_sec();
    off_t off = 0;
    while (off < (off_t)SRC_SIZE) {
        ssize_t n = sendfile(out, in, &off, CHUNK);
        if (n < 0) { perror("sendfile"); return -1; }
        if (n == 0) break;
    }
    double en = now_sec();
    printf("sendfile   copy: %.3f s\n", en - st);
    close(in); close(out);
    return 0;
}

int main(void) {
    if (ensure_src()) return 1;
    if (copy_read_write()) return 1;
    if (copy_sendfile()) return 1;
    unlink(SRC);
    return 0;
}

```


## `app/matrix_mult.c`

```
#include <stdio.h>
#include <time.h>
#ifndef N
#define N 384
#endif
static double A[N][N], B[N][N], C[N][N];

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(void) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            A[i][j] = (i + 1) * 0.5 + j * 0.25;
            B[i][j] = (j + 1) * 0.75 - i * 0.125;
        }
    double st = now_sec();
    for (int i = 0; i < N; i++)
        for (int k = 0; k < N; k++) {
            double a = A[i][k];
            for (int j = 0; j < N; j++)
                C[i][j] += a * B[k][j];
        }
    double en = now_sec();
    volatile double sink = C[N/2][N/2];
    (void)sink;
    printf("N=%d elapsed=%.3f s checksum=%f\n", N, en - st, C[N/2][N/2]);
    return 0;
}

```


## `app/malloc_bench.c`

```
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define THREADS 4
#define ITERS 2000000U

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static uint64_t xs(uint64_t *s) {
    *s ^= *s << 13; *s ^= *s >> 7; *s ^= *s << 17; return *s;
}

static void *worker(void *arg) {
    uint64_t s = 0x9e3779b97f4a7c15ULL ^ (uintptr_t)arg;
    for (uint32_t i = 0; i < ITERS; i++) {
        size_t sz = 32 + (size_t)(xs(&s) % 224); /* 32..255 bytes */
        char *p = malloc(sz);
        if (!p) { perror("malloc"); exit(1); }
        memset(p, (int)(sz & 0xff), sz);
        free(p);
    }
    return NULL;
}

int main(void) {
    pthread_t t[THREADS];
    double st = now_sec();
    for (int i = 0; i < THREADS; i++) {
        if (pthread_create(&t[i], NULL, worker, (void *)(uintptr_t)(i + 1))) {
            perror("pthread_create"); return 1;
        }
    }
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);
    double en = now_sec();
    printf("threads=%d iters_per_thread=%u elapsed=%.3f s\n", THREADS, ITERS, en - st);
    return 0;
}

```


## `app/cpu_burner.c`

```
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char **argv) {
    int seconds = argc > 1 ? atoi(argv[1]) : 5;
    time_t start = time(NULL);
    volatile unsigned long long x = 0;
    while (time(NULL) - start < seconds) x++;
    printf("cpu_burner done x=%llu\n", x);
    return 0;
}

```


## `build/gcc_bench_matrix.sh`

```
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

```


## `sched/show_sched_state.sh`

```
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

```


## `sched/run_affinity_compare.sh`

```
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

```


## `sched/chrt_realtime_demo.sh`

```
#!/usr/bin/env bash
set -euo pipefail
echo "WARNING: SCHED_FIFO/RR can starve normal tasks. Run only on a test box."
chrt -p $$ || true
echo "Example (requires CAP_SYS_NICE/root): chrt -f 50 ./app/cpu_burner 3"
echo "Rollback/cleanup: kill the task; no persistent setting is changed by this script."

```


## `cgroup/cpuset_cpu_demo.sh`

```
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

```


## `malloc/run_allocator_compare.sh`

```
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

```


## `profiling/perf_stat_record.sh`

```
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

```


## `profiling/bpftrace_one_liners.sh`

```
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

```


## `profiling/flamegraph.sh`

```
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

```
