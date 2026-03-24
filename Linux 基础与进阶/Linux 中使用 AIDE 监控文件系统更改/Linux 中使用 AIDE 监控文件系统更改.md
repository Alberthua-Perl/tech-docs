# Linux 中使用 AIDE 监控文件系统更改

## 文档目录

- [Linux 中使用 AIDE 监控文件系统更改](#linux-中使用-aide-监控文件系统更改)
  - [文档目录](#文档目录)
  - [1. 使用 AIDE 检测文件系统更改](#1-使用-aide-检测文件系统更改)
    - [1.1 使用 AIDE 分析文件系统更改](#11-使用-aide-分析文件系统更改)
      - [1.1.1 安装 AIDE](#111-安装-aide)
      - [1.1.2 配置 AIDE](#112-配置-aide)
      - [1.1.3 管理 AIDE 数据库](#113-管理-aide-数据库)
  - [2. 使用 AIDE 调查文件系统更改](#2-使用-aide-调查文件系统更改)
    - [2.1 结合使用 AIDE 和 Audit](#21-结合使用-aide-和-audit)
    - [2.2 配置 AIDE 和 Audit](#22-配置-aide-和-audit)

-----

## 1. 使用 AIDE 检测文件系统更改

### 1.1 使用 AIDE 分析文件系统更改

#### 1.1.1 安装 AIDE

- 在正在运作的服务器上，常常要在其文件系统中添加、删除和修改文件。
- 但对某些文件（如可执行程序和配置文件）的意外更改可能表示未经授权的修改或其他安全问题。
- 因此，务必要监控这些文件的内容、权限或其他特征的变化。
- RHEL 提供高级入侵检测环境（advanced intrusion detection environment，AIDE）这一用户空间实用程序。
- AIDE 监控对文件的各种更改，包括权限或所有权更改、时间戳更改（修改或访问时间戳）或内容更改。
- 安装 AIDE
  - 默认情况下，通常不安装 aide 软件包。
  - 需要先安装和配置此软件包，并且构建其初始数据库，然后才能检查文件系统。
  
  ```bash
  $ sudo yum install -y aide
  # 安装 aide 软件包
  ```

#### 1.1.2 配置 AIDE

| 文件路径 | 功能 |
| ----- | ----- |
| /etc/aide.conf | AIDE 主配置文件 |
| /var/lib/aide/aide.db.gz | 旧 AIDE 数据库文件（一般作为 AIDE 检查基线） |
| /var/lib/aide/aide.db.new.gz | 新 AIDE 数据库文件（执行 aide --update 命令后生成） |
| /var/log/aide/aide.log | AIDE 检查执行日志 |

- AIDE 配置文件控制 AIDE 监控哪些文件的更改，以及监控每个文件的哪些特征。
- 默认情况下，AIDE 仅监控 /etc 目录中大多数文件的权限更改，但更密切地监控特定的文件。
- 作为安全管理员，可能希望精确调整 AIDE 监控主机文件系统中不同部分的内容。
- AIDE 附带了一个配置合理的默认 /etc/aide.conf 文件，可使用其构建初始化的数据库。
- ✅ 若希望或需要精确调整 AIDE 监控的内容，则应在构建或更新 AIDE 数据库之前修改该文件。
- /etc/aide.conf 文件中的每行为指令（directive）。
- 该文件包含三种类型的行：**配置行**、**选择行**、**宏行**
- 注释以 "#" 开头，无任何作用。
- **配置行（configuration line）：**
  - 配置行调整 AIDE 的配置参数。
  - 配置行的作用：全局调整 AIDE 的功能行为、设置组定义（group definition）
  - 选择行使用组定义来指定 AIDE 在检测文件系统更改时应监控的文件的特征。
  - 配置行语法：parameter = value
  - 配置参数（parameter）如下所示：
    - `database`:
      - AIDE 在运行检查时从中读取数据库的位置（旧 AIDE 数据库文件），通常是本地文件。
    - `database_out`:
      - AIDE 在更新时写入数据库的位置（新 AIDE 数据库文件），通常也是本地文件，并且必须与输入数据库不同。
    - `gzip_dbout`:
      - 若此参数设置为 yes，则 AIDE 将创建一个新数据库并使用 gzip 命令对其进行压缩。
      - 配置行还可以创建组定义，组定义与选择行一起使用，以设置要监控的文件的特征。
      - /etc/aide.conf 文件中组定义示例：定义 PERMS 的组，若选择行使用该组定义，将监控该行选择的文件中权限（p）、用户（u）、组（g）、访问控制列表（acl）、SELinux 上下文（selinux）和文件系统扩展属性（xattr）的更改。

      ```bash
      # Access control only
      PERMS = p+u+g+acl+selinux+xattrs
      ```

- **选择行（selection line）：**
  - 选择行指定 AIDE 监控的文件和目录，以及 AIDE 将要监视的更改。
  - 选择行可以是常规（regular）、等于（equals）或否定（negative）。
  - 常规选择行：文件或目录的绝对路径匹配的正则表达式 + 组定义
    - 与该正则表达式匹配的文件和目录将添加到 AIDE 数据库中，并且执行行的组定义指定的检查。
    - 若正则表达式（regex）为 /etc，则正则表达式将递归匹配 /etc 目录中的所有文件和目录。
  - **<font color=orange>等于选择行：等号（=）开头 + 正则表达式 + 组定义</font>**
    - 通过考量该行的组定义提及的所有检查，AIDE 记录与正则表达式匹配的文件。
    - ❗若正则表达式以正斜杠（/）字符结尾，则等于选择行仅匹配目录的子项，子目录的子项不会递归匹配。
  - **<font color=orange>否定选择行：感叹号（!）开头 + 正则表达式</font>**
    - AIDE 不监控与否定选择行匹配的文件或目录。

  ```bash
  /etc    PERMS         # 常规选择行：匹配 /etc 目录及递归匹配该目录中的所有子文件与子目录，使用 PERMS 组定义检查。
  =/testdir    PERMS    # 等于选择行：精确匹配 /testdir 目录，不匹配其中的子文件与子目录，使用 PERMS 组定义检查。
  !/etc/mtab            # 否定选择行：AIDE 不监控该文件
  ```

- **宏行（macro line）：**
  - 宏行设置或清除变量，这些变量可用于引用整个 AIDE 配置文件中多次出现的冗长 URL 或文件系统路径。

  ```bash
  @@define DBDIR /var/lib/aide
  @@define LOGDIR /var/log/aide
  ```

#### 1.1.3 管理 AIDE 数据库

- 初始化 AIDE 数据库
  - 安装 AIDE 后，确保 AIDE 知道文件系统的当前状态至关重要。
  - AIDE 使用文件系统的已知状态，作为检测和报告文件系统更改的参考点。
  - 理想情况下，可以在安装后尽快构建 AIDE 数据库。
  - ✅ AIDE 通过将文件的当前状态与存储在 AIDE 数据库中的预期状态信息进行比较来进行操作。
  - 若没有基准数据库，AIDE 就没有办法确定系统预期状态。
  
  ```bash
  $ sudo aide --init
  Start timestamp: 2026-03-24 05:38:30 -0400 (AIDE 0.16)
  AIDE initialized database at /var/lib/aide/aide.db.new.gz

  Number of entries:      53290

  ---------------------------------------------------
  The attributes of the (uncompressed) database(s):
  ---------------------------------------------------

  /var/lib/aide/aide.db.new.gz
    MD5      : zsvBj7QcnILtB+K44c/LAA==
    SHA1     : fQPZdee95d0PxLEbqSHX/NRjexg=
    RMD160   : 7OV2VMwHpNGiG1tDet1l5ru7/t8=
    TIGER    : NxEFUq/3mq++ECWh6a9TTF05RtAA/3xB
    SHA256   : dzUavCy1Pqkd4JzjtCrxZ1FE/ikV+k9M
               I0yLXVhA/QY=
    SHA512   : AnvXmQFPq+gOQ7UsUFLx4s+zr/TRHert
               D9tBHvtibxukCEHAiN0YBYybQdx8TDXy
               BmDxYkjQpx0YBeIxzOIoTQ==


  End timestamp: 2026-03-24 05:38:59 -0400 (run time: 0m 29s)
  # 初始化 AIDE 数据库

  $ sudo mv /var/lib/aide/aide.db.new.gz /var/lib/aide/aide.db.gz
  # 初始化数据库后，需重命名此数据库名称用于之后的比对，否则无法比对。
  ```

- 通过 AIDE 验证完整性
  - 在 AIDE 知道当前文件系统状态后，可以通过与已知状态进行比较来检测文件系统更改。
  
  ```bash
  $ sudo aide --check
  Start timestamp: 2026-03-24 06:02:23 -0400 (AIDE 0.16)
  AIDE found differences between database and filesystem!!

  Summary:
    Total number of entries:      53271
    Added entries:                1
    Removed entries:              0
    Changed entries:              0

  ---------------------------------------------------
  Added entries:
  ---------------------------------------------------

  f++++++++++++++++: /testdir/testfile

  ---------------------------------------------------
  The attributes of the (uncompressed) database(s):
  ---------------------------------------------------

  /var/lib/aide/aide.db.gz
    MD5      : d0Lh/0+2cjhUnSY6p4x6XA==
    SHA1     : N8BbXTsVuzQ/EYga2JgSTGbENC8=
    RMD160   : Oe4M537Kndiydy90CgqESGPDGV4=
    TIGER    : OJ024S29aYlp+fXcgROA061DG33wLSIv
    SHA256   : NLSsSpY3uHLIRrM0ByuuZqrn78hEqfAt
               v4hERnNqCS4=
    SHA512   : LJ2+uSiYIRHO+rgMo3UTurqxGgHz3ZN8
               EarMewOvDnxDcn276aLd73sqWhV5a1wc
               ZYjItFRGvSKgRBbSz0wdxA==


  End timestamp: 2026-03-24 06:02:47 -0400 (run time: 0m 24s)
  # 将现有系统文件的状态与已保存的数据库进行比较
  ```

  - 默认情况下，报告将打印到标准输出和 /var/log/aide/aide.log 文件中。
  - 生产环境中，应定期执行 AIDE 检查，可使用 `cron job`、`systemd timer unit` 或其他方式来自动运行 `AIDE` 检查。

    ```bash
    $ sudo touch /etc/cron.d/aide
    $ sudo cat > /etc/crod.d/aide <<EOF
    * * */7 * *    root    /usr/sbin/aide --check >> /var/log/aide.log
    EOF
    ```

- 更新 AIDE 数据库
  - ✅ 推荐做法：
    - 在对系统进行预期更改后，务必要更新 AIDE 数据库。
    - 如，软件包更新或授权的配置文件调整可能会更改受监控文件的时间戳权限或校验和（checksum）。
    - 为避免 AIDE 报告误报，需要更新数据库以反映这些授权的更改。
    - 确认 AIDE 报告的所有剩余更改都已获得授权后，运行以下命令来更新 AIDE 数据库。
  
  ```bash
  $ sudo aide --update
  Start timestamp: 2026-03-24 06:12:08 -0400 (AIDE 0.16)
  AIDE found differences between database and filesystem!!
  New AIDE database written to /var/lib/aide/aide.db.new.gz
  ...
  # 更新 AIDE 数据库

  $ sudo ls -lh /var/lib/aide/
  total 6.0M
  -rw-------. 1 root root 3.0M Mar 24 05:58 aide.db.gz
  -rw-------. 1 root root 3.0M Mar 24 06:12 aide.db.new.gz    # 更新后的数据库作为新的检查基线
  ```

  - ❗更新 AIDE 数据库后，不要忘将其替换旧的 AIDE 数据库，以使用新的数据库作为检查基线（baseline），除非仍使用旧数据库作为检查基线。
  - 初始化或更新 AIDE 数据库时，将增加系统负载！
  
  ```bash
  $ man 5 aide.conf
  # 查看 AIDE 配置文件详细说明
  ```

-----

## 2. 使用 AIDE 调查文件系统更改

### 2.1 结合使用 AIDE 和 Audit

- AIDE 可用于检测主机文件系统上的更改，但其局限在于 **无法告知导致变化的原因**。
- 如，可将 AIDE 配置为报告 /etc/passwd 和 /etc/shadow 的更改，但不会生成关于谁在何时使用哪个工具进行更改的详细信息，这是审计规则的用武之地，它们可以帮助获取所发生情况的详细信息。Linux Audit 系统可以监控文件以查找可能导致更改的活动，并记录有关活动时间以及所涉及的进程和用户的许多有用信息。
- Linux Audit 系统可以监控文件以查找可能导致更改的活动，并记录有关活动时间以及所涉及的进程和用户的许多有用信息。
- **AIDE 和 Audit 的组合可有效解决此问题。**
- 通过配置两个系统来监控关键的文件和目录，可使用 AIDE 定期报告对系统进行的意外更改。
- ✅ 然后，可查看自更改之前的上一个 AIDE 报告以来的审计日志，以帮助确定可能负责的活动。

### 2.2 配置 AIDE 和 Audit

- 要使此方法有效，需配置 AIDE 和 Audit 以监控相同的文件。
- 首先确保正确配置 AIDE，编辑 /etc/aide.conf 文件，初始化 AIDE 数据库，设置 cron job 来定期运行 AIDE 检查并将报告发送给系统管理员。
- 示例：
  - 编辑 /etc/aide.conf 文件：

    ```bash
    # Extended content + file type + access.
    CONTENT_EX = sha512+ftype+p+u+g+n+acl+selinux+xattrs    # 组定义
    ...
    /usr    CONTENT_EX    # 常规选择行
    ```

  - 编辑 /etc/audit/rules.d/*.rules 文件添加如下文件系统（监视）规则：

    ```bash
    -w /usr -p wa -k usr-modification
    ```

- 以上示例 AIDE 将检查 /usr 目录的子项，不递归检查子目录的子项，设置文件系统（监视）规则递归监控 /usr 目录的写入或更改权限。
- ✅ 推荐做法：若不需要为审计目的收集信息，则可减少相应审计内容，这样可减少日志中的 “噪音”，减小日志大小，并提高性能，具体取决于规则的结构，如上述事例中可避免使用 -p rwxa。
