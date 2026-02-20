# 第六章 使用 AUDIT 记录系统事件

-----

## 配置 Audit 以记录系统事件

### Linux Audit 系统

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

### 使用 Audit 审计系统

- **auditd 是 Linux 审计子系统的用户空间组件（user-space component）。**

- 在守护进程启动时，启动 auditd 的 systemd 单元文件，通常会加载所有持久性审计规则。

- 当 Audit 规则加载至内核后，内核使用规则报告相关的事件。

- 内核将事件相关的信息发送给 auditd 守护进程。

- auditd 守护进程的角色：收集由内核报告的审计事件信息，并将其保存至日志文件中。

- ✅ 审计流程：audit_rules --> kernel --> audit_events --> auditd --> log_file

- auditd 守护进程运行时，事件信息将被写入 **<span style="color:blue">/var/log/audit/audit.log</span>**，该守护进程不运行时，rsyslog 接收内核的审计信息。

- 若没有任何额外的配置，仅有限数量的消息通过审计系统，主要是身份认证和授权消息（用户正在登录、正在使用sudo等）和SELinux消息。

- 使用 auditctl 命令添加控制审计系统的审计规则、监视文件或有关任何系统调用的记录信息。

### 配置 auditd

- /etc/audit/auditd.conf：auditd 守护进程主配置文件

- /etc/audit/audit.rules：auditd 加载的审计规则

- 不要编辑以上文件，auditd在启动时将从 /etc/audit/audit.d/ 中生成该文件。

- /etc/audit/rules.d：手动配置审计规则的目录

- 该目录中的所有审计规则在 auditd 启动时被合并入 /etc/audit/audit.rules中，并加载至内核中。

- /etc/audit/rules.d/audit.rules：手动管理的审计规则文件

```bash
$ sudo systemctl [start|stop|restart|enable|disable] auditd.service
# 启动、停止、重启、开机自启动、开机不自启动auditd服务
```

- auditd启动后，其systemd单元文件自动从/etc/audit/rules.d中重建审计规则集，并将其加载至内核中。

- 当auditd重新加载时，它会尝试使用augenrules命令重建规则文件并加载新的规则。

- ❗若加载了使当前规则不可变的控制规则，可能会失败，需重新引导系统以更改加载的规则集。

- 调整auditd设置以管理存储：
  
  - 根据加载的审计规则，审计日志可能会非常快速地变大。
  
  - ✅ 推荐做法：/var/log/audit目录最好是带有自有文件系统的挂载点，这样就不会有其他进程因存储空间和性能调优原因而与审计日志竞争。
  
  - 若审计系统由于缺少存储空间而无法将事件记录到磁盘，则系统必须立即停止或降级到单用户模式，这是为了确保不会在没有记录的情况下发生任何事件。
  
  - auditd守护进程具有相关的机制，可以在即将发生这种情况时发出警告，并在存储变低时采取各种操作。
  
  - /etc/audit/auditd.conf 配置文件参数（此处有图片）
    
    ```bash
    $ man 5 auditd.conf
    # 查看auditd服务配置文件参数的详细信息
    ```

- 调整auditd配置以调优性能：
  
  调整以下/etc/audit/auditd.conf参数以提高auditd服务性能：
  
  - flush = INCREMENTAL_ASYNC：达到 freq 指定的写入次数后将记录异步清理到存储，以提高系统性能。
  
  - 但由于系统崩溃时审计消息可能丢失，此时需将此参数设置为 DATA 或 SYNC。
  
  - 以相对较低的 freq 设置使用 INCREMENTAL_ASYNC，可确保记录快速同步到文件系统，同时保持良好的系统性能。
  
  - freq = 50：在每50条记录后清空Audit日志（假设设置 flush=INCREMENTAL_ASYNC）。
  
  - 若 flush 设置为 DATA 或 SYNC，则忽略此设置，并且每次写入存储都是同步的。
  
  - ❗ 只有设置 flush = INCREMENTAL_ASYNC 时，freq参数才起作用！

### 使用auditd进行远程日志记录

- 可通过以下两种方式将审计消息发送到远程系统：
  
  - 可将审计消息发送到本地 rsyslog 服务并将其转发至rsyslog配置的远程服务器。
  
  - 可将系统配置为将审计消息发送到远程auditd服务。

- 配置客户端：
  
  - 使用以上两种方式均需对系统的/etc/audit/auditd.conf文件进行以下更改：
    
    - log_format = ENRICHED：在传输每个事件之前解析UID、GID、系统调用数量、架构和套接字地址信息等，保证该日志信息在远程系统上有意义，远程系统上可能具有以上映射的不同值。
    
    - name_format = HOSTNAME：在每条消息中包含主机名，区别审计消息所属的主机。
    
    - 针对于第一种方式：
      
      - 设置客户端 /etc/audisp/plugins.d/syslog.conf 文件中 active = yes 可将消息发送到rsyslog。
      
      - 还需配置本地rsyslog服务，将日志消息发送到/etc/rsyslog.conf文件中定义的远程日志服务器。
      
      - 补充示例：使用loganalyzer解析远程rsyslog服务器（MySQL作为日志数据存储后端）
        
        收集的auditd审计日志信息（此处有图片）
    
    - 针对于第二种方式：
      
      - 需在客户端安装 audispd-plugins 软件包。
      
      - 设置客户端 /etc/audisp/plugins.d/au-remote.conf 文件中的 active = yes。
      
      - 设置客户端 /etc/audisp/audisp-remote.conf 文件中的 remote_server 指令为远程auditd服务器的IP地址或主机名，若远程服务器未监听默认 60/TCP 端口的话，需更新 port 指令。

- 配置服务端：
  
  - 针对于第一种方式：
    
    - 配置rsyslog服务/etc/rsyslog.conf文件，配置其为服务端并监听 514/UDP 或 514/TCP 端口，接收来自客户端的审计消息。
  
  - 针对于第二种方式：
    
    - 设置/etc/audit/auditd.conf文件中的 tcp_listen_port = 60 指令（60/TCP）用以监听远程客户端的审计消息。
    
    - 设置firewalld以放行 60/TCP 端口。
    
    - ❗ 必须以重启操作系统的方式来重启auditd服务使其配置生效。（此处有图片）

- 常用离线帮助文档：
  
  ```bash
  $ man 5 auditd.conf
  # 查看auditd服务配置文件的详细使用信息
  $ man 5 audisp-remote.conf
  # 查看audisp-remote配置文件信息
  $ man 7 audit.rules
  # 查看内核审计系统的规则集合信息
  ```

-----

## 检查Audit日志

### 解读Audit消息

- /var/log/audit/audit.log中记录的Audit事件在紧凑的格式中纳入许多的信息。

- 一个事件实际上可能会记录多个不同类型的审计记录到日志中，作为单独的消息。

- 这些记录可能各自包含多个字段，记录事件的相关信息。

- 示例：/var/log/audit/audit.log文件中未经处理的单个审计事件（audit event）相关的多条审计记录（audit record）(此处有图片)

### 搜索事件（event）

- 审计系统附带 ausearch 命令，该命令用于搜索审计日志。

- 可使用 ausearch 搜索和过滤各种类型的事件。

- 可将数值转换为更易读的值，如用户名或系统调用名，以解读这些事件。

- 常用的ausearch命令选项 (此处有图片)

```bash
$ sudo ausearch -i -a <event_id>
# 查看指定审计事件的全部审计记录
```

```bash
$ man ausearch
# 该命令用于查询auditd守护进程日志(此处有图片)
```

### Audit消息报告

- aureport 命令可用于获取审计消息的快速概览和特定类型事件的详细报告。

- 常用选项：$ man aureport (此处有图片)

- ❗ aulast 命令与 aulastlog 命令和 last 命令与 lastlog 命令极其类似，但前者不解析 /var/log/wtmp 与 /var/log/btmp 文件。

### 追踪程序

- autrace 命令可用于调查进程执行的系统调用，该命令与 strace 命令非常类似。

- 运行autrace命令将删除所有自定义审计规则，并将其替换为专门用于追踪指定程序的规则。

- 执行完成后，autrace命令将清除这些规则，然后提供示例ausearch命令来调查这些事件。

- 这对于排除故障或调查感兴趣的程序非常有用。

- 示例：使用autrace调查ip命令的系统调用 (此处有图片)

```bash
$ sudo autrace <command>
# 追踪命令或程序的系统调用
```

```bash
$ sudo ausearch --raw -p <pid> | aureport -i --file
# 查看指定命令或程序使用的文件（包含系统调用）
```

```bash
$ sudo ausearch --raw | aureport -i --file
# 查看审计规则涉及的文件
```

- ❗ autrace命令将删除所有活动的审计规则，或者要求在运行之前删除所有活动规则，可能导致丢失来自现有规则会记录的其他进程的事件，若审计规则被锁定，autrace命令将不能正常工作。

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


