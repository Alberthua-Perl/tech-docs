# Linux 磁盘性能测试

## 文档说明

本文档采用 **FIO** 与 **smartctl** 工具测试磁盘与文件系统的健康状况、I/O 读写性能。

## 文档目录

- [Linux 磁盘性能测试](#linux-磁盘性能测试)
  - [文档说明](#文档说明)
  - [文档目录](#文档目录)
  - [1. fio 命令性能测试示例与解读](#1-fio-命令性能测试示例与解读)
    - [1.1 裸盘性能测试：**命令行参数方式**](#11-裸盘性能测试命令行参数方式)
    - [1.2 文件系统性能测试：**fio 配置文件方式**](#12-文件系统性能测试fio-配置文件方式)
    - [1.3 fio 命令行参数释义](#13-fio-命令行参数释义)
  - [2. fio 并发数与虚拟内存使用量说明](#2-fio-并发数与虚拟内存使用量说明)
    - [2.1 I/O 并发数计算](#21-io-并发数计算)
    - [2.2 虚拟内存使用量估算](#22-虚拟内存使用量估算)
    - [2.3 fio 运行导致的 OOM 问题分析](#23-fio-运行导致的-oom-问题分析)
  - [2.4 fio 测试脚本示例](#24-fio-测试脚本示例)
  - [3. smartctl 磁盘健康检查脚本示例](#3-smartctl-磁盘健康检查脚本示例)
  - [4. Linux 内核故障注入框架（fail\_io）](#4-linux-内核故障注入框架fail_io)
    - [4.1 启用前提](#41-启用前提)
    - [4.2 示例命令](#42-示例命令)
  - [参考链接](#参考链接)

## 1. fio 命令性能测试示例与解读

### 1.1 裸盘性能测试：**命令行参数方式**

```bash
$ time fio --name=128K_write_4.result --filename=/dev/nvme0n1 --size=1T \
  --direct=1 --iodepth=4096 --numjobs=4 \
  --bs=128K --rw=write \
  --thread --ioengine=libaio --time_based --runtime=120 \
  --group_reporting \
  --output=./128K_write_4_6.result
```

### 1.2 文件系统性能测试：**fio 配置文件方式**

fio 配置文件名：fio-xfs.job

```ini
[global]                  # 全局设置
directory=/mnt/fio-xfs    # 文件系统挂载目录
filename=testfile         # 文件系统中生成的测试文件名称
size=1G                   # 测试文件的容量大小，测试完成后依然保留
#time_based
#runtime=120
direct=1                  # O_DIRECT 模式，绕过 Linux page cache（页缓存）
ioengine=libaio           # Linux 原生异步 I/O 引擎
group_reporting           # 汇总所有 job 结果为单一报告

[seqread]                 # 局部设置：总并发数 1 x 32 = 32，进程数量 1
rw=read                   # job 类型：顺序读
bs=128k                   # I/O 块大小
iodepth=32                # 每个 job 类型中 job 的 I/O 队列深度
numjobs=1                 # 每个 job 类型中的副本数（并发线程/进程数）
stonewall                 # 每个 job 类型中 job 之间的隔离屏障

[seqwrite]                # 总并发数 1 x 32 = 32，进程数量 1
rw=write
bs=128k
iodepth=32
numjobs=1
stonewall

[randread]                # 总并发数 4 x 128 = 512，进程数量 4
rw=randread
bs=4k
iodepth=128
numjobs=4
stonewall

[randwrite]               # 总并发数 4 x 128 = 512，进程数量 4
rw=randwrite
bs=4k
iodepth=128
numjobs=4
stonewall

[randrw]                  # 总并发数 4 x 64 = 256，进程数量 4
rw=randrw
bs=4k
iodepth=64
numjobs=4
rwmixread=70
stonewall

# 注意：
#   1. 总 I/O 并发数 = 32 x 2 + 512 x 2 + 256 = 1344，进程总数 14
#   2. 每个 job 类型中的参数设置优先级高于 [global] 中的设置，可进行覆盖。
#   3. [global] 中的设置对每个 job 类型中的 job 均生效
#   4. stonewall 表明每个 job 类型中的 job 都是依次串行执行
```

运行测试 fio 命令：

```bash
$ sudo fio ./fio-xfs.job
seqread: (g=0): rw=read, bs=(R) 128KiB-128KiB, (W) 128KiB-128KiB, (T) 128KiB-128KiB, ioengine=libaio, iodepth=32
seqwrite: (g=1): rw=write, bs=(R) 128KiB-128KiB, (W) 128KiB-128KiB, (T) 128KiB-128KiB, ioengine=libaio, iodepth=32
randread: (g=2): rw=randread, bs=(R) 4096B-4096B, (W) 4096B-4096B, (T) 4096B-4096B, ioengine=libaio, iodepth=128
...
randwrite: (g=3): rw=randwrite, bs=(R) 4096B-4096B, (W) 4096B-4096B, (T) 4096B-4096B, ioengine=libaio, iodepth=128
...
randrw: (g=4): rw=randrw, bs=(R) 4096B-4096B, (W) 4096B-4096B, (T) 4096B-4096B, ioengine=libaio, iodepth=64
...
fio-3.7
Starting 14 processes
seqread: Laying out IO file (1 file / 1024MiB)
Jobs: 4 (f=4): [_(10),m(4)][100.0%][r=197MiB/s,w=82.9MiB/s][r=50.3k,w=21.2k IOPS][eta 00m:00s]
seqread: (groupid=0, jobs=1): err= 0: pid=6774: Fri Sep  4 17:47:33 2026
   read: IOPS=32.4k, BW=4046MiB/s (4242MB/s)(474GiB/120001msec)
    slat (nsec): min=1370, max=16450k, avg=14395.78, stdev=58622.24
    clat (nsec): min=1433, max=45046k, avg=972746.23, stdev=1094764.16
     lat (usec): min=67, max=54471, avg=987.52, stdev=1097.10
    clat percentiles (usec):
     |  1.00th=[  196],  5.00th=[  281], 10.00th=[  343], 20.00th=[  441],
     | 30.00th=[  537], 40.00th=[  635], 50.00th=[  734], 60.00th=[  857],
     | 70.00th=[  996], 80.00th=[ 1188], 90.00th=[ 1631], 95.00th=[ 2245],
     | 99.00th=[ 5866], 99.50th=[ 8029], 99.90th=[12780], 99.95th=[15008],
     | 99.99th=[21103]
   bw (  MiB/s): min=  743, max= 6572, per=84.08%, avg=3401.94, stdev=1356.51, samples=239
   iops        : min= 5947, max=52576, avg=27215.02, stdev=10852.06, samples=239
  lat (usec)   : 2=0.01%, 4=0.01%, 50=0.01%, 100=0.02%, 250=3.10%
  lat (usec)   : 500=23.04%, 750=25.06%, 1000=19.08%
  lat (msec)   : 2=23.42%, 4=4.46%, 10=1.56%, 20=0.25%, 50=0.01%
  cpu          : usr=0.85%, sys=51.70%, ctx=128331, majf=0, minf=15
  IO depths    : 1=0.1%, 2=0.1%, 4=0.1%, 8=0.1%, 16=0.1%, 32=100.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.1%, 64=0.0%, >=64=0.0%
     issued rwts: total=3884080,0,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=32
seqwrite: (groupid=1, jobs=1): err= 0: pid=6775: Fri Sep  4 17:47:33 2026
  write: IOPS=15.6k, BW=1946MiB/s (2041MB/s)(228GiB/120002msec)
    slat (nsec): min=1955, max=26305k, avg=18045.98, stdev=98676.03
    clat (usec): min=7, max=237633, avg=2035.38, stdev=5849.17
     lat (usec): min=86, max=237638, avg=2053.88, stdev=5849.91
    clat percentiles (usec):
     |  1.00th=[   330],  5.00th=[   529], 10.00th=[   676], 20.00th=[   889],
     | 30.00th=[  1057], 40.00th=[  1221], 50.00th=[  1385], 60.00th=[  1598],
     | 70.00th=[  1860], 80.00th=[  2245], 90.00th=[  3032], 95.00th=[  4146],
     | 99.00th=[  9896], 99.50th=[ 16319], 99.90th=[102237], 99.95th=[133694],
     | 99.99th=[193987]
   bw (  MiB/s): min=  609, max= 3196, per=94.18%, avg=1832.98, stdev=589.57, samples=239
   iops        : min= 4877, max=25573, avg=14663.36, stdev=4716.63, samples=239
  lat (usec)   : 10=0.01%, 20=0.01%, 100=0.01%, 250=0.30%, 500=4.01%
  lat (usec)   : 750=8.65%, 1000=13.38%
  lat (msec)   : 2=48.11%, 4=20.21%, 10=4.35%, 20=0.55%, 50=0.21%
  lat (msec)   : 100=0.12%, 250=0.11%
  cpu          : usr=2.61%, sys=27.90%, ctx=145215, majf=0, minf=11
  IO depths    : 1=0.1%, 2=0.1%, 4=0.1%, 8=0.1%, 16=0.1%, 32=100.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.1%, 64=0.0%, >=64=0.0%
     issued rwts: total=0,1868355,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=32
randread: (groupid=2, jobs=4): err= 0: pid=6779: Fri Sep  4 17:47:33 2026
   read: IOPS=116k, BW=455MiB/s (477MB/s)(53.3GiB/120002msec)
    slat (nsec): min=653, max=72889k, avg=27108.69, stdev=365329.33
    clat (usec): min=26, max=109603, avg=4370.92, stdev=4694.42
     lat (usec): min=109, max=109605, avg=4398.38, stdev=4719.94
    clat percentiles (usec):
     |  1.00th=[  537],  5.00th=[  807], 10.00th=[ 1029], 20.00th=[ 1401],
     | 30.00th=[ 1795], 40.00th=[ 2311], 50.00th=[ 2933], 60.00th=[ 3687],
     | 70.00th=[ 4686], 80.00th=[ 6194], 90.00th=[ 9110], 95.00th=[12780],
     | 99.00th=[23462], 99.50th=[28705], 99.90th=[44303], 99.95th=[51119],
     | 99.99th=[69731]
   bw (  KiB/s): min=19575, max=205012, per=17.81%, avg=82878.59, stdev=30440.62, samples=956
   iops        : min= 4893, max=51253, avg=20719.26, stdev=7610.16, samples=956
  lat (usec)   : 50=0.01%, 100=0.01%, 250=0.01%, 500=0.67%, 750=3.30%
  lat (usec)   : 1000=5.26%
  lat (msec)   : 2=24.96%, 4=29.30%, 10=28.10%, 20=6.82%, 50=1.53%
  lat (msec)   : 100=0.06%, 250=0.01%
  cpu          : usr=1.64%, sys=17.16%, ctx=427183, majf=0, minf=550
  IO depths    : 1=0.1%, 2=0.1%, 4=0.1%, 8=0.1%, 16=0.1%, 32=0.1%, >=64=100.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.1%
     issued rwts: total=13963156,0,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=128
randwrite: (groupid=3, jobs=4): err= 0: pid=6783: Fri Sep  4 17:47:33 2026
  write: IOPS=82.5k, BW=322MiB/s (338MB/s)(37.7GiB/120007msec)
    slat (nsec): min=668, max=101474k, avg=39516.06, stdev=471620.22
    clat (usec): min=131, max=135552, avg=6167.15, stdev=5974.85
     lat (usec): min=184, max=135555, avg=6207.05, stdev=6011.85
    clat percentiles (usec):
     |  1.00th=[  799],  5.00th=[ 1287], 10.00th=[ 1663], 20.00th=[ 2278],
     | 30.00th=[ 2900], 40.00th=[ 3556], 50.00th=[ 4359], 60.00th=[ 5342],
     | 70.00th=[ 6718], 80.00th=[ 8717], 90.00th=[12649], 95.00th=[16909],
     | 99.00th=[29754], 99.50th=[36439], 99.90th=[54789], 99.95th=[64226],
     | 99.99th=[87557]
   bw (  KiB/s): min=18218, max=151571, per=20.00%, avg=65962.26, stdev=23746.68, samples=956
   iops        : min= 4554, max=37892, avg=16490.21, stdev=5936.66, samples=956
  lat (usec)   : 250=0.01%, 500=0.09%, 750=0.72%, 1000=1.37%
  lat (msec)   : 2=13.20%, 4=30.47%, 10=38.36%, 20=12.57%, 50=3.08%
  lat (msec)   : 100=0.15%, 250=0.01%
  cpu          : usr=1.85%, sys=12.16%, ctx=392435, majf=0, minf=39
  IO depths    : 1=0.1%, 2=0.1%, 4=0.1%, 8=0.1%, 16=0.1%, 32=0.1%, >=64=100.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.1%
     issued rwts: total=0,9895609,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=128
randrw: (groupid=4, jobs=4): err= 0: pid=6787: Fri Sep  4 17:47:33 2026
   read: IOPS=66.0k, BW=262MiB/s (274MB/s)(30.7GiB/120020msec)
    slat (nsec): min=644, max=29235k, avg=7648.97, stdev=91422.24
    clat (nsec): min=1845, max=99233k, avg=2576711.11, stdev=2393228.25
     lat (usec): min=41, max=99234, avg=2584.73, stdev=2397.35
    clat percentiles (usec):
     |  1.00th=[  334],  5.00th=[  586], 10.00th=[  799], 20.00th=[ 1106],
     | 30.00th=[ 1352], 40.00th=[ 1614], 50.00th=[ 1926], 60.00th=[ 2278],
     | 70.00th=[ 2769], 80.00th=[ 3523], 90.00th=[ 4948], 95.00th=[ 6652],
     | 99.00th=[12125], 99.50th=[14746], 99.90th=[22938], 99.95th=[27657],
     | 99.99th=[42730]
   bw (  KiB/s): min=22011, max=143800, per=24.97%, avg=66878.24, stdev=23150.88, samples=960
   iops        : min= 5502, max=35950, avg=16719.40, stdev=5787.74, samples=960
  write: IOPS=28.7k, BW=112MiB/s (118MB/s)(13.1GiB/120020msec)
    slat (nsec): min=709, max=35965k, avg=8198.92, stdev=96400.65
    clat (usec): min=10, max=98381, avg=2876.09, stdev=2540.43
     lat (usec): min=82, max=98384, avg=2884.66, stdev=2545.18
    clat percentiles (usec):
     |  1.00th=[  545],  5.00th=[  832], 10.00th=[ 1037], 20.00th=[ 1319],
     | 30.00th=[ 1565], 40.00th=[ 1844], 50.00th=[ 2147], 60.00th=[ 2540],
     | 70.00th=[ 3064], 80.00th=[ 3851], 90.00th=[ 5407], 95.00th=[ 7242],
     | 99.00th=[12911], 99.50th=[15664], 99.90th=[24249], 99.95th=[29754],
     | 99.99th=[44303]
   bw (  KiB/s): min= 9598, max=59768, per=24.97%, avg=28659.93, stdev=9950.33, samples=960
   iops        : min= 2399, max=14942, avg=7164.82, stdev=2487.59, samples=960
  lat (usec)   : 2=0.01%, 20=0.01%, 50=0.01%, 100=0.01%, 250=0.26%
  lat (usec)   : 500=2.18%, 750=4.73%, 1000=6.65%
  lat (msec)   : 2=36.48%, 4=33.19%, 10=14.65%, 20=1.65%, 50=0.17%
  lat (msec)   : 100=0.01%
  cpu          : usr=1.97%, sys=13.61%, ctx=530667, majf=0, minf=51
  IO depths    : 1=0.1%, 2=0.1%, 4=0.1%, 8=0.1%, 16=0.1%, 32=0.1%, >=64=100.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.1%, >=64=0.0%
     issued rwts: total=8036928,3444151,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=64

Run status group 0 (all jobs):
   READ: bw=4046MiB/s (4242MB/s), 4046MiB/s-4046MiB/s (4242MB/s-4242MB/s), io=474GiB (509GB), run=120001-120001msec

Run status group 1 (all jobs):
  WRITE: bw=1946MiB/s (2041MB/s), 1946MiB/s-1946MiB/s (2041MB/s-2041MB/s), io=228GiB (245GB), run=120002-120002msec

Run status group 2 (all jobs):
   READ: bw=455MiB/s (477MB/s), 455MiB/s-455MiB/s (477MB/s-477MB/s), io=53.3GiB (57.2GB), run=120002-120002msec

Run status group 3 (all jobs):
  WRITE: bw=322MiB/s (338MB/s), 322MiB/s-322MiB/s (338MB/s-338MB/s), io=37.7GiB (40.5GB), run=120007-120007msec

Run status group 4 (all jobs):
   READ: bw=262MiB/s (274MB/s), 262MiB/s-262MiB/s (274MB/s-274MB/s), io=30.7GiB (32.9GB), run=120020-120020msec
  WRITE: bw=112MiB/s (118MB/s), 112MiB/s-112MiB/s (118MB/s-118MB/s), io=13.1GiB (14.1GB), run=120020-120020msec

Disk stats (read/write):
  vda: ios=25728489/15106312, merge=156463/102053, ticks=44147604/36693398, in_queue=61054857, util=82.29%
```

### 1.3 fio 命令行参数释义

| 参数 | 值 | 含义 | 影响 |
| :----- | :----- | :----- | :----- |
| `--name` | 128K_write_4.result | **测试任务标识名**，用于日志和报告中区分不同测试 | 仅命名，无性能影响 |
| `--filename` | /dev/nvme0n1 | **目标设备路径**，裸盘测试（非文件系统） | 直接操作块设备，绕过文件系统开销 |
| `--size` | 1T | **每个 job 类型中单个 job 的测试数据总量**为 1TB | 决定测试数据范围；若设备 < 1T 则全设备 |
| `--direct` | 1 | **O\_DIRECT 模式**，绕过 Linux page cache | 测真实设备性能，排除内存缓冲干扰 |
| `--iodepth` | 4096 | **每个 job 类型中 job 的 I/O 队列深度**为 4096 | 极大值，内存消耗 ≈ 4096 × 128K × 4 jobs = **2GB+** |
| `--numjobs` | 4 | **每个 job 类型中的副本数（并发线程/进程数）**为 4 | 把同一个 job 复制 N 份并行跑，总并发 I/O = 4 × 4096 = **16384** |
| `--thread` | （无值） | 使用 **pthread 线程**而非多进程 | 减少内存开销，共享地址空间 |
| `--ioengine` | libaio | **Linux 原生异步 I/O 引擎** | 真正非阻塞，需 iodepth > 1 才有意义 |
| `--time_based` | （无值） | **基于时间运行**，非基于数据量 | 配合 runtime 使用，忽略 size 完成度 |
| `--runtime` | 120 | **持续运行 120 秒** | 测试总时长，确保进入稳态 |
| `--group_reporting` | （无值） | **汇总所有 job 结果**为单一报告 | 不显示每个 job 单独结果 |
| `--bs` | 128K | **I/O 块大小**为 128 KiB | 顺序写场景，最大化带宽 |
| `--rw` | write | **job 类型，单个 job 的 I/O 模式，此处为纯顺序写** | 无读取，测试写带宽和稳定性；可以是 seqread / seqwrite / randread / randwrite / randrw 等 |
| `--output` | ./128K_write_4_6.result | **结果输出到文件** | 便于后续分析和存档 |
| `--stonewall` | （无值） | **每个 job 类型中 job 之间的隔离屏障** | 有它则 job 串行，无它则 job 并行 |

## 2. fio 并发数与虚拟内存使用量说明

### 2.1 I/O 并发数计算

- 每个 job 类型中 I/O 并发数 = numjobs x iodepth
- 整体总 I/O 并发数 = 各个 job 类型的 I/O 并发数总和

### 2.2 虚拟内存使用量估算

| 组件 | 估算 | 说明 |
| :----- | :----- | :----- |
| I/O 数据缓冲 | `numjobs × iodepth × bs` | libaio 提交队列 |
| 完成事件缓冲 | `numjobs × iodepth × 16B` | io_event 结构体 |
| fio 内部状态 | `numjobs × ~64KB` | 每 job 上下文 |
| 文件映射（非 direct） | `size` 或 `filesize` | page cache |

1️⃣ direct 模式：

```bash
--numjobs=4 --iodepth=128 --bs=4k --direct=1
```

- 数据缓冲 = 4 x 128 x 4K = 2MB
- 完成事件 = 4 x 128 x 16B = 8KB
- 内部状态 = 4 x 64K = 256KB
- **总计 ≈ 2.26MB**

2️⃣ 非 direct 模式：使用 **文件系统页缓存**

```bash
--numjobs=4 --iodepth=32 --bs=1M --direct=0 --size=100G
```

- 数据缓冲 = 4 x 32 x 1M = 128MB
- page cache = 100G（文件映射至内存）
- **总计 ≈ 128MB + 100GB（若总共 2G 内存将耗光内存，触发 OOM）**

**<font color=red>注意：direct=1 时使用 1️⃣ 计算，direct=0 时由 size 决定内存。</font>**

### 2.3 fio 运行导致的 OOM 问题分析

笔者实验环境中系统物理内存为 2GiB，

```bash
# 背景1：当前系统可用的物理内存 1526MiB（1.49GiB）
[root@servera ~]# free -m
              total        used        free      shared  buff/cache   available
Mem:           1828         187        1574          16          67        1526
Swap:             0           0           0

# 背景2：fio 命令测试运行片刻后，进程被杀死。
[root@servera ~]# fio --name=128K_write_4.result --filename=/dev/vdb1 --size=1G --direct=1 --iodepth=4096 --numjobs=8 --bs=128K --rw=write --thread --ioengine=libaio --time_based --runtime=120 --group_reporting --output=./128K_write_4_6.result
Killed8 (f=0): [W(8)][5.5%][r=0KiB/s,w=0KiB/s][r=0,w=0 IOPS][eta 02m:00s]

# 背景3：fio 执行过程中的内存分配情况
[root@servera ~]# tail -n 4 /var/log/messages
Sep  9 21:12:12 servera kernel: [ 7996]     0  7996  1479058   395002  3629056        0             0 fio
Sep  9 21:12:12 servera kernel: Out of memory: Kill process 7996 (fio) score 845 or sacrifice child
Sep  9 21:12:12 servera kernel: Killed process 7996 (fio) total-vm:5916232kB, anon-rss:1172516kB, file-rss:0kB, shmem-rss:407492kB
Sep  9 21:12:12 servera kernel: oom_reaper: reaped process 7996 (fio), now anon-rss:0kB, file-rss:0kB, shmem-rss:407496kB

# 背景4：
[root@servera ~]# grep Commit /proc/meminfo
CommitLimit:      936444 kB
Committed_AS:     425784 kB

# 分析：估算 fio 虚拟内存使用量
[root@servera ~]# bc
bc 1.07.1
Copyright 1991-1994, 1997, 1998, 2000, 2004, 2006, 2008, 2012-2017 Free Software Foundation, Inc.
This is free software with ABSOLUTELY NO WARRANTY.
For details type `warranty'.
4096*8*128    # fio 数据缓存的虚拟内存：4194304kB < total-vm:5916232kB
4194304
scale=3; (1172516+407496)/2^20    # 根因：fio 已用物理内存 anon-rss:1172516kB + shmem-rss:407492kB = 1.506G > available:1.49G 直接导致 OOM！
1.506
quit
```

## 2.4 fio 测试脚本示例

测试脚本文件名 fio_qps_test.sh：

```bash
#!/bin/bash
#
# FIO QPS 压测脚本

TEST_FILE="/data/fio_testfile"   # 测试文件路径 (不要放在数据库数据目录!!!)
SIZE="1G"                        # 测试文件大小
BS="4k"                          # 块大小
RUNTIME="30"                     # 每个测试持续时间(秒)
IODEPTH="64"                     # 队列深度

# 检查是否安装 fio
if ! command -v fio &>/dev/null; then
    echo "未检测到 fio，请先安装，即 yum install -y fio 或 apt install -y fio"
    exit 1
fi

echo "============================================="
echo " FIO 磁盘 QPS 压测开始 "
echo " 文件: $TEST_FILE, 大小: $SIZE, 块大小: $BS, 时长: ${RUNTIME}s, 队列深度: $IODEPTH"
echo "============================================="

# 结果保存变量
RESULTS=()

function fio_test() {
    NAME=$1
    RW=$2
    MIX=$3

    if [[ "$MIX" != "" ]]; then
        MIX_OPT="--rwmixread=$MIX"
    else
        MIX_OPT=""
    fi

    echo "测试: $NAME ..."
    OUT=$(fio --name=$NAME --filename=$TEST_FILE --size=$SIZE --bs=$BS --rw=$RW $MIX_OPT \
          --ioengine=libaio --iodepth=$IODEPTH --runtime=$RUNTIME --direct=1 --group_reporting 2>/dev/null)

    IOPS=$(echo "$OUT" | grep -E 'iops=' | head -n1 | sed -E 's/.*iops=([0-9\.kK]+).*/\1/')
    BW=$(echo "$OUT" | grep -E 'bw=' | head -n1 | sed -E 's/.*bw=([0-9\.A-Za-z\/]+).*/\1/')

    RESULTS+=("$NAME | $RW | $IOPS | $BW")
}

# 随机读
fio_test "randread_test" "randread" ""

# 随机写
fio_test "randwrite_test" "randwrite" ""

# 混合读写 (70%读 30%写)
fio_test "randrw_test" "randrw" "70"

echo
echo "================= 压测结果 ================="
printf "%-12s | %-12s | %-10s | %-10s\n" "测试场景" "模式" "IOPS(QPS)" "带宽"
echo "-------------------------------------------------------------"
for FIELD in "${RESULTS[@]}"; do
    echo "$FIELD"
done

echo "============================================="
echo "注意: 压测可能影响线上服务，请勿在生产库数据盘直接运行！"
```

运行与结果：

```bash
./fio_qps_test.sh
================= 压测结果 =================
测试场景        | 模式          | IOPS(QPS)  | 带宽
-----------------------------------------------------
randread       | randread      | 1520       | 6.2MB/s
randwrite      | randwrite     | 980        | 3.9MB/s
randrw         | randrw        | 1100       | 4.5MB/s
=============================================
```

## 3. smartctl 磁盘健康检查脚本示例

```bash
#!/bin/bash
#
# Use smartctl tool to check SSD or NVMe disk health status.
# Modified by hualongfeiyyy@163.com on 2025-08-25.
#

DEV=$1
sudo smartctl -H "$DEV" | grep -q "PASSED" || { echo "❌ HEALTH FAILED"; exit 1; }

# NVMe 检查
if [[ $DEV == *nvme* ]]; then
    warn=$(sudo smartctl -A "$DEV" | awk '/Critical Warning/ {print $3}')
    used=$(sudo smartctl -A "$DEV" | awk '/Percentage_Used/ {print $3}')
    [[ $warn -ne 0 ]] && echo "⚠️ Critical Warning=$warn"
    [[ $used -gt 90 ]] && echo "⚠️ Percentage_Used=$used%"
else  # SATA/SAS
    reall=$(sudo smartctl -A "$DEV" | awk '/Reallocated_Sector_Ct/ {print $10}')
    pend=$(sudo smartctl -A "$DEV" | awk '/Current_Pending_Sector/ {print $10}')
    [[ $reall -gt 0 ]] && echo "⚠️ Reallocated=$reall"
    [[ $pend  -gt 0 ]] && echo "⚠️ Pending=$pend"
fi
```

## 4. Linux 内核故障注入框架（fail_io）

### 4.1 启用前提

### 4.2 示例命令

Linux 内核故障注入框架（**fail_io**）的配置，用于 **人为制造块设备 I/O 错误**，通常用于测试文件系统容错、多路径切换、应用降级逻辑等场景。

| 命令 | 功能 |
| :----- | :----- |
| `echo "8:16" > /sys/kernel/debug/fail_io/blkdevs` | **指定目标设备**：`8:16` = 主设备号 8、次设备号 16，即 `/dev/sdb`（`ls -l /dev/sdb` 可验证） |
| `echo 2 > /sys/kernel/debug/fail_io/probability` | **故障概率 2%**：每 100 个 I/O 请求中，约 2 个会被注入失败 |
| `echo 0 > /sys/kernel/debug/fail_io/ignore_reads` | **不忽略读请求**：读 I/O 也参与故障注入（0=不忽略，1=忽略） |
| `echo 0 > /sys/kernel/debug/fail_io/ignore_writes` | **不忽略写请求**：写 I/O 也参与故障注入 |
| `echo 1 > /sys/kernel/debug/fail_io/should_fail` | **总开关打开**：开始生效 |

## 参考链接
