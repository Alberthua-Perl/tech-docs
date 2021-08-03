## Linux 常用排错命令与工具

#### 文档说明：

- 该文档中涉及的命令与参考链接仅作为工具资料提供排错思路或依据。
- 若更深层次的分析与追踪故障原因需配合业务应用代码与 kernel 源码等进一步分析。
- 该文档将根据所使用的命令持续更新其用法与使用案例。

#### 文档目录：

- journalctl 命令使用示例
- sosreport 命令使用示例
- Performance Co-Pilot (PCP) 软件包使用示例
- systemctl 命令使用示例
- 物理 CPU 个数、CPU 核心数、逻辑 CPU 个数、超线程的关系
- CPU 信息查看相关命令示例
- dmidecode 命令使用示例
- 硬件设备调试相关命令示例
- 常见物理服务器及硬件示例
- Kernel module 与虚拟化相关命令示例
- Linux 存储栈（storage stack）相关命令示例
- rpm 命令使用示例
- yum 命令使用示例
- 基础网络调试相关命令示例
- 内存泄漏与内存溢出
- 共享库相关命令示例
- 系统调用与库调用
- strace 与 ltrace 命令使用示例

#### journalctl 命令使用示例：

```bash
$ journalctl -n <number>
# 指定显示最后 10 条日志，也可以指定条目数量。

$ journalctl -ef
# 实时刷新最新的日志
$ journalctl _SYSTEMD_UNIT=<unit_file_name>
# 查看 systemd 单元的日志信息
# 查看 systemd.journal-filelds(7) man 该帮助获取更加详细的搜索字段。
$ journalctl -u <unit_file_name>
# 查看 systemd 单元的日志信息

$ journalctl -p <priority>
# 显示 debug、info、notice、warning、err、crit、alert 和 emerg 该级别及其之上的日志。
$ journalctl -p emerg..err
# 查看 emerg 到 err 级别的日志信息

$ journalctl --since today
# 查看当天的所有日志信息
$ journalctl --since "-1 hour"
# 查看前 1 小时的所有日志信息
$ journalctl --since "YYYY-MM-DD hh:mm:ss" --until "YYYY-MM-DD hh:mm:ss"
# 显示指定时间范围内的日志，必须使用双引号。
# 如果省略日期，则命令会假定日期为当天；如果省略时间，则命令假定为自 00:00:00 起的整天。
# yesterday、today 与 tomorrow 可以指定日志时间段，可参考 systemd.time(7) man 帮助。

$ journalctl -o verbose
# 显示更加详细的日志信息

$ journalctl -b --list-boots
# 查看系统重启的次数与信息
$ journalctl -b <number>
# 查看指定重启的详细信息 
```

#### sosreport 命令使用示例：

```bash
$ sosreport -l
# 列出 SOS 支持的插件与可用的选项
$ sosreport -e <plugin_name>
# 启用当前禁用的插件
$ sosreport -n <plugin_name>
# 禁用当前已启用的插件
$ sosreport -k <plugin_option>
$ sosreport -k xfs.logprint
# 使用 xfs.logprint 选项以收集 XFS 文件系统的相关信息 
```

#### Performance Co-Pilot (PCP) 软件包使用示例：

```bash
$ yum install -y pcp pcp-gui
# 安装 PCP 与 PCP 图形化软件包
$ systemctl enable --now pmcd.service pmlogger.service
# 启动并开机自启 pmcd 与 pmlogger 守护进程
# pmlogger 服务将指标日志存储于 /var/log/pcp/pmlogger/<hostname>/ 目录中

$ pminfo
# 查看 Co-Pilot 数据库中的性能指标的类型，可通过 pmval 命令列出数据库中的数据。
$ pminfo -dt <metrics_type>
# 查看指定指标类型的说明
$ pminfo -dt kernel.percpu.cpu.idle

$ pmval -s 5 -t 2 proc.nprocs
  metric:    proc.nprocs
  host:      servera.lab.example.com
  semantics: instantaneous value
  units:     none
  samples:   5
  interval:  2.00 sec
          111
          111
          111
          111
          111
# 实时刷新时间间隔 2 秒，共统计 5 次的瞬时进程数。
$ pmval -a /var/log/pcp/pmlogger/workstation.lab.example.com/20210609.14.52.0 <metrics_type>
# 查看默认指标数据归档文件中指定的指标类型日志
# -a 选项指定性能指标的归档日志

$ pmstat -s <sample_number> -t <number>[seconds|minutes] 
# 高层次的系统性能查看工具，在指定的时间间隔内（默认 5 秒刷新一次），共统计指定次数（类似于 vmstat 命令）。
$ pmatop
# 实时刷新系统资源使用信息（类似于 top 命令）
```

PCP 软件包除提供命令行模式的性能指标输出外，还提供 `GUI` 图形化界面及 Web 图形化界面，并可与 `Grafana` 集成显示。
该软件包提供强大而丰富的系统性能监控指标与参数，关于 PCP 软件包及相关命令的使用方法，可参考如下 `Red Hat Access` 链接获取更为详细的技术指导：

- RHEL 7 性能监控之 PCP：
  http://www.361way.com/rhel7-pcp/5149.html
  
- How do I install Performance Co-Pilot (PCP) on my RHEL server to capture performance logs：
  https://access.redhat.com/solutions/1137023

- **Index of Performance Co-Pilot (PCP) articles, solutions, tutorials and white papers**：
  **https://access.redhat.com/articles/1145953**
  
- Interactive web interface for Performance Co-Pilot：
  https://access.redhat.com/articles/1378113

- Introduction to storage performance analysis with PCP：
  https://access.redhat.com/articles/2450251

#### systemctl 命令使用示例：

```bash
$ systemctl list-dependencies <unit_name>
# 列出指定单元文件依赖的其他单元文件，绿色点表示处于 active 状态，红色点表示 inactive 状态。
$ yum install -y graphviz
# 安装单元文件依赖的图形化处理工具
$ systemd-analyze dot <unit_name> | dot -Tsvg > <filename>.svg
# 生成指定单元文件的依赖性关系图
$ man 5 systemd.unit
# systemd 单元文件的详细说明文档

$ systemctl enable debug-shell.service
# 启用 debug-shell 服务
# 注意：该服务运行于 TTY9 上，并且不需要 root 密码即可登录，对能够访问本地控制台的用户开放，使用完毕后需及时关闭该服务！
```

可在系统 GRUB2 引导时进行中断，使用 `systemd.unit=emergency.target` 参数进入紧急模式，该模式中 **`/`** 为 `ro`（只读状态），而使用 `systemd.unit=rescue.target` 参数进入救援模式，该模式中 **`/`** 为 `rw`（读写）状态。

关于 systemd 更为详尽的指导可参考 https://access.redhat.com/articles/754933

#### 物理 CPU 个数、CPU 核心数、逻辑 CPU 个数、超线程的关系：

Linux 内核会将多核 CPU 当做多个单核 CPU 来识别，如 Linux 会将 2 个 4 核的 CPU 当做 8 个单核 CPU 来识别，但两者的性能并不完全等价！

1. 物理 CPU 个数（physical id）：服务器主板插槽（socket）上实际插入的 CPU 个数

2. CPU 核心数（cpu cores or core id）：单块 CPU 上面处理数据的芯片组的数量，如双核、四核等。

3. 逻辑 CPU 个数（processor）：

   物理 CPU 个数（physical id）x 每颗 CPU 的核心数（core id）x 每个核心的超线程数（CPU 支持的话）

CPU 在同一时刻只能处理一个任务，相当于一个 CPU 核心在同一时刻只能执行一个线程，而采用超线程技术，就可以将单个核心当做多个核心来使用，即同一时刻可处理多个任务，提高 CPU 性能。安装的CPU数量越多，从超线程获得的性能方面的提高就越少，这种情况下反而会造成系统资源浪费。

#### CPU 信息查看相关命令示例：

```bash
$ grep 'model name' /proc/cpuinfo | cut -d ':' -f 2 | uniq -c
# 查看 CPU 型号与逻辑 CPU 的个数
$ grep 'physical id' /proc/cpuinfo | uniq | wc -l
# 查看主机上物理 CPU 个数

$ grep 'core id' /proc/cpuinfo | sort | uniq | wc -l
$ grep 'cpu cores' /proc/cpuinfo | uniq | cut -d ':' -f 2 | sed 's/^[[:space:]]*//'
# 查看每个物理 CPU 的核心数
# 注意：使用 uniq 命令时必须将重复的行排序到一起后才能去除，否则不能去除重复！

$ grep 'processor' /proc/cpuinfo | wc -l
$ lscpu | grep '^CPU(s)' | cut -d ':' -f 2 | sed 's/^[[:space:]]*//'
$ lscpu | grep '^CPU(s)' | awk -F ':' '{ print $2 }' | sed 's/^[[:space:]]*//'
# 查看主机上逻辑 CPU 的个数
# 使用 lscpu 命令查看 CPU 信息，其中的 "CPU(s)" 为逻辑 CPU 数目。

$ getconf LONG_BIT
# 查看 CPU 的工作模式，即 32-bit 或 64-bit。
# 若显示返回 32，不代表不支持 64-bit。
$ grep 'flags' /proc/cpuinfo | grep 'lm' | wc -l
# 查看 CPU 是否支持 64-bit，若返回值大于 0，则支持 64-bit。
# lm 指 long mode，支持 lm 则支持 64-bit。
```

#### dmidecode 命令使用示例：

```bash
$ man dmidecode
# 查看 dmidecode 命令的详细使用方法
# 可搜索 TYPES 关键字查询该命令支持的 DMI 类型，也可搜索 -s 查看该命令支持的类型关键字。

$ dmidecode | grep 'Product Name'
# 查看服务器型号
$ dmidecode | grep 'Serial Number' | grep -Ev "Not|None"
# 查看服务器主板序列号
$ dmidecode -s system-serial-number
# 查看服务器系统序列号

$ dmidecode -t Memory | grep -P -A5 "Memory\s+Device" | grep Size | sed -s 's/^[ \t]*//g'
# 查看系统当前已安装的内存大小与内存槽位数
# -P 选项为 grep 命令支持 Perl 正则表达式
$ dmidecode -t Memory | grep -P -A16 "Memory\s+Device" | grep Speed
# 查看已安装的内存速率，未安装内存的槽位其速率为 Unknown。
$ dmidecode -t 16
  # dmidecode 3.2
  Getting SMBIOS data from sysfs.
  SMBIOS 2.7 present.

  Handle 0x01A2, DMI type 16, 23 bytes
  Physical Memory Array
          Location: System Board Or Motherboard
          Use: System Memory
          Error Correction Type: None
          Maximum Capacity: 161 GB
          // 系统支持的最大扩展内存
          Error Information Handle: Not Provided
          Number Of Devices: 64
          // 系统支持的内存插槽数，单根内存条最大支持容量为 "Maximum Capacity/Number Of Devices"。
# 16 代表 Physical Memory Array
# 注意： 需考虑内存槽位是否被占满，以及是否可被扩展到最大内存。
$ dmidecode -t 16 | grep "Maximum Capacity" | sed -s 's/^[ \t]*//g'
# 查看系统可支持的最大内存数

$ dmidecode -t 0,1
# 同时查看两种 DMI 类型的信息（BIOS 与 processor）
```

`dmidecode` 允许在 Linux 系统下获取有关硬件方面的信息，其遵循 `SMBIOS`（System Management BIOS）/ `DMI`（Desktop Management Interface） 标准，该标准由 DMTF（Desktop Management Task Force）开发，其输出的信息包括 BIOS、系统、主板、处理器、内存、缓存等等。

`DMI`（Desktop Management Interface）充当了管理工具和系统层之间接口的角色。它建立了标准的可管理系统更加方便了计算机厂商和用户对系统的了解。DMI 的主要组成部分是 Management Information Format（MIF）数据库。这个数据库包括了所有有关计算机系统和配件的信息。通过 DMI，用户可以获取序列号、计算机厂商、串口信息以及其它系统配件信息。

查看硬件相关的命令与文件：

- lshw 命令：

  通过读取 `/proc` 目录下各种文件的内容和 DMI 表来生成硬件信息。

- /sys/class/dm/id/：

  该目录中具有部分 DMI 信息。

- dmesg 命令：

  在 Linux 上 syslogd 或 klogd 启动前用来记录内核消息（启动阶段的消息）。
  
  它通过读取内核的环形缓冲区来获取数据，在排查问题或只是尝试获取系统硬件信息时，该命令非常有用。
  
- hwinfo 命令：

  可提供比 lshw、dmidecode、dmesg 命令更为详细的硬件信息。

  它使用 libhd 库 libhd.so 来收集系统上的硬件信息。

  该工具是为 OpenSuSE 特别设计的，后来其它发行版也将它包含在其官方仓库中（RHEL中来自 EPEL 源）。

  ```bash
  $ ldd $(which hwinfo)
          linux-vdso.so.1 (0x00007ffde40f9000)
          libhd.so.21 => /lib64/libhd.so.21 (0x00007f70bb4b7000)
          libc.so.6 => /lib64/libc.so.6 (0x00007f70bb0f5000)
          libx86emu.so.1 => /lib64/libx86emu.so.1 (0x00007f70baed1000)
          /lib64/ld-linux-x86-64.so.2 (0x00007f70bbba2000)
  ```

关于 dmidecode 命令的使用可参考如下链接：

- linux下查看主板内存槽与内存信息（dmidecode）：

  https://blog.csdn.net/d12345678a/article/details/53908889

- dmidecode命令详解（获取硬件信息）：

  http://www.linuxidc.com/Linux/2015-12/126814.htm

#### 硬件设备调试相关命令示例：

```bash
# ----- Detect different driver disk device ----- 
$ hdparm -I /dev/sda
# 查看或设置 IDE/SATA 设备的参数

$ lsscsi -v
  [3:0:0:0]    cd/dvd  NECVMWar VMware SATA CD01 1.00  /dev/sr0
    dir: /sys/bus/scsi/devices/3:0:0:0  [/sys/devices/pci0000:00/0000:00:11.0/0000:02:04.0/ata4/host3/target3:0:0/3:0:0:0]
  [26:0:1:0]   disk    VMware,  VMware Virtual S 1.0   /dev/sda
    dir: /sys/bus/scsi/devices/26:0:1:0  [/sys/devices/pci0000:00/0000:00:10.0/host26/target26:0:1/26:0:1:0]
  list_ndevices: scandir: /sys/class/nvme/: No such file or directory
  NVMe module may not be loaded
# 列举 SCSI 设备与 NVMe 设备，该命令从 sysfs 文件系统中读取设备相关信息。

# ----- Detect PCI device -----
$ lspci -v
# 查看所有 PCI 设备的概要信息
$ lspci -v -k -nn -s <bus>:<slot>.<function>
# 根据 PCI 的插槽位置信息查看该设备的详细信息

$ lsusb -v
# 查看系统上的 USB 设备信息

# ----- Hardware error report tools -----
$ yum install -y mcelog
# 安装 x86 平台硬件故障报告工具 mcelog 软件包，主要针对 CPU 与内存的检测。
$ systemctl start mcelog.service
$ systemctl enable mcelog.service
# 启动 mcelog 服务并设置开机自启动
# mcelog 服务可以守护进程的方式运行（推荐），也可以 cron 计划任务的方式运行（该方式存在无法记录硬件故障的问题）。
# 计划任务方式运行的 crontab 文件位于 /etc/cron.hourly/mcelog.cron

$ yum install rasdaemon
# 安装 rasdaemon 软件包，可使用该软件包替代 mcelog。
$ systemctl start rasdaemon.service
$ systemctl enable rasdaemon.service
# 启动 rasdaemon 服务并设置开机自启动
$ ras-mc-ctl --errors
# 查看硬件错误日志信息

# ----- Memory test tool -----
$ yum install -y memtest86+
# 安装 memtest86+ 内存测试软件包
$ memtest-setup
# 该命令将在 /etc/grub.d/ 目录中生成 20_memtest86+ 脚本，因此需重新生成 GRUB2 主配置文件。 
$ grub2-mkconfig -o /boot/grub2/grub.cfg
# 重新生成 Legency 模式的 GRUB2 引导文件
# 可在系统引导界面进入内存测试界面进行测试
# 注意：memtest86+ 内存测试通常在物理机上运行测试，而不是虚拟机上运行！
```

![](https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/nic-pci-info.jpg)

<img src="https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/memtest86-test.jpg" style="zoom:;" />

#### 常见物理服务器及硬件示例：

![](https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/general-hardware-info.png)

#### Kernel module 与虚拟化相关命令示例：

```bash
# ----- Kernel module command -----
$ lsmod
# 查看当前已加载的内核模块列表
$ modprobe -v <module_name>
# 加载指定的内核模块
$ modprobe -rv <module_name>
# 卸载指定的模块（依然可重新加载）
$ modinfo -p <module_name>
# 查看内核模块的选项
$ cd /sys/module/<module_name>/parameters/; ls -lh
# 该目录用于存放内核模块的选项

$ modprobe -v st buffer_kbs=64
  insmod /lib/modules/3.10.0-327.el7.x86_64/kernel/drivers/scsi/st.ko buffer_kbs=64
# 加载内核模块时可同时指定相应的选项

$ vim /etc/modprobe.d/st.conf
  options st buffer_kbs=64 max_sg_segs=512
# 可在该文件中创建指定内核模块的选项，当模块加载时将自动加载该选项。

# ----- KVM virtualization -----
$ modprobe -v kvm_intel
$kvm_amd
# 加载 KVM 在不同 CPU 平台下的内核模块，若加载报错，说明 CPU 不支持虚拟化或在 BIOS/UEFI 中禁用虚拟化。
# Intel CPU 支持虚拟化的 flag 为 vmx，AMD CPU 支持虚拟化的 flag 为 svm。

$ virsh capabilities
# 查看 hypervisor 支持的虚拟化类型（默认为 qemu-kvm 虚拟化）
$ virsh nodecpustats --percent
# 查看 1 秒内 hypervisor 的 CPU 使用百分比
$ virsh nodememstats
# 查看 hypervisor 的内存使用情况

$ xmllint <filename>.xml
# 判断 XML 文件语法的合法性（单纯语法检查）
$ virt-xml-validate <kvm_domain_filename>.xml
# 判断 KVM 虚拟机 XML 定义文件的合法性（字段及标签等检查）
```

![](https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/lsmod-cmd.jpg)

#### Linux 存储栈（storage stack）相关命令示例：

```bash
# ----- Linux memory manage -----
# VFS also maintains several caches to improve storage I/O performance, the inode cache, dentry
# cache, buffer cache, and page cache. Of these, the most important is the page cache. The page
# cache is dynamically allocated from free memory on the system, and is used to cache pages of
# data from ﬁles being read or written.
$ man 5 proc
# 搜索 /proc/sys/vm/drop_caches 的详细信息

$ echo 3 > /proc/sys/vm/drop_caches
# 同时清除 page cache、dentries cache 与 inodes cache
# 注意：以上 3 种类型的 cache 均由 VFS 虚拟文件系统管理分配至空余内存中，并且清除 cache 前需提前执行 sync 操作！

# ----- Device Mapper frame -----
# Device mapper is a powerful mechanism in the kernel to create 1:1 mappings of blocks in one
# block device to blocks in another, logical block device.
$ dmsetup ls
# 查看 devicemapper 映射的逻辑设备列表

$ lsblk [--path]
NAME            MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
sr0              11:0    1    1M  0 rom
vda             253:0    0   10G  0 disk
└─vda1          253:1    0   10G  0 part /
vdb             253:16   0    1G  0 disk
├─vdb1          253:17   0  200M  0 part
│ └─myvg1-mylv1 252:0    0  260M  0 lvm
└─vdb2          253:18   0  100M  0 part
  └─myvg1-mylv1 252:0    0  260M  0 lvm
# 预先创建相应的卷组与逻辑卷
$ dmsetup table /dev/mapper/myvg1-mylv1
  0 401408 linear 253:17 2048
  401408 131072 linear 253:18 2048
# 查看 devicemapper 映射的逻辑设备映射表，其中可确定逻辑设备与物理设备之间的线性（linear）映射关系（1:1）。
# 计算逻辑卷容量是否一致：(401408 sectors + 131072 sectors) * 512 byte / 1024 / 1024 = 260MiB

# ----- Disk IO scheduler -----
$ cat /sys/block/<device>/queue/scheduler
# 查看指定磁盘设备的 IO 调度算法，一般为 deadline、cfq、noop 三种。
$ echo deadline > /sys/block/<device>/queue/scheduler
# 更改磁盘设备 IO 调度算法

# ----- Filesystem repair -----
# The data migration can be performed with various common utilities such as dd. 
# This will provide the functional hardware foundation that is required for ﬁle system maintenance.
### ext2/ext3/ext4 filesystem repair ###
$ e2fsck -n <filesystem>
# 将文件系统设置为只读模式，检测文件系统故障但不进行修复。
$ e2fsck -y <filesystem>
# 不进入交互模式直接修复
$ dumpe2fs <filesystem>
# 查看 ext2/ext3/ext4 文件系统的超级块（super block）与块组（blocks group）的信息
$ dumpe2fs <filesystem> | grep -i 'Backup superblock'
# 查看文件系统的备份的 superblock
$ e2fsck -n <filesystem> -b <superblock>
# 使用备份的 superblock 恢复故障的 superblock
### xfs filesystem repair ###
$ xfs_repair -n <filesystem>
# 检测 xfs 文件系统故障但不进行修复
$ xfs_repair <filesystem>
# 自动修复 xfs 文件系统故障

$ diff -s <file1> <file2>
# 判断两个文件的内容是否完全相同

# ----- Crypt LUKS disk -----
$ dmsetup ls --target crypt
# 查看已加密的 devicemapper 逻辑设备
$ cryptsetup luksHeaderBackup <device> --header-backup-file <backup_file>
# 备份 LUKS 加密设备的 header 头部信息（包括 key slot 信息）至新的备份文件中
$ cryptsetup luksHeaderRestore <device> --header-backup-file <backup_file>
# 通过备份的 header 头部信息文件恢复 LUKS 加密设备，该操作将覆盖当前的 header 头部信息，即清除加密的 key slot。
# 注意：
#   清除 LUKS 加密设备的 key slot 方法：
#   1. 添加新的 key slot 密钥，再通过新密钥删除旧密钥。
#   2. 直接使用备份的 header 头部信息文件覆盖当前的 header 以去除当前的 key slot。
$ cryptsetup luksOpen <device> <dm_logical_device_name>
# 映射加密的 LUKS 设备至 devicemapper 逻辑设备
```

关于磁盘 IO 调度的相关技术信息可参考如下链接：

- Understanding the Deadline IO Scheduler：

  https://access.redhat.com/articles/425823

- Using the Deadline IO Scheduler：

  https://access.redhat.com/solutions/32376

- Understanding the Noop IO Scheduler：

  https://access.redhat.com/articles/46958

- Unable to change IO scheduler for virtio disk /dev/vda in RHEL 7.1：

  https://access.redhat.com/solutions/1305843

- RHEL7 Storage Docs：

  https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/7.2_release_notes/storage#idp1704576

关于 cryptsetup 加密的相关技术信息可参考如下链接：

- All about LUKS, cryptsetup, and dm-crypt：

  https://access.redhat.com/articles/193443

- How to recover lost LUKS key or passphrase：

  https://access.redhat.com/solutions/1543373

#### rpm 命令使用示例：

1. RPM GPG 公钥保存目录：**`/etc/pki/rpm-gpg`**
2. RPM 软件包中文件的状态标识：man rpm -> 搜索 -V

![](https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/rpm-verify.JPG)

```bash
rpm 命令常用选项：
  -a，--all                查询所有的软件包
  -i，--install            安装软件包
  -c，--configfiles        列出软件包的所有配置文件
  -d，--docfiles           列出软件包的所有文档文件
  -U，--upgrade            升级软件包
  -e，--erase              卸载软件包
  -v，--verbose            查看详细的输出内容
  -h，--hash               查看软件包安装过程中的哈希标志
  -q，--query              查询软件包相关信息
  -f，--file               查询指定文件所属的软件包
  -p，--package            查询或确认未安装的软件包文件（该软件包必须提前下载）
  -l，--list               列出软件包中的文件
  --info                   查询软件包的详细信息
  --nodeps                 忽略软件包的依赖关系
  --import                 导入软件包的 GPG 公钥
  --allmatches             卸载匹配的全部软件包
  --scripts                列出软件包安装、升级与卸载过程中的所有脚本文件
  --changelog              列出软件包的更改信息

# ----- RPM package examples -----
$ rpm -evh --nodeps <package_name>                           
# 卸载指定软件包，但不卸载依赖的软件包。
$ rpm -qpl <not_installed_package_name>                      
# 查看未安装的软件包中的文件

$ rpm -q --info <package_name>
# 查看已安装软件包的详细信息
$ rpm -ql <package_name>                           
# 查看已安装的软件包中的文件
$ rpm -qc <package_name>
# 查看当前已安装的软件包中的配置文件
$ rpm -qd <package_name>
# 查看当前已安装的软件包中的文档
$ rpm -qf <filename>
# 查询指定文件所属的软件包
$ rpm -q --scripts <package_name>
# 查看当前已安装的软件包中安装、升级与卸载过程中运行的脚本
$ rpm -q --changelog <package_name>
# 查看当前已安装的软件包更改信息

# ----- RPM GPG key examples -----
$ rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-<pubkey_name>
# 导入软件包 GPG 公钥至 RPM 数据库中，导入的 GPG 公钥使用 ID 进行区分。
# 注意：
#   1. 可将 GPG 公钥导入 RPM 数据库，再将该 GPG 公钥文件保存至该目录中。
#   2. 只有导入 RPM 数据库的 GPG 公钥，才能使用 rpm 命令查看。
#   3. 保存至该目录中的 GPG 公钥，可防止因初始化 RPM 数据库而造成的 GPG 公钥丢失！ 
         
$ rpm --import https://<url_of_rpm_gpg_key>
# 从相应 URL 上导入 GPG 公钥至 RPM 数据库中
$ rpm -q gpg-pubkey                                          
# 查看已安装导入至 RPM 数据库中的 GPG 公钥
$ rpm -e --allmatches gpg-pubkey-<id>
# 从 RPM 数据库中删除相应的 GPG 公钥
$ rpm -q --info gpg-pubkey-<id>
# 查看相应RPM数据库中 GPG 公钥信息

# ----- rpm2cpio command examples -----
$ rpm2cpio <package_name>.rpm | cpio -it
# 查看未安装的 rpm 软件包中的文件
$ rpm2cpio <package_name>.rpm | cpio -id
# 提取 rpm 软件包文件至当前工作目录中
$ rpm2cpio <package_name>.rpm | cpio -id "*txt"
# 提取 rpm 软件包中的指定文件

# ----- Recover a corrupt RPM database -----
$ lsof | grep /var/lib/rpm
# 确认 /var/lib/rpm 目录是否被进程占用
$ rm /var/lib/rpm/__db*
# 删除 RPM 数据库索引文件
$ tar -jcvf rpmdb-$(date +%Y%m%d-%H%M).tar.bz2 /var/lib/rpm
# 备份原始 RPM 数据库
$ cd /var/lib/rpm
$ /usr/lib/rpm/rpmdb_verify Packages
# 确认 RPM 数据库的合法性
$ mv Packages Packages.broken
# 更改原始 RPM 数据库名称
$ /usr/lib/rpm/rpmdb_dump Packages.broken | /usr/lib/rpm/rpmdb_load Packages
# 修复原始 RPM 数据库
$ /usr/lib/rpm/rpmdb_verify Packages
  BDB5105 Verification of Packages succeeded.
# 确认修复的 RPM 数据库的合法性
$ rpm -v --rebuilddb
  error: rpmdbNextIterator: skipping h#       4 region trailer: BAD, tag 0 type 0 offset 0 count 0
# 重建 RPM 数据库的索引文件
$ rpm -qa > /dev/null
# 再次查询软件包确认数据库正确

# ----- Verify RPM package file status -----
$ rpm -V <package_name>
# 查看指定软件包中文件的更改状态，若相关文件未发生更改则无返回信息，发生更改的文件具有属性更改信息返回。
$ rpm -V openssh-server
  SM5....T.  c /etc/ssh/sshd_config
# /etc/ssh/sshd_config 文件相较于原始 RPM 软件包中发生了文件大小、文件权限、文件内容、修改时间的更改。   
$ rpm -Va
# 查看所有软件包中文件的更改状态
$ rpm --setperms <package_name>
# 恢复文件为软件包中定义的权限
```

#### yum 命令使用示例：

1. 配置 yum 软件源优先级：

   1）CentOS 7.x/RHEL 7.x 配置 yum 软件源优先级时，需安装 **`yum-plugin-priorities`** 软件包。

   2）yum 插件的配置文件目录：**`/etc/yum/pluginconf.d/`**

   3）yum 软件源优先级功能是否启用：**`/etc/yum/pluginconf.d/priorities.conf`**

      ![](https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/yum-priority-1.png)

   4）编辑 **`/etc/yum.repos.d/*.repo`** 文件

      <img src="https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/yum-priority-2.png" style="zoom:80%;" />

   5）priority=*N*（取值 **`1~99`**），数值越大优先级越低。

```bash
yum 命令常用选项：
  --enablerepo=<repo_name>     使用一个或多个 yum 软件仓库（可使用 wildcard 通配符）
  --disablerepo=<repo_name>    禁用一个或多个 yum 软件仓库（可使用 wildcard 通配符）
  --showduplicates             查看软件仓库中不同版本的软件包（默认只显示最新版）
  --nogpgcheck                 禁用 GPG 签名检查
  --installroot=<path>         指定 rpm 软件包的安装根路径

$ yum --disablerepo=\* --enablerepo=elrepo-kernel list available
# 查看指定的 yum 软件仓库 elrepo-kernel 中可用的软件包
$ yum provides <filename>
# 查看文件由哪些包提供
$ yum whatprovides <command>
# 查看命令由哪些包提供
$ yum deplist <package_name>
# 查看安装包的依赖文件及提供软件包的信息
# 注意：
#   $ rpm -qR <package_name>
#   # 查看软件包的依赖文件

$ yum list --showduplicates <package_name>
# 查看软件源中所有可用的软件包
$ yum list --showduplicates gitlab-ce
# 查看 GitLab 源中所有可用的不同版本 gitlab-ce 软件包

$ yum reinstall -y <package_name>
# 重新安装相关软件包
$ yum reinstall -y kernel
# 重新安装 kernel 软件包 
# 使用场景：若误删 vmlinuz-* 内核文件或 initramfs-* 内存文件系统ramdisk，可使用该方法重新安装内核恢复。  

$ yum downgrade -y <package_name>
# 更新指定软件包为旧版本
# 注意：
#   $ rpm -U --oldpackage <package_name>
#   # a

$ yum install -y yum-plugin-versionlock
# 安装 yum 版本锁定插件软件包
$ yum versionlock add <package_name_wildcard>
# 将指定的软件包进行版本锁定
$ yum versionlock delete <package_name_wildcard>
# 去除指定软件包的版本锁定
$ yum versionlock clear
# 去除所有软件包的版本锁定
$ yum versionlock list
# 查看版本锁定的软件包
# 注意：指定软件包一定版本锁定，将无法进行升级与降级！

$ yumdownloader --destdir <dest_dir> <package_name>
# 下载软件包至指定目录中

$ yum grouplist
$ yum groupinfo "<group_name>"
$ yum groupinstall "<group_name>" \
  -y --setopt=group_package_types=mandatory,default,optional
# RHEL 7.x/8.x：根据包组中软件包的类型安装相应包组软件
# 注意：
#   $ yum groupinstall "Legacy UNIX Compatibility" \
#     -y --setopt=group_package_types=mandatory,default,optional

$ yum install -y yum-plugin-verify
# 安装 yum-plugin-verify 软件包，用于确认软件包中文件的状态。
$ yum verify <package_name>
# 只确认软件包的状态，不确认软件包配置文件的状态。
$ yum verify-rpm <package_name>
# 确认软件包配置文件的状态
$ yum verify-rpm vsftpd
  Loaded plugins: langpacks, search-disabled-repos, verify
  ==================== Installed Packages ====================
  vsftpd.x86_64 : Very Secure Ftp Daemon
      File: /etc/vsftpd/vsftpd.conf
      Tags: configuration
          Problem:  checksum does not match
          Current:  sha256:37bd013bbecaf93450ba74cbd85c9dcc60d6c47822b0c1e404d6057eba779eb3
          Original: sha256:6e011bfb61a81c33377e78ed1aaef6d204516d390dd6a3daf973ce82ec7eadf2
                                     --------
          Problem:  size does not match
          Current:          5029 B
          Original:         5030 B
                                     --------
          Problem:  mtime does not match
          Current:  Sat Jun 26 23:43:34 2021 (2154 days, 3:37:07 later)
          Original: Mon Aug  3 20:06:27 2015
  verify-rpm done
# 更改 /etc/vsftpd/vsftpd.conf 配置文件后，可被 yum verify-rpm 命令检测出更改的属性。  
```

#### 基础网络调试相关命令示例：

1. ping 与 ping6 命令常用选项：

   <img src="https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/ping-ping6-options.JPG" style="zoom:80%;" />

   > 💥注意💥：有时网络连通性问题由 MTU（最大传输单元）所致，需测试 MTU 大小并进行调整。
   
2. **`nmap`** 命令使用示例：

   1）nmap 是功能强大的网络扫描工具，可以扫描单个主机和大型网络。

   2）它主要用于安全审核和渗透测试。

   3）nmap 是端口扫描的首选工具，除端口扫描外，nmap 还可以检测 MAC 地址、操作系统类型、内核版本等。

   4）nmap 默认发送一个 ARP 的 ping 数据包，来探测目标主机 **`1-1000`** 范围内所开放的所有端口。

   ```bash
   $ nmap -vv [<fqdn>|<target_ip_address>]
   # 扫描指定主机是否运行及监听的端口号，及扫描的详细信息。
   $ nmap -vv 172.25.250.250
     Starting Nmap 6.40 ( http://nmap.org ) at 2021-06-28 00:19 CST
     Initiating ARP Ping Scan at 00:19
     Scanning 172.25.250.250 [1 port]
     Completed ARP Ping Scan at 00:19, 0.01s elapsed (1 total hosts)
     Initiating Parallel DNS resolution of 1 host. at 00:19
     Completed Parallel DNS resolution of 1 host. at 00:19, 0.00s elapsed
     Initiating SYN Stealth Scan at 00:19
     Scanning 172.25.250.250 [1000 ports]
     Discovered open port 22/tcp on 172.25.250.250
     Discovered open port 53/tcp on 172.25.250.250
     Completed SYN Stealth Scan at 00:19, 5.18s elapsed (1000 total ports)
     Nmap scan report for 172.25.250.250
     Host is up (0.00070s latency).
     Scanned at 2021-06-28 00:19:26 CST for 5s
     Not shown: 998 filtered ports
     PORT   STATE SERVICE
     22/tcp open  ssh
     53/tcp open  domain
     MAC Address: 52:54:00:B5:8B:12 (QEMU Virtual NIC)
   
     Read data files from: /usr/bin/../share/nmap
     Nmap done: 1 IP address (1 host up) scanned in 5.27 seconds
                Raw packets sent: 1991 (87.588KB) | Rcvd: 16 (1.012KB)
   
   $ nmap -n <target_network>/<prefix>
   # 扫描目标网段中运行的主机 IP 与监听的端口
   # 若不使用 -n 选项，将显示相应主机的 FQDN。
   $ nmap -n 172.25.250.0/24
     Starting Nmap 6.40 ( http://nmap.org ) at 2021-06-28 00:03 CST
     Nmap scan report for 172.25.250.11
     Host is up (0.0031s latency).
     Not shown: 999 filtered ports
     PORT   STATE SERVICE
     22/tcp open  ssh
     MAC Address: 52:54:00:00:FA:0B (QEMU Virtual NIC)
     ...
     
   $ nmap -n -sn <target_network>/<prefix>
   # 扫描目标网段中运行的主机 IP，不进行端口扫描。
   $ nmap -n -sn 172.25.250.0/24
     Starting Nmap 6.40 ( http://nmap.org ) at 2021-06-28 00:09 CST
     Nmap scan report for 172.25.250.11
     Host is up (0.0012s latency).
     MAC Address: 52:54:00:00:FA:0B (QEMU Virtual NIC)
     Nmap scan report for 172.25.250.250
     Host is up (0.00063s latency).
     MAC Address: 52:54:00:B5:8B:12 (QEMU Virtual NIC)
     Nmap scan report for 172.25.250.254
     Host is up (0.0013s latency).
     MAC Address: 52:54:00:00:FA:FE (QEMU Virtual NIC)
     Nmap scan report for 172.25.250.10
     Host is up.
     Nmap done: 256 IP addresses (4 hosts up) scanned in 2.00 seconds
     
   $ nmap -p <ip_range> <target_ip_address>
   # 扫描指定主机的端口范围
   # 注意：端口范围不可超过 65535
   $ nmap -p 20-120 172.25.250.11
     Starting Nmap 6.40 ( http://nmap.org ) at 2021-06-28 00:21 CST
     Nmap scan report for serverb.lab.example.com (172.25.250.11)
     Host is up (0.0016s latency).
     Not shown: 100 filtered ports
     PORT   STATE SERVICE
     22/tcp open  ssh
     MAC Address: 52:54:00:00:FA:0B (QEMU Virtual NIC)
   
     Nmap done: 1 IP address (1 host up) scanned in 2.33 seconds
     
   $ nmap -p <port1>,<port2>,...,<portN> <target_ip_address>
   # 扫描指定主机的相应端口
   $ nmap -p 20,80 172.25.250.11
   
   $ nmap -sT -p- <target_ip_address>
   # 扫描指定主机上 TCP 连接的所有端口（65535 个）
   # 注意：
   #   -sT    指定扫描 TCP 端口
   #   -sU    指定扫描 UDP 端口
   #   -p-    指定扫描所有端口（65535 个），不指定时，默认扫描 1000 个。  
   
   $ nmap -O <target_ip_address>
   # 探测指定主机的操作系统类型
   $ nmap -A <target_ip_address>
   # nmap 命令万能开关：启用操作系统探测、版本探测、脚本扫描、路由追踪
   ```

3. nc 命令使用示例：

   1）nc 命令（netcat）具有客户端模式与服务端模式

   ```bash
   $ yum install -y nmap-ncat
   # 安装 nmap 与 ncat 软件包
   $ ls -l /usr/bin/nc
     lrwxrwxrwx. 1 root root 4 Jun 27 22:09 /usr/bin/nc -> ncat
   # nc 命令为 ncat 的软链接
   
   # ----- nc client mode -----
   $ nc -v [<server_fqdn>|<server_ip_address>] <server_port>
   # 查看与指定服务端端口连接的详细状态
   # 可使用 -v、-vv、-vvv、-vvvv 查看更为详细的状态
   $ nc -v serverb.lab.example.com 22
   # 查看 serverb.lab.example.com 的 22 端口连接状态
   
   # ----- nc server listen mode -----
   $ nc -l <port>
   # 防火墙指定端口放行，监听该端口的连接状态，可配合客户端实时通信。
   $ nc servera.lab.example.com 1210
     hello servera.lab.example.com
     quit
     ^C
   $ nc -l 1210
     hello servera.lab.example.com
     quit
     ^C
   # nc 监听模式可与客户端模式配合获取客户端发送的数据
   
   $ nc -l 1210 -e /bin/bash
   # nc 监听模式：接收的信息由 -e 选项指定的命令执行
   $ nc servera.lab.example.com 1210
     ls
     anaconda-ks.cfg
     rpmdb-20210626-1702.tar.bz2
     pwd
     /root
    
   # ----- nc transfer file: client to server -----
   servera: $ nc -l 9000 > <filename>
   # nc 监听模式：接收来自客户端模式的数据将其重定向写入文件中
   serverb: $ nc -v servera.lab.example.com 9000 < <filename>
   # nc 客户端模式：将本地文件传输至服务端
   # 注意：nc 命令传输文件或目录无需输入密码，传输完成后需验证文件 md5sum。
   
   # ----- nc transfer file: server to client -----
   servera: $ nc -l 9000 < <filename>
   # nc 监听模式：监听本地 9000 端口，并发送本地文件至客户端。 
   serverb: $ nc -v servera.lab.example.com 9000 > <filename>
   # nc 客户端模式：接收来自服务端的文件
     
   # ----- nc transfer multiple files in directory -----
   servera: $ nc -l 9000 | tar -zxvf -
   # nc 监听模式：接收客户端的多个文件
   serverb: $ tar -zcvf - * | nc -v servra.lab.example.com 9000
   # nc 客户端模式：使用 tar 命令发送多个文件至服务端
   
   # ----- Test network bandwidth -----
   servera: $ nc -v serverb.lab.example.com 9000 < /dev/zero
   serverb: $ nc -l 9000 > /dev/null
   # 测试 serverb 节点的对应 NIC 接口的网速
   # 若需测试 servera 对应的 NIC 接口的网速可将测试命令互换
   ```

   2）使用 nc 命令测试网速时，可配合 **`iptraf-ng`** 工具可视化网络速率。

   3）iptraf-ng 命令来自 iptraf-ng 软件包。

   4）以上 nc 测试的 iptarf-ng 如下所示：

   ![](https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/nc-iptraf-ng.jpg)

4. tcpdump 命令示例：

   1）tcpdump 可使用 `-w` 选项将抓包结果写入以 `.pcap` 结尾的文件中。

   2）该抓包文件可通过 tcpdump 命令的 -r 选项进行读取，或使用 Wireshark 读取。

   > 💥注意💥：
   >
   > 1. 可使用 Wireshark 抓包显示数据包中的明文密码，如 vsftpd 登录、Apache HTTPD 的用户认证（基于密码文件或 SDBM 文件型数据库的 Basic 认证的方式）。
   > 2. vsftpd 软件包并不安全，建议使用 sftp 进行文件传输！
   
   ```bash
   $ man 7 pcap-filter
   # 查看 tcpdump 的包过滤语法使用说明
   $ man 8 tcpdump
   # 查看 tcpdump 命令的详细使用方法
   $ man 8 tcpslice
   # 该命令可用于提取或合并 tcpdump 的抓包文件
   $ man 4 wireshark-filter
   # 查看 Wireshark 的过滤器语法使用
   
   $ tcpdump -n -i <interface> -w <filename>.pcap 
   # 抓取来自指定网口的流量，不转换主机的 IP 地址为 FQDN，并将抓包结果写入指定文件中。
   $ tcpdump -n -i <interface> icmp and host <target_ip_address>
   # 抓取来自指定网口且来自指定主机的 ICMP 流量。
   $ tcpdump -r <filename>.pcap
   # 查看抓包文件信息
   # 注意：可使用 -v 或 -vv 选项显示更为详细的抓包信息
   ```
   
   3）关于 tcpdump 命令更为详细的使用方法可参考如下链接：
   
     a. 超详细的网络抓包神器 tcpdump 使用指南（米开朗基杨）：
   
   ​     https://mp.weixin.qq.com/s/J3Rdrof9ts9b6_paJk1KJw
   
     b. Linux 网络分析必备技能：tcpdump 实战详解：
   
   ​     https://mp.weixin.qq.com/s/vzNgYRZigR1Buay17gcfrg
   
     c. 可能是目前最简单易懂且实用的 tcpdump 和 Wireshark 抓包及分析教程：
   
   ​     https://mp.weixin.qq.com/s/9OvL5VXrGad2q-Hxf9wGRw
   
     d. 最简单的 Wireshark 和 TCP 入门指南：
   
   ​     https://mp.weixin.qq.com/s/D2jipFrVOluHGcIB9izKVQ

#### 内存泄漏与内存溢出：

1. 内存泄漏（memory leak）：

   1）指程序在申请内存后，无法释放已申请的内存空间，导致系统无法及时回收内存并且分配给其他进程使用。

   2）通常少次数的内存无法及时回收并不会对程序造成什么影响，但是如果在系统内存本身就比较少获取多次导致内存无法正常回收时，就会导致内存不够用，最终导致内存溢出。

2. 内存溢出（out of memory）：

   1）指程序申请内存时，没有足够的内存供申请者使用，导致数据无法正常存储到内存中。

   2）也就是说若需要 int 类型的存储数据大小的空间，但是却存储一个 long 类型的数据，这样就会导致内存溢出。

3. 两者的关系：

   1）内存泄露最终会导致内存溢出，由于系统中的内存是有限的，如果过度占用资源而不及时释放，最后会导致内存不足，从而无法给所需要存储的数据提供足够的内存，从而导致内存溢出。

   2）导致内存溢出也可能是由于在给数据分配大小时没有根据实际要求分配，最后导致分配的内存无法满足数据的需求，从而导致内存溢出。

4. 两者的区别：

   1）内存泄露是由于 `GC` 无法及时或者无法识别可以回收的数据进行及时的回收，导致内存的浪费。
   
   2）内存溢出是由于数据所需要的内存无法得到满足，导致数据无法正常存储到内存中。
   
   3）内存泄露的多次表现就会导致内存溢出。

5. 内存泄漏的分类（根据发生方式分类）：

   1）常发性内存泄漏：

      发生内存泄漏的代码会被多次执行到，每次被执行的时候都会导致一块内存泄漏。
      
   2）偶发性内存泄漏：

      发生内存泄漏的代码只有在某些特定环境或操作过程下才会发生。

      常发性和偶发性是相对的，对于特定的环境，偶发性的也许就变成了常发性的。

      所以测试环境和测试方法对检测内存泄漏至关重要。
      
   3）一次性内存泄漏：

      发生内存泄漏的代码只会被执行一次，或者由于算法上的缺陷，导致总会有一块仅且一块内存发生泄漏。

      比如，在类的构造函数中分配内存，在析构函数中却没有释放该内存，所以内存泄漏只会发生一次。
      
   4）隐式内存泄漏：

      程序在运行过程中不停的分配内存，但是直到结束的时候才释放内存。

      严格的说这里并没有发生内存泄漏，因为最终程序释放了所有申请的内存。

      但是对于一个服务器程序，需要运行几天，几周甚至几个月，不及时释放内存也可能导致最终耗尽系统的所有内存。

      所以，我们称这类内存泄漏为隐式内存泄漏。
   
   > 💥注意💥：
   >
   > 1. 内存泄漏与内存溢出应注重从应用代码角度去解决问题。
   > 2. 泄漏虚拟内存虽然不好，但是泄漏物理内存更加不好。

#### 共享库相关命令示例：

共享库（shared library）的查询过程：

> 🚩注意🚩：共享库也称 C 函数库、共享对象（shared object）、动态链接库

1. 应用在编译时，需链接到提供相关功能的共享库上。

2. 编译器将检查所需的共享库是否存在。

3. 编译好的应用可执行文件中包含所需共享库的信息，包括其绝对路径、共享库名称与版本（`DT_SONAME`）。

   ```bash
   $ objdump -p /usr/lib64/libyaml-0.so.2.0.5 | grep SONAME
     SONAME               libyaml-0.so.2
   # 获取指定共享库的 SONAME，SONAME 为共享库的软链接。
   ```

4. RHEL 7.x 中使用 `/lib64/ld-linux-x86-64.so.2` 作为 `run-time linker`。

5. run-time linker 在应用可执行文件运行时通过两种方式查找对应的共享库：

   > 🚩注意🚩：以下两种方式任选其一

​     1）run-time linker 可读取 DT_SONAME，再根据环境变量 `LD_LIBRARY_PATH` 定义的存储目录查找共享库。

​     2）run-time linker 也可读取 DT_SONAME 及共享库缓存文件 `/etc/ld.so.cache`，再根据该文件查找共享库。

6. run-time linker 最终将共享库映射至应用运行时的内存中。

   ```bash
   $ ldconfig -p
   # 查看共享库缓存文件 /etc/ld.so.cache，该文件中包含之前读取的共享库清单列表。
   $ strings /etc/ld.so.cache | grep '^/'
   # 查看共享库缓存文件中的共享库列表，文本方式输出。
   $ ldd <application_name>
   # 查看可执行文件所需的共享库
   # ldd 命令实质为 Shell 脚本，而非应用程序。
   ```

   > 💥注意💥：若相应应用程序缺少指定的共享库，安装共享库后，需使用 ldconfig 命令更新共享库缓存文件。

#### 系统调用与库调用：

1. 系统调用（system call）：

   1）系统调用可理解是操作系统为用户提供的一系列操作的接口（API），这些接口提供了对系统硬件设备功能的操作。

   2）如，`hello world` 程序会在屏幕上打印出信息，程序中调用了 `printf()` 函数，而库函数 `printf` 本质上是调用了系统调用`write()` 函数，实现了终端信息的打印功能。

2. 库调用（library call）：

   1）库函数可以理解为是对系统调用的一层封装。

   2）系统调用作为内核提供给用户程序的接口，它的执行效率是比较高效而精简的，但有时我们需要对获取的信息进行更复杂的处理，或更人性化的需要，我们把这些处理过程封装成一个函数再提供给程序员，便于程序编码。

   3）库函数有可能包含有一个或多个系统调用，也可能没有系统调用，如有些操作不需要涉及内核的功能。
   4）无论是应用程序或库函数都有可能不调用系统调用而直接运行。
   
   <img src="https://github.com/Alberthua-Perl/summary-scripts/blob/master/docs/images/system-library-call.jpg" style="zoom:80%;" />
   
3. 系统调用的意义：

   1）避免了用户直接对底层硬件进行编程。

      如 `hello world` 程序将信息打印到终端，终端对系统来说是硬件资源，若没有系统调用，用户程序需要自己编写终端  

      设备的驱动，以及控制终端如何显示的代码。

   2）隐藏背后的技术细节。

      如读写文件，若使用了系统调用，用户程序无须关心数据在磁盘的哪个磁道和扇区，以及数据要加载到内存什么位置。

   3）保证系统的安全性和稳定性。

      用户程序不能直接操作内核地址空间，而系统调用的功能由内核实现，用户只需调用接口，无需关心细节。

   4）方便程序的移植性。

      若针对一个系统资源的功能操作如 write()，大家都按照自己思路去实现该功能，那么程序的移植性非常差。

4. 两者的区别：

   1）所有 C 函数库是相同的，而各个操作系统的系统调用是不同的（CPU 架构的差异）。

   2）函数库调用是调用函数库中的一个程序，而系统调用是调用系统内核的服务。

   3）函数库调用是与用户程序相联系，而系统调用是操作系统的一个进入点。

   4）函数库调用是在用户地址空间执行，而系统调用是在内核地址空间执行。

   5）函数库调用的运行时间属于用户时间（`user time`），而系统调用的运行时间属于系统时间（`system time`）。

   6）函数库调用属于过程调用，开销较小，而系统调用需要切换到内核上下文环境然后切换回来，开销较大。

   7）在 C 函数库 **`libc`** 中大约 300 个程序，在 UNIX 中大约有 90 个系统调用。

   8）函数库典型的 C 函数：system()、fprintf()、malloc()

   9）典型的系统调用：chdir()、fork()、write()、brk()

   > 🚩注意🚩：
   >
   > 1. 参考《C 专家编程》中的附录 A.4，书中关于两者区别为函数库调用是语言或应用程序的一部分，而系统调用是操作系统的一部分。
   > 2. Linux 共享库（`C 函数库`）来自于 glibc 软件包。

5. 库函数调用大概花费时间为半微妙，而系统调用所需要的时间大约是库函数调用的 70 倍（35微秒），由于系统调用会有内核上下文切换的开销。

6. 纯粹从性能上考虑，应该尽可能地减少系统调用的数量，但是必须记住，许多 C 函数库中的程序通过系统调用来实现功能。

7. 以上说明的库函数调用性能远高于系统调用的前提是库函数中没有使用系统调用，再来解释下某些包含系统调用的库函数，然而其性能确实也要高于系统调用。

> ✅总结✅：
>
> **Command-line utility -> Invokes functions from system libraries (glibc) -> Invokes system calls -> Invokes kernel** 

#### strace 与 ltrace 命令使用示例：

1. 操作系统的两种模式：

   1）内核模式：操作系统内核使用的一种强大的特权模式，运行于内核空间（`kernel space`）。

   2）用户模式：用户使用命令行程序和图形用户界面（GUI）的模式，运行于用户空间（`user space`）。

2. 系统调用与库函数调用非常相似，即它们都接受并处理参数然后返回值，唯一的区别是系统调用进入内核，而库函数调用不进入。

3. 从用户空间切换到内核空间使用 `trap` 机制实现。

4. 通过使用 C 函数库（Linux 系统上又称为 `glibc`），大部分系统调用对用户隐藏。

5. 尽管系统调用本质上是通用的，但是发出系统调用的机制在很大程度上取决于机器（CPU 架构的差异）。

6. `strace` 命令：

   trace system calls and signals，跟踪用户进程与 Linux 内核之间的交互（系统调用），及所接收的信号情况。

7. `ltrace` 命令：

   a library call tracer，跟踪进程调用库函数的情况。

8. 若 strace 命令没有任何输出，并不代表此时进程发生阻塞，也可能进程正在执行某些不需要与系统其它部分发生通信的事情。

9. strace 从内核接收信息，且无需以任何特殊方式来构建内核。

```bash
$ man 2 <syscall_name>
# 查看指定系统调用的详细信息

strace 命令常用选项：
  -v                  输出所有的系统调用，一些系统调用关于环境变量、状态、输入输出等由于调用频繁，默认不输出。
  -d                  默认 strace 将结果输出到 STDOUT，该选项输出 STDERR 的调试信息。
  -f                  跟踪由 fork 或 clone 调用所产生的子进程
  -ff                 常与 -o <filename> 一起使用，则所有进程的跟踪结果输出到相应的 <filename>.pid 中，pid 是各进程的进程号。
  -F                  跟踪 vfork 调用，使用 -f 选项时，vfork 调用不被跟踪。
  -c                  统计每一系统调用的所执行时间、调用次数和出错的次数等。
  -p PID              跟踪正在运行的进程的系统调用
  -t                  在跟踪的输出中的每一行前加上时间戳
  -tt                 在跟踪的输出中的每一行前加上时间戳与微秒级
  -ttt					  	  以微秒级输出跟踪的调用执行的时间戳
  -T                  末尾输出每个系统调用的执行时间
  -r                  前置输出每个系统调用的执行时间
  -a NUMBER           设置系统调用返回值的输出位置，默认为 40。
  -o FILENAME         将 strace 跟踪结果输出写入指定文件
  -s STRINGSIZE       指定输出的字符串的最大长度，默认为 32，文件名一直全部输出。
  
  -e EXPR             指定表达式用以控制如何跟踪
                      [qualifier=][!]value1[,value2]...
                      qualifier 只能是 trace, abbrev, verbose, raw, signal, read, write 其中之一。
                      value 是用来限定的符号或数字
                      默认的 qualifier 是 trace，感叹号是否定符号。
  -e open             等价于 -e trace=open，表示只跟踪 open 调用。
  -e trace!=open      表示跟踪除了 open 以外的其他调用，有两个特殊的符号 all 和 none。
  -e trace=open,close,rean,write
                      表示只跟踪这四个系统调用，默认的为 set=all。
  -e trace=file       只跟踪有关文件操作的系统调用
  -e trace=process    只跟踪有关进程控制的系统调用
  -e trace=network    跟踪与网络有关的所有系统调用
  -e strace=signal    跟踪所有与系统信号有关的系统调用
  -e trace=ipc        跟踪所有与进程通讯有关的系统调用
  -e signal=SET       指定跟踪的系统信号，默认为 all，如 signal=!SIGIO（或 signal=!io）表示不跟踪 SIGIO 信号。
  -e read=SET         输出从指定文件中读出的数据，如 -e read=3,5。
  -e write=SET        输出写入到指定文件中的数据

# 注意：
#   1. strace 命令跟踪进程的系统调用，输出中的每行代表一个系统调用的执行情况。
#   2. 每行中包含系统调用的名称、系统调用的参数及其执行的返回值。

  
$ strace -c ip link show dev ens33
  2: ens33: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP mode DEFAULT group 
     default qlen 1000
      link/ether 00:0c:29:96:61:1c brd ff:ff:ff:ff:ff:ff
      altname enp2s1
  % time     seconds  usecs/call     calls    errors syscall
  ------ ----------- ----------- --------- --------- ----------------
   26.12    0.001282          40        32           mmap
   17.77    0.000872          62        14           mprotect
    6.13    0.000301          33         9           read
    ...
    0.67    0.000033          16         2         1 arch_prctl
    0.00    0.000000           0         1         1 access
    0.00    0.000000           0         1           execve
  ------ ----------- ----------- --------- --------- ----------------
  100.00    0.004908                   129         4 total
# 注意：输出中各项含义
#   % time：进程执行耗时占总时间百分比
#   seconds：进程执行总时间
#   usecs/call：单个命令执行时间
#   calls：调用次数
#   errors: 出错次数
#   syscall: 系统调用名称

$ strace -ff -o <filename> <pid1> <pid2>
# 跟踪多个多进程的进程，并将其跟踪结果写入对应的 .pid 文件中。

$ strace -T -v -d -Fff -e trace=[process|signal|ipc] -s 128 -o app-strace.log <pid>
# 应用性能或故障分析：跟踪指定进程的详细系统调用过程与执行时间
```

```bash
$ ltrace <process_name>
# 跟踪进程的 C 库函数调用
$ ltrace -p <pid>
# 跟踪已运行进程的 C 库函数调用
$ ltrace -t -f -p <pid>
# 跟踪已运行进程所创建的子进程/线程的 C 库函数调用
```

参考链接：

- Understanding system calls on Linux with strace：

  https://opensource.com/article/19/10/strace
