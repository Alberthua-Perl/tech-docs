# Linux 基础命令快速入门

## 环境说明

- 该文档中涉及的 Linux 命令与相关配置已在 `RedHat Enterprise Linux 6~9` 中验证。
- 其他 Linux 发行版中可能存在差异，需自行验证。

## 文档目录

- Shell 相关概念
- 远程登录方法
- Linux 文本编辑器
- Shell 变量设置
- 系统时间设置
- 文件与目录管理
- 文件与目录的权限管理
- 用户信息
- 用户与用户组管理
- 用户密码生命周期管理
- 进程管理
- Systemd 服务管理
- RPM 软件包管理
- YUM 与 DNF 软件包管理
- RHEL 7/8/9 系统引导过程
- Linux 磁盘管理
- LVM 逻辑卷管理
- Linux 中导致系统无法正常启动的原因
- 基础网络管理
- 🧪 Lab 示例

## Shell 相关概念

- RHEL, Fedora, CentOS, OpenSUSE 与 SUSE 中默认使用 bash
- Debian, Ubuntu 中默认使用 shell
- Alpine 中默认使用 ash
- IBM AIX 中默认使用 ksh
- 还用其他类型的 shell：zsh、csh
- Shell 命令的格式：`command + [subcommand] + [options] + arguements`
- 正则表达式：
  - 小写字母：`[a-z]`
  - 大写字母：`[A-Z]`
  - 数字：`[0-9]`、`\d`
  - 字母与数字：`[a-zA-Z0-9]`
  - 空白：`\s`、`\t`、`\n`、`\r`、`\f`、`\v`
- 文件写入磁盘的整体流程（此处不考虑中间缓存层）：
  - 用户空间层（user space）：
    - 编辑指定文件
    - 写文件请求
    - 调用系统调用（syscall）
  - 内核空间层（kernel space）：
    - 根据文件所在的文件系统类型调用虚拟文件系统（Virtual File System, VFS）中的驱动，如 vfat, ext, xfs, btrfs 与 ntfs 等。
    - 磁盘驱动
    - 磁盘设备
- 文件系统中的文件只有真正占据存储空间才会在 ls 命令中显示其逻辑大小。
- /proc 目录中的子文件与子目录是系统运行与内核在内存中的映射，它不占据真正文件系统的存储空间，所以逻辑大小为 0。

## 远程登录方法

- KVM 带外管理登录：`iLO`、`iBMC`
- Telnet 登录：明文密码登录不安全（不推荐）
- SSH 登录：
  - 方法1：明文密码登录（password）
  - 方法2：基于密钥的免密登录（publickey）
    - 相关文件与目录的权限：
      - $HOME/.ssh/: `0700`
      - $HOME/.ssh/*key: `0600`
      - $HOME/.ssh/authorized_keys: `0600`
      - $HOME/.ssh/config: `0600`

## Linux 文本编辑器

- Nano
- Vim:
  - 插入模式（Insert mode）：
    - a,i: 进入插入模式
    - o: 在当前光标下方插入新行
    - Esc: 退出插入模式至命令行模式
  - 命令行模式（Command mode）：
    - *N*yy: 拷贝光标所在行之后的指定行数
    - p: 从光标所在行下方粘贴内容
    - gg: 光标定位至首行
    - Shift+g: 光标定位至末行
    - Shift+a: 光标定位至行尾并进入插入模式
    - u: 撤销上一次修改
    - x: 删除单个字符
    - dd: 删除光标所在行
  - 可视模式（Visual mode）：
    - Shift+v: 选择一行或多行
    - Ctrl+v: 选择一个或多个块
    - Esc: 退出可视模式至命令行模式
  - 扩展命令行模式（Extended command mode）：
    - `:wq`: 保存并退出
    - `:x`: 保存并退出
    - `:q!`: 强制退出不保存任何数据
    - `:set nu`: 设置行号
    - `:set nonu`: 取消设置行号
- Emacs
- Geditor: GNOME3 中的图形化文本编辑器

## Shell 变量设置

```bash
$ set
  # 查看当前 shell 环境中的所有变量
$ unset <shell_var>
  # 重置指定的 shell 变量
$ env
$ export [-n] <env_var>
  # 移除指定的环境变量
```

## 系统时间设置

```bash
$ date
$ date -u
$ date -s "2022-03-26 18:10:00"
$ date +'%Y-%m-%d %H:%M:%S'
$ date -d "1970-01-01 UTC 18389 day"
  # 计算从 1970-01-01 起的第 18389 天的日期
$ hwclock -w
  # 从系统时间同步设置 RTC 时间（BIOS 时间）
$ hwclock -s
  # 从 RTC 时间同步设置系统时间  

$ timetatectl [status]
$ timedatactl set-timezone Asia/Shanghai
$ tzselect
  # 根据 IANA 数据库查询时区
$ timedatectl set-time 2025-03-11 15:13:00
  # 此方法必须提前关闭 NTP 时间同步服务（chronyd）
$ timedatectl set-ntp [true|false]
  # 启用或禁用 NTP 时间同步服务（chronyd)
```

## 文件与目录管理

```bash
$ file /path/to/file
$ cat [-n] /path/to/file
  # -n 选项：显示行号
$ less /path/to/file
$ more /path/to/file
$ tail -n20 /etc/passwd | tee t20-passwd | less
$ tail -n15 /etc/passwd | tee -a t20-passwd | less
  # 最后的 less 命令显示的是前面的命令输出而不是重定向文件的内容。
$ tail -n <number> /path/to/file
$ tail -f /path/to/file
  # 追踪实时日志刷新
$ head -n <number> /path/to/file
  # 默认查看文本文件的前 10 行
$ head -n6 /etc/passwd > h6-passwd
$ head -n6 /etc/passwd >> h6-passwd
$ head -n6 /etc/paswd 2>> h6-passwd
$ head -n6 /etc/paswd >> h6-passwd 2> /dev/null
$ head -n6 /etc/paswd &>> h6-passwd
$ head -n6 /etc/paswd >> h6-passwd 2>&1  
$ wc [-l|-w|-c] /path/to/file 
$ history
  # 每个 bash shell 会话窗口只记录自身执行的命令历史直到该窗口退出。
  # 当前的命令历史被记录在内存中,  退出窗口之后将写入 $HOME/.bash_history 文件中。
$ history -c
  # 清除当前 shell 会话窗口的历史记录 
$ pwd
$ cd /path/to/dir
$ cd -
$ ls [-lhdRtaZi] /path/to/file_or_dir
$ touch /path/to/file
  # 创建空文件，也可用于修改文件的时间戳。
$ mkdir -p /path/to/dir1/dir2
$ cp /path/to/file /path/to/dir/
$ cp --preserve=mode,ownership,timestamps /path/to/file /path/to/dir/
$ cp -Rv --preserve=mode,ownership,timestamps /path/to/dir1 /path/to/dir2/
$ mv /path/to/file /path/to/dir/
$ rm -rf /path/to/dir
$ rmdir /path/to/emptydir
$ ln /path/to/file /path/to/file_hardlink
$ ln -s /path/to/file /path/to/file_softlink
  # 创建 raw 设备：SAP, Oracle, Ceph 等用于创建 raw 设备提高 IO 效率而不通过文件系统层的写入
  # 对不同软件版本的调用解耦：/path/to/nginx -> /path/to/nginx-1.12.x, /path/to/nginx-1.15.x
```

## 文件与目录的权限管理

- chmod 命令：
  - 符号法（symbolic）：
    - 用户：u, g, o, a
    - 权限：r(4), w(2), x(1), X
  - 八进制数值法

```bash
$ chown [-R] <username>:<groupname> /path/to/file_or_dir
  # 建议使用 ":" 作为分隔符，防止用户名与所有组名称中出现 "." 而造成命令解析的歧义！
$ chgrp <groupname> /path/to/file_or_dir
$ chmod u+s /sbin/parted  ##suid: 4---
$ chmod u-s /sbin/parted
$ chmod g+s /path/to/dir  ##sgid: 2---
$ chmod g-s /path/to/dir
$ chmod o+t /path/to/dir  ##sbit: 1---
$ chmod o-t /path/to/dir
```

## 用户信息

```bash
$ who
$ whoami
$ w
$ id [username]
```

## 用户与用户组管理

- 用户：
  - RHEL 5/6:
    - UID 0: root（超级用户）
    - UID 1~499: 系统或应用用户
    - UID 500+ included: 常规用户
  - RHEL 7/8/9:
    - UID 0: root（超级用户）
    - UID 1~999: 系统或应用用户
    - UID 1000+ included: 常规用户
    - max user UID: 60000
- 用户组：
  - 主要组（primary group）：
    - 创建方法1：随用户创建同时创建，组名称与用户名相同。
    - 创建方法2：手动创建
  - 次要组（secondary group）：
    - 手动添加
- su 与 sudo 命令：
  - /etc/sudoers.d/appuser1:
    - `appuser1  ALL=(ALL)  ALL`
    - `appuser1  ALL=(ALL)  NOPASSWD: /usr/sbin/lvs,/usr/sbin/pvs`
    - `appuser1  ALL=(ALL)  NOPASSWD: ALL`（sudo -i 或 sudo su - 可直接提权为 root）
  - su 与 su -：前者为非登录 shell，后者为登录 shell。
  - sudo -i 与 sudo su -：前者为非登录 shell，后者为登录 shell。
- 用户密码加密方式：
  - RHEL 5: MD5
  - RHEL 6: SHA256
  - RHEL 7/8/9: SHA512
- 用户账户文件：
  - /etc/passwd: 本地用户信息
  - /etc/shadow: 本地用户密码存储文件（影子文件）
    - 第二个字段含义：
      - 第一个 `$`：SHA512 加密方式
      - 第二个 `$`：SALT 值
      - 第三个 `$`：SALT 值与明文密码通过 SHA512 加密算法获得的哈希值
  - /etc/group: 本地用户组信息
  - /etc/profile.d/*: 优先级高于 /etc/profile
  - /etc/profile: 系统全局的环境设置文件（对所有用户均有效）
  - /etc/bashrc:
    - 系统全局的环境设置文件（对所有用户均有效）
    - 可设置 umask
  - $HOME/.bash_profile: 当前用户的环境设置文件
  - $HOME/.bashrc: 当前用户的环境设置文件
  - /etc/login.defs:
    - 定义用户行为
    - 定义用户家目录
    - 定义密码生命周期
    - 定义密码加密算法
    - 注意：Ubuntu 与 SUSE 15 默认情况下不为用户自动创建家目录！
  - /etc/security/pwquality.conf: 定义默认密码长度至少是 8 位

```bash
$ useradd chksys

$ groupadd devgrp0
$ useradd -u 2000 -g devgrp0 -m -d /opt/chkdev -s /bin/bash chkdev
  # devgrp0 用户组作为主要组，指定 UID 2000、家目录与登录 shell 类型，创建 chkdev 用户。

$ useradd -u 900 -r -m -d /opt/nginx -s /sbin/nologin nginx
  # 使用非登录 shell 创建系统用户

$ usermod -u 1100 chksys
$ usermod -m -d /opt/chksys chksys
  # chksys 用户的 /home/chksys 家目录将迁移至 /opt 中，原来的家目录将不复存在。
  # 请务必确认该用户可迁移家目录,  需提前与其他团队沟通告知！

$ usermod -L chksys
  # 用户的锁定也是 SSH 无法远程登录的原因之一
$ usermod -U chksys
$ usermod -G devgrp0 operator0
  # 将 devgrp0 用户组设置为 operator0 的次要组
$ usermod -aG wheel operator0
  # 将 wheel 用户组追加为 operator0 的次要组

$ gpasswd -d operator0 wheel
  # 将 operator0 用户从 wheel 用户组中删除
$ groupadd -g 900 -r nginx
$ groupmod -n webapp nginx
$ groupmod -g 905 webapp
$ groupdel webapp
$ userdel [-r] <username>
  # 加 -r：同时删除家目录与邮件信息不做保留，有可能其中包含重要数据，造成数据丢失。
  # 不加 -r：不会删除家目录数据并且持久保留，但是源目录的 uid 与 gid 会被之后新创建的同 uid 与 gid 的用户所占用。
```

## 用户密码生命周期管理

```bash
$ chage -m 2 -M 90 -W 5 -I 2 chksys
  # 设置 chksys 用户的密码生命周期：
  #   -m 2：2天内用户不可自行需改密码
  #   -M 90：自密码修改之日起90天后密码失效
  #   -W 5：在密码到期前5天发出警告
  #   -I 2：密码过期后可继续使用2天
$ chage -l chksys
  # 查看 chksys 用户的密码状态
$ chage -E 2022-08-01 chksys
  # 设置 chksys 用户的账户过期日期
$ chage -d 0 chkdev
  # 强制用户下次登录时更改密码
```

## 进程管理

- 进程间通信方式：
  - 共享内存（shared memory）
  - 管道（pipeline）
  - 信号量（signal）
  - 套接字（socket）：RAW 套接字、Unix 域套接字（local）、UDP 套接字、TCP 套接字
  - 跨主机间：RPC、gRPC/proto-buff、REST、GraphQL 等
- Linux 中任务（task）与进程等同
- 平均负载（load average）：Running（运行的进程）+ Runnable（待运行的进程）+ Deep Sleeping（深度睡眠不可中断进程）
- CPU 物理使用率（%）：该值可能是小于等于 100%（单核CPU），也可能大于 100%（多核CPU）。这个指标只受到 Running 状态的任务影响。
- 一般情况下，(load average)/(total cpu cores) $\leq$ 70% 表示系统资源使用缓和。
- Linux 中的两种系统调优配置方法：
  - 静态调优：主要的操作对象为 kernel 参数（/proc 目录中）
  - 动态调优：主要的操作对象为 tuned-profile（包含两种类型的参数, 即 kernel 参数与 profile 自定义参数）

```bash
$ ps aux
$ ps -efL
  # 查看系统进程及其子线程的信息（NLWP）
$ ps -L <pid>
  # 查看进程子线程的信息
$ pgrep -l -u <username>
  # 查看指定用户的进程列表
$ pkill -<SIGNALNAME> -u <username>
  # 向指定用户运行的所有进程发送信号
$ pkill -P <ppid>
  # 终止由父进程生成的所有子进程
$ killall -<SIGNALNAME> <process_name>
  # 向指定的所有进程发送信号
$ pstree -p <username>
  # 查看指定用户的进程树

$ <command> &
$ jobs
$ fg %<job_number>
$ bg %<job_number>
## 注意：jobs 返回列表中的进程在当运行的 Shell 会话关闭后终止作业的运行！
```

## Systemd 服务管理

- RHEL 5/6: SysV, init(PID 1)
- RHEL 7/8/9: systemd(PID 1)
- 使用 systemd 管理的发行版：RHEL, Fedora, CentOS, CentOS-Stream, Rocky, OpenSUSE, SUSE, Debian, Ubuntu

```bash
$ systemctl -t help
  # 查看系统上支持的单元类型
$ systemctl list-units --type=service
  # 查看已加载到内存中的 service 类型单元文件
$ systemctl list-unit-files --type=service
  # 查看系统上存在的 service 类型单元文件（是否加载到内存中均可）

$ systemctl enable <name>.service
$ systemctl enable --now <name>.service
  # 立即启动服务，并设置服务开机自启动。
$ systemctl start <name>.service
$ systemctl stop <name>.service
$ systemctl disable <name>.service
$ systemctl status <name>.service
$ systemctl is-active <name>.service
$ systemctl is-enabled <name>.service
$ systemctl is-failed <name>.service
$ systemctl --failed --type=<type>
$ systemctl reload <name>.service
$ systemctl mask <name>.service
$ systemctl umask <name>.service
$ systemctl list-dependencies
  # 系统全局正向依赖单元文件
$ systemctl list-dependencies --reverse
  # 系统全局反向依赖单元文件（冲突的服务）
$ systemctl list-dependencies <name>.service
  # 指定服务的正向依赖单元文件
$ systemctl list-dependencies <name>.service --reverse
  # 指定服务的反向依赖单元文件（冲突的服务）
```

- GUI 模式与 CLI 模式：

```bash
$ systemctl get-default
  # 查看系统当前开机启动模式
$ systemctl set-default [multi-user.target|graphical.target]
  # 设置不同的开机启动登录模式
$ systemctl isolate multi-user.target
$ systemctl isolate graphical.target
  # 切换不同的系统登录方式
```

## RPM 软件包管理

- 使用 `RPM` 包管理器的发行版：Fedora, RHEL, CentOS, Rocky, OpenSUSE, SUSE
- 适应 `DEB` 软件包的发行版：Debian, Ubuntu

```bash
$ rpm -qf /path/to/file
$ rpm -ql /path/to/rpmpackage
$ rpm -qa | grep rpmpackage_name
$ rpm -qi /path/to/rpmpackage
$ rpm -qc /path/to/rpmpackage
$ rpm -qd /path/to/rpmpackage
$ rpm -q --changelog /path/to/rpmpackage
$ rpm -q --scripts /path/to/rpmpackage
```

## YUM 与 DNF 软件包管理

- RHEL 8/9 中 yum 与 dnf 命令都是 `dnf-3` 命令的软链接！
- 软件包组：
  - 常规组：
    - 软件包的集合
    - 包含三种安装模式的软件包：`mandatory`, `default`, `optional`
  - 环境组：
    - 常规组的集合  

```bash
$ yum search "pattern"
$ yum install [-y] packagename
$ yum remove [-y] packagename
$ yum update [-y] packagename
$ yum upgrade [-y]
$ yum downgrade [-y] packagename
$ yum repolist
$ yum makecache
$ yum clean all
$ yum list packagename
$ yum history undo
$ yum info packagename
$ yum provides /path/to/file
  # 查看绝对路径的文件来自于哪个 RPM 软件包
$ yum provides */filebasename
  # 查看基本文件名的文件来自于哪个 RPM 软件包
$ yum group info groupname
$ yum group list hidden
  # 查看所有包组列表（显示隐藏包组）
$ yum group install [-y] groupname  
```

## RHEL 7/8/9 系统引导过程

- power on system
- system POST
- UEFI or BIOS search boot device in boot list including NIC, disk, USB, etc
- 💪 If boot device is disk, UEFI or BIOS transform system control to GRUB2 (boot loader).
- GRUB2 read grub2 kernel options (verify where is kernel and initramfs)
- 💪 load kernel and initramfs into memory as memory rootfs
- kernel and initramfs load all available devices through kernel module, initramfs include different device drivers.
- /sbin/init (systemd) in memory load initrd.target including /sysroot (real system rootfs)
- /etc/fstab in /sysroot used to mount devices (*.mount, *.device)
- 💪 kernel transform system control to systemd (initramfs --> real-disk-fs)
- systemd load and start all available *.target
- load and start all available unit file
- login system through multi-user.target or graphical.target

## Linux 磁盘管理

- 存储设备的识别符：
  - SCSI 驱动：`/dev/sdX (/dev/sd[a-z])`
  - IDE 驱动：`/dev/hdX (/dev/hd[a-z])`
  - NVMe 驱动：`/dev/nvmeX (/dev/nvmeXnM)`
  - virtio_blk 驱动：`/dev/vdX (/dev/vd[a-z])`
- 磁盘分区方案：
  - MBR：
    - 主引导记录：master boot record (512 bytes)
    - boot loader (`GRUB2` or `iLILO`) + partition tables (primary table1~4, extended, logical)
    - MBR Gap ~ 1MiB
    - 分区方法1：`/dev/vda: 20G -> p1:5G, p2:3G, p3:6G, p4:6G (4 个主分区)`
    - 分区方法2：`primary1(5G) + primary2(3G) + primary3(6G) + extented4(1K) + logical5 + logical6 + ... + logical 15`
  - GPT：
    - GPT + partition tables (128个分区)
    - 磁盘头尾的 GPT 备份
- 系统磁盘（RAID0、RAID1）：
  - legency BIOS + MBR + partition + system data
  - UEFI BIOS + GPT + partition + system data
  - /boot 分区：
    - 只能是裸分区，不能做成逻辑卷！
    - 该分区支持的可被引导的文件系统：btrfs, ext3, vfat, xfs
- 数据磁盘：
  - partition + application data
  - 此类磁盘不受分区方案影响，因为不会被引导。
- 磁盘分区工具：
  - fdisk：
    - IBM
    - msdos(mbr)
  - gdisk:
    - msdos and gpt
  - parted:
    - GNU
    - disk partition tool
    - msdos and gpt
    - CLI 或交互式
- 文件系统的类型：
  - RHEL 6/7: ext4(default), ext3, vfat, btrfs, gfs, ntfs
  - RHEL 8/9: xfs(default), ext3, ext4, vfat, ntfs
- 持久化挂载文件系统的步骤：
  - 磁盘分区
  - mkfs -t [ext4|xfs|vfat] /dev/vd*MN*
  - mkdir /path/to/mountpoint
  - vim /et/fstab
    /dev/vd*MN*  /path/to/mountpoint   ext4  defaults   0 0
  - mount -a
  - df -Th /path/to/mountpoint
- SWAP 交换空间：
  - 本质是磁盘分区
  - Linux 虚拟内存：物理内存 + SWAP
    - top 命令中的 `VIRT`、`RES`
    - ps 命令中的 `VSZ`、`RSS`
  - 创建 SWAP 分区的方式：
    - mkswap /dev/sdXn
    - vim /etc/fstab
      /dev/sdXn  swap  swap  defaults  0 0
    - swapon -a
    - swapon -s
- 更改 SWAP 优先级：
  - sync: 换出 SWAP 中非活动页面至物理内存中
  - swapoff -a
  - vim /etc/fstab
    /dev/sdXn  swap  swap  defaults,pri=N  0 0    ##添加 pri=N 参数以更改 swap 优先级
  - swapon -a
  - swapon -s

  ```bash
  # 如下过程所示：
  [root@serverb ~]# cat /etc/fstab
  /dev/vdb2  swap      swap defaults  0 0
  /dev/vdb3  swap      swap defaults,pri=5  0 0
  [root@serverb ~]# swapon -s
  Filename                                Type            Size    Used    Priority
  /dev/vdb2                               partition       511996  0       -2
  /dev/vdb3                               partition       1048572 0       5
  ```

## LVM 逻辑卷管理

- 逻辑卷：logical volume manager, LVM
- 实现的方式：
  - 一个或多个磁盘分区或整盘
  - 物理卷：physical volume, PV
  - 卷组：
    - volume group, VG
    - 物理扩展单元：physical extent, PE（默认为 4M，也可指定为 8M, 16M, 32M）
  - 逻辑卷：logical volume, LV
  - Linux kernel 中的 `devicemapper` 驱动框架：
    - 自 kernel 2.6.x 起加入内核
    - 以内核模块的形式运行
    - 使用场景：stratis, vdo, crypt, docker-image-graph-driver, mutipath

## Linux 中导致系统无法正常启动的原因

- 系统安装期间出现：
  - 安装程序找不到磁盘驱动器：系统与服务器硬件不兼容。已有的磁盘驱动无法识别新硬件（将现有系统的 initramfs 中添加对应的驱动程序，如 RAID 阵列卡驱动，重新封装 iso 镜像，再次安装系统）。但请注意，修改过的系统原厂极大可能不做售后与支持！
- 系统常规使用期间出现：
  - 进入 emergency 模式：/etc/fstab 未能正常读取，该文件中出现文件系统类型指定错误。
  - 开机直接出现 kernel panic (Call trace)
  - 开机出现文件系统报错 ext3/4 error, xfs error，需要进入 rescue 模式中，修复指定的分区。
  - 开机的 GRUB2 引导界面丢失无法选择对应的内核版本
  - GRUB2 本身的错误导致的引导失败
  - SELinux 错误配置，将 `SELINUX=enforc` 修改错误，导致系统检测 SELinux 上下文无法通过。

## 基础网络管理

- 确认网络状态：

  ```bash
  $ ifconfig [-a]
  $ netstat -an
  $ netstat -antulp
  $ route -n
    # 以上的命令来自于 net-tools 软件包已被弃用

  $ ip [-s] link show
  $ ip address show
    # 与 ip a s 命令相同
  $ ip -br a s
    # 以简要的形式查看 ip 信息
  $ ip address del 172.25.250.9/24 dev eth0
  $ ip address add 172.25.250.110/24 dev eth0
  $ ip address flush dev eth0
    # 清除指定网络接口的所有 IP 地址
  $ ip route show
  $ ss -an
    # 查看系统中所有的 socket 连接，并转换为相应端口（LISTEN 与 ESTAB 状态）。
  $ ss -tulnp
    # 查看监听 UDP 与 TCP 端口的进程，并转换为相应端口。
  ```

- 网络服务的变更：
  - RHEL 5/6:
    - service network [start|stop|restart]
  - RHEL 7：
    - network.service:
      - initscripts manage
    - NetworkManager.service:
      - /etc/sysconfig/network-scripts/ifcfg-*: `NM_CONTROLLED=no`
      - 该参数可使 `network.service` 与 `NetworkManager.service` 服务共存，并且后者不会影响前者配置 IP 的状态。
  - RHEL 8/9：
    - NetworkManager.service:
      - `nmcli` or `nmtui` --> `dbus.socket` --> `NetworkManager.service` --> `provision network`
  - SLE15SP6：
    - wicked.service：相当于兼容 `initscripts` 的管理方式，如 `ifdown`、`ifup` 等命令与 `ifcfg-*` 配置文件等（man 5 ifcfg）。
    - NetworkManager.service：与 RHEL 中的使用方法完全一致
- 使用 nmcli 配置网络：
  - 有用的 man 手册：nmcli-examples(7), nm-settings-keyfile(5)
  - 命令行方式：
    - `$ nmcli connection add con-name "System eth0" type ethernet dev eth0`
    - `$ nmcli connection modify "System eth0" ipv4.addresses 172.25.250.110/24 ipv4.gateway 172.25.250.254 ipv4.dns 172.25.250.254 ipv4.method manual`
    - 同一台主机上可以存在多个默认路由，但是在这种情况下，可能发生间断性的访问中断。
    - 在生产的节点上，强烈建议只配置一个默认路由，而 `ipv4.gateway` 是默认路由的配置选项，因此，我们建议在 nmcli connection modify 过程中不要加入 ipv4.gateway 配置，并且将对应网口配置 `/etc/sysconfig/network-scripts/ifcfg-*` 中的 `DEFROUTE=yes` 去除！
    - `$ nmcli connection show`
    - `$ nmcli connection down "System eth0"`  ##等同于 ifdown "System_eth0"
    - `$ nmcli connection up "System eth0"`    ##等同于 ifup "System_eth0"
  - 配置文件方式：
    - /etc/NetworkManager/system-connections/*.nmconnection
    - /etc/sysconfig/network-scripts/ifcfg-*
- 主机名设置：
  - 命令行方式：
    - `$ hostnamectl hostname rh199.lab.example.com`
  - 配置文件方式：
    - `$ echo "servera.lab.example.com" > /etc/hostname`
    - `$ echo "servera.lab.example.com" > /proc/sys/kernel/hostname`
  - 主机名解析文件：
    - 静态解析：/etc/hosts
    - 动态解析：/etc/resolv.conf
  - 名称解析缓存的实现：
    - dnsmasq 软件包
    - nscd 软件包（推荐）：RHEL 与 SUSE 中均可使用
    - unbound 软件包：该服务功能齐全，但较重。
    - dns_resolver 内核模块
  - 测试主机名解析：

    ```bash
    $ host <FQDN>
    $ host <ipaddress>

    $ dig <FQDN> @<nameserver>
      # 默认返回完全限定主机名的 A 记录
    $ dig A <FQDN> @<nameserver>
      # 查询完全限定主机名的 A 记录
    $ dig SOA <FQDN> @<nameserver>
      # 查询完全限定主机名所在域内的权威名称解析服务器地址
    ```  

## 🧪 Lab 示例

- 使用基于密钥的免密登录的方法：
  - 方法1：拷贝客户端用户的 SSH 公钥至服务端用户的 authorized_keys 文件中
  - 方法2：ssh-copy-id 命令指定客户端用户 SSH 私钥拷贝其公钥

- SELinux 文件上下文标签的问题：
  - 描述：如何使用自定义 Web 根目录与端口实施 Web 服务？
  - 实验过程：

    ```bash
    ## root@serverb: web server
    $ yum install -y httpd
    $ mkdir /webapp
    $ echo "<b>Test Customized Web Page.</b>" > /webapp/index.html
    $ mv /etc/httpd/conf.d/welcome.conf /etc/httpd/conf.d/welcome.conf.bak
    $ vim /etc/httpd/conf.d/webapp.conf
      Listen 8182
      <VirtualHost *:8182>
        ServerName  serverb.lab.example.com
        DocumentRoot  "/webapp"
        <Directory "/webapp">
          AllowOverride none
          Require all granted
        </Directory>
      </VirtualHost>
    $ semanage port -a -t http_port_t -p tcp 8182
      # allow selinux to use tcp 8182 port
    $ firewall-cmd --permanent --zone=public --add-port=8182/tcp
    $ firewall-cmd --reload
    $ chcon -Rt httpd_sys_content_t /webapp
      # set selinux httpd file context
    $ systemctl enable --now httpd.service
      # root@servera: web client
    $ curl http://serverb.lab.example.com:8182
      <b>Test Customized Web Page.</b>
    ```

- NFSv4 与 autofs：
  - 描述：RHEL 9 中如何配置 nfs-server 和 nfs-client？
  - 实验过程：

    ```bash
    ## 注意：rpcbind 服务必须在 nfs-server 服务运行之前运行！
    ## root@serverb: 作为 nfs-server
    # 创建 nfs-server 并被 nfs-client 挂载共享
    $ mkdir -p /shared 
    $ mkdir -p /mnt/tp1/data{1..10}
    $ vim /etc/exports.d/shared.exports
      /shared  172.25.250.0/255.255.255.0(rw,sync,no_root_squash)
      /mnt/tp1  172.25.250.0/255.255.255.0(rw,sync,no_root_squash)
    $ systemctl start nfs-server.service
    $ systemctl enable nfs-server.service
    $ exportfs -a
      # 可不重启 nfs-server 服务而直接导出共享目录
    $ showmount -e localhost
    $ firewall-cmd --zone=public --permanent --add-service=nfs
    $ firewall-cmd --zone=public --permanent --add-service=mountd
    $ firewall-cmd --zone=public --permanent --add-service=rpc-bind
    $ firewall-cmd --zone=public --reload
    $ firewall-cmd --zone=public --list-all

    ## root@servera: 作为 nfs-client
    $ systemctl status rpcbind.service
    $ mkdir /mnt/shared
    $ mount 172.25.250.11:/shared /mnt/shared
    $ df -Th /mnt/shared
    ```

  - 描述：RHEL 9 中如何配置 autofs 自动挂载客户端来直接或间接挂载 NFS 共享？
  - 实验过程：

    ```bash
    ## root@servera: 作为 nfs-client
    # 场景1： autofs 自动挂载直接映射的共享
    $ yum install -y autofs
    $ vim /etc/auto.master.d/direct.autofs
      /- /etc/auto.direct
    $ vim /etc/auto.direct
      /shared -rw,sync,fstype=nfs4 serverb.lab.example.com:/shared
    $ systemctl enable --now autofs.service
    $ cd /shared
      # 触发自动挂载 nfs-server 的 /shared 共享
    $ df -Th
      # serverb.lab.example.com:/shared nfs4      9.9G  1.6G  8.4G  16% /shared
        
    # 场景2：autofs 自动挂载通配符支持的间接映射的共享
    $ vim /etc/auto.master.d/indirect.autofs
      /tp-data /etc/auto.indirect
    $ vim /etc/auto.indirect
      * -rw,sync,fstype=nfs4 serverb.lab.example.com:/mnt/tp1/&
    $ systemctl restart autofs.service
    $ cd /tp-data/data3
    $ cd /tp-data/data5
      # 触发自动挂载 nfs-server 的间接映射共享目录
    ```
