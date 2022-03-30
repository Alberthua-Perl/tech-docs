## Linux 配置资源限制

### 文档说明：

- 该文档中使用的 OS 版本为 `RHEL 6.x/7.x`，其他 Linux 发行版可自行测试。

### 文档目录：

- Linux 资源限制的级别

- Linux 中针对不同用户对系统资源限制

### Linux 资源限制的级别：

- 系统级别（kernel level）：
  
  Linux 内核参数对系统全局资源的限制，如 `fs.file-max`、`fs.file-nr` 等。

- 进程级别（process level）：
  
  资源限制配置文件与 `ulimit` 命令对用户 shell 环境资源的限制。

### Linux 中针对不同用户对系统资源限制：

- Linux 资源限制基本概要：
  
  - RHEL 6.x/7.x 中的用户级系统资源限制由 PAM 中的 `pam_limits.so` 模块实现。
  
  - `/etc/security/limits.d/[20|90]-nproc.conf` 与 `/etc/security/limits.conf` 为该模块的配置文件。
  
  - 确保用户登录的 PAM 配置文件 `/etc/pam.d/login` 中包含 pam_limits.so 模块（`system-auth` 中已包含）。

- Linux 资源限制的方式：
  
  - 对于 Linux 资源限制可使用配置文件的方式与 ulimit 命令行的方式实现。
  
  - 配置文件的方式：
    
    - Linux 资源限制的主配置文件：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-1.jpg)
      
      - RHEL 7.x 中的配置文件：
        
        - /etc/security/limits.d/20-nproc.conf
        
        - /etc/security/limits.conf
      
      - RHEL 6.x 中的配置文件：
        
        - /etc/security/limits.d/90-nproc.conf
        
        - /etc/security/limits.conf
    
    - /etc/security/limits.conf 配置文件中的字段说明：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-3.jpg)
      
      - `domain` ：用户分类
        
        - 超级用户（root）
        
        - 全局用户（*）
        
        - 指定用户/组
      
      - `type` ：类型
        
        - `soft` ：软限制，即超出该限制系统发出警告。
        
        - `hard` ：硬限制，即限制的最大值。
        
        - `-` ：同时设置了 soft 与 hard 值。
      
      > 💥 SLES 11 SP4 只有该配置文件限制用户的系统资源，配置方式相同！
    
    - `/etc/security/limits.d/[20|90]-nproc.conf`：
      
      针对用户 nproc 最大进程数的限制，在 RHEL 5.x 中不存在。
  
  - `/etc/security/limits.d/[20|90]-nproc.conf` 优先级大于 `/etc/security/limits.conf`
  
  - 关于以上两个配置文件的重要说明：
    
    - 若 [20|90]-nproc.conf 中，root、全局用户、指定用户的 soft 值小于 limits.conf 中相应用户的 hard 值，那么 root、全局用户、指定用户均使用 [20|90]-nproc.conf 中的 soft 值。
    
    - 若 [20|90]-nproc.conf 中, root、全局用户、指定用户的 soft 值（可设置为 unlimited）大于 limits.conf 中相应用户的 hard 值，那么 root、全局用户、指定用户使用 limits.conf 中的 hard 值。
    
    - 👉 [20|90]-nproc.conf 具有优先权，而 limits.conf 具有决定权。
    
    - 👉 指定用户/组不受全局用户限制的影响。
    
    - 可将 /etc/security/limits.d/[20|90]-nproc.conf 移除, 只由 limits.conf 进行限制，降低配置复杂度。
    
    - 其他限制选项不受 [20|90]-nproc.conf 影响。
    
    - 示例 1：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-2.jpg)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-4.jpg)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-5.jpg)
    
    - 示例 2：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-6.jpg)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-7.jpg)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-8.jpg)
    
    - 示例 3：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-9.jpg)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-10.jpg)
    
    - 示例 4：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-11.jpg)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/rhel-suse-resource-limit-12.jpg)
  
  - ulimit 命令行的方式：
    
    - ulimit 命令能临时使当前登录用户的 shell 环境资源限制立即生效，但仅对当前环境有效，退出重新登陆后将失效。
    
    - 可以将对用户的 ulimit 命令写入 `$HOME/.bash_profile` 中，实现永久资源限制。
    
    - ulimit 命令常用选项：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-resource-limit/ulimit-options.jpg)
      
      ```bash
      $ man bash -> 搜索 ulimit
        -H                      设置用户 shell 环境的资源硬限制
        -S                      设置用户 shell 环境的资源软限制
                                若 -H 与 -S 同时指定或不指定，两者默认设置为相同的值。
        -a                      查看用户 shell 环境的当前全部资源限制
        -n [<file_handle_num>]  查看或设置打开的文件句柄的最大数量
        -u [<proc_num>]         查看或设置用户的最大可用进程数
        -T <thread_num>         设置用户的最大线程数
      ```

- Linux 资源限制的使用场景：
  
  - 高并发应用：
    
    - nofile（用户打开最大的文件句柄数），若设置过小将报错。
    
    - nproc（用户打开的最大进程数），若设置过小将报错。
  
  - 消息队列服务
