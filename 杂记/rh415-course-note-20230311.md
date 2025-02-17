# RH415 课堂笔记

## 当前重要的 RedHat 高级课程简介

- RH415: Linux Security
- RH342: Linux Troubleshooting
- RH442: Linux Performance Tuning
- DO447: Ansible Advanced
- DO188: Container Basic and Developmemt (podman skopeo buildah)
- DO280: Kubernetes on OpenShift
- DO288: Container App Development on OpenShift
- DO316: OpenShift Virtualization (KubeVirt)

## 课程环境中的用户名与密码

- workstation, tower, servera~e: `root:redhat`, `student:student(sudo)`
- foundation0: `root:Asimov`

## 声明的类型

- 命令式声明：告诉你该怎么做就必须怎么做
- 陈述式声明：通过无限地调整最终符合预定目标的行为
- 幂等性：只要达到预期的目标，无论如何更改指令都不做相应的状态修改。

## Ansible 基础配置

- control node: 控制节点

  ```bash
  $ sudo yum install -y ansible
  # 说明：控制节点通过 SSH 免密登录的方式登录受管主机并下发执行任务。
  $ mkdir ./ansible
  # 创建 Ansible 项目目录，名称为 ansible。
  $ cd ansible
  $ vim ansible.cfg
  # ini 格式
  $ vim inventory
  # ini 格式  
  ```

- managed nodes: 受管主机
- Ansible 连接中使用的用户说明：
  控制节点的普通用户devops --SSH免密登录--> 受管主机的登录用户devops --特权升级sudo--> 受管主机的root
- Ansible 的 playbook 是一个 yaml 格式的文件，用于将下发任务的集合组织在一起。
- playbook 中包含一个或多个 play，它们共同组成一个数组结构，其中每个 play 中包含一个或多个任务。
- Python 数组的定义为 `["a", "b", "hello world"]`，可转换为如下形式：

  ```yaml
  - "a"
  - "b"
  - "hello world"
  ```
  
- Ansible 中的正则表达式采用 `regex` 参数实现，实际上是调用 Python 的 `re` 模块。

## devicemapper 内核模块说明

- Linux kernel module - devicemapper:
- kernel >= 2.6.x 才支持该模块
- 以此模块支持的技术点：LVM, stratis, VDO, devicemapper-multipath, docker-graph-driver, LUKs

## LUKs 设备相关

- LUKs 命令示例：

  ```bash
  $ sudo cryptsetup luksFormat /path/to/device
  $ sudo cryptsetup luksAddKey --key-slot <num> /path/to/device
  $ sudo cryptsetup luksOpen /path/to/device <logicalName>
  $ sudo cryptsetup luksClose /dev/mapper/<logicalName>
  $ sudo cryptsetup luksRemoveKey /path/to/device
  $ sudo cryptsetup luksDump /path/to/device
  ```
  
- 重要配置文件：`/etc/crypttab`

## RHEL 中的两大日志系统

- systemd-journald: 在系统 boot 早起即启动，日志存储于内存中。
- rsyslog: 相较于 systemd-journald 日志系统较晚启动，一般在系统启动中系统控制权交由 systemd 交管后才启动。
- 两者的关系：systemd-journald 日志系统不区分日志类型，rsyslog 区分日志的类型与等级并且可将日志于 /var/log/ 中落盘归档。

## 更改系统的时间

- date 命令：

  ```bash
  $ sudo date -s "2023-03-18 20:00:00"
  ```
  
- timedatectl 命令：
  - 如果使用本地采用了上游 NTP 时间同步服务器，那么无法使用该命令更改时间，只能通过 date 命令强行修改。
  - 如果可使用该命令，那么可参考：sudo timedatectl set-time "2023-03-18 20:30:00"

## PAM 文件的调用关系

- 所在目录：/etc/pam.d/
- pam 文件的关系：
  - system-auth --link --> system-auth-local (可手动修改) --include --> system-auth-ac (authconfig 命令修改)
  - password-auth --link --> password-auth-local (可手动修改) --include --> password-auth-ac (authconfig 命令修改)
- authconfig 对 `*-ac` 文件做修改，对 `*-local` 文件不生效。
- 引入 `*-local` 文件的目的：
  不直接手动修改 `*-ac` 文件而是通过 authconfig 自动生成修改，而手动更新的部分在 `*-local` 文件中，
  这样不会因为手动更新 `*-ac` 文件而导致老配置的丢失。

## PAM 密码相关模块操作

- `pam_pwquality.so` 模块更改密码复杂度
- 以上模块中 `enforce_for_root` 参数的使用
- `pam_pwhistory.so` 模块记录密码更改历史

## PAM 关键的模块与配置文件的对应

- `pam_time.so`：
  - 配置文件：/etc/security/time.conf
  - 功能：限制一个或多个用户以及用户组在指定时间内的登录，登录可以是本地 login 也可以是远程 ssh，telnet 不做考虑。
- `pam_access.so`：
  - 配置文件：/etc/security/access.conf
  - 功能：限制用户或用户组从指定的 IPv4、IPv6、主机名或域名到节点的登录
- `pam_pwquality.so`：
  - 配置文件：/etc/security/pwquailty.conf
  - 功能：更改密码复杂度、限制普通用户或 root 用户的密码更改以及密码更改的交互信息
- `pam_pwhistory.so`：
  - 配置文件：NA
  - 功能：记录 passwd 命令对指定用户的密码更改历史，记录于 /etc/security/opasswd 中。
- `pam_faillock.so`：
  - 配置文件：NA
  - 功能：锁定失败登录的用户
  - 命令：

    ```bash
    $ sudo faillock --user <user>
    $ sudo faillock --user <user> --reset
    ```

## PAM 联想到的问题

- /etc/hosts.deny 与 /etc/hosts.allow 和 PAM 有关系吗？
  这两个文件属于 tcp-wrapper 四层防火墙与 PAM 没有直接关系
- `pam_faillock.so` 模块对用户的锁定只针对于明文口令，与密钥无关！

## 关于 auditd 的默认行为

- auditd 守护进程默认可接收 su, sudo, selinux 的更改，可将日志记录于 /var/log/audit/audit.log 中。

## auditd 日志查询相关命令

- auditd 日志查询与汇总：

  ```bash
  $ sudo aureport --executable --summary
  $ sudo aureport --login
  # 通过以上命令可获取相关类型的事件 ID
  $ sudo ausearch -i -a <event_id>
  $ sudo ausearch --raw -a <event_id>
  $ sudo ausearch -m LOGIN --format csv > /path/to/file.csv
  # 将登录类型的审计事件以 CSV 格式导出
  $ sudo autrace /path/to/executable_file
  # 追踪可执行文件的审计，返回相关 PID 信息。
  $ sudo ausearch --raw -p <pid> | aureport -i --file
  ```
  
- 自定义 auditd 审计规则：

  ```bash
  $ sudo auditctl -w /bin -p x -F auid>=500 -F euid=0 -k privileged-escaution
  ```
  
- auditctl 命令中的 `-p` 选项属性 a 指代：
  - 文件或目录的所有者、所属组、其他用户的所属权限
  - 访问控制列表 ACL 的更改
  - SELinux 文件或目录上下文标签的更改 (context 或 label)
  - lsattr 中的写保护标记的更改
  - touch 命令对文件或目录的 ctime, atime, mtime 的更改

## aide 工具注意点

- 优点：aide 通过在两个不同时间点之间建立数据库，比对数据库之间的不同信息来确定文件或目录发生的状态变化。
- 缺点：aide 只能监控文件或目录在不同时间点的状态，不能监控这两种状态间是如何改变的！
- 通过 auditd 工具来弥补 aide 工具的不足
- /var/lib/aide/aide.db.gz 初始的检查基线：
  - 不一定是安装完 aide 即刻生成的数据库，可能是安装完 aide 后生成的，也可能是基于某个时间点手动生成的数据库。
  - 该数据库作为以后数据库比对的标准
- /etc/aide.conf 中的选择行说明：

  ```bash
  =/testdir   # 监测目录本身，不对其中的子文件与子目录生效
  =/testdir/  # 监测目录中的子文件与子目录，但是不对子目录做递归。
  /testdir    # 监测目录以及其中的子文件与子目录
  ```

## /etc/selinux/config 中的 selinux 状态

- enforcing: 强制模式，强制执行 selinux 规则。
- permessive: 被动模式，不执行 selinux 规则，但是在后台记录 selinux 事件，以作为后续 selinux 策略启动时的依据。
- disabled: 禁用模式，拒绝执行 selinux 规则。
- 将 disabled 模式的主机切换为 enforcing 模式：遵循该过程 `disabled -> permessive -> enforcing`
