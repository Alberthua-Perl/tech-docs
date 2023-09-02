## Linux 系统性能优化探究

### 文档说明：

- 以下示例均在 RHEL 8 中验证实现，若针对其他 Linux 发行版请自行测试。

- 该文档用于描述 Linux 中常用的系统监控与性能优化工具的功能与使用场景。

- ✍ 由于笔者水平有限，该文档在实践过程中将不断丰富及改进。

### 文档目录：

- 进程在虚拟内存地址空间的布局（layout）

- Linux 常用系统性能监控工具

- Linux 系统资源限制之 cgroup

- 🔥 Linux 内核性能计数器之 perf

- kernel 相关软件包下载

- 进程的调度与优先级

- CPU 时钟周期、机器周期、指令周期的关系

- x86_64 架构的常用寄存器示例

- CPU Cache 缓存架构

### 进程在虚拟内存地址空间的布局（layout）：

- Linux 内核使用分段与分页机制实现进程虚拟内存地址、线性内存地址至物理内存地址的转换，而虚拟内存地址空间的分段信息可在 `/proc/<pid>/maps` 中确定，如下图所示。
  
  ![](pictures/linux-process-memory-layout.png)
  
  `/proc/<pid>/maps` 中的 16 进制虚拟内存地址从显示的低地址位向高地址位扩展，并且在连续的地址空间之间为了保证数据安全性存在一定的 `gap` 区域，而右侧示意图中显示除了进程自身的虚拟内存地址空间外，还存在内核虚拟内存地址空间，两者共同协作完成进程所需执行的任务。

- 对于指定进程的全部状态信息可在 `/proc/<pid>/status` 文件中查看，如上述进程的栈（stack）大小为 132 KiB（占 33 个 page）。
  
  ```bash
  $ sudo grep VmStk /proc/4429/status
    VmStk:       132 kB
  ```

### Linux 常用系统性能监控工具：

> 📜 以下命令均可使用 man 命令查询详尽的使用说明

- Linux 性能观测性工具图谱：
  
  ![](pictures/linux-performance-observability.jpg)

- Linux 静态性能工具图谱：
  
  ![](pictures/linux-static-performance-tools.jpg)

- `ps` 命令详解：
  
  ```bash
  ### GNU 风格的命令行 ###
  $ pidof <process_name>
  # 根据进程名称查找进程 ID
  
  $ ps -p $(pidof <process_name>)
  # 查看相应进程的概要信息
  $ ps -p $(pidof nginx)
    PID TTY      STAT   TIME COMMAND
    865 ?        Ss     0:00 nginx: master process /usr/sbin/nginx
    866 ?        S      0:00 nginx: worker process
    867 ?        S      0:00 nginx: worker process 
  # 查看 Nginx 相关进程的概要信息
  
  $ ps -p <pid> -o etime
  # 查看指定进程自启动为止的消耗（elapsed）时间
  $ ps --forest -C <cmdlist>
  # 查看指定命令列表的进程树 结构
  $ ps --forest -C nginx -o pid,ppid,cmd
  # 查看 Nginx 进程的进程树与 pid、ppid 和 cmd
  
  $ ps -ef
  $ ps -efL
  # 全格式输出系统上运行的所有进程，并显示各个进程的线程数（NLWP）。
  # 注意：Linux 内核不区分进程与线程，将线程视为轻量级进程（LWP）。
  $ ps -L -C <process_name>
  # 查看指定进程的线程信息
  $ ps -L [-p|p|-q|q] <pid>
  # 查看指定进程的线程信息
  $ ps -U <user_name>
  # 查看指定用户的进程信息
  
  ### BSD 风格命令行 ###
  $ ps axu
    USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
    root         1  0.0  0.7 180608 13420 ?        Ss   Apr04   0:05 /usr/lib/systemd/systemd --switched-root --system --deserialize 18
    root         2  0.0  0.0      0     0 ?        S    Apr04   0:00 [kthreadd]
    root         3  0.0  0.0      0     0 ?        I<   Apr04   0:00 [rcu_gp]
    root         4  0.0  0.0      0     0 ?        I<   Apr04   0:00 [rcu_par_gp]
    root         6  0.0  0.0      0     0 ?        I<   Apr04   0:00 [kworker/0:0H-kblockd]
    root         8  0.0  0.0      0     0 ?        I<   Apr04   0:00 [mm_percpu_wq]
    root         9  0.0  0.0      0     0 ?        S    Apr04   0:01 [ksoftirqd/0]
    root        10  0.0  0.0      0     0 ?        I    Apr04   0:01 [rcu_sched]
    root        11  0.0  0.0      0     0 ?        S    Apr04   0:00 [migration/0]
    root        12  0.0  0.0      0     0 ?        S    Apr04   0:00 [watchdog/0]
    ...
  # 查看所有用户及进程的扩展详情
  # 使用 man ps 查看命令输出的 HEADER 详细说明
  # 常见的 HEADER 说明：
  #   USER：user，也称为 euser，即运行进程的有效用户 ID。
  #   PID：pid，即进程 ID。
  #   %CPU：%cpu，也称为 cputime 或 realtime ratio，即进程占所有进程的 CPU 使用时间百分比。
  #   %MEM：%mem，即进程的常驻物理内存（resident set size）使用率。
  #   VSZ：vsz，即进程的虚拟内存大小（单位为 KiB）。
  #   RSS：rss，即进程的常驻内存大小（非 swapped 的物理内存大小），其单位为 KiB。
  #   TTY：tname，即进程所在的终端，其中 "?" 代表进程无需运行终端。
  #   STAT：stat，即进程的状态。
  #   START：start_time，即进程启动的时间或日期。
  #   TIME：time，即进程从启动到现在积累的 CPU 使用时间，格式为 "[DD-]HH:MM:SS"，该值逐渐累加。
  #   CLS：cls，即进程的调度类别（见下文 "进程的调度与优先级说明"）。
  #   COMMAND：comm，即进程的可执行程序名称。
  
  $ ps axum
  # 查看系统上运行的所有进程，并在每个进程下显示该进程的所有线程。
  $ ps axl
  # 长列表格式输出系统上运行的所有进程
  $ ps axjf
  # 查看所有进程的进程树信息，与 pstree 命令类似。
  
  $ ps ax --format pid,%mem,comm --sort -%mem
  $ ps axo pid,%mem,comm --sort -%mem
  # 查看进程以进程 ID、进程的物理内存使用率以及进程的可执行程序名称输出，并以物理内存使用率
  # 的降序（从高到低）排序。
  ```
  
  💥 关于 ps 与 top 命令输出中虚拟内存与常驻内存的说明：
  
  进程的 VSZ 与 RSS 在进程的 `/proc/<pid>/status` 中分别对应 `VmSize` 与 `VmRSS`，而 `VmRSS = RssAnon + RssFile`。
  
  ```bash
  $ sudo cat /proc/4429/status
    ...
    VmSize:   149300 kB
    RssAnon:            3304 kB
    RssFile:            5320 kB
    ...
  $ ps axo pid,vsz,rss,comm | grep 4429
    4429 149300  8624 nginx
  ```
  
  top 命令对 PID 4429 的输出如下所示，其中 `VIRT`（进程的虚拟内存）与 ps 命令的 VSZ 相同，`RES`（进程的常驻内存）与 ps 命令的 RSS 相同，SHR 与 `/proc/<pid>/status` 的 RssFile 相同。
  
  ```bash
  $ top -n 1 -p 4429
    Tasks:   1 total,   0 running,   1 sleeping,   0 stopped,   0 zombie
    %Cpu(s):  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
    MiB Mem :   1828.8 total,   1268.2 free,    210.0 used,    350.7 buff/cache
    MiB Swap:      0.0 total,      0.0 free,      0.0 used.   1450.3 avail Mem
  
     PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
    4429 nginx     20   0  149300   8624   5320 S   0.0   0.5   0:01.04 nginx 
  # top 命令执行 1 秒立即返回
  ```
  
  通过 ps 与 top 命令可查看进程的虚拟内存大小，除此之外也可直接计算进程在虚拟内存中的分段范围而获得其虚拟内存的大小，在 Linux 中进程的虚拟内存分段的映射位于 `/proc/<pid>/maps` 文件中，计算方法如下所示：
  
  ```bash
  $ sudo cat /proc/<pid>/maps | \
    awk '{print $1}' | \
    awk -F'[-]' '{ s=strtonum("0x"$1); e=strtonum("0x"$2); sum+=e-s } END { print sum/1024 }'
  # 统计指定进程的虚拟内存地址空间中的地址数量，每个地址对应 1 字节。
  # 以上命令返回的单位为 KiB
  ```

- `htop` 命令详解：

- `free` 命令详解：

- 监控虚拟内存使用情况：`vmstat` 命令
  
  - vmstat 命令来自于 `procps-ng` 软件包
  
  - 该命令不带参数将显示自启动以来统计信息的平均值
  
  - 该命令具有多种统计汇总模式，包括 VM 模式（VM mode）、磁盘模式（disk mode）、磁盘分区模式（disk partition mode）与 slab 模式（slab mode）等，默认情况下以 VM 模式输出。
  
  - 默认情况下，命令输出的内存单位为 KiB，更改单位的选项：
    
    - `-S k` 选项：单位 KB
    
    - `-S m` 选项：单位 MB
    
    - `-S M` 选项：单位 MiB
    
    ```bash
    root@ceph-node0:~# vmstat -S M 2 5
    procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
     r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
     1  0      0    121      9    160    0    0     7   120  188  276  0  0 99  0  0
     1  0      0    121      9    160    0    0     0    19  305  474  0  0 100  0  0
     0  0      0    121      9    160    0    0     0    19  328  462  0  1 99  0  0
     0  0      0    121      9    160    0    0     0    33  327  473  0  1 99  0  0
     0  0      0    120      9    160    0    0     0    39  351  514  0  0 100  0  0
    # 默认 VM 模式输出，单位为 MiB，每隔 2 秒采样，共采样 4 次。
    ```
  
  - vmstat 命令 VM 模式输出的详细说明，如下所示：
    
    ![](pictures/vmstat-header-info.jpg)
    
    ![](pictures/linux-process-schedule.jpg)

- 🔥 sysstat 软件包相关命令：
  
  - 该软件包中主要包含的命令：mpstat、iostat、pidstat、sar 等
  
  - 监控 CPU 的使用情况：`mpstat` 命令
    
    mpstat 命令将使用 `/proc/stat` 与 `/proc/interrupts` 文件进行检索，监控统计 CPU 的使用状态。
    
    ```bash
    $ sudo mpstat -P { <cpu_list> | ALL } \
      -N { <node_list> | ALL } \
      <interval> <count>
    # mpstat 命令查看指定 CPU 核心或 NUMA 节点的使用状态
    
    $ sudo mpstat -P ALL 1 10
    # 实时监控所有 CPU 核心的使用状态，每隔 1 秒采集样本共采集 10 次。
    
    $ sudo mpstat -P 1 -N 0 -o JSON 2 5 > mpstat-dump.json
    # 实时监控 1 号逻辑 CPU、0 号 NUMA 节点的 CPU 状态，每隔 2 秒采集样本共采集 5 次，
    # 结果输出为指定 JSON 文件。
    ```
    
    ![](pictures/mpstat-demo.png)
    
    若需启用实时输出的高亮显示，可设置 `S_COLORS` 环境变量为 `always` 或 `auto`。
  
  - 监控磁盘的 I/O 使用情况：`iostat` 命令
    
    iostat 命令使用内核性能计数器统计生成两类报告：CPU 使用报告、设备使用报告
    
    ```bash
    $ sudo iostat -cdz --human 2 10
    # 每 2 秒采集样本共采集 10 次，实时监控 CPU 与磁盘设备的状态。
    # 输出中的 tps 事务数又称为 IOPS
    # 注意：
    #   -c 选项：显示 CPU 使用率报告
    #   -d 选项：显示设备使用率报告
    #   -z 选项：省略统计无任何活动的设备
    #   --human 选项：显示人类可读的容量格式
    ```
    
    ![](pictures/iostat-demo.png)
  
  - 监控进程的使用情况：`pidstat` 命令
    
    ```bash
    $ sudo pidstat -p <pid> -u -t -r <interval> <count>
    # 查看进程的状态统计信息
    # 注意：
    #   -p 选项：进程 PID
    #   -u 选项：报告 CPU 使用率
    #   -t 选项：报告页面错误（page faults）与内存使用率
    #   -r 选项：报告指定进程的线程统计情况
    ```
  
  - 系统性能监控与报告工具：`sar` 命令

### Linux 系统资源限制之 cgroup：

- 该部分内容请参看 [Linux 系统资源限制](https://github.com/Alberthua-Perl/tech-docs/blob/master/Linux%20%E7%9A%84%E5%9F%BA%E7%A1%80%E4%B8%8E%E8%BF%9B%E9%98%B6/Linux%20%E7%B3%BB%E7%BB%9F%E8%B5%84%E6%BA%90%E9%99%90%E5%88%B6.md)。

### 🔥 Linux 性能计数器之 perf：

> 以下仅为 perf 的部分说明与示例，详细内容请阅读参考链接。

- Linux 性能计数器（Performance Counters，`PCL`）是一种基于内核（kernel-based）的子系统，提供用于收集和分析性能数据的框架，它可用于监视处理器的性能。

- Linux 内核可提供 `perf_event` 接口，它是一个用于在应用程序和内核之间传递性能数据的接口。具体来说，PCL 可与 perf_event 接口一起使用，以便在应用程序中记录和监视处理器的性能。在使用 PCL 的情况下，应用程序可以使用 `pcl_register_event` 函数来注册 PCL 事件，以便在处理器性能达到预定义阈值时触发相应的事件。然后，内核会将这些事件转换为 perf_event 事件，并将它们发送到应用程序的 perf_event_buffer 中。

- 具体来说，应用程序可以使用以下步骤来使用 PCL 和 perf_event 接口：
  
  - 使用 `pcl_register_event` 函数注册 PCL 事件
  
  - 打开 perf_event_device 文件
  
  - 使用 `perf_event_mmap` 函数将 perf_event_device 文件的内存映射到应用程序的内存中
  
  - 使用 `perf_event_read` 函数从 perf_event_device 文件中读取事件数据
  
  - 将事件数据转换为 perf_event 数据结构，并将它们传递给应用程序。

> 以上 pcl_register_event 函数、perf_event_mmap 函数、perf_event_read 函数均为系统调用。

- 使用 PCL 和 perf_event 接口可以帮助应用程序更好地监视处理器性能，并提高系统的性能。

- ✨ perf 是一个基于 `Linux 2.6+` 系统的分析工具，它抽象了在 Linux 中性能度量中 CPU 的硬件差异，提供一个简单的用户空间命令行界面，它基于内核的 **`perf_events`** 接口。

- perf 可定义一组常用性事件，并提供工具来列出事件或在报告中记录下来，供日后分析报告数据。

- 以下是来自性能优化大神 [Brendan](https://www.brendangregg.com/perf.html) 对于 perf 工具在各个维度的使用概括：
  
  ![](pictures/linux-perf_events-event-sources.png)

- 关于 perf 中的事件（events）：
  
  - perf 工具支持一系列的可测量事件，该工具和底层内核接口可以测量来自不同来源的事件。
  
  - 1️⃣ 一些事件是纯粹的内核计数，在这种情况下的事件被称为 **软事件**（software events），如 context-switches、minor-faults 等。
  
  - 2️⃣ 另一个事件来源是处理器本身和它的性能监视单元（Performance Monitoring Unit，`PMU`）。它提供了一个事件列表来测量微体系结构的事件，如周期数（the number of cycles）、失效的指令（instructions retired）、L1 缓存未命中（L1 cache misses）等等。这些事件被称为 **PMU 硬件事件** 或简称为 **硬件事件**（hardware events），它们因处理器类型和型号而异。
  
  > 💥 注意：PMU 硬件事件在虚拟机内不可用，因此，perf 不能在虚拟机中用于硬件性能追踪，但是可直接安装运行于裸金属主机上。
  
  - 3️⃣ `perf_events` 接口还提供了一组通用的的硬件事件名称。在每个处理器，这些事件被映射到一个 CPU 的真实事件，若真实事件不存在则事件不能使用。可能会让人混淆，这些事件也被称为 **硬件事件** 或 **硬件缓存事件**。
  
  - 4️⃣ 最后，还有由内核 `ftrace` 基础实现的 **tracepoint 事件**。但只有 `2.6.3x` 和更新版本的内核才提供这些功能。

- 常用的 perf 子命令包括 list、`stat`、`top`、record、report、archive 等。

- perf top 子命令：
  
  ```bash
  $ sudo perf top -g
  # 查看实时的全局性能计数器概览
  
  $ sudo perf top -p <pid>
  # 查看指定进程的性能计数器概览，包括程序自身的函数、库调用、系统调用或内核函数调用等，
  # 并且可选中其中的函数调用定位汇编指令在 CPU 中的资源消耗。
  ```
  
  以下示例使用 Nginx 作为 Web 服务端，如下 ab 命令压测显示的 nginx worker 进程在 perf 中的各函数调用与 `iowrite16` 内核函数的汇编指令：
  
  ```bash
  $ ab -n 10000 -c 500 http://servera.lab.example.com/index.html
  # -n 选项指定总请求次数，-c 选项指定请求的并发数。
  ```
  
  ![](pictures/perf-top-pid-1.png)
  
  ![](pictures/perf-top-pid-2.png)

- perf stat 子命令：
  
  - 功能：
    
    用于在运行指令时，监测并分析其统计结果。虽然 perf top 也可以指定 pid，但必须先启动程序才能查看信息，而 perf stat 能完整统计程序整个生命周期的信息。
  
  - 语法：
    
    ```bash
    $ perf stat [-e <EVENT> | --event=EVENT] [-a] <command>
    $ perf stat [-e <EVENT> | --event=EVENT] [-a] - <command> [<options>]
    $ perf stat [-e <EVENT> | --event=EVENT] [-a] record [-o file] - <command> [<options>]
    $ perf stat report [-i file]
    ```
  
  - 示例：
    
    ```bash
    $ sudo perf stat
      ^C  # Ctrl+C 键盘中断
       Performance counter stats for 'system wide':
    
             99,877.41 msec cpu-clock                 #    8.000 CPUs utilized          
                49,467      context-switches          #    0.495 K/sec                  
                 2,271      cpu-migrations            #    0.023 K/sec                  
                    32      page-faults               #    0.000 K/sec                  
         8,714,198,418      cycles                    #    0.087 GHz                      (50.02%)
                     0      stalled-cycles-frontend                                       (49.98%)
                     0      stalled-cycles-backend    #    0.00% backend cycles idle      (50.03%)
                     0      instructions              #    0.00  insn per cycle           (49.98%)
                     0      branches                  #    0.000 K/sec                    (50.02%)
                     0      branch-misses             #    0.00% of all branches          (49.97%)
    
          12.485261956 seconds time elapsed 
    # 查看系统全局（system wide）性能事件
    ```
    
    ```bash
    $ sudo perf stat -a sleep 10s
    # 系统全局范围中
    $ sudo perf stat \
      -e cycles,instructions,cache-references,cache-misses,bus-cycles \
      -a sleep 10s
    # -a 选项指定系统全局范围，-e 选项指定事件，在 10s 后完成系统所有 CPU 事件的收集。
    
    $ sudo perf stat \
      -e cpu-cycles,cycles,cpu-clock,task-clock \
      -e cpu-migrations,instructions,context-switches \
      -e L1-dcache-loads,L1-dcache-load-misses \
      -e L1-icache-loads,L1-icache-load-misses \
      -e branch-misses,branch-load-misses,branch-loads \
      -e cache-misses,cache-references \
      -e dTLB-load-misses,dTLB-loads \
      -e alignment-faults,page-faults \
      -e major-faults,minor-faults \
      -e emulation-faults
      ^C
    # 实时监控收集指定的事件
    
    ### 以下数据来源于网络（非上述命令输出），仅做示例。
         672,865,652      branch-misses                                                 (30.77%)
      42,350,535,495      cache-misses              #   15.496 % of all cache refs      (30.77%)
     273,295,562,408      cache-references          #  166.215 M/sec                    (30.77%)
    3,778,399,302,053     cpu-cycles                #    2.298 GHz                      (30.77%)
     829,980,435,349      instructions              #    0.22  insn per cycle           (30.77%)
                   0      alignment-faults          #    0.000 K/sec                  
           1,756,245      context-switches          #    0.001 M/sec                  
        1,644,230.53 msec cpu-clock                 #    4.000 CPUs utilized          
                 436      cpu-migrations            #    0.000 K/sec                  
                   0      dummy                     #    0.000 K/sec                  
                   0      emulation-faults          #    0.000 K/sec                  
                 941      major-faults              #    0.001 K/sec                  
               2,320      minor-faults              #    0.001 K/sec                  
               4,081      page-faults               #    0.002 K/sec                  
        1,644,230.50 msec task-clock                #    4.000 CPUs utilized          
      42,328,918,389      L1-dcache-load-misses     #   15.50% of all L1-dcache hits    (30.77%)
     273,031,826,149      L1-dcache-loads           #  166.054 M/sec                    (30.77%)
    3,778,361,474,614     L1-dcache-stores          # 2297.951 M/sec                    (30.77%)
       5,328,248,923      L1-icache-load-misses                                         (30.77%)
         673,894,585      branch-load-misses        #    0.410 M/sec                    (30.77%)
      40,049,919,326      branch-loads              #   24.358 M/sec                    (30.77%)
       1,428,531,103      dTLB-load-misses          #    0.59% of all dTLB cache hits   (30.77%)
     244,177,528,248      dTLB-loads                #  148.506 M/sec                    (30.77%)
    
     411.058928454 seconds time elapsed
    ```
    
    对于上述命令中部分选项的说明，如下所示：
    
    ![](pictures/perf-stat-mind.jpg)
    
    > 🔥 注意：以下所示的事件可能在不同的平台上会返回 `<not supported>`
    
    - cpu-cycles：
      
      只计算程序在 CPU 上执行的 CPU 周期数，即除去中断、异常等其他周期的 CPU 周期数。
    
    - cycles：
      
      程序执行期间的总 CPU 周期数，包括指令周期、浮点周期、访存周期、中断周期、异常周期等等。
    
    - cpu-clock：CPU 消耗的时间
    
    - task-clock (msec): 
      
      CPU 处理 task 所消耗的时间，表示目标任务真正占用 CPU 的时间，单位 ms。`CPUs utilized` 表示 CPU 使用率，该值越高代表程序是 CPU bound（计算密集型）而非 IO bound（I/O密集型），除去等待 I/O 与其他阻塞操作的时间。
    
    - cpu-migrations：进程运行过程中从一个 CPU 迁移到另一个 CPU 的次数
    
    - `instructions`：
      
      执行的指令条数。insns per cycle：即 `IPC`，每个 CPU 周期执行的指令条数，IPC 比上面的 CPU 使用率更能说明 CPU 的使用情况（很多指令需要多个处理周期才能执行完毕），IPC 越大越好，说明程序充分利用了处理器的特征。
    
    - alignment-faults：
      
      统计内存对齐错误发生的次数，当访问的非对齐的内存地址时，内核会进行处理，已保存不会发生问题，但会降低性能。
    
    - branches：这段时间内发生分支预测的次数。现代的 CPU 都有分支预测方面的优化。
    
    - branch-instructions：分支预测成功次数
    
    - branch-misses：这段时间内分支预测失败的次数，该值越小越好。
    
    - cache-references：cache 命中次数
    
    - cache-misses：cache 失效次数
    
    - `context-switches`：
      
      上下文切换次数，前半部分是切换次数，后面是平均每秒发生次数（M 是 10^6 次方）。
    
    - L1-dcache-loads：L1 数据缓存读取次数
    
    - L1-dcache-load-missed：L1 数据缓存读取失败次数
    
    - LLC-loads：末级缓存读取次数
    
    - LLC-load-misses：末级缓存读取失败次数
    
    > LLC 表示 Last-Level Cache（最后一级缓存）是 CPU Cache 层级结构中的最后一级缓存，也称为智能缓存或共享缓存，有时也称为 L3 缓存。
    
    - major-faults：页错误，内存页已经被换出到硬盘上，需要将页面换入。
    
    - minor-faults：页错误，内存页在物理内存中，只是没有和逻辑页进行映射。
    
    - 💥 `page-faults`：
      
      缺页异常的次数。当程序请求的页面尚未建立、请求的页面不在物理内存中，或请求的页面虽然在内存中，但物理地址和虚拟地址的映射关系尚未建立时，都会触发一次缺页异常。另外 `TLB` 不命中，页面访问权限不匹配等情况也会触发缺页异常。
    
    - stalled-cycles-frontend 和 stalled-cycles-backend：CPU 停滞统计
    
    - XXX seconds time elapsed：程序持续时间

- perf record 与 report 子命令记录与报告事件：
  
  ```bash
  $ sudo perf record -o cs-syswide.data -e context-switches -a sleep 10s
  # 系统全局范围内记录 context-switches 事件，记录时间为 10s，并将结果写入指定文件。
  $ sudo perf report -i cs-syswide.data [--stdio]
  # 交互模式查看记录的事件
  # --stdio 选项：标准输出中查看事件
  ```

### kernel 相关软件包下载：

- 与系统诊断与优化的部分工具依赖内核的版本，如 perf、SystemTap 等，因此在使用此类工具时需安装内核相关的软件包，并且其版本必须与当前系统内核完全一致。

- 相关的内核软件包安装可参考 [How can I download or install kernel debuginfo packages for RHEL systems?](https://access.redhat.com/solutions/9907#masthead)

### 进程的调度与优先级：

- 📜 `man 7 sched` 命令查看进程调度的策略与系统调用

- Linux 中的进程调度策略：
  
  - 实时调度策略（real-time scheduling policy）
    
    - 由实时调度策略调度的进程需在限制的时间内完成任务，常见的策略有 `FF`。
    
    - 实时调度的进程比非实时调度的进程获得更多的 CPU 使用时间，由于其需要在有限的时间内完成并返回。
  
  - 非实时调度策略（non-real-time scheduling policy）
    
    常见的策略有 `TS`

- Linux 进程调度优先级的分类：
  
  ![](pictures/linux-process-priorities.jpg)
  
  - 系统优先级（system priority）
  
  - 实时优先级（real-time priority）
  
  - 过时的优先级（obsolete priority）：`/ˈɒbsəliːt/`
    
    该优先级类型为 `BSD` 风格类型，使用 ps 命令的 `l` 选项查看，其优先级的值与 `pri` 选项不同。

- Linux 中的上下文切换（context switch）：
  
  - 上下文切换的过程：
    
    ![](pictures/linux-context-switch-1.png)
    
    ![](pictures/linux-context-switch-2.png)
    
    - 当一个程序正在执行的过程中，中断（interrupt）或系统调用（system call）发生可以使得 CPU 的控制权从当前进程转移到操作系统内核。
    
    - 操作系统内核负责保存进程 $P_1$ 在 CPU 中的上下文到 $PCB_1$（PCB 即为进程的 `task_struct` 结构体）中。
    
    - 从 $PCB_2$ 取出进程 $P_2$ 的 CPU 上下文，将 CPU 控制权转移给进程 $P_2$， 开始执行进程 $P_2$ 的指令。
  
  - 发生上下文切换的 3 中场景：
    
    - 1️⃣ 进程无法运行下去：
      
      如等待 IO 完成，或者等待某个资源、某个事件等。
    
    - 2️⃣ 进程仍在运行，但内核不让它继续使用 CPU：
      
      如进程的时间片用完，或者优先级更高的进程抢占，因此该进程必须交出 CPU 的使用权。
    
    - 3️⃣ 进程还可运行，但其自身的算法决定主动交出 CPU 于别的进程：
      
      用户程序可以通过系统调用 `sched_yield()` 来交出 CPU，内核则可以通过函数 `cond_resched()` 或者 `yield()` 来做到。
  
  - 上下文切换又可分为：
    
    - 自愿上下文切换（voluntary /ˈvɒləntri/）
    
    - 非自愿上下文切换（involuntary）或强制切换
  
  > 自愿上下文切换时进程不再处于运行状态，而非自愿上下文切换时进程仍处于运行状态。
  
  - 以上场景 1️⃣ 属于自愿上下文切换，而场景 2️⃣ 与 3️⃣ 属于非自愿上下文切换。

- ps 命令常用参数示例：
  
  ```bash
  $ sudo ps axo pid,pri,rtprio,ni,cls,cputime,psr,comm
      PID PRI RTPRIO  NI CLS     TIME PSR COMMAND
        1  19      -   0  TS 00:00:10   0 systemd
        2  19      -   0  TS 00:00:00   1 kthreadd
        3  39      - -20  TS 00:00:00   0 rcu_gp
        4  39      - -20  TS 00:00:00   0 rcu_par_gp
        6  39      - -20  TS 00:00:00   0 kworker/0:0H-kblockd
        8  39      - -20  TS 00:00:00   0 mm_percpu_wq
        9  19      -   0  TS 00:00:00   0 ksoftirqd/0
       10  19      -   0  TS 00:00:01   1 rcu_sched
       11 139     99   -  FF 00:00:00   0 migration/0
       12 139     99   -  FF 00:00:00   0 watchdog/0
       13  19      -   0  TS 00:00:00   0 cpuhp/0
       14  19      -   0  TS 00:00:00   1 cpuhp/1
       15 139     99   -  FF 00:00:00   1 watchdog/1
       16 139     99   -  FF 00:00:00   1 migration/1 
       ...
       1677  19      -   0  TS 00:00:00   1 nginx
       1678  19      -   0  TS 00:00:00   0 nginx
       1679  19      -   0  TS 00:00:00   1 nginx
       ...
  # 查看系统所有进程的 PID、系统优先级、实时优先级、nice 值、cls 调度策略、
  # CPU 实际使用时间（realtime）、进程当前被分配的逻辑 CPU 与对应的命令
  
  $ cat /proc/<pid>/status
    ...
    voluntary_ctxt_switches:        1  # 自愿上下文切换数目
    nonvoluntary_ctxt_switches:     4  # 非自愿上下文切换数目
  # voluntary_ctxt_switches, nonvoluntary_ctxt_switches: since Linux 2.6.23
  # 以上数目是进程被调度运行后的累加值
  # 注意：
  #   自愿上下文切换数目占多数说明进程相对 CPU 的资源需求不高，而非自愿上下文切换数目较多
  #   的话，需考虑其对 CPU 的资源需求可能存在瓶颈，被内核强制调度切换。
  ```
  
  从上述 ps 命令的输出中可知：
  
  - CLS 调度策略为 TS（SCHED_NORMAL）的进程不具有实时优先级（RTPRIO）
  
  - ✨ 进程具有最高的实时优先级（99）而具有最低的系统优先级（139）彼此间不矛盾，因为实时优先级是进程在限定时间内需完成的任务可，需抢占 CPU 资源快速完成任务，而任务完成后不再占用 CPU 资源具有较低的系统的优先级，此类进程通常为内核进程。

### CPU 时钟周期、机器周期、指令周期的关系：

- CPU 的最小时间单位是时钟周期，而一个机器周期包括若干个时钟周期，而指令周期，则包含若干个机器周期。

- 🤘 按粒度排序：**指令周期 > 机器周期 > 时钟周期**

- 时钟周期（Clock Cycle）：
  
  - 时钟周期也称为 **振荡周期**，定义为 **振荡频率** 的倒数。
  
  - 时钟周期是计算机中 CPU 的最基本、最小的时间单位。
  
  - 在一个时钟周期内，CPU 仅完成一个最基本的动作。

- 机器周期（Machine Cycle）：
  
  - 机器周期也称为 CPU 周期（CPU Cycle），在计算机中，为了便于管理，常把一条指令的执行过程划分为若干个阶段。
  
  - 如，取指令、存储器（即主存）读、存储器写等，这每一项工作称为一个 **基本操作**（注意：每一个基本操作都是由若干 CPU 最基本的动作组成）。完成一个 **基本操作** 所需要的时间称为机器周期。通常用内存中读取一个指令的最短时间来规定 CPU 周期。

- 指令周期（Intruction Cycle）：
  
  - 计算机从取指令到指令执行完毕的时间。

- 💻 一个完整的指令周期可包含以下五个阶段：
  
  ![](pictures/instruction-cycle.jpg)
  
  - 取指令（Instruction Fetch）：
    
    - CPU 从指令寄存器指向的内存地址读取指令。
    
    - 指令寄存器也称为程序计数器（Program Counter，`PC`），其主要作用是 CPU 根据指令地址从内存里把具体的指令加载到指令寄存器中，并存储下一条要执行的指令在内存中的地址。当 CPU 执行完当前指令后，会从指令寄存器中读取下一条指令的地址，并将其加载到指令寄存器中，然后开始执行下一条指令。
  
  - 指令译码（Instruction Decode）：
    
    CPU 解码取出的指令，并确定执行该指令所需要的操作。此阶段还会确定读取操作数的地址，并把它们传送到执行阶段。
  
  - 执行指令（Execute）：
    
    CPU 执行指令并计算结果。操作数在指令译码阶段被读取并在执行阶段进行计算。
  
  - 访问存储器（Memory Access）：
    
    若当前指令需要访问内存，则在该阶段中访问内存。
  
  - 写回数据（Write Back）：
    
    最终计算结果被写回到 CPU 的寄存器或者内存。
  
  - 以上五个阶段组成一个指令周期。需要注意的是，不是所有的指令都需要以上五个阶段全部执行，一些比较简单的指令可能只需要执行前三个或者前四个阶段。而有些复杂的指令可能需要多个指令周期才能完成。

- 对于一个指令周期来说，取出一条指令，然后执行它，至少需要两个 CPU 周期。取出指令至少需要一个 CPU 周期，执行至少也需要一个 CPU 周期，复杂的指令则需要更多的 CPU 周期。而一个 CPU 周期是若干时钟周期之和。
  
  ![](pictures/three-cycle-relationship.jpg)

### x86_64 架构的常用寄存器示例：

- x86_64 架构是 x86 架构的 64 位扩展，它包括了一些与 32 位版本不同的寄存器，以下列举了 x86_64 架构中常用寄存器的全称和简称：
  
  - 通用寄存器 (General-Purpose Registers)：
    
    - RAX (Accumulator Register)
    
    - RBX (Base Register)
    
    - RCX (Counter Register)
    
    - RDX (Data Register)
    
    - RSI (Source Index Register)
    
    - RDI (Destination Index Register)
    
    - RBP (Base Pointer Register)
    
    - RSP (Stack Pointer Register)
    
    - R8-R15：新增 8 个通用寄存器，即 R8、R9、R10、R11、R12、R13、R14、R15。
  
  - 程序计数器 (Program Counter Registers, PC)：
    
    - RIP (Instruction Pointer Register)：
      
      存储 CPU 正在执行的指令的地址。
  
  - 标志寄存器 (Flags Registers)：
    
    - RFlags (Flags Register)：
      
      存储 CPU 状态的二进制标志，包含了 x86_32 架构中的 EFlags 寄存器。
  
  - 💪 控制寄存器 (Control Registers)：
    
    - CR0 (Control Register 0)：
      
      用于开启和关闭虚拟内存、保护模式和分页机制等功能的控制寄存器。
    
    - CR2 (Control Register 2)：
      
      存储最近一次的访问内存产生的缺页异常的地址值。
    
    - **`CR3`** (Control Register 3)：
      
      CR3 寄存器在 x86 架构中称为 **页表基址寄存器**（Page Table Base Register，`PTBR`），它存储的是操作系统在虚拟内存管理中的页表的基地址（起始地址）。页表基址寄存器是在页表实现中的核心组成部分，通过 PTBR 可以定位到页表在内存中的位置，使 CPU 能够正确地将虚拟地址转换为物理地址。在进程切换时，操作系统会更新 PTBR 中的值，以指向相应进程的页表（每个进程都有一个独立的页表）。
    
    - CR4 (Control Register 4)：
      
      用于激活一些高级的 CPU 功能，如调试寄存器、全局页面等。
    
    - CR8 (Control Register 8)：
      
      用于控制中断的优先级。

- 在 x86_64 架构中，32 位的寄存器名称的前缀由 E 改为 R，如，RAX、RBX、RCX 等。此外，x86_64 架构还引入了 8 个新增的通用寄存器 R8-R15。这些寄存器的作用和 32 位的通用寄存器一样，可以用于存储数据和进行各种运算。

### CPU Cache 缓存架构：

- 计算机存储体系整体分层示意：
  
  ![](pictures/compute-storage-arch.jpg)

- CPU Cache 缓存架构拓扑示例：
  
  使用 `lstopo` 命令以获取如下拓扑，分别来自于 Intel Core i5 与 i7 处理器。
  
  ![](pictures/foundation0-cpu-topo.png)
  
  ![](pictures/lenovo-t580-cpu-cache-topo.png)
  
  ![](pictures/dell-poweredge-r720-cpu-topo.png)
  
  ```bash
  $ sudo yum install -y hwloc-gui
  # 安装基于 GUI 的系统拓扑信息软件包
  
  $ sudo lstopo --logical --no-io
  Machine (9951MB total) + Package L#0
    NUMANode L#0 (9951MB)
    L3 L#0 (3072KB)
      L2 L#0 (256KB) + L1d L#0 (32KB) + L1i L#0 (32KB) + Core L#0 + PU L#0
      L2 L#1 (256KB) + L1d L#1 (32KB) + L1i L#1 (32KB) + Core L#1 + PU L#1
      L2 L#2 (256KB) + L1d L#2 (32KB) + L1i L#2 (32KB) + Core L#2 + PU L#2
      L2 L#3 (256KB) + L1d L#3 (32KB) + L1i L#3 (32KB) + Core L#3 + PU L#3
  # 查看主机的 CPU 逻辑拓扑信息
  
  $ sudo sudo lstopo --physical --no-io
  Machine (9951MB total) + Package P#0
    NUMANode P#0 (9951MB)
    L3 (3072KB)
      L2 (256KB) + L1d (32KB) + L1i (32KB) + Core P#0 + PU P#0
      L2 (256KB) + L1d (32KB) + L1i (32KB) + Core P#1 + PU P#1
      L2 (256KB) + L1d (32KB) + L1i (32KB) + Core P#2 + PU P#2
      L2 (256KB) + L1d (32KB) + L1i (32KB) + Core P#3 + PU P#3 
  # 查看主机的 CPU 物理拓扑信息
  
  $ sudo lstopo --no-io
  Machine (9951MB total) + Package L#0
    NUMANode L#0 (P#0 9951MB)
    L3 L#0 (3072KB)
      L2 L#0 (256KB) + L1d L#0 (32KB) + L1i L#0 (32KB) + Core L#0 + PU L#0 (P#0)
      L2 L#1 (256KB) + L1d L#1 (32KB) + L1i L#1 (32KB) + Core L#1 + PU L#1 (P#1)
      L2 L#2 (256KB) + L1d L#2 (32KB) + L1i L#2 (32KB) + Core L#2 + PU L#2 (P#2)
      L2 L#3 (256KB) + L1d L#3 (32KB) + L1i L#3 (32KB) + Core L#3 + PU L#3 (P#3) 
  # 查看主机的 CPU 逻辑与物理拓扑信息
  # 注意：
  #   PU 表示物理处理单元 (Pysical Processing Unit)，指的是映射到物理处理器的标识符。
  #   P# 表示处理器 (Package) 标识符，也是映射到物理处理器上的标识符。
  ```

- 使用 `valgrind` 命令测试程序的 CPU Cache 在 `L1` 缓存与 `LL` 缓存（最慢的一级缓存或最后一级缓存）中的命中率，为程序的优化提供相应线索：
  
  ```bash
  $ man valgrind
  # 查看 valgrind 命令的使用方法
  
  $ sudo valgrind --tool=cachegrind ./cache-test1
  ==1798== Cachegrind, a cache and branch-prediction profiler
  ==1798== Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote et al.
  ==1798== Using Valgrind-3.14.0 and LibVEX; rerun with -h for copyright info
  ==1798== Command: bin/cache-test1
  ==1798== 
  --1798-- warning: L3 cache found, using its data for the LL simulation.    # 使用 L3 缓存作为 LL 缓存
  Starting
  Finished
  ==1798== 
  ==1798== I   refs:      6,750,717,128    
  ==1798== I1  misses:            1,011    # L1 指令缓存未命中数
  ==1798== LLi misses:            1,002    # LL 指令缓存未命中数
  ==1798== I1  miss rate:          0.00%   # L1 指令缓存未命中率（I1 misses/I refs）
  ==1798== LLi miss rate:          0.00%   # LL 指令缓存未命中率（LLi miss rate/I refs）
  ==1798== 
  ==1798== D   refs:      3,937,858,532  (3,375,270,491 rd   + 562,588,041 wr)  
  ==1798== D1  misses:       35,159,402  (        2,516 rd   +  35,156,886 wr)  # L1 数据缓存未命中数
  ==1798== LLd misses:       35,158,984  (        2,126 rd   +  35,156,858 wr)  # LL 数据缓存未命中数 
  ==1798== D1  miss rate:           0.9% (          0.0%     +         6.2%  )  # L1 数据缓存未命中率（D1 misses/D refs）
  ==1798== LLd miss rate:           0.9% (          0.0%     +         6.2%  )  # LL 数据缓存未命中率（LLd misses/D refs）
  ==1798== 
  ==1798== LL refs:          35,160,413  (        3,527 rd   +  35,156,886 wr)
  ==1798== LL misses:        35,159,986  (        3,128 rd   +  35,156,858 wr)
  ==1798== LL miss rate:            0.3% (          0.0%     +         6.2%  )
  ```

- CPU Cache 缓存命中率的 C 程序代码示例：
  
  左侧为 cache1.c，右侧为 cache2.c。
  
  ![](pictures/cpu-cache-valgrind-test-demo.png)
  
  以上代码用以定义一个 7500 个行元素与 7500 个列元素的二维数组。左侧示例先定义行，在每行中以列进行递增，每行中的列元素的值是整型变量 `i`（行的索引号）与整型变量 `j`（列的索引号）的乘积，而右侧示例先定义列，在每列中以行进行递增。因此，两者编译后使用 valgrind 命令进行缓存命中率测试，结果如下所示：
  
  ![](pictures/cpu-cache-valgrind-test-1.png)
  
  ![](pictures/cpu-cache-valgrind-test-2.png)
  
  从测试结果可知 cache1 的 L1 数据写缓存未命中率（6.2%）明显低于 cache2 的（100.0%），其原因在于 CPU Cache 缓存以缓存行（Cache line）的方式进行存储，先定义行再以列进行递增的效率更高。

### 参考链接：

- [如何理解 CPU steal time](https://www.cnblogs.com/my-show-time/p/15893877.html)

- ❤ [Linux kernel profiling with perf](https://perf.wiki.kernel.org/index.php/Tutorial)

- [Linux 性能分析工具 Perf 简介](https://segmentfault.com/a/1190000021465563)

- [进程切换：自愿 (voluntary) 与强制 (involuntary)](http://linuxperf.com/?p=209)
