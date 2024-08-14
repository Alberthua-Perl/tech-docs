## NAS 与 NFS 存储相关

### 文档说明：

- 该文档为 NAS 与 NFS 存储相关原理与操作汇总。

### 文档目录：

- 案例：AIX 挂载 Linux NFS 共享目录故障排查
- 案例：不同网段 NAS 存储的路由配置故障排查
- NAS 存储路由配置示例

### 案例：AIX 挂载 Linux NFS 共享目录故障排查

- 问题描述：
  
  Linux NFS 服务端已完成相应配置，并已导出共享目录，但 AIX 客户端无法挂载 NFS 目录。

- 排查过程：
  
  ```bash
  # 以下操作均在 AIX 上以 root 用户执行
  $ nfso -a | grep nfs_use_reserved_ports
  # 查看 AIX 客户端 NFS 挂载的保留端口状态    
  # Linux NFS 服务端在响应 AIX 的 mount 请求时, AIX 需要使用 NFS 的保留端口, 默认情况下该端口不开放。    
  $ nfso -o nfs_use_reserved_ports=1
  # 开启 AIX 客户端 NFS 挂载的保留端口
  
  $ mount <linux_nfs_server_ip>:<nfs_shares_dir> <local_dir>
  # AIX 客户端挂载 Linux NFS 共享目录
  # 如，mount 182.242.2.134:/cmddata /cmddata
  ```

### 🚀 案例：不同网段 NAS 存储的路由配置故障排查

- 问题描述：
  
  - 系统需添加新的 NAS 存储路由，但新的 NAS 机头 IP 地址与系统分配的使用 NAS 存储的 IP 地址不属于同一网段。
  
  - 系统上已挂载其他 NAS 存储，并且与新的 NAS 存储网段相同。

- 排查过程：
  
  - 卸载已挂载的 NAS 存储：
    
    - 在配置新的 NAS 存储路由前，需将原先的 NAS 存储卸载，配置成功后再将其挂载。
    
    - 切换新的 NAS 存储路由，原先的 NAS 存储从生产网段切换至 NAS 存储网段，若不卸载可能将导致系统挂起死机！
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/nas-nfs-demo/nas-route-change-1.jpg)
  
  - 配置新的 NAS 存储路由：
    
    - 系统使用新的网络接口 eth2 配置新的 IP 地址用于 NAS 存储挂载，将 NAS 存储流量与生产网段流量隔离。
    
    - 182.251.80.1 网关将系统的 NAS 存储数据转发至 NAS 存储网段 182.251.8.0/255.255.252.0 中。
    
    - 挂载新的 NAS 存储前需查看系统已挂载的 NAS 存储的网段, 确认新的 NAS 存储网段是否与其相同, 若两者使用相同的NAS存储网段，需将原先的 NAS 存储卸载，配置新的 NAS 存储路由及网段后，再将原先的 NAS 存储挂载。
    
    - 原先的 NAS 存储使用其他网段挂载至系统, 在使用新的 NAS 存储网段后可能导致权限问题而无法访问挂载！
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/nas-nfs-demo/nas-route-change-2.jpg)
  
  - mount 命令挂载 NAS 存储：
    
    - 系统挂载 NAS 存储时，写入开机自启动脚本中, 防止因写入 `/etc/fstab` 而导致系统超时（timeout）无法启动！    
    
    - mount 命令挂载示例：
      
      ```bash
      $ mount -o rw,soft,intr,timeo=300,retrans=3,vers=3 \
        <nas_head_ip>:<nas_shared_dir> <local_dir>
      
      # ----- 示例 -----
      $ mount -o rw,soft,intr,timeo=300,retrans=3,vers=3 \
      182.251.8.94:/hw_c_edwh_edwh /home/stage/EDWH
      # 挂载 NAS 存储时可能由于 NAS 存储与系统 NFS 协议版本不一致而导致错误。
      # 使用 vers=3 参数指定 NFS v3 版本进行挂载。 
      ```
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/nas-nfs-demo/nas-route-change-3.jpg)

### NAS 存储路由配置示例：

- 示例 1：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/nas-nfs-demo/nas-route-example-1.jpg)

- 示例 2：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/nas-nfs-demo/nas-route-example-2.jpg)
