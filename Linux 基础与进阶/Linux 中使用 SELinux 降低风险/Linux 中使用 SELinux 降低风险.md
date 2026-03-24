# Linux 中使用 SELinux 降低风险

## 从禁用状态启用SELinux

### 复习SELinux基本概念

- Security Enhanced Linux（SELinux）：安全增强的Linux（额外的系统安全层）

- SELinux的主要目标：防止已遭泄露的系统服务访问用户数据

- SELinux可以充分限制程序（主要是系统服务）的权限，仅授予程序成功完成工作所需的权限。

- SELinux有一组安全规则确定哪个进程可以访问哪些文件、目录和端口。

- 若没有明确的访问规则，SELinux会拒绝操作。

- ✅ 每个文件、进程和端口均有一个特定的安全标签（security label），称为SELinux上下文（context）。

- SELinux规则使用上下文控制访问。

- 对于文件和目录，SELinux会将上下文存储为文件系统的扩展属性（xattr）。

- 对于进程和端口，内核会将上下文保留在内存中。

- RHEL中SELinux的大部分规则基于第三个元素，即上下文类型（context type）。

- 进程的上下文类型也称为域（domain）。

- 更改文件和目录的SELinux上下文

- 定义SELinux默认文件上下文规则

- 标记SELinux端口

- 使用SELinux布尔值
  
  - SELinux布尔值是更改SELinux策略行为的开关。
  
  - 可以启用或禁用SELinux的访问规则集。
  
  - 安全管理员可以使用SELinux布尔值来有选择地调整策略。

- 访问文档
  
  - 每个SELinux命令都有一个man page。
  
  - 每个semanage子命令也都有自己的man page。
  
  - 域（进程的上下文类型）也具有man page。
  
  ```bash
  $ sudo yum install -y policycoreutils-devel
  # 安装SELinux核心策略开发工具软件包
  ```
  
  ```bash
  $ sudo sepolicy manpage -a -p <directory_path>
  # 指定路径为所有域构建man page。
  ```
  
  - -a 选项：为所有域构建man page
  
  - -p 选项：指定路径构建man page（标准的man page存放路径为 /usr/share/man/man8/），默认构建于 /tmp 目录中。
  
  ```bash
  $ sudo sepolicy manpage -d <domain>
  # 为指定的域构建man page
  ```
  
  ```bash
  $ sudo sepolicy manpage -d [httpd_t|sshd_t]
  # 为httpd_t域或sshd_t域构建man page
  ```

### 配置SELinux模式

- 若将现有系统直接从disabled模式切换到enforcing模式，有可能会发现访问违规和其他SELinux问题。

- 通常而言，最好是安装时将系统配置为permissive模式或enforcing模式。

- 系统由permissive模式转换至enforcing模式要比disabled模式转换更加容易。

### 从禁用模式启用SELinux

- 将SELinux从disabled模式切换至enforcing模式下，需遵循如下步骤，以免SELinux阻止应用。

- 将系统切换到permissive模式。

- 在此模式下，SELinux不会强制执行该策略，但至少会在 /var/log/audit/audit.log 文件中报告拒绝记录。

- disabled模式切换到permissive模式，需重启系统并重新标记文件系统。

- 再次查看 audit.log 文件的拒绝记录。

- permissive模式下SELinux不会强制拒绝。

- 解决拒绝，需要分析每个拒绝，掌握可疑应用的行为，然后才能做出适当的响应。

- 重复步骤将系统切换到permissive模式和解决拒绝，需要分析每个拒绝，掌握可疑应用的行为，然后才能做出适当的响应，直到所有拒绝均已解决为止。

- 在经过了确认已解决的所有问题后，切换到enforcing模式。

- 查看SELinux访问冲突审计事件
  
  - SELinux拒绝访问资源之后，还将记入一个审计事件。
  
  - 若auditd服务已启动，系统将会把SELinux审计事件记入 /var/log/audit/audit.log。
  
  - 否则，系统将SELinux消息发送至 /var/log/messages 文件。
  
  - 示例：创建自定义的Apache httpd根目录 /web/data，配置基于域名的虚拟主机后访问该站点，若不设置根目录的SELinux类型上下文，将拒绝访问。
  
  ```bash
  $ sudo ausearch -i -m AVC -ts boot
  # 查看自启动后相关的SELinux审计记录
  ```

- 使用允许域
  
  - 在permissive模式下，SELinux将记录它应该拒绝的访问，但始终都会允许访问。
  
  - 若将系统切换到permissive模式（可能为调试特定进程），此举可能会造成严重的安全后果，因为其他所有服务此时也将无法得到SELinux的保护。
  
  - 有些情况下，可能正在将系统从disabled模式切换到enforcing模式，而且只剩下一两个SELinux域的AVC问题还未解决。
  
  - 多数情况下，可以将系统设置为enforcing模式，但应当在确认相关问题之后，再将这一两个域由permissive模式切换到enforcing模式。
  
  - 可以将系统设为enforcing模式，只将特定域设为 permissive 模式。
  
  ```bash
  $ sudo semanage permissive -a <domain>
  # 设置特定域为permissive模式使其成为无限制域，但策略依然将记录访问的违规记录。
  ```
  
  ```bash
  $ sudo semanage permissive -a httpd_t
  # 将Apache httpd_t域设置为permissive模式
  ```
  
  ```bash
  $ sudo semanage permissive -l
  # 列出当前处于permissive模式的域 
  ```
  
  ```bash
  $ sudo semanage permissive -d httpd_t
  # 将Apache httpd_t域切换为enforcing模式
  ```

- 使用Ansible修复SElinux
  
  - 若具有一组相同的系统，运行相同的应用，则可使用Ansible配置SELinux，然后在这些系统上以可重现且相同的方式修复拒绝。
  
  - ❗ RHEL 7.4引入系统角色作为技术预览。
  
  - 在Ansible控制节点上安装 rhel-system-roles 软件包即可部署这些角色。
  
  - SELinux角色（role）的功能
    
    - 设置SELinux的模式：enforcing、permissive、disabled
    
    - 恢复文件和目录的上下文（等同于restorecon命令）
    
    - 标记SELinux端口（等同于semanage port命令）
    
    - 添加新的文件上下文规则（等同于semanage fcontext命令）
    
    - 切换布尔值（等同于setsebool命令）
  
  - SELinux角色示例（见教材）：确保SELinux处于 enforcing 模式，/virtual目录的内容设置了类型 httpd_sys_content_t，端口 8080/TCP 的标签已设为 http_port_t，并且已启用了SELinux的布尔值 httpd_enable_homedirs。

-----

## 使用受限用户控制访问

### 定义SELinux用户

- SELinux策略对其SELinux用户有所定义，与Linux用户截然不同。

- Linux用户登录时，被映射到一个SELinux用户。

- 与Audit UID一样，SELinux用户在登录会话期间无法更改。

- 通常，多个Linux用户可以映射到同一SELinux用户。

- 该策略能对具体某些SELinux用户可以执行的操作施加额外的SELinux强制限制。

```bash
$ sudo semanage user -l
# 查看SELinux用户
```

- (此处有图片)

- 各个SELinux用户均有权限充当最后一列的一组SELinux角色。

- 这些角色定义了SELinux用户可以运行的程序
  
  - sysadm_r 角色：可使用su和sudo命令
  
  - xguest_r 角色：可限制用户可使用的命令，且只允许通过Firefox访问网络。

- 将Linux用户映射到SELinux用户
  
  - 登录时，SELinux会将Linux用户映射到SELinux用户。
  
  - 因此，Linux用户会继承相应SELinux用户受到的限制。
  
  ```bash
  $ sudo semanage login -l
  # 查看Linux用户映射到SELinux用户的关系
  ```
  
  - (此处有图片)
  
  - 如上所示，SELinux将Linux root用户映射到 unconfined_u SELinux用户。
  
  - __default__ 条目：SELinux将未明确映射到SELinux用户的所有Linux用户映射到unconfined_u SELinux用户。
  
  - 映射到 unconfined_u 的Linux用户（“无限制用户”）没有其他基于用户（user-based）的SELinux限制。
  
  - SELinux策略（policy）的其他限制仍然适用。
  
  - SELinux利用 system_u 用户进行系统服务，不要将其用于Linux用户。
  
  - ❗ 默认情况下，在全新的RHEL安装中，Linux用户将映射到SELinux用户unconfined_u，该SELinux用户不受其他特定用户的规则（user-specific rule）限制。
  
  ```bash
  $ id -Z
  # 查看登录的Linux用户关联的SELinux用户
  ```
  
  - ❗ 必须重新使用SSH登录的方式验证Linux用户与SELinux用户的映射关系，不可使用 su 命令进行用户切换。
  
  ```bash
  $ sudo semanage login -a -s <selinux_user> <linux_user>
  # 将Linux用户映射至指定SELinux用户
  ```
  
  ```bash
  $ sudo semanage login -d -s <selinux_user> <linux_user>
  # 删除Linux用户至指定SELinux用户的映射
  ```
  
  - (此处有图片)
  
  ```bash
  $ sudo semanage login -m -s user_u -r s0 __default__
  # 更改默认映射
  ```
  
  - (此处有图片)
  
  ```bash
  $ sudo useradd -Z <selinux_user> <linux_user>
  # 创建Linux用户时指定SELinux用户的映射
  ```

- 对比SELinux用户
  
  - 在RHEL中，SELinux自带一组预定义的SELinux用户。
  
  - 很少需要创建新的SELinux用户，这些现有用户已涵盖大多数用例。
  
  - 受限SELinux用户示例：
  
  - user_u :
    
    - 标准的非管理用户帐户
    
    - SELinux限制映射至 user_u 的Linux用户通过 su 或 sudo 提权为root用户，或防止执行大多数 setuid（SUID） 程序。
    
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
