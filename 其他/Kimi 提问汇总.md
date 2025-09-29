# 📚 Kimi 提问汇总

## Linux 内核

- **源码分析**：Linux 从加电启动 → start 内核→ 用户空间？
- Linux 内核模块参数设置后，没有在 parameters 中显示，是什么原因？module_param() 声明在哪里查看？
- 如何理解 Linux 中 dmesg 命令的 ring buffer？
- 🔥 Linux 内核自身的内存管理在物理内存上是如何组织分布的？
  - Linux 内核在初始化阶段就把整台机器的物理 RAM 抽象成一张 “多级目录表”，随后所有分配/回收动作都在这张表里完成。
  - “自顶向下” 的顺序：从 NUMA 节点 → 内存域（Zone）→ 页帧（Page Frame）→ 每 CPU/伙伴/Slab 三级缓存
  - Linux 把物理 RAM 先切成 NUMA 节点，节点内再切成 Zone，Zone 里用 **伙伴系统** 管理 2^n 页块，每页对应一个 `struct page`；小对象再由 **slab** 切分，形成 “节点-域-页-对象” 四级物理内存版图。
- 🔥 Linux 内核自身运行所在的内存与用户空间程序运行所在的内存有何区别？
  
  - Linux 内核永远跑在 **高地址、特权级、固定映射** 的那一段内存里；所有用户态进程只能呆在 **低地址、非特权、按需映射** 的另一半。两边用页表+特权级完全隔离，互不干扰。
  
  | 维度 | 内核空间 (Kernel Space) | 用户空间 (User Space) |
  | ----- | ----- | ----- |
  | **虚拟地址范围** | x86-64：ffff 8000 0000 0000 以上（128 TiB 高地址）<br>x86-32：0xC000 0000 ~ 0xFFFF FFFF（高 1 GB）| x86-64：0000 0000 0000 0000 ~ 0000 7FFF FFFF FFFF（128 TiB 低地址）<br>x86-32：0x0000 0000 ~ 0xBFFF FFFF（低 3 GB）|
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

- 🔥 Linux 内核如何为自己分配内存? 如何为用户空间进程分配内存？
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


- 请详细描述从 CR3 寄存器开始的线性地址到物理地址的寻址过程？
- 上述 CR3 寄存器是什么结构?
- Linux 内核是如何操作 MMU 和 TLB，请详述过程？
- 详解 /proc/meminfo 文件中的各项参数？

在 Linux 运行的 x86-64 模式下：
硬件把段基址强制设为 0，限长视为无限 →
逻辑地址 = 线性地址（恒等映射，段部件不做任何算术）。
因此用户代码里看到的地址就是线性地址，Linux 只通过四级/五级页表把它映射到物理页；分段机制既不参与分配，也不提供隔离。
结论：
“分段”在 64-bit Linux 中名存实亡，所有虚拟地址↔物理地址的转换完全由分页机制完成。

## Linux 系统编程

- 二进制、十进制、八进制与十六进制数据间的转换？
- 请编写多个 C 语言程序示例，并使用 makefile 完成编译。
- 为什么 C 程序中函数传参都使用了字符串指针而不是直接使用字符串，这样有何好处？
- char *s 这里的 s 是字符串数组，而 *s 是指向 s 的指针？
- intel 8086 CPU 与内存间的地址总线数量是多少？
- intel x86_64 CPU 与内存间的地址总线数量是多少？

## Linux 性能优化

- Linux 中 PCP 的 pmlogger 默认是采集所有的性能指标吗？是否可以自定义只需要的性能指标，并且采集的时间间隔能指定吗？

## Linux 网络

### Linux ARP 特性

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

## 系统组件

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

## 硬件监测

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

## 系统服务

- LDAP：
  - 本身是一种协议
  - LDAP 的实现：OpenLDAP (Community), 389ds/389-ds-base (redhat)
  - Kerberos 身份认证服务：OpenLDAP+Kerberos, 389-ds-base+Kerberos
  - 集成解决方案：FreeIPA (Community) ==> RedHat IdM (redhat)
- RH342v8.4 课程环境中 sssd 实验存在问题，需要排错。
- HAProxy：
  - haproxy.cfg 配置文件参数说明 (option 参数、timeout 参数、listen 段)

## 系统安全

- OpenSSL 中的 3DES 对称加密算法中 PBKDF2 是什么？老式的 EVP_BytesToKey 是什么？
- [隐藏了十年的 sudo 漏洞爆出：无需密码就能获取 root 权限](https://blog.csdn.net/zhangge3663/article/details/113501578)
  - CVE-2019-14287
  - CVE-2019-18634

## Shell 脚本

- `set` 命令如何调整 shell 行为：`-o` 选项、`-e` 选项？
- 如何使用 `expr` 命令与 `bc` 命令联合处理浮点数？
- 如何使用 `sed` 命令删除指定行、在指定行前后插入新行？
- `awk` 脚本如何引用 shell 中的变量方法？
- `shellcheck` 软件包如何检查 shell 语法？
- shell 截取字符串与统计字符串长度的方法？
- shell 变量中包含多个字符串时匹配空行使用 `[[ var == [[:space:]] ]]`
- if 条件判断如何结合正则表达式？
- while 条件判断的无限循环使用？

## Ceph

- 如何理解 Ceph MDS 缓存以及 MDS 客户端的缓存一致性？

## Jenkins
  
- Jenkinsfile 中的 script { ... } 块作用是什么？与直接使用 sh 有何区别？
- 安装 Pipeline Stage View 查看流程方块视图
- Jenkins 中的凭据插件：
  - Credentials Binding Plugin: withCredentials 指令 (动态使用)
- Credentials Plugin: checkout 步骤 credentialsID (静态使用)

## Ansible

- python 如何调用 ansible-playbook？ansible-runner 如何使用？

## 容器 & Kubernetes

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
- flatpak 如何利用 bwrap 实现容器化? 如何利用 ostree 实现应用的升级与回滚？
- 如何查看 flatpak 应用的当前版本和历史版本？
- 请给出一个 Kubernetes Operator 的实现的具体示例
- Kubernetes 从哪个版本开始支持 Tekton 与 ArgoCD？
- Kubernetes api `managedFileds`（v1.18新增）：server-side apply（服务端应用）说明
- OpenShift v4.6 OAuth2 server 认证的原理？
- OpenShift v4.10 集群中的证书工作机制？集群内部使用的证书由集群内部自身管理，不可使用用户自定义的证书，该功能直至 v4.11 版本依然未被解决，而 `router pod` 与 `web console pod` 的证书可替换为用户自定义的证书。
- OpenShift v4.10 etcd 数据库的备份与恢复？
- 如何理解 OpenShift v4.10 中的 `extension APIs` 与 `Operator` 概念？

### K8s NetworkPolicy 与 Calico NetworkPolicy 间的关系

- kubernetes v1.8 版本开始支持 `networkpolicy` 类型的 API，但其原生支持的功能较弱。
- 因此，可以使用 `calico` 的 networkpolicy 做功能的扩展。
- `nerdctl` 的端口不是用户态监听的，而是直接 `iptables nat` 转发的。

## CNI 如何玩转 Linux Bridge

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

## 机器学习

- 函数 y=1/(1+e^(-x)) 求导？
- 超平面与法向量详细介绍？
- 如何求直线方程与平面方程的法向量？
- 点到超平面距离公式如何推导？超平面表达式如何推导？
- 向量内积的柯西·施瓦茨不等式？
- [十大回归算法与代码示例](https://mp.weixin.qq.com/mp/appmsgalbum?action=getalbum&__biz=MzkwNjY5NDU2OQ==&scene=1&album_id=3510733534568808456&count=3#wechat_redirect)
- 详细描述线性回归的训练过程？
- 最小二乘法的理解及推导过程？
- 举例说明最小二乘法的具体步骤？
- 均方根误差是不是通过最小二乘法推导而来的？
- 如何使用 python 绘制曲面与平面相切及其切点？
- 机器学习中的 AC、SN、MCC、MSE、RMSE？
- NumPy 数组表示的矩阵维度，请给出具体示例？
- 如何修改 ipynb 的字体以及安装 Git 扩展？
- 误差反向传播算法 (BP) 的详细数学推导？

## 深度学习

- tensorflow.keras.datasets 下载的数据存放路径是什么？
- 下采样操作是什么？
- CNN 训练过程中反向传播在哪些阶段实现？
- CNN 中每个卷积层中的不同卷积核的值是如何确定的？
- 为什么激活函数可以有效解决梯度消失问题？
- 深度学习中为什么会出现梯度消失与梯度爆炸，请详细说明原因？

## 大模型

- 位置编码与词向量化哪个先执行？
- LLM 提取 token 的 python 示例 (各大模型的方式存在差异)？
- tokenizer 不属于 Transformer 的一部分，而是每个模型实现的一部分，是吗？
- 为什么每个 Transformer 模型都需要一个特定的 tokenizer？
- BPE 是什么？
  