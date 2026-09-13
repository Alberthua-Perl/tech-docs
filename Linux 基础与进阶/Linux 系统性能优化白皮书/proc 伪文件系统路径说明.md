# /proc 伪文件系统路径说明

## 文档目录

- [/proc 伪文件系统路径说明](#proc-伪文件系统路径说明)
  - [文档目录](#文档目录)
  - [Linux 内核参数帮助文档](#linux-内核参数帮助文档)
  - [/proc 伪文件系统说明](#proc-伪文件系统说明)
  - [sysctl 命令的常用方式](#sysctl-命令的常用方式)
  - [/proc 中常用的子目录与子文件](#proc-中常用的子目录与子文件)

## Linux 内核参数帮助文档

- 内核源码参考：`linux-<version>/Documentation/sysctl`
- 查看内核参数详细说明：`man 5 proc`

## /proc 伪文件系统说明

- 在内核启动过程中 proc 伪文件系统被挂载至 /proc。
- 当加载静态内核，或后续加载动态内核模块或驱动程序时，会在 /proc 目录下自动创建文件。
- 可通过更新配置 proc 伪文件系统中的对应文件来实现内核的静态优化，通常可使用 sysctl 命令实时更新内核参数，或者更新 sysctl 配置文件以实现永久更新内核参数。
- 查看 sysctl 的 `--system` 选项说明，系统启动过程中读取配置文件的优先级 **从上往下** 依次降低，建议使用 `/etc/sysctl.d/*.conf` 配置文件将自定义的内核参数与系统默认的内核参数解耦，分别管理。
  
  ```plaintext
  --system
        Load settings from all system configuration files. Files are read from  directories  in  the  following
        list  in  given  order  from top to bottom.  Once a file of a given filename is loaded, any file of the
        same name in subsequent directories is ignored.
        /run/sysctl.d/*.conf
        /etc/sysctl.d/*.conf
        /usr/local/lib/sysctl.d/*.conf
        /usr/lib/sysctl.d/*.conf
        /lib/sysctl.d/*.conf
        /etc/sysctl.conf
  ```

## sysctl 命令的常用方式

```bash
$ sudo sysctl -a
# 查看当前可用的所有内核参数

$ sudo sysctl -q <kernel_arg>
# 查看指定的内核参数当前值

$ sudo sysctl --system
# 加载系统上所有的配置文件中的内核参数

$ sudo sysctl -p /path/to/sysctl-conf
# 加载指定的 sysctl 配置文件使内核参数永久生效。
```

## /proc 中常用的子目录与子文件

| 路径 | 说明 |
| ----- | ----- |
| /proc/cpuinfo | 提供处理器的信息 |
| /proc/meminfo | 提供全局内存使用的信息 |
| /proc/swaps | 提供交换空间使用的信息 |
| /proc/*pid*/ | 提供系统上特定进程的信息 |
| **/proc/*pid*/maps** | 提供特定进程的线性内存中各段的布局信息 |
| **/proc/*pid*/smaps** | 提供特定进程在线性内存中各段的详细内存布局情况 |
| **/proc/*pid*/status** | 提供特定进程的状态信息 |
| /proc/cmdline | 提供最近用于启动内核的内核参数和选项 |
| /proc/interrupts | 提供各个逻辑 CPU 核心上各类硬中断数量的分布状态 |
| /proc/irq/<irq_num> | 提供各个硬中断对逻辑 CPU 亲和性的信息，其中硬中断号可从 /proc/interrupts 获取。 |
| /proc/mdstat | 查看软 RAID 磁盘阵列的状态 |
| /proc/scsi/scsi | 查看系统全部 SCSI 设备信息 |
| **/proc/net/dev** | 查看网络接口的实时状态 |
| /proc/net/bonding/bond0 | 查看双网卡的链路聚合状态 |
| **/proc/sys/** | 子目录中包含用于内核优化的可配置参数，大部分文件均可通过 root 写入。 |
| /proc/sys/kernel/osrelease | 查看系统发行版类型 |
| /proc/sys/kernel/threads-max | 查看系统上支持的最大线程数量 |
| /proc/filesystems | 查看系统已加载的文件系统类型 |

```bash
$ sudo cat /etc/filesystems
# 查看系统指定的测试挂载文件系统类型

$ sudo ls -ld /lib/modules/$(uname -r)/kernel/fs/
# Linux 内核支持的文件系统驱动程序目录
```

💥 **注意：**

- 修改 /proc/sys/ 中的文件会导致正在运行的系统立即发生变化。
- 优化目的是改善系统性能，但修改 /proc/sys/ 也可能对其他子系统产生不利影响。
- 所有 /proc/sys/ 文件更改均直接写入内存中的内核，因此所作更改无法在重启后保持。
