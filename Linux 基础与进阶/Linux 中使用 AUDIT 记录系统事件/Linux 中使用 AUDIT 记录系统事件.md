# Linux 中使用 AUDIT 记录系统事件

## 文档目录

- [Linux 中使用 AUDIT 记录系统事件](#linux-中使用-audit-记录系统事件)
  - [文档目录](#文档目录)
  - [1. 配置 Audit 以记录系统事件](#1-配置-audit-以记录系统事件)
    - [1.1 Linux Audit 系统](#11-linux-audit-系统)
    - [1.2 使用 Audit 审计系统](#12-使用-audit-审计系统)
    - [1.3 配置 auditd](#13-配置-auditd)
    - [1.4 使用 auditd 进行远程日志记录](#14-使用-auditd-进行远程日志记录)
  - [2. 检查 Audit 日志](#2-检查-audit-日志)
    - [2.1 解读 Audit 消息](#21-解读-audit-消息)
    - [2.2 搜索事件：ausearch 命令](#22-搜索事件ausearch-命令)
    - [2.3 Audit 消息报告：aureport 命令](#23-audit-消息报告aureport-命令)
    - [2.4 追踪程序：autrace 命令](#24-追踪程序autrace-命令)
  - [编写自定义审计规则](#编写自定义审计规则)
    - [添加规则](#添加规则)
    - [删除规则](#删除规则)
    - [检查规则](#检查规则)
    - [使规则不可变](#使规则不可变)
    - [持久规则](#持久规则)
  - [启用预打包的审计规则集](#启用预打包的审计规则集)
    - [预打包的审计规则集](#预打包的审计规则集)
    - [全终端按键记录](#全终端按键记录)

-----

## 1. 配置 Audit 以记录系统事件

### 1.1 Linux Audit 系统

- Linux Audit 系统是内核中的一种机制，提供了一种在系统上跟踪与安全相关的事件和信息的方法。
- 可定义一组加载到内核中的规则，以指定它在审计日志中记录哪些事件。
- **auditd 系统守护进程将记录的事件写入本地磁盘，或将它们转发到远程日志服务器。**
- 然后，可使用此信息来发现和调查违反或企图违反系统安全策略的原因。
- 可将 Audit 配置为根据用户身份、安全上下文或其他标签包含或排除事件。
- Audit 系统可收集的事件相关信息：
  - 事件的日期和时间、类型以及结果。
  - 主题和对象的敏感度标签。
  - 事件与触发事件的用户身份之间的关系。
  - 对 Audit 配置的所有修改，以及尝试访问 Audit 日志文件的行为。
  - 所有身份认证机制的使用情况，如 SSH 和 Kerberos 等。
  - 对任何受信任数据库的更改，如 /etc/passwd。
  - 尝试在系统中导入或导出信息的行为。

### 1.2 使用 Audit 审计系统

- **auditd 是 Linux 审计子系统的用户空间组件（user-space component）。**
- 在守护进程启动时，启动 auditd 的 systemd 单元文件，通常会加载所有持久性审计规则。
- 当 Audit 规则加载至内核后，内核使用规则报告相关的事件。
- 内核将事件相关的信息发送给 auditd 守护进程。
- auditd 守护进程的角色：收集由内核报告的审计事件信息，并将其保存至日志文件中。
- ✅ 审计流程：*audit_rules* ⬅ *kernel* ➡ *audit_events* ➡ *auditd* ➡ *log_file*
- auditd 守护进程运行时，事件信息将被写入 **<span style="color:orange">/var/log/audit/audit.log</span>**，该守护进程不运行时，rsyslog 接收内核的审计信息。
- 若没有任何额外的配置，仅有限数量的消息通过审计系统，主要是身份认证和授权消息（用户正在登录、正在使用sudo等）和SELinux消息。
- 使用 auditctl 命令添加控制审计系统的审计规则、监视文件或有关任何系统调用的记录信息。

### 1.3 配置 auditd

- /etc/audit/auditd.conf：auditd 守护进程主配置文件
- /etc/audit/audit.rules：auditd 加载的审计规则
  
  > 注意：不要编辑以上文件，auditd 在启动时将从 /etc/audit/audit.d/ 中生成该文件！

- /etc/audit/rules.d：手动配置审计规则的目录

  > 注意：该目录中的所有审计规则在 auditd 启动时被合并入 /etc/audit/audit.rules 中，并加载至内核中。

- /etc/audit/rules.d/audit.rules：手动管理的审计规则文件

```bash
$ sudo systemctl [start|stop|restart|enable|disable] auditd.service
# 启动、停止、重启、开机自启动、开机不自启动 auditd 服务
```

- auditd 启动后，其 systemd 单元文件自动从 /etc/audit/rules.d 中重建审计规则集，并将其加载至内核中。
- 当 auditd 重新加载时，它会尝试使用 **augenrules** 命令重建规则文件并加载新的规则。
- ❗若加载了使当前规则不可变的控制规则，可能会失败，需重新引导系统以更改加载的规则集。
- 调整 auditd 设置以管理存储：
  - 根据加载的审计规则，审计日志可能会非常快速地变大。
  - ✅ 推荐做法：/var/log/audit 目录最好是带有自有文件系统的挂载点，这样就不会有其他进程因存储空间和性能调优原因而与审计日志竞争。
  - 若审计系统由于缺少存储空间而无法将事件记录到磁盘，则系统必须立即停止或降级到单用户模式，这是为了确保不会在没有记录的情况下发生任何事件。
  - auditd 护进程具有相关的机制，可以在即将发生这种情况时发出警告，并在存储变低时采取各种操作。
  - /etc/audit/auditd.conf 配置文件参数：

    | 参数/指令（directive） | 描述（description） |
    | ----- | ----- |
    | log_file | 用于存储审计日志文件的位置，默认为 /var/log/audit/audit.log。 |
    | max_log_file | 最大审计日志文件大小（以 MB 为单位）。<br> 当到达此阈值，触发 max_log_file_action 参数设置的操作。 |
    | max_log_file_action | 当审计日志文件大小到达 max_log_file 后执行的操作。<br> 默认设置为 `ROTATE`，这将轮转日志文件，由 num_logs 参数指定保持旧日志文件的数量；如果设置为 `KEEP_FILES`，日志文件将轮转且不被删除（忽略 num_logs 参数）。KEEP_FILES 将最终占满日志文件所在的存储，除非备份或移除这些旧日志文件。 |
    | space_left | 审计日志文件所在的文件系统上剩余的可用空间大小（以 MB 为单位），用于触发 space_left_action 参数中设置的操作。<br> 此参数必须设置为一个数字，以便管理员有足够的时间来响应并释放文件系统空间，并在文件系统仅剩余 admin_space_left 的可用空间前采取行动。 |
    | space_left_action | 默认设置为 `SYSLOG`，这记录 warning 日志。<br> 如果设置为 `EMAIL`，系统上配置了 /usr/lib/sendmail（Postfix 或 Sendmail 提供），email 会发送到由 action_mail_acct 参数指定的地址。可选地，也可设置为 `EXEC /path/to/script` 或 `/path/to/script` 命令来运行。 |
    | admin_space_left | 审计日志文件所在的文件系统上可用空间的绝对最小值（以 MB 为单位），用于触发 admin_space_left_action 参数设置的操作。<br> 此参数必须设置为⼀个数字，确保留出⾜够的空间来记录管理员执⾏的操作。此参数通常会暂停 auditd 守护进程或停⽌整个系统，以保留少量的可⽤空间来修复问题。 |
    | admin_space_left_action | 到达 admin_space_left 参数设置的阈值时执行的操作。<br> 默认设置为 `SUSPEND`，这将引起 auditd 停止向文件系统写入审计记录。许多安全策略要求将此参数设置为 `SINGLE`（将系统置于单用户模式，允许管理员进行修复）或 `HALT`（暂停整个系统）。由于系统没有维持完整的审计日志，这些安全策略设置这些值来禁止操作此系统。 |
    | disk_full_action | 审计日志所在文件系统没有可用空间时触发的操作。<br> 此参数可设置为 `SUSPEND`、`SINGLE` 或 `HALT`。此参数可确保当审计⽆法再记录事件时，系统已关闭或在单⽤⼾模式下。 |
    | disk_error_action | 审计日志所在文件系统还有可用空间，但检测到错误时触发的操作。<br> 默认设置为 `SUSPEND`，这将停止记录审计事件。也可设置为 `SINGLE` 或 `HALT`。 |

    ```bash
    $ man 5 auditd.conf
    # 查看 auditd 服务配置文件参数的详细信息
    ```

- 调整以下 /etc/audit/auditd.conf 参数以提高 auditd 服务性能：
  - flush = INCREMENTAL_ASYNC：
    - 达到 freq 指定的写入次数后将记录异步清理到存储，以提高系统性能。
    - 但由于系统崩溃时审计消息可能丢失，此时需将此参数设置为 DATA 或 SYNC。
    - 以相对较低的 freq 设置使用 INCREMENTAL_ASYNC，可确保记录快速同步到文件系统，同时保持良好的系统性能。
  - freq = 50：
    - 在每 50 条记录后清空 Audit 日志（假设设置 flush = INCREMENTAL_ASYNC）。
    - 若 flush 设置为 DATA 或 SYNC，则忽略此设置，并且每次写入存储都是同步的。
  - ❗只有设置 flush = INCREMENTAL_ASYNC 时，freq 参数才起作用！
  
  > 说明：可以理解为减小批次的记录，提高异步刷新的频率来提高审计记录的性能。

### 1.4 使用 auditd 进行远程日志记录

- 可通过以下两种方式将审计消息发送到远程系统：
  - 1️⃣ 可将审计消息发送到本地 rsyslog 服务并将其转发至 rsyslog 配置的远程服务器。
  - 2️⃣ 可将系统配置为将审计消息发送到远程 auditd 服务。
- **配置客户端：**
  - 使用以上两种方式均需对系统的 /etc/audit/auditd.conf 文件进行以下更改：
    - log_format = ENRICHED：在传输每个事件之前解析 UID、GID、系统调用数量、架构和套接字地址信息等，保证该日志信息在远程系统上有意义，远程系统上可能具有以上映射的不同值。
    - name_format = HOSTNAME：在每条消息中包含主机名，区别审计消息所属的主机。
  - 针对于 **第一种方式**：
    - 1️⃣ 需在客户端安装 audispd-plugins 软件包。
    - 2️⃣ 设置客户端 **<font color=orange>/etc/audit/plugins.d/syslog.conf</font>** 文件中 active = yes 可将消息发送到 rsyslog。
    - 3️⃣ 配置本地 rsyslog 服务，将日志消息发送到 /etc/rsyslog.conf 文件中定义的远程日志服务器。
    - ⚗️ 补充实验：[LogAnalyzer 集成 auditd 日志](https://github.com/Alberthua-Perl/sc-col/tree/master/rsyslog-loganalyzer-viewer#loganalyzer-%E9%9B%86%E6%88%90-auditd-%E6%97%A5%E5%BF%97)
  - 针对于 **第二种方式**：
    - 1️⃣ 需在客户端安装 audispd-plugins 软件包。
    - 2️⃣ 设置客户端 **<font color=orange>/etc/audit/plugins.d/au-remote.conf</font>** 文件中的 active = yes。
    - 3️⃣ 设置客户端 **<font color=orange>/etc/audit/audisp-remote.conf</font>** 文件中的 remote_server 指令为远程 auditd 服务器的 IP 地址或主机名，若远程服务器未监听默认 60/TCP 端口的话，需更新 port 指令。
- **配置服务端：**
  - 针对于 **第一种方式：**
    - 1️⃣ 配置 rsyslog 服务 /etc/rsyslog.conf 文件，配置其为服务端并监听 514/UDP 或 514/TCP 端口，接收来自客户端的审计消息。（参考前文补充实验）
  - 针对于 **第二种方式：**
    - 1️⃣ 设置 /etc/audit/auditd.conf 文件中的 tcp_listen_port = 60 指令（60/TCP）用以监听远程客户端的审计消息。
    - 2️⃣ 设置 firewalld 以放行 60/TCP 端口。
    - ❗必须以重启操作系统的方式来重启 auditd 服务使其配置生效。

    > 注意：<br>
    > 默认情况下，以上两种方式传输审计日志均通过未加密的明文方式实现，缺少安全加密极可能导致关键信息暴露或窃取。<br>
    > rsyslog 服务必须通过 TLS 认证与加密保护审计日志传输；auditd 守护进程使用 Kerberos 认证与加密（客户端 /etc/audisp/audisp-remote.conf 与服务端 /etc/audit/auditd.conf）。<br>
    > 查看 man 手册：audisp-remote.conf(5)，auditd.conf(5)

- 📝 常用离线帮助文档：
  
  ```bash
  $ man 5 auditd.conf           # 查看 auditd 服务配置文件的详细使用信息
  $ man 5 audisp-remote.conf    # 查看 audisp-remote 配置文件信息
  $ man 7 audit.rules           # 查看内核审计系统的规则集合信息
  ```

-----

## 2. 检查 Audit 日志

### 2.1 解读 Audit 消息

- /var/log/audit/audit.log 中记录的 Audit 事件在紧凑的格式中纳入许多的信息。
- 一个事件实际上可能会记录多个不同类型的审计记录到日志中，作为单独的消息。
- 这些记录可能各自包含多个字段，记录事件的相关信息。
- 示例：/var/log/audit/audit.log 文件中未经处理的单个审计事件（audit event）相关的多条审计记录（audit record）<br>
  1️⃣ **type=SYSCALL msg=audit(1371716130.596:28708)**: arch=c000003e **syscall=2** success=yes exit=4 a0=261b130 a1=90800 a2=e a3=19 items=1 ppid=2548 pid=26131 **auid=1000** uid=0 gid=0 euid=0 suid=0 fsuid=0 egid=0 sgid=0 fsgid=0 tty=pts0 ses=1 comm="aureport" exe="/sbin/aureport" subj=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023 **key="audit-access"**<br>
  2️⃣ type=CWD msg=audit(1371716130.596:28708):  cwd="/root"<br>
  3️⃣ type=PATH msg=audit(1371716130.596:28708): item=0 name="/var/log/audit" inode=17998 dev=fd:01 mode=040750 ouid=0 ogid=0 rdev=00:00 obj=system_u:object_r:auditd_log_t:s0<br>
  
  以上输出显示了三条审计记录，它们都属于审计事件 **28708** 的一部分。<br>
  - "type=SYSCALL" 记录指明了 "type" 字段的内容。每条审计记录都有一个记录类型，有时也被称为消息类型，这种类型是由每个记录开头的 "type" 字段体现。此记录为 "SYSCALL" 记录。
  - "msg=audit(1371716130.596:28708)" 这条记录指出了 "msg" 字段的内容。该 "msg" 字段给出了此记录的 **时间戳** 以及 **审计事件编号**。冒号前的数字（在本例中为 1371716130.596）是以秒为单位表示的自起始时间点（即 1970 年 1 月 1 日 00:00 UTC）以来的时间戳。可使用 `date --date=@<起始时间>` 命令将起始时间转换为本地时间。冒号后的数字（28708）是审计事件编号，该记录与该事件的其他记录共享此编号。
  - "syscall=2" 记录指代的是系统调用字段。第一条记录的类型为 "SYSCALL"，这表示的是向内核发出的系统调用的相关信息。这个系统调用字段表示的是所发出的系统调用的编号（而非其名称）。系统调用编号与名称之间的对应关系在不同的处理器架构中可能会有所不同（X86/ARM/MIPS/RISC-V 等架构），这就是为什么在没有帮助的情况下直接解读原始格式的日志可能会很困难的原因之一。可以使用 ausearch 命令来提供帮助。
  
    ```bash
    $ sudo uname -a
    Linux foundation0.ilt.example.com 5.14.0-570.12.1.el9_6.x86_64 #1 SMP PREEMPT_DYNAMIC Fri Apr 4 10:41:31 EDT 2025 x86_64 x86_64 x86_64 GNU/Linux
    # 查看系统内核版本
    $ sudo head -n 10 /usr/include/asm/unistd_64.h    # x86_64 架构中系统调用与编号对应关系
    #ifndef _ASM_UNISTD_64_H
    #define _ASM_UNISTD_64_H

    #define __NR_read 0
    #define __NR_write 1
    #define __NR_open 2
    #define __NR_close 3
    #define __NR_stat 4
    #define __NR_fstat 5
    #define __NR_lstat 6
    ```

  - "auid=1000" 记录指代的是 "auid" 字段。该 "auid" 字段记录了触发此事件的用户的审计用户标识（Audit UID）。这是触发此事件的用户最初用于 **<font color=orange>登录此机器的账户的标识</font>**，即便该用户使用了 sudo 或 su 命令以其他用户身份登录也是如此，即 **<font color=orange>原始登录用户</font>**。

### 2.2 搜索事件：ausearch 命令

- 审计系统附带 ausearch 命令，该命令用于搜索审计日志。
- 可使用 ausearch 搜索和过滤各种类型的事件。
- 可将数值转换为更易读的值，如用户名或系统调用名，以解读这些事件。
- 常用的 ausearch 命令选项：
  
  | 选项 | 描述 |
  | ----- | ----- |
  | -i | 解析日志记录，将数值转换为名称。当拥有原始日志文件时，此功能非常有用。 |
  | --raw | 打印原始日志条目，甚至不要在事件之间添加分隔符。可配合其他能够解析原始日志格式的工具。与选项 -r 等效。 |
  | -a `<EVENT-ID>` | 显示具有 `<EVENT-ID>` 作为事件标识的该事件的所有记录。 |
  | -m `<MESSAGE-TYPE>` | 显示所有包含 `<MESSAGE-TYPE>` 作为其消息类型记录的事件。与长选项 --message 等效。 |
  | -f `<FILENAME>` | 搜索与特定文件名相关的所有事件。与长选项 --file 等效。 |
  | -k `<KEY>` | 查找所有标有 `<KEY>` 键的事件。 |
  | --start `[start-date]` `[start-time]` | 仅需在 *start-date* 和 *start-time* 之后进行搜索。如果未指定开始时间，搜索将默认设定为午夜（midnight）。如果未指定开始日期，搜索将默认设定为今天（today）。 <br> 时间格式取决于当前的地区设置。还可以使用其他值，如 **recent**（过去十分钟内）、**this-week**、**this-month** 和 **this-year**。 <br> **--end** 可用于查找在特定日期和时间之前发生的事件，并且其使用方式与 **--start**相同。 |

  ```bash
  $ sudo ausearch -i -if /path/to/file
  # 查看指定文件的全部审计记录

  $ sudo ausearch -i -a <EVENT-ID>
  # 查看指定审计事件的全部审计记录

  $ sudo ausearch -p <PID> --raw
  # 查看指定进程的原始审计日志条目

  $ sudo ausearch -m <MESSAGE-TYPE>
  # 根据消息类型搜索审计记录

  $ sudo ausearch -f /path/to/file
  # 根据文件名称搜索审计记录
  ```

- 🔎 **解释审计日志条目：**

  🎯 Audit 审计事件字段汇总：[Audit Event Fields | RHEL Audit System Reference](https://access.redhat.com/articles/4409591)，如果由于访问权限问题，可直接访问 [GitHub 链接](https://github.com/Alberthua-Perl/tech-docs/blob/master/Linux%20%E5%9F%BA%E7%A1%80%E4%B8%8E%E8%BF%9B%E9%98%B6/Linux%20%E4%B8%AD%E4%BD%BF%E7%94%A8%20AUDIT%20%E8%AE%B0%E5%BD%95%E7%B3%BB%E7%BB%9F%E4%BA%8B%E4%BB%B6/RHEL%20Audit%20System%20Reference%20-%20Red%20Hat%20Customer%20Portal.pdf)。

  ```bash
  $ sudo ausearch -i -a 28708
  ----
  type=PATH msg=audit(07/31/2023 10:15:30.596:28708) : item=0 name=/var/log/
  audit inode=17998 dev=fd:01 mode=dir,750 ouid=root ogid=root rdev=00:00
  obj=system_u:object_r:auditd_log_t:s0
  type=CWD msg=audit(07/31/2023 10:15:30.596:28708) :  cwd=/root
  type=SYSCALL msg=audit(07/31/2023 10:15:30.596:28708) : arch=x86_64 syscall=open
  success=yes exit=4 a0=261b130 a1=90800 a2=e a3=19 items=1 ppid=2548 pid=26131
  auid=student uid=root gid=root euid=root suid=root fsuid=root egid=root
  sgid=root fsgid=root tty=pts0 ses=1 comm=aureport exe=/sbin/aureport
  subj=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023 key=audit-access
  ```

  - 每个审计事件之间都以四个破折号隔开。
  - 此输出显示了 **PATH**、**CWD** 和 **SYSCALL** 事件类型。
  - PATH 记录是与该事件相关的文件。该文件名为 /var/log/audit（name=/var/log/audit），其 inode 为 17998（inode=17998）。该文件位于文件系统上的一个设备上，其主/次设备号为 253，1（dev=fd:01，设备编号采用**十六进制格式**）。通过使用 ls -l 命令在 /dev 目录中查看，可以看到 /dev/dm-1 设备具有这些编号，并与一个逻辑卷相关联。该文件是一个目录，具有八进制权限 750（mode=dir,750），由 root 用户和 root 组所有（ouid=root ogid=root），具有 SELinux 类型 auditd_log_t（obj=system_u:object_r:auditd_log_t:s0）。
  - CWD 记录指的是与引发此事件的进程相关联的当前工作目录，在此例中为 /root 目录。
  - SYSCALL 记录即触发此事件的系统调用。使用 **open()** 系统调用（syscall=open）成功（success=yes）打开了由 PATH 记录指定的文件（即 /var/log/audit 目录）。此调用由具有 26131 PID 的进程执行（pid=26131）。该调用由 /sbin/aureport 可执行文件启动（exe=/sbin/aureport），并以 root 有效用户 ID（euid=root）和无限制 SELinux 域 （subj=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023）由 root 用户（uid=root）运行。该命令在 pts/0 虚拟终端（tty=pts0）上运行，可能是图形终端窗口或远程登录会话。用户**最初**以 student 用户身份登录（auid=student），此后不知何故变成了 root 用户。此记录设置了审计访问键，以便使用 ausearch 命令更容易找到其事件（key=audit-access）。

### 2.3 Audit 消息报告：aureport 命令

- aureport 命令可用于获取审计消息的快速概览和特定类型事件的详细报告。
- 常用选项：
  
  ```bash
  $ aureport --help
  usage: aureport [options]
        -a,--avc                        Avc report
        -au,--auth                      Authentication report
        --comm                          Commands run report
        -c,--config                     Config change report
        -cr,--crypto                    Crypto report
        --debug                         Write malformed events that are skipped to stderr
        --eoe-timeout secs              End of Event Timeout
        -e,--event                      Event report
        --escape option                 Escape output
        -f,--file                       File name report
        --failed                        only failed events in report
        -h,--host                       Remote Host name report
        --help                          help
        -i,--interpret                  Interpretive mode
        -if,--input <Input File name>   use this file as input
        --input-logs                    Use the logs even if stdin is a pipe
        --integrity                     Integrity event report
        -k,--key                        Key report
        -l,--login                      Login report
        -m,--mods                       Modification to accounts report
        -ma,--mac                       Mandatory Access Control (MAC) report
        -n,--anomaly                    aNomaly report
        -nc,--no-config                 Don't include config events
        --node <node name>              Only events from a specific node
        -p,--pid                        Pid report
        -r,--response                   Response to anomaly report
        -s,--syscall                    Syscall report
        --success                       only success events in report
        --summary                       sorted totals for main object in report
        -t,--log                        Log time range report
        -te,--end [end date] [end time] ending date & time for reports
        -tm,--terminal                  TerMinal name report
        -ts,--start [start date] [start time]   starting data & time for reports
        --tty                           Report about tty keystrokes
        -u,--user                       User name report
        -v,--version                    Version
        --virt                          Virtualization report
        -x,--executable                 eXecutable name report
        If no report is given, the summary report will be displayed
  ```

- ❗aulast 命令与 aulastlog 命令和 last 命令与 lastlog 命令极其类似，但前者不解析 `/var/log/wtmp` 与 `/var/log/btmp` 文件。

  | 特性 | /var/log/wtmp | /var/log/btmp |
  | ----- | ----- | ----- |
  | **记录内容** | **成功登录**历史 | **失败登录**尝试（暴力破解痕迹） |
  | **数据格式** | 二进制（utmp 结构） | 二进制（utmp 结构） |
  | **读取命令** | `last` | `lastb` |
  | **审计价值** | 追踪用户会话、取证分析 | 检测攻击行为、安全审计 |
  | **日志轮转** | 通常保留（`wtmp.1`） | 通常保留（`btmp.1`） |
  | **权限要求** | root 可读 | root 可读 |

  ```bash
  $ sudo last
  # 查看系统上全局成功登录记录

  $ sudo last <username>
  # 查看系统上指定用户的成功登录记录

  $ sudo lastb
  # 查看系统上全局失败登录记录

  $ sudo lastb <username>
  # 查看系统上指定用户的失败登录记录
  ```

### 2.4 追踪程序：autrace 命令

- autrace 命令可用于调查进程执行的系统调用，该命令与 strace 命令非常类似。
- 💥 运行 autrace 命令将删除所有自定义审计规则，并将其替换为专门用于追踪指定程序的规则。
- 执行完成后，autrace 命令将清除这些规则，然后提供示例 ausearch 命令来调查这些事件。
- 这对于排除故障或调查感兴趣的程序非常有用。
- 示例：使用 autrace 调查 ip 命令的系统调用

  ```bash
  $ sudo autrace /usr/sbin/ip link show dev eth0
  Waiting to execute: /usr/sbin/ip
  2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP mode DEFAULT group default qlen 1000
      link/ether 52:54:00:00:fa:0a brd ff:ff:ff:ff:ff:ff
      altname enp1s0
  Cleaning up...
  Trace complete. You can locate the records with 'ausearch -i -p 1959'

  $ sudo ausearch -i -p 1959                            # 方法1：输出此进程的解析的审计日志
  ...
  $ sudo ausearch --raw -p 1959 | aureport -i --file    # 方法2：输出此进程的原始审计日志，配合 aureport 命令解析。

  File Report
  ===============================================
  # date time file syscall success exe auid event
  ===============================================
  1. 03/15/26 23:53:23  newfstatat yes /usr/sbin/autrace root 326
  2. 03/15/26 23:53:24 /usr/sbin/ip execve yes /usr/sbin/ip root 330
  3. 03/15/26 23:53:24 /etc/ld.so.preload access no /usr/sbin/ip root 333
  4. 03/15/26 23:53:24 /etc/ld.so.cache openat yes /usr/sbin/ip root 334
  5. 03/15/26 23:53:24  newfstatat yes /usr/sbin/ip root 335
  6. 03/15/26 23:53:24 /lib64/libbpf.so.1 openat yes /usr/sbin/ip root 338
  7. 03/15/26 23:53:24  newfstatat yes /usr/sbin/ip root 340
  8. 03/15/26 23:53:24 /lib64/libelf.so.1 openat yes /usr/sbin/ip root 347
  9. 03/15/26 23:53:24  newfstatat yes /usr/sbin/ip root 349
  10. 03/15/26 23:53:24 /lib64/libmnl.so.0 openat yes /usr/sbin/ip root 357
  11. 03/15/26 23:53:24  newfstatat yes /usr/sbin/ip root 359
  12. 03/15/26 23:53:24 /lib64/libcap.so.2 openat yes /usr/sbin/ip root 367
  13. 03/15/26 23:53:24  newfstatat yes /usr/sbin/ip root 369
  14. 03/15/26 23:53:24 /lib64/libc.so.6 openat yes /usr/sbin/ip root 375
  15. 03/15/26 23:53:24  newfstatat yes /usr/sbin/ip root 380
  16. 03/15/26 23:53:24 /lib64/libz.so.1 openat yes /usr/sbin/ip root 389
  17. 03/15/26 23:53:24  newfstatat yes /usr/sbin/ip root 391
  18. 03/15/26 23:53:24  newfstatat yes /usr/sbin/ip root 447
  19. 03/15/26 23:53:24 /etc/iproute2/group openat yes /usr/sbin/ip root 448
  20. 03/15/26 23:53:24  newfstatat yes /usr/sbin/ip root 449
  ```

  > 说明：
  > newfstatat 系统调用

- 命令示例：

  ```bash
  $ sudo autrace /path/to/command
  # 追踪命令或程序的系统调用

  $ sudo ausearch --raw -p <PID> | aureport -i --file
  # 查看指定命令或程序使用的文件（包含系统调用）

  $ sudo ausearch --raw | aureport -i --file
  # 查看审计规则涉及的文件
  ```

- ❗autrace 命令将删除所有活动的审计规则，或者要求在运行之前删除所有活动规则，可能导致丢失来自现有规则会记录的其他进程的事件，若审计规则被锁定，autrace 命令将不能正常工作。

-----

## 编写自定义审计规则

### 添加规则

- auditctl 命令用于添加审计规则。

```bashag-0-1jf9v3dh4ag-1-1jf9v3dh4
$ auditctl --help
# 查看auditctl命令使用概要(此处有图片)
```

- ✅ 审计规则的顺序原则：先入为主
- 若使用不同的键（key）配置了两条规则，一条规则将所用于记录对/etc目录的所有访问，另一条用于记录对/etc/sysconfig目录的所有访问，按照该顺序，任何对/etc/sysconfig目录的访问都不会触发第二条规则，因为/etc目录及其子目录中的每个操作都会匹配第一个规则，永远不会到达第二个规则。
- 若颠倒两个规则的顺序并访问/etc/sysconfig，那么记录对/etc/sysconfig的访问的规则将匹配该事件，访问/etc/sysconfig所需的/etc目录上的操作可能会作为独立的事件与第二个规则匹配，在此情况下，对/etc本身的操作超出了/etc/sysconfig规则的范围，不会与它匹配，而与/etc目录的规则匹配。
- 审计规则的三种类型：
  - 文件系统规则（监视）：审计对文件和目录的访问
  - 系统调用规则：审计系统调用的执行，该调用由与内核通信的进程发出，以访问系统资源。
- 设置文件系统规则（监视）:
  - 可添加的最简单的一种规则是监视（watch）。
  - 可在文件和目录上设置监视，并且可以将这些监视配置为发生特定类型的访问（读取、写入、属性更改和执行）时触发。
  - open() 系统调用会触发监视，单独的 read() 或 write() 系统调用不触发监视。
  
  ```bash
  $ sudo auditctl -w <file_or_dir> -p <permissions> -k <key>
  # 设置基础的文件系统自定义审计规则ag-0-1jf9v3dh4ag-1-1jf9v3dh4w 选项：为要监视的文件或目录的名称，若路径是目录，则规则将递归匹配该目录中的所有内容和子目录，但不包括作为挂载点的子目录，该规则不会跨文件系统。
  ```
  
  - -w 选项：为要监视的文件或目录的名称，若路径是目录，则规则将递归匹配该目录中的所有内容和子目录，但不包括作为挂载点的子目录，该规则不会跨文件系统。
  - -p 选项：按照权限类型获取监控的访问的列表。
    - r：读取访问
    - w：写入访问
    - x：执行访问
    - a：更改属性

  - -k 选项：设置对审计记录的键（key），可通过特定的ausearch查询来查找。
  - 示例：

    ```bash
    $ sudo auditctl -w /etc/passwd -p wa -k user-edit
    # 对/etc/passwd文件设置写入与更改属性权限的审计规则，
      ausearch查找键为user-edit。
    ```

    ```bash
    $ sudo auditctl -w /etc/sysconfig -p rwa -k sysconfig-access
    # 对/etc/sysconfig/目录中的文件及子目录递归设置读取、
      写入与更改属性权限的审计规则，ausearch查找键为sysconfig-access。
    ```

    ❗ 文件系统的审计规则不能跨文件系统，若对 / 目录设置审计规则，而 /tmp 目录挂载于另一文件系统，对 / 目录的审计规则不对 /tmp 目录生效。

- 设置系统调用规则:
  - 系统调用审计规则定义如下所示：(此处有图片)
  - -a 选项：审计规则的列表（list）设置：
    - exit：最常用，在所有系统调用退出时评估。
    - user：评估源自于用户空间内的事件
    - task：很少使用，仅在fork(2)和clone(2)系统调用期间检查。
    - exclude：偶尔使用，用于将事件从日志记录中完全过滤掉。
  - 若将 -a 替换为 -A，Audit会将规则插入到所指定的列表的顶部，而不插入到底部。
  - acition 可以是 always（始终记录某一事件）或 never（永不记录这一事件）。
  - ✅ 在大多数情形中，<list>,<action> 为 exit,always，即在系统调用退出时始终记录相应的事件。
  - -F 选项：允许根据审计记录字段指定过滤器，如 auid、arch 和 exit。
  - -C 选项：允许比较字段，如，比较调用进程的 Audit UID 和被访问的文件的obj_uid（所有者）。
  - -S 选项：根据系统调用进行过滤。
  - ❗ 在64位系统中，应通过按名称指定系统调用来编写规则两次，一次使用 -F arch=b32 选项，另一次使用 -F arch=b64 选项。
  - 由于每个系统调用不会在32位和64位架构中转换为相同名称的相同编号，而64位系统确实提供应用可能使用的32位系统调用。
  - 示例：
    - 针对原始Audit用户ID等于或大于500的所有用户，审计rename和renameat系统调用的32位版本，若进程位于mysqld_t SELinux域下，不触发该审计规则，并且添加rename键到日志。(此处有图片)
    - 递归审计root用户对/home目录下不由现在作为root工作的原始用户所拥有的文件或目录的每一文件系统访问。

      ```bash
      $ sudo auditctl -a exit,always -F dir=/home/ -F uid=0 -C auid!=obj_uid
      ```

    - 当Audit启动时，它会将 Audit UID 4294967295 分配给任何现有的进程。
    - 若要从规则中排除这些进程，可将 -F auid!=4294967295 选项添加到规则中。
    - ❗ 针对系统中发出的每一系统调用检查系统调用规则，将降低系统性能。
    - 因此，应该尽力使系统调用规则列表保持简短和通用。
- 设置控制规则
  - 该规则用于修改Linux Audit的内核配置。
  - 它们往往简短，通常设置在规则列表的顶部，但 -e 2 规则除外，该规则用于防止进一步更改审计规则集。
  - 示例：

    ```bash
    $ sudo auditctl -D
    # 删除所有审计规则
    ```

    ```bash
    $ sudo auditctl -W /bin
    # 删除监控/bin的文件系统规则，路径/bin必须与规则完全匹配。
    ```

### 删除规则

- -W 选项：删除文件系统监视规则
- -d <list>,<action> 选项：删除通过-a或-A添加的审计规则
- -D 选项：删除所有审计规则

### 检查规则

```bash
$ sudo auditctl -l
# 查看当前的审计规则集￼(此处有图片)
```

```bash
$ sudo auditctl -s
# 检查Audit的当前状态，显示有关用于存储审计消息的内部缓冲区大小的信息（当存储无法跟上时）以及处于活跃状态的任何速率限制。
```

### 使规则不可变

```bash
$ sudo auditctl -e 0
# 禁用审计，但不关闭auditd守护进程。
```

```bash
$ sudo auditctl -e 1
# 重新启用审计
```

```bash
$ sudo auditctl -e 2
# 将审计规则配置为不可变，将无法再添加、删除或更改规则，也不能停止Audit子系统。
```

- 若要能够重新进行更改，需重新引导整个系统。
- ❗ 若在 /etc/audit/rules.d/audit.rules 或以 .rules 结尾的规则文件中设置 -e 2 规则，它必须是最后加载的规则，因为在设置了这一规则后，之后尝试设置规则都不被允许。

### 持久规则

- 审计规则可通过auditctl命令行设置，也可通过规则文件使其持久化。
- 可将审计规则写入 /etc/audit/rules.d/audit.rules 或 .rules 结尾的规则文件中使审计规则持久化，每个审计规则前可使用 "#" 进行注释。

```bash
$ sudo auditctl -R <filename>：
# 重新读取审计规则文件使其生效ag-0-1jf9v3dh4ag-1-1jf9v3dh4
```

- 示例：(此处有图片)

```bash
$ man 7 audit.rules
# 内核审计子系统规则说明ag-0-1jf9v3dh4ag-1-1jf9v3dh4
```

-----

## 启用预打包的审计规则集

### 预打包的审计规则集

- 若需实施安全策略，可查找随audit软件包预安装的审计规则集。
- 这些规则位于 /usr/share/doc/audit-*/rules/ 目录中，作为后缀名为 .rules 的文件提供。
- /usr/share/doc/audit-*/rules/README-rules 文件提供了基础背景以及定义如何使用规则，是否还应加载其他规则文件等。
- 加载规则时顺序很重要，每个规则文件的名称都以数字开头，以确保以正确的顺序加载它们。
- 提供的规则文件示例：
  - 30-nispom.rules：旨在满足《国家工业安全计划操作手册》的“信息系统安全”章节的要求
  - 30-pci-dss-v31.rules：旨在满足支付卡行业数据安全标准（PCIDSS）v3.1规定的要求
  - 30-stig.rules：旨在满足美国国防部安全技术实施指南（STIG）的要求
- 启用预打包规则集
  - 拷贝一个或多个 .rules 文件至 /etc/audit/rules.d 目录中以使用其中一个预先打包的规则集。

    ```bash
    $ sudo augenrules --load
    # 重新加载审计规则
    ```
  
  - 这些示例文件不保证完全与所写的规则相符，而是提供配置环境的起点。
  - 复制其中一个默认规则集后，需要查看其文件，并按照说明针对特定环境启用或禁用某些规则。

### 全终端按键记录

- 某些审计策略要求记录用户进行的每次按键。
- Audit与 pam_tty_audit PAM模块联合提供该功能。
- 每一次按键都记录在审计日志 /var/log/audit/audit.log 中。
- 将 pam_tty_audit.so 模块添加到 /etc/pam.d/system-auth 和 /etc/pam.d/password-auth 文件中以启用按键记录。
- 因此系统中启用终端功能的所有守护进程将记录其按键，除非在PAM配置中明确禁用。
- ❗ pam_tty_audit.so 模块仅实施 session 功能，将模块添加到PAM中的任何其他部分，都会阻止任何用户登录。
- pam_tty_audit.so 模块采用 enable 或 disable 选项。
- 这两个选项都将逗号分隔的用户名模式列表作为参数，其作用分别为启用和禁用。
- 配置 /etc/pam.d/system-auth 文件：(此处有图片)
- 配置 /etc/pam.d/password-auth 文件：(此处有图片)
- 将上述文件配置完成后，切换用户以验证全终端按键记录，如下所示：(此处有图片)
- ❗ 若 enable= 模式和 disable= 模式都与某一用户匹配，则适用命令行中的最后一个。

```bash
ag-0-1jf9v3dh4ag-1-1jf9v3dh4$ sudo aureport --tty
# 查看pam_tty_audit模块中指定用户的终端按键记录ag-0-1jf9v3dh4ag-1-1jf9v3dh4
```

- aureport --tty 命令无法确定相应的按键记录用户，可配合ausearch命令来确定。(此处有图片)
- 按键记录可能需要系统上的大量存储空间！
- ✅ 推荐做法：使用按键记录可能存在某些法律限制或要求，在实施按键记录之前，应与法律顾问讨论该问题。
