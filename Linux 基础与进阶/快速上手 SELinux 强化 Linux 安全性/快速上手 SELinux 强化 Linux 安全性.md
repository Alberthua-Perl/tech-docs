# 🧱 快速上手 SELinux 强化 Linux 安全性

## 文档目录

- [🧱 快速上手 SELinux 强化 Linux 安全性](#-快速上手-selinux-强化-linux-安全性)
  - [文档目录](#文档目录)
  - [1. 从禁用状态启用 SELinux](#1-从禁用状态启用-selinux)
    - [1.1 复习 SELinux 基本概念](#11-复习-selinux-基本概念)
    - [1.2 配置 SELinux 模式](#12-配置-selinux-模式)
    - [1.3 从禁用模式启用 SELinux](#13-从禁用模式启用-selinux)
  - [2. 使用受限用户控制访问](#2-使用受限用户控制访问)
    - [2.1 定义 SELinux 用户（SELinux User）](#21-定义-selinux-用户selinux-user)
    - [限制（confine）用户账户](#限制confine用户账户)

## 1. 从禁用状态启用 SELinux

### 1.1 复习 SELinux 基本概念

- SELinux 核心功能：SELinux（Security-Enhanced Linux，安全增强的 Linux）通过强制访问控制（MAC）机制，结合类型强制（TE）、基于角色的访问控制（RBAC）和多级安全（MLS/MCS），对 Linux 用户、进程及系统资源实施细粒度的安全隔离与访问限制。
- SELinux 通过以下核心机制实现强制访问控制：
  - 身份标识：SELinux 用户（User）与 Linux 用户映射
  - 权限边界：角色（Role）约束用户可访问的域
  - 类型强制：安全上下文（Context）中的类型（Type）决定资源访问规则
  - 层级隔离：MLS/MCS 实现数据机密性和类别隔离
- 最终效果：限制进程只能访问其被明确允许的资源，防止权限提升和横向移动，即使账户被攻破也能最小化损害。

> 从层级关系理解 SELinux 实现机制：
> SELinux 是一种强制访问控制（MAC）安全机制，通过以下层次化策略 <br>
> 限制 Linux 主体（用户、进程）对客体（文件、端口、资源）的访问：
>
> 1. 身份层：Linux 用户 → 映射到 SELinux User（标识安全身份）
> 2. 角色层：SELinux User → 授权访问特定 Role（权限边界）
> 3. 类型层：Role → 允许进入特定 Domain（Type），Domain 通过 TE 规则决定可访问的 Resource Type（核心强制机制）
> 4. 层级层：MLS/MCS 提供额外的敏感度/类别隔离（可选高级特性）
>
> **<font color=orange>最终安全上下文格式：`user:role:type:level`</font>**

- SELinux 是额外的系统安全层
- SELinux 可以充分限制 **<font color=orange>程序（主要是系统服务）</font>** 的权限，仅授予程序成功完成工作所需的权限。
- SELinux 有一组安全规则确定哪个进程可以访问哪些文件、目录和端口。
- 若没有明确的访问规则，SELinux 会拒绝操作。
- ✅ **<font color=orange>每个文件、进程和端口均有一个特定的安全标签（security label），称为 SELinux 上下文（context）。</font>**
- SELinux 规则使用上下文控制访问。
- ⭐ 文件和目录：SELinux 会将上下文存储为文件系统的扩展属性（xattr）。
- 🌈 进程和端口：内核会将上下文保留在内存中，进程的上下文类型也称为域（domain）。
- RHEL 中 SELinux 的大部分规则基于第三个元素，即 **上下文类型（context type）**。
- 更改文件和目录的 SELinux 上下文
- 定义 SELinux 默认文件上下文规则
- 标记 SELinux 端口
- 使用 SELinux 布尔值：
  - SELinux 布尔值是更改 SELinux 策略行为的开关。
  - 可以启用或禁用 SELinux 的访问规则集。
  - 安全管理员可以使用 SELinux 布尔值来有选择地调整策略。
- 访问文档：
  - 每个 SELinux 命令都有一个 man page
  - 每个 semanage 子命令也都有自己的 man page
  - 域（进程的上下文类型）也具有 man page
  
  ```bash
  $ sudo dnf install -y policycoreutils-devel
  # 安装 SELinux 核心策略开发工具软件包

  $ sudo sepolicy manpage -a -p /path/to/dir
  # 指定路径为所有域构建 man page
  # -a 选项：为所有域构建 man page
  # -p 选项：指定路径构建 man page（标准的man page存放路径为 /usr/share/man/man8/），默认构建于 /tmp 目录中。

  $ sudo sepolicy manpage -a -p /opt/selinux-man    # 指定目录创建所有域 man page
  $ man --ascii /opt/selinux-man/sssd_selinux.8     # 解析 ASCII 格式显示 man page

  $ sudo sepolicy manpage -d <domain>
  # 为指定的域构建 man page

  $ sudo sepolicy manpage -d sshd_t -p /opt
  # 在 /opt 目录中为 sshd_t 域构建 man page
  ```

### 1.2 配置 SELinux 模式

- 若将现有系统直接从 disabled 模式切换到 enforcing 模式，有可能会发现访问违规和其他 SELinux 问题。
- 通常而言，最好是安装时将系统配置为 permissive 模式或 enforcing 模式。
- **系统由 permissive 模式转换至 enforcing 模式要比 disabled 模式转换更加容易。**

### 1.3 从禁用模式启用 SELinux

- 将 SELinux 从 disabled 模式切换至 enforcing 模式下，需遵循如下步骤，以免 SELinux 阻止应用。
- 将系统切换到 permissive 模式。
- 在此模式下，SELinux 不会强制执行该策略，但至少会在 /var/log/audit/audit.log 文件中报告拒绝记录。
- disabled 模式切换到 permissive 模式，需重启系统并重新标记文件系统。
- 再次查看 audit.log 文件的拒绝记录。
- permissive 模式下 SELinux 不会强制拒绝。
- 解决拒绝，需要分析每个拒绝，掌握可疑应用的行为，然后才能做出适当的响应。
- 重复步骤将系统切换到 permissive 模式和解决拒绝，需要分析每个拒绝，掌握可疑应用的行为，然后才能做出适当的响应，直到所有拒绝均已解决为止。
- 在经过了确认已解决的所有问题后，切换到 enforcing 模式。
- 查看 SELinux 访问冲突审计事件：  
  - SELinux 拒绝访问资源之后，还将记入一个审计事件。  
  - 若 auditd 服务已启动，系统将会把 SELinux 审计事件记入 /var/log/audit/audit.log。  
  - 否则，系统将 SELinux 消息发送至 /var/log/messages 文件。  
  - 示例：创建自定义的 Apache httpd 根目录 /web/data，配置基于域名的虚拟主机后访问该站点，若不设置根目录的 SELinux 类型上下文，将拒绝访问。
  
    ```bash
    $ sudo ausearch -i -m AVC -ts boot
    # 查看自启动后相关的 SELinux 审计记录

    $ sudo ausearch -i -m AVC,USER_AVC,SELINUX_ERR,USER_SELINUX_ERR -ts recent
    # 查看最近 10 分钟的 SELinux 审计记录
    ```

- 使用允许域：
  - 在 permissive 模式下，SELinux 将记录它应该拒绝的访问，但始终都会允许访问。
  - 若将系统切换到 permissive 模式（可能为调试特定进程），此举可能会造成严重的安全后果，因为其他所有服务此时也将无法得到 SELinux 的保护。
  - 有些情况下，可能正在将系统从 disabled 模式切换到 enforcing 模式，而且只剩下一两个 SELinux 域的 AVC 问题还未解决。
  - 多数情况下，可以将系统设置为 enforcing 模式，但应当在确认相关问题之后，再将这一两个域由 permissive 模式切换到 enforcing 模式。
  - 可以将系统设为 enforcing 模式，只将特定域设为 permissive 模式。
  
  ```bash
  $ sudo semanage permissive -a <domain>
  # 设置特定域为 permissive 模式使其成为无限制域，但策略依然将记录访问的违规记录。

  $ sudo semanage permissive -a httpd_t
  # 将 Apache httpd_t 域设置为 permissive 模式

  $ sudo semanage permissive -l
  # 列出当前处于 permissive 模式的域 

  $ sudo semanage permissive -d httpd_t
  # 将 Apache httpd_t 域切换为 enforcing 模式
  ```

- 使用 Ansible 修复 SElinux：
  - 若具有一组相同的系统，运行相同的应用，则可使用 Ansible 配置 SELinux，然后在这些系统上以可重现且相同的方式修复拒绝。
  - 在 Ansible 控制节点上安装 rhel-system-roles 软件包即可部署这些角色。
  - SELinux 角色（role）的功能：
    - 设置 SELinux 的模式：enforcing、permissive、disabled
    - 恢复文件和目录的上下文（等同于 restorecon 命令）
    - 标记 SELinux 端口（等同于 semanage port 命令）
    - 添加新的文件上下文规则（等同于 semanage fcontext 命令）
    - 切换布尔值（等同于 setsebool 命令）
  - SELinux 角色示例：确保 SELinux 处于 enforcing 模式，/virtual 目录的内容设置了类型 httpd_sys_content_t，端口 8080/TCP 的标签已设为 http_port_t，并且已启用了SELinux 的布尔值 httpd_enable_homedirs。

-----

## 2. 使用受限用户控制访问

### 2.1 定义 SELinux 用户（SELinux User）

- SELinux 策略对其 SELinux 用户有所定义，与 Linux 用户截然不同。
- **Linux 用户登录时，被映射到一个 SELinux 用户。**
- 与 Audit UID 一样，SELinux 用户在登录会话期间无法更改。
- 通常，多个 Linux 用户可以映射到同一 SELinux 用户。
- 该策略能对具体某些 SELinux 用户可以执行的操作施加额外的 SELinux 强制限制。

```bash
$ sudo semanage user -l

                Labeling   MLS/       MLS/
SELinux User    Prefix     MCS Level  MCS Range                      SELinux Roles

guest_u         user       s0         s0                             guest_r
root            user       s0         s0-s0:c0.c1023                 staff_r sysadm_r system_r unconfined_r
staff_u         user       s0         s0-s0:c0.c1023                 staff_r sysadm_r system_r unconfined_r
sysadm_u        user       s0         s0-s0:c0.c1023                 sysadm_r
system_u        user       s0         s0-s0:c0.c1023                 system_r unconfined_r
unconfined_u    user       s0         s0-s0:c0.c1023                 system_r unconfined_r
user_u          user       s0         s0                             user_r
xguest_u        user       s0         s0                             xguest_r
# 查看各个 SELinux 用户与对应的一组 SELinux 角色
```

- SELinux 用户（SELinux User）的详细说明如下表所示：

  | SELinux 用户 | 功能作用 | 典型用途 |
  | ----- | ----- | ----- |
  | **`unconfined_u`** | **无限制用户**，拥有几乎完全的访问权限，不受 SELinux 策略的严格限制。<br> 默认情况下，在全新的 RHEL 安装中，Linux 用户将映射到 SELinux 用户 **<font color=orange>unconfined_u</font>**。 | 普通用户、管理员日常操作 |
  | **`sysadm_u`**  | **系统管理员用户**，拥有管理系统的高权限，但受 SELinux 审计和策略约束。 | 系统维护、配置管理（需要进行系统配置但不希望完全无限制的管理员） |
  | **`user_u`** | **受限普通用户**，权限受到严格限制，不能执行特权操作，不能执行 SUID 程序，不能 su 切换到其他用户。 | 普通终端用户、需要最小权限原则的环境 |
  | **`guest_u`** | **来宾用户**，权限极度受限，通常只能访问自己的文件，不能访问网络、不能执行大多数程序。 | 临时账户、公共访问终端 |
  | **`staff_u`** | **工作人员用户**，类似普通用户但可能有一些额外权限。 | 开发人员、技术支持 |
  | **`system_u`** | **系统用户**，用于运行系统服务和守护进程，不用于交互式登录。 | 系统服务（httpd、sshd 等） |
  | **`root`** | **超级用户**，在 SELinux 下也需要映射到特定 SELinux user。 | 通常映射为 `unconfined_u` 或 `sysadm_u` |
  | **`xguest_u`** | **极受限来宾用户**，通常用于 kiosk 模式或浏览器沙箱。 | 公共终端、浏览器沙箱 |
  | **`dbadm_u`** | **数据库管理员用户**，专门管理数据库服务。 | DBA 角色 |
  | **`webadm_u`** | **Web 管理员用户**，专门管理 Web 服务。 | Web 服务器管理 |

- 将 Linux 用户映射到 SELinux 用户：
  - 登录时，SELinux 会将 Linux 用户映射到 SELinux 用户。
  - 因此，Linux 用户会继承相应 SELinux 用户受到的限制。
  
  ```bash
  $ id -Z
  unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023
  # 查看登录的 Linux 用户关联的 SELinux 用户（第一个字段）

  $ sudo semanage login -l
  
  Login Name           SELinux User         MLS/MCS Range        Service

  __default__          unconfined_u         s0-s0:c0.c1023       *
  root                 unconfined_u         s0-s0:c0.c1023       *
  # 查看默认 Linux 用户映射到 SELinux 用户的关系
  # SELinux 将 Linux root 用户映射到 unconfined_u SELinux 用户。
  # __default__ 条目：SELinux 将未明确映射到 SELinux 用户的所有 Linux 用户映射到 unconfined_u SELinux 用户。


  ### 修改默认映射 ###
  $ sudo semanage login -m -s user_u -r s0 __default__
  # 修改系统默认的 __default__ 映射，使得所有未指定明确映射的 Linux 用户，
  # 在登录时默认映射到 user_u SELinux 用户，并分配 s0 级别。
  $ sudo semanage login -l

  Login Name           SELinux User         MLS/MCS Range        Service

  __default__          user_u               s0                   *    # 已修改
  root                 unconfined_u         s0-s0:c0.c1023       *
  ```
  
  > 注意：必须重新使用 SSH 登录的方式验证 Linux 用户与 SELinux 用户的映射关系，不可使用 su 命令进行用户切换。
  
  ```bash
  $ sudo useradd -Z <selinux_user> <linux_user>
  # 创建 Linux 用户时指定 SELinux 用户的映射

  $ sudo semanage login -a -s <selinux_user> <linux_user>
  # 将已有的 Linux 用户映射至指定 SELinux 用户

  $ sudo semanage login -d -s <selinux_user> <linux_user>
  # 删除 Linux 用户与指定 SELinux 用户的映射

  ### 示例 ###
  $ sudo useradd seuser1
  $ sudo echo "redhat" | passwd --stdin seuser1
  $ sudo semanage login -a -s system_u seuser1
  $ sudo semanage login -l

  Login Name           SELinux User         MLS/MCS Range        Service

  __default__          unconfined_u         s0-s0:c0.c1023       *
  root                 unconfined_u         s0-s0:c0.c1023       *
  seuser1              system_u             s0-s0:c0.c1023       *

  $ sudo semanage login -d -s system_u seuser1
  $ sudo semanage login -l

  Login Name           SELinux User         MLS/MCS Range        Service

  __default__          unconfined_u         s0-s0:c0.c1023       *
  root                 unconfined_u         s0-s0:c0.c1023       *
  ```

- 受限 SELinux 用户示例：
  - **`user_u`**:
    - 标准的非管理用户帐户
    - SELinux 限制映射至 user_u 的 Linux 用户通过 su 或 sudo 提权为 root 用户，或防止执行大多数 SUID 程序。
    - (此处有图片)
  - sysadm_u ：
    - 该SELinux用户用于系统管理
    - SELinux允许映射到 sysadm_u 的Linux用户使用 su 和 sudo。
    - 映射到 sysadm_u 的用户默认无法使用SSH登录，需将 ssh_sysadm_login 布尔值（boolean）设为 on。
    - (此处有图片)
    - (此处有图片)
  - staff_u ：
    - 映射到 staff_u 的Linux用户可以使用 sudo，但不能使用 su。
    - staff_u 适用于需要使用 sudo 执行特定任务的普通用户，但无法通过使用 su 获得完整的root权限。
    - 如，Web开发者可能需要 sudo 重新启动httpd服务。
- SELinux的用户布尔值
  
  - ssh_sysadm_login布尔值
  
  - 如下所示，限制用户是否可以在其家目录或 /tmp 中运行。
  
  - user_exec_content、sysadm_exec_content 和 staff_exec_content 分别用于 user_u、sysadm_u 和 staff_u。
  
  - (此处有图片)
  
  - 限制用户账户示例：user_u SELinux用户与user_exec_content布尔值的关系 (此处有图片)

### 限制（confine）用户账户

- 实施用户限制常规方法
  
  - 更新默认的SELinux映射，将Linux用户与 user_u 关联。
  
  - (此处有图片)
  
  - 将系统管理员映射到 sysadm_u SELinux用户。
  
  - (此处有图片)
  
  - （可选）将需要sudo权限的Linux用户映射到 staff_u，然后配置sudo。
  
  - (此处有图片)

- 限制不同的用户账户
  
  - 若需限制所有Linux用户，通常首先需修改默认的映射为 user_u 映射。
  
  - 因此，SELinux默认为将所有Linux用户映射到权限最小的一个SELinux用户。
  
  - 如需加强安全防护，还可阻止 user_u 中的用户在其家目录或 /tmp 目录中执行程序。
  
  - 可将 user_exec_content 布尔值设为 off（见如上示例）
  
  ```bash
  $ sudo setsebool -P user_exec_content off
  # 将user_exec_content布尔值设置为off
  ```

- 限制系统管理员
  
  - 若需限制系统管理员，将其Linux用户映射至 sysadm_u SELinux用户。
  
  ```bash
  $ sudo semanage login -a -s sysadm_u <sysadmin_name>
  ```
  
  ```bash
  $ sudo useradd -G wheel -Z sysadm_u <sysadmin_name>
  # 创建新用户时，将其映射为sysadm_u SELinux用户。 
  ```
  
  - 将指定的系统管理员用户添加至wheel组中，使其具有sudo权限。
  
  - 默认情况下，映射至 sysadm_u 的用户不可通过SSH登录系统（由 ssh_sysadm_login 布尔值定义）
  
  - ❗ 删除Linux用户时，务必删除其映射（2种方式）。(此处有图片)

- 限制staff用户
  
  - 某些标准的Linux用户可能需要以root的身份运行特定命令。
  
  - 将其映射至 staff_u SELinux用户帐户，然后配置 sudo。
  
  - 配置方法与 user_u、sysadm_u 类似。

- ❗ xguest_u 和 guest_u 等受限用户所受限制比 user_u 还多。
