# 📚 Kimi 提问汇总

## 0. 目录

- [📚 Kimi 提问汇总](#-kimi-提问汇总)
  - [0. 目录](#0-目录)
  - [1. Linux 内核原理](#1-linux-内核原理)
  - [2. Linux 系统编程](#2-linux-系统编程)
  - [3. Linux 性能优化](#3-linux-性能优化)
  - [4. Linux 网络](#4-linux-网络)
    - [4.1 Linux ARP 特性](#41-linux-arp-特性)
  - [5. 系统组件](#5-系统组件)
  - [6. 硬件监测](#6-硬件监测)
  - [7. 系统服务](#7-系统服务)
  - [8. 系统安全](#8-系统安全)
  - [9. Ceph](#9-ceph)
  - [10. Jenkins](#10-jenkins)
  - [11. 容器 \& Kubernetes](#11-容器--kubernetes)
    - [11.1 K8s NetworkPolicy 与 Calico NetworkPolicy 间的关系](#111-k8s-networkpolicy-与-calico-networkpolicy-间的关系)
  - [12. OpenShift](#12-openshift)
  - [13. CNI 如何玩转 Linux Bridge](#13-cni-如何玩转-linux-bridge)
  - [14. Python](#14-python)
  - [15. 机器学习](#15-机器学习)
  - [16. 深度学习](#16-深度学习)
  - [17. Transformer](#17-transformer)
  - [18. 大模型基础](#18-大模型基础)
  - [19. vLLM 推理](#19-vllm-推理)

## 1. Linux 内核原理

- Linux 内核中 head_64.S 的解读？
- Linux 从加电启动 → start 内核 → 用户空间过程的源码分析？
- Linux 内核模块参数设置后，没有在 parameters 中显示，是什么原因？module_param() 声明在哪里查看？
- 如何理解 Linux 中 dmesg 命令的 ring buffer？
- x86_64 模式下 Linux 中的 “段”：
  - 在 x86_64 下 Linux 早已把 “段” 晾在一边 —— 段只起恒等映射作用，既不参与地址分配，也不提供隔离；
  - 所有 “虚拟地址空间” 的划分、保护、换入换出完全由页表（页机制）包办。
  - 段与页的关系是：段负责 “把逻辑地址→线性地址” 这一步，但 64-bit 模式下它被固定成 0 基址、无限长，因而这一步恒等；真正的内存管理发生在页表，把线性地址→物理地址。
  - 硬件把段基址强制设为 0，限长视为无限 → 逻辑地址 = 线性地址（恒等映射，段部件不做任何算术）。
  - 因此用户代码里看到的地址就是线性地址，Linux 只通过四级/五级页表把它映射到物理页；分段机制既不参与分配，也不提供隔离。
  - 结论：“分段” 在 64-bit Linux 中名存实亡，所有虚拟地址 ↔ 物理地址的转换完全由分页机制完成。

- 🎉 Linux 对物理内存划分的图谱？

  | 物理地址区间 (示例) | Zone 类型 | 内核虚拟窗口 | 主要用途 / 备注 |
  | ----- | ----- | ----- | ----- |
  | 0 ~ 1 MB | ZONE\_DMA | `__va(x)` 直接映射 | ISA/老式 PCI 16-bit DMA；**预留** |
  | 1 MB ~ 896 MB | ZONE\_NORMAL | `__va(x)` 直接映射 | 内核代码、数据、slab、伙伴热页；**>90% 内核分配** |
  | 896 MB ~ 4 GB (可选) | ZONE\_DMA32 | `__va(x)` 直接映射 | 32 位设备 DMA；64 位系统才有 |
  | > 4 GB ~ 物理顶 | ZONE\_MOVABLE + 剩余 NORMAL | `__va(x)` 直接映射 | 用户进程、page cache、匿名页、CMA、热插拔页；**大页/巨页**也在这里 |
  | 任意离散页框 | **无 Zone** | **vmalloc 区** (新建页表) | 模块、ioremap、大缓冲区、per-cpu、kmap\_atomic 临时映射 |
  | 任意页框 | **无 Zone** | **vmemmap 区** (线性数组) | `struct page` 描述符仓库，一一对应物理页 |
  | 设备 MMIO / 保留区 | ZONE\_DEVICE | **ioremap 窗口** | PCIe 寄存器、ROM、ACPI、IOMMU 窗口；**非 RAM** |
  | 低 1 MB 预留 | **无 Zone** | **固定映射区** | APIC、IOAPIC、HPET、early ioremap；**编译期常量 VA** |

  64 位 Linux 把物理内存切成 DMA → NORMAL → MOVABLE 三带，全部通过 128 TB 直接映射区实现零成本内核寻址；</br>
  离散或设备用途再走 vmalloc/ioremap 另建页表 —— 物理地址→Zone→虚拟窗口三张地图一一对应，构成内核视角的完整物理内存图谱。

- 🔥 Linux 内核如何为自己分配物理内存？如何为用户空间进程分配物理内存？
  - 把 Linux 内核的内存分配机制拆成两条主线：
    - “内核给自己用” —— 内核地址空间
    - “内核给用户进程用” —— 用户地址空间
  - 内核为自己分配内存（Kernel Space）
  
    | 场景 | 核心 API | 物理页来源 | 虚拟地址 | 释放/归还 |
    | ----- | ----- | ----- | ----- | ----- |
    | **伙伴系统** 大块连续页 | `alloc_pages(gfp_mask, order)` | 伙伴系统 2ⁿ 页框 | 固定线性映射区（直接映射）或临时 `kmap` | `__free_pages()` |
    | **slab/slub** 小对象 | `kmalloc(size, gfp)` → 走 slab 缓存 | slab 把伙伴页切成对象 | 直接映射区 | `kfree()` |
    | **vmalloc** 非连续大块 | `vmalloc(size)` | 每次一页，物理不连续 | vmalloc 区（直接映射之外） | `vfree()` |
    | **per-CPU 变量** | `alloc_percpu(type)` | 每 CPU 一段 | 每 CPU 固定线性映射 | `free_percpu()` |
    | **bootmem/memblock** 早期 | `memblock_alloc()` | 启动早期预留 | 静态映射 | 启动完成后转交伙伴系统 |

- 🔥 Linux 内核自身的内存管理在物理内存上是如何组织分布的？
  - Linux 内核在初始化阶段就把整台机器的物理 RAM 抽象成一张 “多级目录表”，随后所有分配/回收动作都在这张表里完成。
  - “自顶向下” 的顺序：从 NUMA 节点 → 内存域（Zone）→ 页帧（Page Frame）→ 每 CPU/伙伴/Slab 三级缓存
  - Linux 把物理 RAM 先切成 NUMA 节点，节点内再切成 Zone，Zone 里用 **伙伴系统** 管理 2^n 页块，每页对应一个 `struct page`；小对象再由 **slab** 切分，形成 “节点-域-页-对象” 四级物理内存版图。
- 🔥 Linux 内核自身运行所在的内存与用户空间程序运行所在的内存有何区别？
  - Linux 内核永远跑在 **高地址、特权级、固定映射** 的那一段内存里；所有用户态进程只能呆在 **低地址、非特权、按需映射** 的另一半。两边用页表+特权级完全隔离，互不干扰。
  
  | 维度 | 内核空间 (Kernel Space) | 用户空间 (User Space) |
  | ----- | ----- | ----- |
  | **虚拟地址范围** | x86_64：ffff 8000 0000 0000 以上（128 TiB 高地址）<br>x86-32：0xC000 0000 ~ 0xFFFF FFFF（高 1 GB）| x86_64：0000 0000 0000 0000 ~ 0000 7FFF FFFF FFFF（128 TiB 低地址）<br>x86-32：0x0000 0000 ~ 0xBFFF FFFF（低 3 GB）|
  | **特权级** | Ring 0，可直接执行特权指令、访问所有硬件寄存器 | Ring 3，任何 I/O 或特权指令都会触发 #GP → 进入内核 |
  | **页表** | 所有进程共享一份 “主内核页表”；当 CPU 运行在 Ring 0 时，页表项 PGD 高位恒映射内核 | 每个进程独立一份用户页表；切进程 ⇒ 切 CR3 ⇒ 切换用户页表，但高位内核映射不变 |
  | **物理内存来源** | 启动早期一次性把低端物理页“线性映射”到高地址；高端内存临时用 `kmap` | 通过 `brk/mmap` 向伙伴系统申请，按需建立/撤销映射，可换出到 swap |
  | **内存保护** | 页表始终可读可写；没有 page-fault 换出概念（swappable=No）| 页表项有 R/W/X、Dirty/Accessed、swap 位；可被换出、写保护、COW |
  | **异常/中断** | 自己处理自己；中断向量表、IDT、TSS 都在内核段 | 任何中断/异常 CPU 自动切到 Ring 0 入口，内核保存用户寄存器后处理 |

  - 32 位内核的 高端内存（HIGHMEM）是内核自己也要临时映射才能访问的 “用户可用物理页”；64 位因地址空间巨大，已取消 HIGHMEM。
  - 用户调用 read()、malloc() 等库函数时，CPU 通过 syscall/sysenter 指令从 Ring 3 跳到 Ring 0，栈也瞬间切换到内核栈；返回时再用 sysret/iret 回到 Ring 3，继续跑用户代码。
  - cat /proc/<pid>/maps 只能看到用户地址；sudo cat /proc/kallsyms 才能看到内核符号地址，印证了 “用户看不见内核”。
  - 总结：
    - 用户空间 = 低地址 + Ring 3 + 按需映射
    - 内核空间 = 高地址 + Ring 0 + 永远映射
    - 两边地址空间重叠但权限隔离，用户态永远触碰不到内核那半张页表。

- CR3 寄存器是什么结构？
- 从 CR3 寄存器开始的线性地址到物理地址的寻址过程？
- Linux 用户空间的每个进程有各自的页表，那么 CR3 寄存器中的基址是如何确定的呢？CR3 寄存器中的基址是如何更新的呢？
  - CR3 里的“基址”就是当前进程顶级页表（PGD/PML4）那一页的物理地址；
  - 每次调度器挑中新进程时，内核把 task_struct->mm->pgd_paddr 写进 CR3，硬件立刻用新页表树做地址转换。
  - CR3 基址 = 当前进程顶级页表页的物理地址；
  - 只在调度器切任务时由内核写一次，硬件立刻按新地址开始查表。
- 如果进程被杀死，CR3 寄存器的基址会怎样变化？
  - 进程被杀死时，CR3 本身不会被立即改写；
  - **只有 “调度器再次选中新进程” 那一刻才会把新任务的顶级页表物理地址写进 CR3。**
  - 中间这段时间 CPU 继续用旧进程的页表运行 —— 但已处于内核态，不会访问用户空间，因此无害。
- Linux 内核是如何操作 MMU 和 TLB，请详述过程？
- Linux 内核中操作 MMU 的代码是什么？

  Linux 内核中操作 MMU 的代码分布在启动初始化、进程切换、缺页处理、TLB 刷新四个关键环节，下面按真实源码位置逐层拆解。

  | 功能 | 文件路径 |
  | ----- | ----- |
  | 启动建页表、开 MMU | `arch/x86/kernel/head_64.S` / `arch/arm/mm/proc-v6.S` |
  | 进程切换写 CR3/TTBR | `arch/x86/include/asm/mmu_context.h` / `arch/arm64/include/asm/mmu_context.h` |
  | 缺页建表 | `arch/x86/mm/fault.c` → `mm/memory.c` |
  | TLB 刷新 | `arch/x86/include/asm/tlbflush.h` / `arch/x86/mm/tlb.c` |
  | 直接映射宏 | `arch/x86/include/asm/page_64.h` |
  | 页表项操作 | `include/asm-generic/pgtable.h` |

  Linux 操作 MMU 的代码 = "启动时填页表 → 开分页 → 切进程写 CR3 → 缺页补页表 → 改表后刷 TLB" 五个动作；</br>
  MMU 本身（四级遍历、TLB 查询）是纯硬件自动完成，内核只负责维护页表内容和触发刷新时机 

- 详解 /proc/meminfo 文件中的各项参数？
- 以下 Linux 内核 call trace 代表的含义是？每个符号与后面的十六进制分别代表是？
  
  ```plaintext
  [Sat Jun  7 16:54:14 2025] Call Trace:
  [Sat Jun  7 16:54:14 2025]  <TASK>
  [Sat Jun  7 16:54:14 2025]  dump_stack_lvl+0x34/0x48
  [Sat Jun  7 16:54:14 2025]  dump_header+0x4a/0x201
  [Sat Jun  7 16:54:14 2025]  oom_kill_process.cold+0xb/0x10
  [Sat Jun  7 16:54:14 2025]  out_of_memory+0xed/0x2e0
  [Sat Jun  7 16:54:14 2025]  __alloc_pages_slowpath.constprop.0+0x6e8/0x960
  [Sat Jun  7 16:54:14 2025]  __alloc_pages+0x21d/0x250
  [Sat Jun  7 16:54:14 2025]  __folio_alloc+0x17/0x50
  [Sat Jun  7 16:54:14 2025]  ? policy_node+0x4f/0x70
  [Sat Jun  7 16:54:14 2025]  vma_alloc_folio+0xa3/0x390
  [Sat Jun  7 16:54:14 2025]  do_anonymous_page+0x63/0x520
  [Sat Jun  7 16:54:14 2025]  __handle_mm_fault+0x32b/0x670
  [Sat Jun  7 16:54:14 2025]  ? nohz_balancer_kick+0x31/0x250
  [Sat Jun  7 16:54:14 2025]  handle_mm_fault+0xcd/0x290
  [Sat Jun  7 16:54:14 2025]  do_user_addr_fault+0x1b4/0x6a0
  [Sat Jun  7 16:54:14 2025]  ? sched_clock_cpu+0x9/0xc0
  [Sat Jun  7 16:54:14 2025]  exc_page_fault+0x62/0x150
  [Sat Jun  7 16:54:14 2025]  asm_exc_page_fault+0x22/0x30
  ```

  - 以 Linux 内核 call trace 中的 do_anonymous_page+0x63/0x520 为例，请给出 addr2line 定位源代码行号的示例？
  - 那么上述函数的长度在定位源码行数的过程中没有实际作用吗？
  - 以上示例中，什么是函数入口地址、相对地址与绝对地址？

## 2. Linux 系统编程

- 二进制、十进制、八进制与十六进制数据间的转换？
- 请编写多个 C 语言程序示例，并使用 makefile 完成编译。
- 为什么 C 程序中函数传参都使用了字符串指针而不是直接使用字符串，这样有何好处？
- char *s 这里的 s 是字符串数组，而 *s 是指向 s 的指针？
- 什么是函数声明与定义中的形参与实参？
- intel 8086 CPU 与内存间的地址总线数量是多少？
- intel x86_64 CPU 与内存间的地址总线数量是多少？
- 编写 C 程序，用于拷贝文件数据。如果文件过大，效率会降低，是否可将原数据文件进行拆分，再做拷贝，应该如何实现？如何利用多线程或者异步 I/O 优化上述程序？
- 给出一个示例，使用 linux io_uring 来实现大文件的拷贝？
- io_uring 相比传统的读写操作有哪些优势？

## 3. Linux 性能优化

- Linux 中 PCP 的 pmlogger 默认是采集所有的性能指标吗？是否可以自定义只需要的性能指标，并且采集的时间间隔能指定吗？
- 磁盘 IO 请求的合并是由内核调度算法来完成的吗？和磁盘驱动有关系吗？

## 4. Linux 网络

- 如何在 RHEL9.6 中部署测试 DPDK+SR-IOV？

### 4.1 Linux ARP 特性

- `LVS DR` 模式
- Linux 主机有这么一个特性，假设我们的主机上有两块网卡，比如 eth0、eth1，当 `arp` 请求 eth1 的 `mac` 地址的时候，eth1 会答复，这个是理所当然的，但是 eth0 也会 "好心" 的帮 eth1 回答这个 arp 请求。要防止这样的话，就需要更改下我们的一些内核参数：
  
```bash
net.ipv4.conf.lo.arp_ignore = 1
net.ipv4.conf.all.arp_ignore = 1
```

- 正常情况下只写第二条就好，`all` 指所有设备的 `interface`，当 all 和具体的 interface 比如 lo，按照最大的值生效。
- 另外一个 Linux 的特性就是，对于一个从 `realserver` 发出的 `arp` 请求，其源 IP 是 `VIP`，而出口不会是 lo，这里假设是 eth0，那么这个 arp 请求包里面，源 IP 就是 VIP，源 mac 是 eth0 的 mac，目的 IP 是网关，那么路由器在接收到这个请求的时候，会将将自己的相应接口的 mac 地址放在 arp 响应包中，同时将请求包中的源 IP 及 mac 放在 arp 高速缓存中，那这下可就乱套了，就会使真正的 VIP 得不到正确的请求了。
- 这是因为，正常的情况下，arp 的请求中源 IP 是出去的所在接口的地址，mac 也是出去的接口的 mac，但 Linux 在默认情况下却不是这样的，如果一个接口发出的 arp 请求须经另一个出口出去的时候，源 IP 就不是所出去接口的 IP，那么将内核参数设置为 2 相应的解决了这个问题。
  
```bash
net.ipv4.conf.lo.arp_announce = 2
net.ipv4.conf.all.arp_announce = 2
```

- net.ipv4.conf.all.proxy_arp = 1

## 5. 系统组件

- systemd 的 service 单元文件 [Service] 部分中的 Type 类型介绍？
- D-Bus 框架的工作原理？它与 socket 编程有何不同？
- 如何编写 2 个 python 程序利用 D-Bus 框架进行通信？
- seccomp 组件的功能？seccomp 配置文件介绍？
- xfs_undelete 工具的安装 (git, tcl, tcllib)？
- xfs 文件系统中删除文件或目录的恢复示例？
- xfs 文件系统的结构解析？
- xfs 文件系统是否存在备份的 superblock？
- superblock 损坏后，如何使用从 AG 中提取 superblock 信息修复文件系统？
- nmap 扫描端口报错？
  - Note: Host seems down. If it is really up, but blocking our ping probes, try -Pn
  - Nmap done: 1 IP address (0 hosts up) scanned in 0.08 seconds
- 什么是 ostree？它是如何实现的？请给出 ostree 的相关实验 demo？
- 如何理解 tmpfs，此文件系统在当前运行的内存中是否占据空间？
- 挂载 tmpfs 时，怎样设置大小上限？

## 6. 硬件监测

- 如何理解磁盘固件中的 S.M.A.R.T 子系统？这种固件的形式或者说源码是如何实现的？
- Linux 中 smartctl 命令检查磁盘健康状态？
- Linux 中 HBA 卡队列深度 (io_depth) 过小导致系统宕机，该如何解决？
- 如何根据系统日志定位 Linux 中内存 ECC 的故障插槽，请给出具体的日志与分析过程？
- Linux 内核中的 **内核性能计数器（Kernel Performance Counters）**是由一个专门的内核子系统实现的，这个子系统叫做 perf_events（有时也简称为 Performance Counter Subsystem 或 Linux Performance Counter Subsystem）。
- Linux 内核的 EDAC 子系统是什么？
  - 与 MCE 的区别 (mcelog)
  - MCE (Machine Check Exception) 是 CPU 抛出异常，告诉你“有错误”。
  - EDAC 会进一步告诉你“错误发生在哪根 DIMM、哪个通道”，定位粒度更细。
  
  ```bash
  $ sudo grep "[0-9]" /sys/devices/system/edac/mc/mc*/csrow*/ch*_ce_count
  /sys/devices/system/edac/mc/mc0/csrow2/ch0_ce_count:42457
  /sys/devices/system/edac/mc/mc0/csrow2/ch1_ce_count:0
  /sys/devices/system/edac/mc/mc0/csrow3/ch0_ce_count:9630
  /sys/devices/system/edac/mc/mc0/csrow3/ch1_ce_count:0
  /sys/devices/system/edac/mc/mc1/csrow2/ch0_ce_count:0
  /sys/devices/system/edac/mc/mc1/csrow2/ch1_ce_count:20229
  /sys/devices/system/edac/mc/mc1/csrow3/ch0_ce_count:0
  /sys/devices/system/edac/mc/mc1/csrow3/ch1_ce_count:31858
  # 开启 EDAC 服务或加载 EDAC 内核模块后
  # count 不为 0 的行即代表存在内存错误
  # mc*: 第好多个CPU
  # csrow*：内存通道
  # ch*：通道内的第几根内存
  ```

- 什么是 Linux 内核中的 NMI watchdog（不可屏蔽中断看门狗）？

## 7. 系统服务

- LDAP：
  - 本身是一种协议
  - LDAP 的实现：OpenLDAP (Community), 389ds/389-ds-base (redhat)
  - Kerberos 身份认证服务：OpenLDAP+Kerberos, 389-ds-base+Kerberos
  - 集成解决方案：FreeIPA (Community) ==> RedHat IdM (redhat)
- HAProxy：
  - haproxy.cfg 配置文件参数说明 (option 参数、timeout 参数、listen 段)

## 8. 系统安全

- OpenSSL 中的 3DES 对称加密算法中 PBKDF2 是什么？老式的 EVP_BytesToKey 是什么？
- [隐藏了十年的 sudo 漏洞爆出：无需密码就能获取 root 权限](https://blog.csdn.net/zhangge3663/article/details/113501578)
  - CVE-2019-14287
  - CVE-2019-18634

## 9. Ceph

- 如何理解 Ceph MDS 缓存以及 MDS 客户端的缓存一致性？

## 10. Jenkins
  
- Jenkinsfile 中的 script { ... } 块作用是什么？与直接使用 sh 有何区别？
- 安装 Pipeline Stage View 查看流程方块视图
- Jenkins 中的凭据插件：
  - Credentials Binding Plugin: withCredentials 指令 (动态使用)
- Credentials Plugin: checkout 步骤 credentialsID (静态使用)

## 11. 容器 & Kubernetes

- 如何理解 overlayfs 文件系统，并用示例说明？
- flatpak 与 Podman 容器在实现上的异同点是什么？

  flatpak 与 Podman 虽然都属于 “容器化” 技术，但它们的设计目标、隔离模型和底层实现差异很大，可类比为“桌面沙盒”与“系统级容器”的区别。下面从实现机制角度给出异同点对比。

  | 维度 | flatpak | Podman |
  | ----- | ----- | ----- |
  | 设计定位 | 面向桌面应用的打包与分发：把 GUI 应用连同依赖打成一个包，跨发行版运行。| 面向服务器/开发场景的 OCI 容器引擎：用来运行完整 Linux 发行版或任意服务进程。|
  | 隔离粒度 | 单应用级沙盒（Single-app sandbox）| 操作系统级容器（OS-level container）|
  | 运行时底座 | 依赖“Runtime”概念（如 org.freedesktop.Platform），由 flatpak 预先挂载为只读 ostree 层；应用再叠加自己的 `/app` 层。| 依赖 OCI 镜像，镜像由 layer tar+json 描述，容器启动后联合挂载为可写容器文件系统（overlayfs/ fuse-overlayfs）。|
  | 命名空间使用 | 仅使用 user、pid、network 三个命名空间即可满足桌面需求；默认不隔离 IPC、UTS，也可关闭网络隔离以方便 X11/Wayland 通信。| 完整命名空间套件：user、pid、net、ipc、uts、mnt、cgroup 全部隔离；支持 Pod 内共享 net/uts/ipc。|
  | 用户权限 | 原生无根（rootless）：普通用户即可安装、更新、运行；利用 user-ns 把宿主机 UID 映射为沙盒内 UID 0。| 原生无根：利用 user-ns 把宿主机普通 UID 映射为容器内 root；也可加 `--privileged` 或 root 运行获得更高权限。|
  | 镜像/包格式 | 采用 ostree 仓库格式，分支名形如 `app/org.gimp.GIMP/x86_64/stable`；增量更新、去重存储。| 采用 OCI 标准镜像（Docker v2 schema）；镜像层通过 content-addressable 存储在本地 `/var/lib/containers` 或用户目录。|
  | 网络栈 | 默认走宿主网络（`--share=network`），也可开私有网络段；X11/Wayland、PulseAudio 通过 socket 代理进入沙盒。| 默认使用私有网络命名空间，通过 CNI/plugins 创建 bridge/macvlan/ipvlan；端口需手动 `-p` 映射或 `--network host`。|
  | 进程模型 | 每个应用一个“沙盒”，内部仅跑主进程+子进程；生命周期由 flatpak 前端（如 gnome-software）或用户 `flatpak run` 直接管理。| 每个容器一个 init(可以为 tini、catatonit、systemd)；支持 pod 把多容器捆成原子单元；生命周期由 podman/systemd 托管。|
  | 存储卷 | 通过 `--filesystem=host:ro` 等细粒度路径白名单把宿主机目录挂进沙盒；无通用卷插件概念。| 支持 docker-compatible volume、bind-mount、tmpfs、匿名卷；可插件化扩展存储驱动（overlay/btrfs/zfs）。|
  | 安全策略 | 主要依赖 Bubblewrap 做 seccomp + 能力过滤 + 命名空间；flatpak 门户（portal）机制对文件访问、摄像头、通知等做动态授权。| 默认开启 seccomp、Capabilities 白名单、SELinux/apparmor 标签隔离；支持 `--security-opt` 细调。|
  | 系统服务依赖 | 无守护进程；每次 `flatpak run` 直接 exec bwrap 启动；更新由 ostree 在后台以 pull+hardlink 完成。| 无中央守护进程（对比 dockerd），但容器启动依赖 systemd user session 或 tmpfiles 设置；镜像存储由 containers/storage 库管理。|
  | 与宿主机交互 | 必须显式授权：文件路径、设备、socket、dbus 名均需白名单；默认禁止访问 `/sys`、`/proc` 敏感部分。| 需要显式 `-v` 挂载或 `--privileged` 才能访问宿主机资源；默认完全隔离。|
  | 更新机制 | 基于 ostree 的增量拉取，可回滚；应用与 runtime 可分别升级；更新后旧分支保留以便 `flatpak update --rollback`。| 镜像更新靠 `podman pull` 重新下载层；容器可重启即用新镜像；支持 `podman auto-update` + systemd 实现滚动升级。|

  flatpak 在实现上更像 “用 ostree + bwrap 做的轻量级应用沙盒”，重点解决 “桌面应用跨发行版安装+安全运行”；Podman 则是 “完全基于 OCI 与 Linux 命名空间/cgroup 的系统级容器引擎”，重点解决 “服务/开发环境的一致隔离与部署”。两者都利用 user-namespace 做到无根运行，但在隔离深度、网络模型、存储与生命周期管理上走的是两条不同技术路线。

- flatpak 应用安装方式？

  ```bash
  ### 普通用户 ###
  $ flatpak remote-add --user --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
  $ flatpak search marker
  $ flatpak install --user flathub com.github.fabiocolacio.marker
  $ flatpak run com.github.fabiocolacio.marker  #bwrap 应用启动沙盒运行环境
  $ flatpak update com.wps.Office  #更新
  $ flatpak uninstall --delete-data com.wps.Office  #卸载
  ```

- flatpak 与 ostree 的关系？安装路径在用户的哪个目录？
  - 用户级安装（--user，无需 sudo，推荐）：

    ```plaintext
    应用主文件
    ~/.local/share/flatpak/
    ├─ app/          ← 所有应用（如 com.wps.Office）
    ├─ runtime/      ← 依赖运行时（如 org.freedesktop.Platform）
    ├─ repo/         ← 本地 ostree 仓库（objects/refs 等）
    └─ .changed      ← 标记文件，表示「用户仓库已存在」

    运行时挂载点（启动后可见）
    /var/tmp/flatpak-cache-XXXX（临时 overlayfs 上层，重启自动清）

    真正的只读 /app → 来自 ~/.local/share/flatpak/app/<应用>/current/active/files
    ```

  - 系统级安装（--system，需 root 或 polkit）：
    - /var/lib/flatpak/
    - 目录结构与上面完全一致，只是作用域全局，对所有用户生效。
  - 用户数据 / 配置目录：
  
    ```plaintext
    无论装在哪个作用域，应用写权限默认只能落在：
    ~/.var/app/<应用 ID>/
    ├─ data/         ← 应用自己保存的数据（等价于 ~/.local/share）
    ├─ config/       ← 配置（等价于 ~/.config）
    └─ cache/        ← 缓存（等价于 ~/.cache）
    ```

  - flatpak 与 ostree 的分工：

    ```plaintext
    flatpak（高层）
    ├─ 定义沙箱、权限、门户、远程、引用(ref)
    ├─ 调用 `bubblewrap` 做容器启动
    └─ 把 “应用/运行时/扩展” 当成一条条分支 → 交给 ostree 存取

    ostree（底层）
    ├─ 仅提供 “内容寻址对象库” + “分支指针”
    ├─ 负责去重、增量、回滚、硬链接检出
    └─ 完全不知道 “桌面应用” 概念，只认 commit/tree/blob
    ```

  - 本地 ostree 仓库位置（就是 flatpak 的 repo/）：
    - 用户级：~/.local/share/flatpak/repo
    - 系统级：/var/lib/flatpak/repo
    - 里面结构跟任何 ostree 仓库一样：

      ```bash
      repo/
      ├─ objects/          ← SHA256 内容寻址实体（blob/tree/commit）
      ├─ refs/heads/       ← 分支指针，如
      │   ├─ app/com.wps.Office/x86_64/stable
      │   └─ runtime/org.freedesktop.Platform/x86_64/23.08
      ├─ config            ← 仓库版本、压缩算法
      └─ summary           ← 可选索引
      ```

- 如何使用 ostree 命令读取 flatpak 仓库？
- 什么是 bubblewrap？
- flatpak 如何利用 bwrap 实现容器化？ 如何利用 ostree 实现应用的升级与回滚？
- 如何查看 flatpak 应用的当前版本和历史版本？
- 🔎 如何在 Kubernetes 集群当中安装一个指定的 operator？
- 请给出一个 Kubernetes Operator 与 CRD 的具体实现示例？
- Kubernetes 从哪个版本开始支持 Tekton 与 ArgoCD？
- Kubernetes api `managedFileds`（v1.18新增）：server-side apply（服务端应用）说明
- OpenShift v4.6 OAuth2 server 认证的原理？
- OpenShift v4.10 集群中的证书工作机制？集群内部使用的证书由集群内部自身管理，不可使用用户自定义的证书，该功能直至 v4.11 版本依然未被解决，而 `router pod` 与 `web console pod` 的证书可替换为用户自定义的证书。
- OpenShift v4.10 etcd 数据库的备份与恢复？
- 如何理解 OpenShift v4.10 中的 `extension APIs` 与 `Operator` 概念？
- 💡 OpenShift 安装 operator 的过程中，subscription 资源对象的作用是什么？
- 🌐 KServe 与 Knative 之间的联系？它们的架构如何实现？

### 11.1 K8s NetworkPolicy 与 Calico NetworkPolicy 间的关系

- kubernetes v1.8 版本开始支持 `networkpolicy` 类型的 API，但其原生支持的功能较弱。
- 因此，可以使用 `calico` 的 networkpolicy 做功能的扩展。
- `nerdctl` 的端口不是用户态监听的，而是直接 `iptables nat` 转发的。

## 12. OpenShift

- 如何计算单个 OpenShift 集群中最大能承载的 Pod 数量（IPv4 地址规划）？
- 如何在本地部署一个单节点的 RHACM？
- 如何使用 RHACM + ZTP 配合 GitOps 创建部署新的 OpenShift 集群？

## 13. CNI 如何玩转 Linux Bridge

```bash
### 在宿主机上
$ ip link add cni0 type bridge
$ ip link set cni0 up

### 在容器里
$ ip link add eth0 type veth peer name vethb4963f3
# 创建一对 Veth Pair 设备，其中一个叫作 eth0，另一个叫作 vethb4963f3。

$ ip link set eth0 up
# 启动 eth0 设备

$ ip link set vethb4963f3 netns $HOST_NS
# 将 Veth Pair 设备的另一端（也就是 vethb4963f3 设备）放到宿主机（也就是 Host Namespace）里

$ ip netns exec $HOST_NS ip link set vethb4963f3 up
# 通过 Host Namespace，启动宿主机上的 vethb4963f3 设备。

### 在宿主机上
$ ip link set vethb4963f3 master cni0

### 在容器里
$ ip addr add 10.244.0.2/24 dev eth0
$ ip route add default via 10.244.0.1 dev eth0

### 在宿主机上：
$ ip addr add 10.244.0.1/24 dev cni0
```

## 14. Python

- cookie 与 session 的联系与区别是什么？如何通过 wireshark 抓包的方式获取它们呢？
- 如何创建一个 python 服务端实现示例完整演示 cookie 和 session 之间的关系？
- Python 中有多少种错误类型，分别表示的含义是什么？
- 给出 Python 使用 `__iter__()` 与 `__next__()` 创建迭代器的示例？
  - 文件分块读取器
  - 二叉树中序遍历
- Python 面向对象中的封装与多态？
- Python 中多线程互斥锁、自旋锁与原子操作的实现？内核底层是如何实现的？

## 15. 机器学习

- 数学基础：
  - 什么是雅可比矩阵（Jacobian）？ 
  - 什么是海森矩阵（Hessian Matrix）？
    - 海森矩阵（Hessian Matrix）是多元函数二阶偏导数的方阵，用于描述函数的局部曲率、凹凸性以及极值判别。
    - 在优化、机器学习（牛顿法、XGBoost、深度学习二阶优化）里都是核心工具。
    - 海森矩阵 = 梯度的一阶导数 = 函数局部的曲率仪表盘
    - 正定极小，负定极大，不定鞍点；牛顿法靠它一步下山，XGBoost 用它算叶子权重。
  - 如何理解海森矩阵中每行每列？
  - 函数 $y = \frac{1}{1 + e^{-x}}$ 求导过程？
  - 超平面和法向量详细介绍？
  - 如何推导超平面表达式？如何推导点到超平面距离公式？
  - 如何求直线方程与平面方程的法向量？
  - 请更加具体地给出二维直线方程与平面方程的法向量求解方法与过程？
  - 向量内积的 **柯西·施瓦茨不等式**？
  - 如何理解与推导最小二乘法？
  - 举例说明最小二乘法的具体步骤？
  - 均方根误差是不是通过最小二乘法推导而来的？
  - 分别阐述一阶导数与二阶导数的几何意义（Latex 公式输出）？
- 概率 & 统计：
  - 均值、方差、协方差、标准差、偏差的数学定义？
- 📊 给出 NumPy 与 Pandas 在实际数据清洗过程中的完整示例？
- 评估数据相关性的有哪些指标？
- 详细描述线性回归的训练过程？
- 如何绘制数据集的密度散点图？
- 🎉 什么是 XGBoost（以 LaTex 公式演示）？它是否既可以做预测分析，也可以做多分类呢？它与决策树与随机森林的关系是什么？
- 🧩 什么是 LightGBM（以 LaTex 公式演示）？此算法的数学推导与应用场景是什么？
- 🎲 什么是 GaussianNB（以 LaTex 公式演示）？此算法的数学推导与应用场景是什么？(from sklearn.naive_bayes import GaussianNB)
- 📝 案例：
  - 如何获取 Ames House Prices 数据集，该数据集是以何种方式存储的？如何理解 Ames House Prices 数据集结构？
  - 如何对 Ames House Prices 数据集进行特征工程？
  - ANN 实现 Ames 房价预测的代码示例？
  - 什么是 Bootstrap 采样和有放回采样？
  - 如何计算模型的方差和偏差，并利用这两个值判断欠拟合或过拟合？
  - 如何实现偏差-方差权衡？

## 16. 深度学习

- tensorflow.keras.datasets 下载的数据存放路径是什么？
- 什么是数据标准化？它的功能是什么？
- 什么是正则化？它的功能是什么？
- 数据预处理中 OneHotEncoder 是什么？它的功能是什么？
- ANN 中为何要构建数据加载器 DataLoader？
- CNN 中的下采样操作是什么？
- 梯度下降算法的公式是如何基于泰勒公式推导出来的？
- 以一元函数的泰勒公式为例，详细演示如何推导出梯度下降算法公式？
- 以多元函数的泰勒公式为例，详细演示如何推导出梯度下降算法公式？
- 🔎 神经网络中的计算图（graph）是什么？误差反向传播算法 (BP) 的详细数学推导？
- 🧬 PyTorch 框架中 autograd 机制实现反向传播过程，loss.backward() 逐一更新整个所有梯度，再由 optimizer.step() 更新权重与偏置项，请以鸢尾花数据集为例，采用真实的数据手动计算这一过程？
- CNN 训练过程中反向传播在哪些阶段实现？
- CNN 中每个卷积层中的不同卷积核的值是如何确定的？
- 为什么激活函数可以有效解决梯度消失问题？
- 深度学习中为什么会出现梯度消失与梯度爆炸，请详细说明原因？
- Scikit-learn, PyTorch, TensorFlow 保存的模型格式，这些格式的具体结构是如何组织的？
- 📝 案例：
  - 如何查看 MNIST 或 CIFAR-10 数据集中的图片，并将其以 png 格式保存在指定目录中？
  - CIFAR-10 数据集：使用 CNN 实现图片中物体识别的代码示例？

## 17. Transformer

- 学习路径：Pythoh ➡ Transformer ➡ 提示词工程 ➡ RAG ➡ LangChain ➡ LangGraph ➡ Agent ➡ 多 Agent ➡ 私有化部署 ➡ 微调 ➡ 量化 ➡ 多模态
- 模型格式1：HuggingFace 格式（config.json + tokenizer.json + .safetensors 分片）
- 模型格式2：GGUF（*.gguf）格式
- Transformer 中维度引入的时间线：

```plaintext
 Token ID（整数，无维度概念）
    ↓
【嵌入层】← 维度在这里首次引入！
    │
    └── 嵌入矩阵形状：[vocab_size, hidden_dim]
    │
    输出：[batch, seq_len, hidden_dim]  ← 768/1024/1536 等
    ↓
【位置编码】
    │
    输出：[batch, seq_len, hidden_dim]  ← 同维度
    ↓
【Transformer 层】
    │
    ├── Q/K/V 投影矩阵：[hidden_dim, hidden_dim] 或 [hidden_dim, num_heads × head_dim]
    ├── FFN 第一层：[hidden_dim, 4×hidden_dim]
    ├── FFN 第二层：[4×hidden_dim, hidden_dim]
    └── 输出：[batch, seq_len, hidden_dim]  ← 维度保持不变！
    ↓
【池化层】
    │
    输出：[batch, hidden_dim]  ← 句子向量

维度在嵌入层首次引入（hidden_dim 超参数），后续所有权重矩阵（Q/K/V、FFN、LayerNorm）的维度都继承自这个值，通过残差连接保证维度一致，形成 "768 维贯穿始终" 的架构。
```

- Transformer 维度传递的完整链条：

```plaintext
模型设计时确定 hidden_dim（如 768）
    ↓
嵌入层权重：[vocab_size, 768]
    ↓
位置编码：[max_seq_len, 768]
    ↓
Transformer 层 × N：
    ├─ Q/K/V 投影：[768, 768]
    ├─ Attention 输出：[768, 768]
    ├─ FFN 升维：[768, 3072]
    ├─ FFN 降维：[3072, 768]
    └─ 残差连接：输入输出都是 [batch, seq, 768]
    ↓
池化层：输入 [batch, seq, 768] → 输出 [batch, 768]
    ↓
最终句子向量：768 维
```

- 以 how are you? 为例，解释一下 token 化的过程？维度在何时引入？

```plaintext
以 "how are you?" 为例
    ↓
【Tokenization】BPE 分词
    │
    "how" → 15496
    " are" → 389
    " you" → 345
    "?" → 30
    │
    输出: [15496, 389, 345, 30]  ← 纯整数，无维度
    ↓
【嵌入层】nn.Embedding(50257, 768)
    │
    查表获取每行向量
    │
    15496 → [0.11, 0.67, ..., 0.33]  (768 维)
    389   → [0.23, -0.15, ..., 0.88] (768 维)
    345   → [0.05, 0.22, ..., -0.17] (768 维)
    30    → [-0.08, 0.91, ..., 0.44]  (768 维)
    │
    输出: [4, 768]  ← 维度首次引入！
    ↓
【位置编码】+ [4, 768]
    │
    输出: [4, 768]  ← 维度保持
    ↓
【Transformer × 12】
    │
    ├─ W_q [768,768] → 输出 [4,768]
    ├─ W_k [768,768] → 输出 [4,768]
    ├─ W_v [768,768] → 输出 [4,768]
    ├─ W_ffn1 [768,3072] → 中间 [4,3072]
    ├─ W_ffn2 [3072,768] → 输出 [4,768]
    │
    输出: [4, 768]  ← 维度始终 768
    ↓
【池化】Mean/CLS
    │
    输出: [768]  ← 句子向量

"how are you?" 先被 BPE 切成 4 个 token 并转为整数 ID，此时无维度概念。进入嵌入层后，每个 ID 通过查表映射为 768 维向量，维度在此首次引入。后续所有权重矩阵（Q/K/V、FFN、LayerNorm）的维度都继承自这个 768，通过残差连接保持维度一致，最终输出 768 维的句子向量。
```

- 如何对给定序列 "How are you?" 使用 tokenizer 实现 token 化，计算出 QKV 三个矩阵，给出具体的代码示例与数学计算过程。
- 嵌入后的矩阵每行代表一个 token，每列代表一个（语义）维度。接下来既可以用这个矩阵和 transformer 的权重矩阵分别进行 QKV 三个矩阵的运算。对吗？是的，完全正确。</br>

  Embedding 矩阵的每行是 token 向量，与 Transformer 的 Q/K/V 权重矩阵分别相乘，得到三个投影矩阵，进入 Attention 计算。

```plaintext
Embedding 输出: [batch_size, seq_len, hidden_dim]
                    [1, 4, 4096]

每行 = 一个 token 的向量 (4096 维)
每列 = 一个语义维度

    ↓

分别与三个权重矩阵相乘:

W_Q: [4096, 4096]    W_K: [4096, 4096]    W_V: [4096, 4096]
    ↓                    ↓                    ↓
Q: [1, 4, 4096]      K: [1, 4, 4096]      V: [1, 4, 4096]
```

- 如何理解 batch_size？
- 权重矩阵 W_Q, W_K, W_V 的维度是怎么来的？它们的维度和 Embedding 维度，以及 d_model 维度一样吗？</br>
  W_Q, W_K, W_V 的输入维度始终等于 d_model（4096），但输出维度取决于注意力类型：标准 MHA 输出 4096，GQA 压缩到 1024，MQA 压缩到 128，目的是减少 KV Cache 显存。

## 18. 大模型基础

- ❓如何在 Kubernetes 集群中进行分布式模型训练（图像识别或大语言模型）？数据集是如何拆分至不同 GPU 中的？训练过程中模型的参数是如何协同更新的？
- 运行 OpenVINO 模型服务器的命令（python 原生运行）？如何运行 OpenVINO 容器？
- 运行 ONNX 运行时的命令（python 原生运行）？如何运行 ONNX 容器？
- distilbert 模型能在 24 核心 CPU 和 32 GB 内存的平台上运行吗？
- distilbert 模型如何转换为 ONNX 格式，并且使用 OpenVINO 部署？
- ❓如何理解某个大语言模型？
  - 模型的使用场景
  - 模型运行需要多大的显存空间
  - 模型是否可量化
  - 模型量化的方式
  - 模型的维度
  - 模型的精度
  - 模型的权重
  - 模型生成 token 的速度（每秒多少 token）
  - 模型的并发能多大
  - 模型实现的原理
- 多个模型服务运行时对比：llama.cpp、ONNX、OpenVINO、vLLM

## 19. vLLM 推理

- tokenized 之后的 Embedding 过程不归 vLLM 接管，而是到达大模型之后再做 Embedding？是的</br>
  Embedding 是大模型的第一层神经网络，vLLM 只负责把 token IDs 送到 GPU，实际的向量转换由大模型权重完成。

```plaintext
  Tokenization (vLLM CPU)
    │
    ▼ [101, 2345, 5678, 8910]
Scheduler 组织 batch
    │
    ▼ input_ids 张量送入 GPU
Embedding Layer (大模型第一层)
    │
    ▼ [num_tokens, hidden_dim]
Transformer Layers (大模型主体)
    │
    ▼ ...
```

- 描述 vLLM 的完整推理过程？
- 一个推理请求通过 vLLM 和基于 Transformer 模型后，token 在整个链路中的变化是怎么样的？对应的源码位置在哪里？这个源码的功能是什么？
- vLLM 部署后第1次的推理请求中并没有 KV cache，那么 vLLM 也会使用 KV cache 吗？
  - 会。
  - vLLM 的 KV cache 是“推理过程中在线填充”的，而不是“请求前就已存在”。流程如下：
    - 第 1 次请求进来时，cache 确实为空。
    - 在 第 1 个 forward 阶段（prefill），vLLM 把 prompt 各 token 的 Key/Value 向量逐层算出，并 立即写入物理 cache 块（PagedAttention 的 block table）。
    - 随后的 每个 decode 步 继续把新产生 token 的 KV 追加到同一块表；同一序列后续再发请求，可直接复用这些块，无需重算。
  - 因此：
    - “第 1 次”只是 cache 起点，并非不用 KV cache；
    - 从第 2 次请求起，若命中相同前缀或重用 block，即可享受到缓存加速。
- 将大模型部署于 vLLM 中，那么 vLLM 如何处理大模型的请求？

  ```plaintext
  客户端 HTTP 请求
     ▼
  API server → 拆成 token 序列
     ▼
  Scheduler（Continuous Batching）
     ├─ Prefill 队列 → 一次性生成 KV → 进块表
     └─ Decode 队列 → 逐 token 增量更新 KV
     ▼
  PagedAttention Kernel（GPU）
     ▼
  采样 → 新 token → 异步流式返回
     ▼
  KV 块回收 ← 请求完成
  ```

- 大模型在整个 vLLM 推理过程中如何发挥作用？
  - 可以把 vLLM 的推理流程想象成一条 “token 流水线”
  - 大模型（权重）就是流水线上唯一的那台核心机床；vLLM 只负责把原材料（token）分批、排产、送料、收料，让机床永远处于切削状态， never idle。具体分工如下：
    - 权重常驻显存
      - 启动时 vLLM 一次性把完整模型（FP16/BF16/FP8/Int4）加载到 GPU，之后不再移动；后续所有计算都直接引用这片显存，节省重复 IO。
    - 前向计算 = 机床加工
      - Prefill 阶段：把整段 prompt 拼成 1 个长矩阵 → 调用 1 次 CUDA kernel 链（Attention + FFN + RMSNorm…）→ 产出首 token  logits。
      - Decode 阶段：每步只送 1 个新 token 的嵌入向量 → 同样走完整网络，但 KV 复用之前已算好的块，计算量 ≈ prompt 阶段的 1/seq_len。
    - 模型结构改造 = 机床换夹具
      - vLLM 在加载时会重写 Attention 算子（ops.paged_attention_v2），把原来“连续缓存”换成“块索引”输入；对模型本身参数、层数、激活函数不做任何改动，因此任何 Transformer 架构的大模型都能直接上线。
    - 并行策略 = 多机台联动
      - Tensor Parallel：把一层矩阵按“列-行”切分到多 GPU，单 token 前向一次完成；vLLM 自动插入 NCCL All-Reduce。
      - Pipeline Parallel：把不同层放在不同卡，vLLM 负责 micro-batch 调度，保证流水线无气泡。大模型权重按策略切好后依旧只干一件事——做矩阵乘法，调度细节对它是透明的。
    - 采样与输出 = 质检 + 贴标
      - logits 回传后，vLLM 在 CPU 端做 beam/nucleus/repetition 处理，选出下一个 token；大模型不参与采样，只负责“打分”。
  → 总结：大模型在 vLLM 里就是一块被“分页 + 批量化”调用的大算子；它只关心“输入矩阵维度对不对”，其余负载均衡、内存碎片、连续批、流式返回等全部由 vLLM 框架代劳。因此同样一张 GPU，vLLM 能让大模型每秒处理的 token 数提升一个数量级，而模型本身一行代码都不用改。

- vLLM 是上层的调度器，底层是如何与 CUDA 联系实现 GPU 层面的调度呢？
- 如何理解 CUDA kernel？
- 大模型文件是如何组织的？以 HuggingFace 上的模型为例，说明模型的文件结构，以及如何被 vLLM 加载运行。

```plaintext
以 Llama-2-7B 为例的标准结构：

meta-llama/Llama-2-7b-hf/
├── config.json                 # 模型架构配置（核心）
├── model.safetensors           # 权重文件（新格式，推荐）
├── model.safetensors.index.json # 多文件权重索引（大模型）
├── generation_config.json      # 生成参数默认配置
├── tokenizer.json              # 分词器词汇表和规则
├── tokenizer_config.json       # 分词器配置
├── special_tokens_map.json     # 特殊token映射
├── vocab.json / merges.txt     # BPE分词器文件（旧格式）
└── pytorch_model.bin           # 旧格式权重（PyTorch原生）
```

```plaintext
vLLM 加载流程全景图：

┌─────────────────────────────────────────────────────────────┐
│  Stage 1: 配置解析 (Python/HF Transformers)                  │
│  ─────────────────────────────────────────                   │
│  1. 从 HF Hub 或本地路径读取 config.json                     │
│  2. 确定模型架构 (LlamaForCausalLM → vLLM 的 Llama 实现)      │
│  3. 解析超参数 (hidden_size, num_layers, num_heads, etc.)    │
│                                                              │
│  vLLM/config.py:                                             │
│  ModelConfig → 验证并转换 HF 配置到 vLLM 内部格式            │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│  Stage 2: 权重加载与分布 (关键阶段)                            │
│  ─────────────────────────────────────────                   │
│                                                              │
│  单机单卡 (简单情况):                                         │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐                  │
│  │ Safetensors│ → │ 内存映射  │ → │ GPU显存  │                  │
│  │  文件    │    │ (CPU RAM)│    │ (VRAM)   │                  │
│  └─────────┘    └─────────┘    └─────────┘                  │
│       ↑              ↑              ↑                        │
│  零拷贝加载      按需加载特定层      全量加载                  │
│  (memory-map)   (lazy loading)   (cuda:0)                    │
│                                                              │
│  多卡并行 (Tensor Parallelism):                               │
│  ┌─────────┐         ┌─────────┐         ┌─────────┐       │
│  │  GPU 0  │← NVLink →│  GPU 1  │← NVLink →│  GPU 2  │       │
│  │ [层0-15]│         │ [层16-31]│         │ [层0-15] │       │
│  │ TP Rank0│         │ TP Rank1 │         │ TP Rank2 │       │
│  └─────────┘         └─────────┘         └─────────┘       │
│       ↑                                               ↑      │
│  每个GPU加载部分权重，通过 NCCL All-Reduce 同步中间结果        │
│                                                              │
│  vLLM/model_executor/model_loader.py:                        │
│  load_weights() → 根据并行策略分配权重到各设备                  │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│  Stage 3: 模型初始化与编译 (GPU Kernel 准备)                   │
│  ─────────────────────────────────────────                   │
│                                                              │
│  1. 创建 Transformer 层对象 (nn.Module)                      │
│     ┌─────────────────────────────────────────┐              │
│     │  self.layers = nn.ModuleList([          │              │
│     │    LlamaDecoderLayer(config) for _ in    │              │
│     │    range(config.num_hidden_layers)        │              │
│     │  ])                                       │              │
│     └─────────────────────────────────────────┘              │
│                                                              │
│  2. 权重格式转换 (如有必要)                                    │
│     • FP32 → FP16/BF16 (节省显存)                            │
│     • 合并 Q/K/V 投影矩阵 (优化内存布局)                        │
│                                                              │
│  3. CUDA Graph 捕获 (可选优化)                                │
│     • 对固定形状的推理路径预编译 kernel 序列                    │
│     • 减少 CPU launch overhead                               │
│                                                              │
│  vLLM/model_executor/models/llama.py:                        │
│  LlamaForCausalLM.__init__() → 构建模型结构                   │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│  Stage 4: 运行时执行 (推理循环)                                │
│  ─────────────────────────────────────────                   │
│                                                              │
│  输入文本 → Tokenizer → Token IDs → GPU Tensor               │
│                              ↓                              │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  vLLM Engine (Python)                                   │ │
│  │  ┌─────────┐    ┌─────────┐    ┌─────────────────┐    │ │
│  │  │Scheduler│ → │Batch Prep│ → │ execute_model() │    │ │
│  │  └────┬────┘    └────┬────┘    └────────┬────────┘    │ │
│  └───────┼──────────────┼──────────────────┼─────────────┘ │
│          │              │                  │                 │
│          └──────────────┴──────────────────┘                 │
│                         ↓                                    │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  GPU Kernel Execution (CUDA)                              │ │
│  │  ┌─────────────────────────────────────────────────────┐ │ │
│  │  │  1. Embedding Lookup (token → vector)                │ │ │
│  │  │  2. For each layer:                                  │ │ │
│  │  │     ┌─────────────────────────────────────────────┐   │ │ │
│  │  │     │ • RMSNorm (input)                           │   │ │ │
│  │  │     │ • Q/K/V Projection (Linear)               │   │ │ │
│  │  │     │ • Rotary Position Embedding (RoPE)        │   │ │ │
│  │  │     │ • PagedAttention Kernel (核心，见前文)       │   │ │ │
│  │  │     │ • MLP (Gate Up Proj + SwiGLU + Down Proj) │   │ │ │
│  │  │     │ • Residual Connection                       │   │ │ │
│  │  │     └─────────────────────────────────────────────┘   │ │ │
│  │  │  3. Final Norm + LM Head (logits)                      │ │ │
│  │  └─────────────────────────────────────────────────────┘ │ │
│  └─────────────────────────────────────────────────────────┘ │
│                         ↓                                    │
│  Sampling (GPU) → Token ID → Detokenizer → 输出文本           │
└─────────────────────────────────────────────────────────────┘
```

- vLLM 中的连续批处理是在哪个阶段？

连续批处理（Continuous Batching）在 vLLM 中发生在 Python 调度层，具体在 Scheduler 阶段。

```plaintext
┌─────────────────────────────────────────┐
│  Layer 1: Python 调度层                  │
│  ┌─────────┐  ← 连续批处理发生在这里     │
│  │Scheduler│                              │
│  │  ├─ 维护 waiting/running/swapped 队列 │
│  │  ├─ 每 step 动态选择可加入的请求      │
│  │  └─ 决定哪些请求可以 prefill/decode   │
│  └────┬────┘                              │
│       ↓                                   │
│  ┌─────────┐                              │
│  │Batch Prep│ ← 组装当前 step 的 batch    │
│  │(连续批处理│   (可能混合 prefill+decode) │
│  │ 的结果) │                              │
│  └────┬────┘                              │
└───────┼───────────────────────────────────┘
        ↓
┌─────────────────────────────────────────┐
│  Layer 2: GPU 执行层                     │
│  (模型实际计算，不再做调度决策)            │
└─────────────────────────────────────────┘
```

- vLLM 如何知道多个请求？

```python
# vLLM 如何知道 "这是 3 个独立请求"
# vLLM 内部维护的数据结构
class SequenceGroup:
    seq_id: int              # 唯一标识这个请求
    prompt_token_ids: List[int]  # 分词后的结果
    output_token_ids: List[int]  # 已生成的token
    
# Scheduler 维护的队列
waiting_queue: List[SequenceGroup] = [req0, req1, req2]

# 每次 schedule() 时，选择哪些请求加入当前 batch
selected: List[SequenceGroup] = scheduler.select_batch()
# batch_size = len(selected)  # 由调度策略动态决定
```

```plaintext
实际 vLLM：
  batch_size = scheduler._schedule_waiting() 
             + scheduler._schedule_running()
             + scheduler._schedule_swapped()
             
  这个值是"算出来"的，不是"设进去"的
```

- vLLM 的 worker 在推理的哪个阶段？
- vLLM 中的 waiting 队列和 running 队列是什么数据结构实现的？这两个队列的长度有限制吗？分别存了哪些数据？</br>
  waiting 存的是"待处理的完整请求"（无长度硬限制，受系统内存和客户端限流约束），running 存的是"正在 GPU 计算的请求"（长度受 max_num_seqs 硬性约束）。两者存储的是同一个 SequenceGroup 对象，只是状态不同，在队列间移动时数据完整保留。
- max_num_batched_tokeens 参数含义？
- max_num_batched_tokens 与 max_model_len 参数关系：</br>
  max_model_len 限制单个序列，max_num_batched_tokens 限制batch 总和。它们独立配置，没有强制的大小约束关系。

```plaintext
场景1：短序列高并发（如对话机器人）
  max_model_len = 4096
  max_num_batched_tokens = 2048
  
  含义：支持最长 4096 的上下文，但单次只算 2048 个 token
        （可能是 10 个 200 token 的请求，或 4 个 512 token 的请求）
        
  关系：max_num_batched_tokens < max_model_len ✓ 常见

场景2：长序列优化（如文档处理）
  max_model_len = 32768
  max_num_batched_tokens = 8192
  
  含义：支持 32k 上下文，单次算 8k token
        （可能是一个 8k 的长请求，或 4 个 2k 的请求）
        
  关系：max_num_batched_tokens < max_model_len ✓ 常见

场景3：极端吞吐（短 prompt 为主）
  max_model_len = 4096
  max_num_batched_tokens = 8192
  
  含义：序列最长 4k，但单次 batch 算 8k token
        （必须是 2 个以上的请求才能填满）
        
  关系：max_num_batched_tokens > max_model_len ✓ 也可行
```
