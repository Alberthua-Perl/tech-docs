# 🐙 Red Hat Ceph Storage v5.0 常用命令汇总

## 文档说明

- 该文档涉及的命令在 `Red Hat Ceph Storage v5.0` 集群中进行验证。
- Red Hat Ceph Storage v5.0 对应 `Ceph 16.2.0 pacific (stable)` 开源版本。
- 该文档将持续更新。

## 文档目录

- Ceph 集群状态
- Ceph Monitor 监控器
- Ceph Manager 管理器与模块
- Ceph Cluster Map 集群映射
- Ceph CRUSH Map 映射
- Ceph PG 放置组
- Ceph OSD 对象存储设备
- RADOS 对象操作
- Ceph Pool 存储池
- CephX 认证与用户
- Ceph RBD 镜像
- CephFS 文件系统
- 参考链接

## Ceph 集群状态

- Red Hat Ceph Storage v5.0 中已不再使用旧式的 systemd 管理守护进程的方式管理 Ceph 进程，而使用 `Podman rootfull` 容器的方式封装运行 Ceph 进程，因此，可使用 `cephadm` 命令行工具取代以往的 `ceph-ansible` 来实现集群的部署与管理。

- 管理 Ceph 集群的两种方式：
  
  - 1️⃣ 在管理节点上直接使用 ceph 命令。
  
  - 2️⃣ 进入管理节点的 cephadm shell 临时容器中，使用交互式命令管理。

- cephadm 命令使用：
  
  cephadm 工具一般安装于部署节点或管理节点中，因此运行以下命令需在对应节点中运行。
  
  ```bash
  $ cephadm --help
  $ cephadm version
  $ cephadm inspect-image
  # 查看当前集群使用的 Ceph 容器镜像仓库与版本
  $ cephadm list-networks
  # 查看当前集群使用的网段信息
  
  $ cephadm shell -- <command>
  $ cephadm shell -- ceph status  #示例
  # 在集群 bootstrap 节点使用 cephadm shell 以确认集群健康状态
  # 该命令运行过程中将启动一个临时容器以运行指定的命令
  $ cephadm shell --mount /root/<mount-point>
  # 启动临时容器并将指定目录映射至容器 /mnt 目录上
  
  $ cephadm bootstrap --config /etc/ceph/ceph.conf
  # 集群引导过程中更改集群配置文件以传递修改的配置使其生效
  
  $ ceph versions
  # 查看集群所有服务组件的版本
  $ ceph tell osd.* version
  # 查看所有 osd 的版本
  ```
  
  ![](pictures/cephadm-demo.png)
  
  ![](pictures/cephadm-shell-demo.png)
  
  ✨ 注意：使用 ceph orch host add 添加额外的节点时，需先将集群公钥导出并同步至节点。
  
  ```bash
  $ ceph cephadm get-pub-key > /path/to/ceph.pub
  # 生成集群公钥
  $ ssh-copy-id -f -i /path/to/ceph.pub root@<hostname>
  # 将集群公钥同步至节点，即可添加该节点。
  $ ceph orch host add <node>
  # 添加节点为集群节点
  ```

- Ceph Orchestrator 编排器使用：
  
  ```bash
  $ ceph orch status
    Backend: cephadm
    Available: Yes
    Paused: No
  # 查看集群整体状态
  
  $ ceph orch ls [--service_type=<name>]
    NAME                     RUNNING  REFRESHED  AGE  PLACEMENT                                                                
    alertmanager                 1/1  9m ago     2y   count:1                                                                  
    crash                        4/4  9m ago     2y   *                                                                        
    grafana                      1/1  9m ago     2y   count:1                                                                  
    mgr                          3/3  9m ago     5d   serverc.lab.example.com;serverd.lab.example.com;servere.lab.example.com  
    mon                          3/3  9m ago     5d   serverc.lab.example.com;serverd.lab.example.com;servere.lab.example.com  
    node-exporter                4/4  9m ago     2y   *                                                                        
    osd.default_drive_group     9/12  9m ago     2y   server*                                                                  
    prometheus                   1/1  9m ago     2y   count:1                                                                  
    rgw.realm.zone               2/2  9m ago     23M  serverc.lab.example.com;serverd.lab.example.com
  # 查看 Ceph 集群中的服务（service）状态
  # 使用 --service_type 选项指定 Ceph 服务类型
  $ ceph orch ls --service_type=mon
    NAME  RUNNING  REFRESHED  AGE  PLACEMENT                                                                                        
    mon       4/4  3m ago     2y   clienta.lab.example.com;serverc.lab.example.com;serverd.lab.example.com;servere.lab.example.com
  # 查看 mon 服务的状态
  
  $ ceph orch ps [--daemon_type=<name>]
    NAME                         HOST                     STATUS         REFRESHED  AGE  PORTS  VERSION           IMAGE ID      CONTAINER ID  
    mon.clienta                  clienta.lab.example.com  running (12h)  3m ago     2y   -      16.2.0-117.el8cp  2142b60d7974  6c85145c2b22  
    mon.serverc.lab.example.com  serverc.lab.example.com  running (12h)  3m ago     2y   -      16.2.0-117.el8cp  2142b60d7974  f93b366a9f74  
    mon.serverd                  serverd.lab.example.com  running (12h)  3m ago     2y   -      16.2.0-117.el8cp  2142b60d7974  d35695b5833e  
    mon.servere                  servere.lab.example.com  running (12h)  3m ago     2y   -      16.2.0-117.el8cp  2142b60d7974  97319d05259c
  # 查看 Ceph 集群中的服务实例状态
  # 使用 --daemon_type 选项可指定服务实例的名称进行过滤
  
  $ ceph orch host ls
    HOST                     ADDR           LABELS  STATUS  
    clienta.lab.example.com  172.25.250.10  _admin          
    serverc.lab.example.com  172.25.250.12  _admin          
    serverd.lab.example.com  172.25.250.13                  
    servere.lab.example.com  172.25.250.14
  # 使用编排器查看各节点概要与 label 标签
  # 注意：label 标签的作用在于区分各个节点，对节点进行分组管理，并且一个节点可具有多个标签。
  $ ceph orch host label add <node> _admin
  # 为集群节点添加 admin 管理员 label 标签
  $ ceph orch host label add <node> prometheus
  # 为集群节点添加 prometheus 标签
  $ ceph orch apply prometheus --placement="label:prometheus"
  # 根据集群节点标签部署 prometheus
  
  $ ceph orch device ls [--wide]
  # 查看集群各节点可用与不可用的设备信息，--wide 选项将指定全部的详细设备信息。
  
  $ sudo systemctl list-units --all "ceph*"
  # 登录 Ceph 集群节点查看所有的 ceph 服务单元
  
  $ ceph orch host maintenance enter <node>
  # 将指定节点设置为维护模式
  $ ceph orch host maintenance exit <node>
  # 将指定节点退出维护模式
  ```
  
  👉 部署新的 OSD 设备：
  
  ```bash
  $ ceph orch apply osd --all-available-devices
  # 添加所有 ceph orch device ls 返回的可用节点
  
  $ ceph orch device zap --force <hostname> /path/to/device
  # 强制删除之前 osd 创建的所有分区并清除其中的所有数据为部署 osd 做准备
  $ ceph orch daemon add osd <hostname>:/path/to/device
  $ ceph orch daemon add osd serverd.lab.example.com:/dev/vde  #示例
  # 添加指定主机上的 osd 设备
  ```
  
  ![](pictures/ceph-orch-daemon-add-osd-demo.png)
  
  👉 删除 OSD 设备：
  
  ```bash
  $ ceph device ls | awk /<hostname>/
  # 查看指定节点上磁盘设备与 osd 的对应关系
  $ ceph orch daemon stop osd.$id
  # 停止指定的 osd 守护进程
  # 注意：
  #   1. 若登录至 osd 所在的节点使用 systemctl 命令查看，可发现此服务单元已停止。
  #   2. 可在此节点上使用 systemctl 命令启动此 osd 守护进程
  $ ceph orch daemon rm osd.$id --force
  # 将指定的 osd 守护进程强制移除
  $ ceph orch osd rm status
  # 查看 osd 删除的状态
  $ ceph osd rm $id
  # 将 osd 从集群 osdmap（osd 映射）中删除
  # 注意：
  #   1. 此时，ceph -s 集群状态将显示 crushmap 中依然保留着 osd，而 osdmap 中已移除 osd，两者的状态不匹配。
  #   2. ceph osd tree 显示的 osd 状态为 DNE，当从 crushmap 中移除后将清除。
  $ ceph osd crush rm $id
  # 将 osd 从集群 crushmap（CRUSH 映射）中删除
  ```
  
  ![](pictures/ceph-status-with-noosd-in-osdmap.png)
  
  ✨ 从 RHCS4 开始引入 `ceph-volume` 命令用于创建基于 `BlueStore` 存储引擎的 OSD，分别使用 `ceph-volume lvm prepare` 与 `ceph-volume lvm activate` 子命令创建与激活 OSD 设备，过程如下示意：
  
  ![](pictures/ceph-volume-add-bluestore-osd-backend.jpg)
  
  ![](pictures/ceph-volume-add-bluestore-osd-backend-status.jpg)
  
  ![](pictures/ceph-volume-active-bluestore-osd.jpg)
  
  ```bash
  ### RHCS5 中依然可在
  $ ceph-volume lvm list
  # 登录对应节点查看本节点 osd 与 lv 的对应关系
  $ ceph osd metadata <osd_id>
  # 重要：根据 osd id 查看 osd 的元数据信息
  ```

- Ceph 集群配置：
  
  每个 `ceph monitor` 节点管理一个集中式配置数据库，位于 `/var/lib/ceph/$fsid/mon.$host/store.db/` 中。在集群启动时，Ceph 守护进程解析由命令行选项、环境变量与本地集群配置的配置选项。Ceph 守护进程连接到集群以获取存储在集中式配置数据库中的配置设定。从 RHCS 4 开始弃用 `/etc/ceph/ceph.conf` 集群配置文件，而将集中式配置数据库作为配置存储的首选方式。ceph config set 命令可用于更改集群各类配置，包括 `public_network` 与 `cluster_network`。
  
  ![](pictures/ceph-arch-network.png)
  
  ![](pictures/osd-network-community.jpg)
  
  ☝ Ceph OSD 节点的网络连接示意（每个 OSD 使用一个端口通过 public 网络与客户端及 Mon 通信，一个端口通过 cluster 网络与其他 OSD 间传输数据，一个端口通过 cluster 网络交换 heatbeat 心跳包）
  
  ![](pictures/default-ports-rhcs5.jpg)
  
  ☝ RHCS5 中的默认端口范围
  
  ```bash
  $ ceph -s
  $ ceph status
  $ ceph -w
  # 实时刷新日志
  ```
  
  ```bash
  $ ceph config ls
  # 列出集群所有的可配置参数（RHCS5 包含 2026 个配置参数）
  $ ceph config help <config_setting>
  # 查看指定配置参数的具体信息
  $ ceph config dump
  # 查看集中式配置数据库的配置（区别于集群配置文件）
  $ ceph config show $type.$id
  # 查看特定守护进程正在运行的配置
  $ ceph config show-with-defaults $type.$id
  # 查看特定守护进程正在运行的默认配置
  
  $ ceph config get $type.$id
  # 查看特定守护进程在集中式配置数据库中的配置
  $ ceph config set $type.$id <config_setting>
  # 设置特定守护进程在集中式配置数据库中的配置
  $ ceph config set mon.serverd mon_max_pool_pg_num 65536  #示例
  # 设置 mon.serverd 的 mon_max_pool_pg_num 为 65536
  
  $ ceph config assimilate-conf -i /path/to/config_file
  # 将指定配置文件中的参数融入当前的集中式配置数据库中
  
  $ ceph config get mon mon_allow_pool_delete
  # 查看集群是否允许删除存储池
  $ ceph config get mon mon_compact_on_start
    false
  # 查看 mon 在集群启动过程中是否可收缩数据库，以提高数据库性能。
  $ ceph config set mon mon_compact_on_start true
  # 设置 mon 在集群启动过程中进行数据库压缩
  
  $ ceph config get mon auth_service_required
  $ ceph config get mon auth_cluster_required
  $ ceph config get mon auth_client_required
  # 查看集群的各认证方式
  
  ### 集群运行时覆盖配置设置 ###
  $ ceph tell $type.$id config get <config_setting>
  $ ceph tell mon.serverc config get mon_max_pool_pg_num  #示例
  # 查看 serverc 控制节点上支持的每个存储池可包含的最大 pg 数
  # 注意：
  #   1. 以上命令可在集群运行时覆盖配置，该方式作为临时配置，当守护进程重启后将失效。
  #   2. ceph tell 子命令依然使用 mon。
  ```

## Ceph Monitor 监控器

```bash
$ ceph mon stat
  e5: 3 mons at {serverc.lab.example.com=[v2:172.25.250.12:3300/0,v1:172.25.250.12:6789/0],
  serverd=[v2:172.25.250.13:3300/0,v1:172.25.250.13:6789/0],servere=[v2:172.25.250.14:3300/0,
  v1:172.25.250.14:6789/0]}, election epoch 42, leader 0 serverc.lab.example.com, 
  quorum 0,1,2 serverc.lab.example.com,serverd,servere
# 查看 mon 状态，包括 mon 的 IPv4 地址与 leader mon。

$ ceph quorum_status -f json-pretty
{
  "election_epoch": 42,
  "quorum": [
    0,
    1,
    2
  ],
  "quorum_names": [
    "serverc.lab.example.com",
    "serverd",
    "servere"
  ],
  "quorum_leader_name": "serverc.lab.example.com",
  ...
# 查看 mon 的选举状态
```

## Ceph Manager 管理器

```bash
### Ceph 配置 Dashboard 管理 ###
$ ceph mgr module enable dashboard --force
# 启用 dashboard 模块
$ ceph mgr module ls | \
  perl -MJSON -MYAML -0777 -wnl -e 'print YAML::Dump(decode_json($_))' -
# 查看 mgr 管理的模块并使用 perl 将输出的 JSON 格式转换为 YAML 格式
# 注意：使用 perl 时需提前安装 perl-JSON 与 perl-YAML 软件包以提供对应模块
$ ceph dashboard create-self-signed-cert
# 配置 dashboard 生成证书
$ ceph config set mgr mgr/dashboard/server_addr <dashboard_server_address>
# 配置 dashboard 的主机 IP 地址
$ ceph config set mgr mgr/dashboard/server_port 8080
# 配置 dashboard 监听的非 SSL 端口
$ ceph config set mgr mgr/dashboard/ssl_server_port 8443
# 配置 dashboard 监听的 SSL 端口
$ ceph mgr services
# 获取 mgr 管理的 dashboard URL
$ ceph dashboard ac-user-create \
<dashboard_user> <dashboard_password> administrator
# 创建登录 dashboard 的用户名与密码，以管理员用户身份登录。

### 修改 Dashboard 仪表板密码 ###
$ echo "<password>" > /path/to/dashboard_password.yml
# 注入修改的新密码
$ ceph dashboard ac-user-set-password <dashboard_user> -i /path/to/dashboard_password.yml 
# 设置指定用户与密码
```

Ceph balancer 均衡器：

```bash
$ ceph balancer status
{
    "active": true,
    "last_optimize_duration": "0:00:00.000462",
    "last_optimize_started": "Thu Jan 11 05:51:41 2024",
    "mode": "upmap",
    "optimize_result": "Unable to find further optimization, or pool(s) pg_num is decreasing, or distribution is already perfect",
    "plans": []
} 
# 查看 balancer 均衡器状态
```

## Ceph Cluster Map 集群映射

Ceph Cluster Map 包含 MON Map、OSD Map、PG Map、MDS Map 和 CRUSH Map。

```bash
$ ceph mon dump
  epoch 55
  fsid 2ae6d05a-229a-11ec-925e-52540000fa0c
  last_changed 2023-09-17T23:46:59.748723+0000
  created 2021-10-01T09:30:30.146231+0000
  min_mon_release 16 (pacific)
  election_strategy: 1
  0: [v2:172.25.250.12:3300/0,v1:172.25.250.12:6789/0] mon.serverc.lab.example.com
  1: [v2:172.25.250.14:3300/0,v1:172.25.250.14:6789/0] mon.servere
  2: [v2:172.25.250.13:3300/0,v1:172.25.250.13:6789/0] mon.serverd
  dumped monmap epoch 55
# 查看集群 monmap
$ ceph osd dump
# 查看集群 osdmap
$ ceph pg dump
# 查看集群 pgmap
$ ceph osd crush dump
# 查看集群 crush map（可查看集群整体的 CRUSH 层次结构与故障恢复域）
$ ceph fs dump
# 查看集群 cephfs map
```

## Ceph CRUSH Map 映射

🚀 关于 `CRUSH map` 的说明可参考 [Ceph CRUSH map 概述与实现](https://github.com/Alberthua-Perl/tech-docs/blob/master/%E5%88%86%E5%B8%83%E5%BC%8F%E5%AD%98%E5%82%A8/Ceph%20CRUSH%20map%20%E6%A6%82%E8%BF%B0%E4%B8%8E%E5%AE%9E%E7%8E%B0.md#-ceph-crush-map-%E6%A6%82%E8%BF%B0%E4%B8%8E%E5%AE%9E%E7%8E%B0)

## Ceph PG 放置组与 OSD

Ceph PG、CRUSH 放置规则与 OSD 之间的关系如下图所示：

![](pictures/ceph-pg-crush-osd-mapping.png)

```bash
$ ceph mgr stat
{
    "epoch": 19,
    "available": true,
    "active_name": "serverc.lab.example.com.weqbtr",
    "num_standby": 2
}
# 确认 mgr 的整体状态

$ ceph mgr module enable pg_autoscaler
$ ceph mgr module ls
{
    "always_on_modules": [
        "balancer",
        "crash",
        "devicehealth",
        "orchestrator",
        "pg_autoscaler",
        "progress",
        "rbd_support",
        "status",
        "telemetry",
        "volumes"
    ],
    ...
}
$ ceph osd pool set <pool-name> pg_autoscale_mode on
# 启用 mgr 的 pg_autoscaler 模块并启用指定存储池的自动扩展 pg 功能

$ ceph pg stat
  105 pgs: 105 active+clean; 4.9 KiB data, 181 MiB used, 90 GiB / 90 GiB avail
# 查看集群中所有 pg 的状态
$ ceph pg map <pgid>
# 重要：根据 pd id 查找指定 pg 与 osd 的对应关系
$ ceph pg <pgid> query
# 重要：根据 pg id 查看其详细状态信息
```

## Ceph OSD 对象存储设备

```bash
$ ceph osd df
# osd 级别的存储使用情况
$ ceph osd df tree
# osd 级别的存储使用情况并显示 osd 在 CRUSH map 中的位置
$ ceph osd tree
# osd 磁盘在集群中的分布情况

$ ceph osd stat
  9 osds: 9 up (since 13h), 9 in (since 2y); epoch: e199
# 查看所有 osd 的状态
$ ceph osd find <osd_id>
# 根据 osd id 查找 osd 的状态及所在的节点
$ ceph osd find 3
# 查找 osd.3 的状态及所在的节点
$ ceph osd map <pool_name> <object_name>
# 重要：
#   1. 根据指定的对象名称查找其与 pg 及 osd 的映射关系
#   2. 此命令相较于 ceph pg map <pgid> 更进一步，直接查找对象的映射关系。

$ ceph osd [set|unset] noscrub
# 设置 osd 标记为不执行清理
$ ceph osd [set|unset] nodeep-scrub
# 设置 osd 标记为不执行深度清理
```

## RADOS 对象操作

```bash
$ rados -p <pool_name> put <object_name> /path/to/file
# 指定文件将其上传至集群中以指定的名称命名
```

## Ceph Pool 存储池

```bash
### Ceph 存储池状态 ###
$ ceph osd lspools
$ ceph osd ls pool detail
# 以上两个命令用于查看集群中的存储池

$ rados df
# RADOS 层面查询集群状态
$ ceph df
# Ceph 集群与存储池级别的存储使用情况
$ ceph osd pool stats
# 确认所有存储池的性能统计信息

### Ceph 复制池 ###
$ ceph osd pool create <pool-name> \
  <pg-num> <pgp-num> [replicated] <crush-rule-set>
# 创建复制池
# 注意：
#   1. 默认情况下可只指定存储池名称与 PG 数量，也可自定义 CRUSH 放置规则在创建存储池时指定。
#   2. 若设置了相同的 pg-num 与 pgp-num，之后再调整 pg-num 将自动调整 pgp-num。

$ ceph osd pool application enable <pool-name> [rbd|rgw|cephfs]
# 设置指定的存储池为 rbd、rgw 或 cephfs 应用类型

$ ceph osd pool get <pool-name> all
# 查看存储池的所有参数值
$ ceph osd pool get <pool-name> [size|nodelete|min_size]
# 查看存储池的指定参数值
$ ceph osd pool get <pool-name> pg_autoscale_mode
# 查看存储池的 PG 自动扩展模式
$ ceph osd pool set <pool-name> [size|nodelete|min_size] <value>
# 设置存储池的指定参数值

$ ceph osd pool get mon osd_pool_default_size
# 查看集中配置数据库中定义的默认对象副本数量

$ ceph osd pool rename <old-pool-name> <new-pool-name>
# 重命名存储池名称（不影响池中存储的数据，但需要注意用户对池的权限）

### Ceph 纠删代码池 ###
$ ceph osd erasure-code-profile ls
# 查看纠删代码池的 profile
$ ceph osd erasure-code-profile get <profile-name>
# 查看指定纠删代码池的 profile 具体信息
$ ceph osd erasure-code-profile set <profile-name> \
  k=<num> m=<num> crush-failure-domain=[osd|host|rack]
# 创建纠删代码池的 profile
$ ceph osd erasure-code-profile rm <profile-name>
# 删除指定的纠删代码池的 profile
$ ceph osd pool create <pool-name> \
  <pg-num> <pgp-num> erasure <profile-name> <crush-rule-set>
# 创建纠删代码池
```

关于纠删代码 profile 的设置 RedHat 给出以下推荐方式：

![](pictures/redhat-reasure-code-profile-recommanded.jpg)

## CephX 认证与用户相关

CephX 认证机制：

![](pictures/cephx-request-1.jpg)

Ceph 客户端使用 CephX 协议向 monitor 发送用户创建请求，当 monitor 创建用户后将存储用户名、keyring 与 capability 等信息，并将相同的 keyring 文件返回给客户端，其中 keyring 文件中的 key 为 `secret key`，用于加密与解密 monitor 生成的会话密钥（`session key`）。

![](pictures/cephx-request-2.jpg)

![](pictures/cephx-request-4.png)

每个 monitor 都可以对客户端进行身份验证并分发密钥，这意味着认证依靠 monitor 节点完成，不会存在单点和性能瓶颈。monitor 会返回用于身份验证的数据结构，其中包含获取 Ceph 服务时用到的 session key。所谓 session key 就是客户端用来向 monitor 请求所需服务的凭证，`session key` 是通过客户端的 `secret key` 进行加密传输。

🚀 当 monitor 收到客户端认证请求后，首先生成 session key，然后用客户端的 secret key 加密session key，再发送给客户端，客户端用自身的 secret key 将其解密，拿到 session key。客户端获取 session key 之后，它就可以用此 session key 向 monitor 请求服务，monitor 收到客户端的请求（携带 session key），此时 monitor 会向客户端提供一个 `ticket`（票据），然后使用 session key 加密后发送给客户端。随后客户端使用 session key 解密该票据，使用此票据到对应 OSD 完成认证。

以上过程，需要注意的是，首先，客户端的 secret key 是通过 monitor 节点在创建用户帐号时就生成，所以 monitor 节点有对应客户端的 secret key，通过客户端的 secret key 加密，客户端可以用自身的 secret key 解密。其次，monitor 节点生成的 session key 是有记录的，所以对于不同客户端来说，都有不同的记录，并且该 session key 是有时间限制的，过期即便是对应客户端，也无法正常使用。所以客户端使用对应 session key 向 monitor 请求服务，对应 monitor 都是认可的，monitor 会向其发放 ticket。最后，monitor 和 OSD 都共享客户端的 secret key 和 session key，以及 monitor 发放的 ticket，所以客户端使用 monitor 发放的 ticket，对应 OSD 是认可的。这也意味着不管是哪个 monitor 节点发放的 ticket，对应所有 monitor 节点和 OSD 都可认证。

![](pictures/cephx-request-3.png)

🏷 关于 CephX 认证机制更多可参考 [HIGH AVAILABILITY AUTHENTICATION](https://docs.ceph.com/en/latest/architecture/#high-availabilityauthentication) 中的说明。

```bash
$ ceph auth get-or-create [--id <name>|--name client.<name>] client.<name> \
  mon 'allow r' osd 'allow rw' \
  -o /path/to/ceph.client.<name>.keyring
# 使用指定的 Ceph 用户创建新用户并将其 secret key 注入 keyring 文件
# 也可使用 --keyring 选项指定 keyring 文件用于创建新用户

### 限制用户的访问 ###
$ ceph auth get-or-create client.<name> \
  mon 'profile rbd' osd 'profile rbd pool=<pool-name>'
# 限制用户只能访问指定存储池中的 RBD 镜像
$ ceph auth get-or-create client.<name> \
  mon 'allow r' osd 'allow rw object_prefix <prefix-name>'
# 限制用户只能访问指定前缀的对象
$ ceph auth get-or-create client.<name> \
  mon 'allow r' osd 'allow rw namespace=<namespace>'
# 限制用户只能访问指定 namespace 中的对象
$ ceph fs authorize cephfs client.<name> /path/to/cephfs rw
# 限制用户只能读写 CephFS 中的指定路径
$ ceph auth get-or-create client.<name> \
  mon 'allow r, allow command "auth get-or-create", allow command "auth list"'
# 通过 monitor 命令限制用户只能运行指定的 ceph 命令

$ ceph auth list
# 列出所有的 Ceph 用户
$ ceph auth get client.<name>
# 显示指定用户的详细信息
$ ceph auth print-key client.<name>
# 显示指定用户的 secret key
$ ceph auth export client.<name> > /path/to/keyring
# 导出指定用户的 keyring 至文件
$ ceph auth import -i /path/to/keyring
# 指定用户的 keyring 导入集群

$ ceph auth caps client.<name> mon 'allow r' osd 'allow rw'
# 覆盖更新当前指定用户的现有功能
$ ceph auth caps client.<name> osd ''
# 使用空字符删除指定用户的所有功能

$ ceph auth del client.<name>
# 删除集中配置数据库中的指定用户
$ rm -f /path/to/keyring
# 删除指定用户的 keyring 文件（若存在的话）
```

## Ceph RBD 镜像

- RBD 镜像的特性（feature）如下所示：
  
  - `layering`：是否支持镜像分层、克隆（BIT 码为 1）
  
  - `exclusive-lock`：是否支持分布式独占锁机制以限制同时仅能一个客户端访问当前镜像（BIT 码为 4）
  
  - `object-map`：是否支持对象映射，主要用于加速导入、导出及已用容量统计等操作，依赖于 exclusive-lock 特性（BIT 码为 8）。
  
  - `fast-diff`：是否支持快照间的快速比较操作，依赖于 object-map 特性（BIT 码为 16）。
  
  - `deep-flatten`：是否支持克隆分离时解除在克隆镜像时创建的快照与其父镜像之间的关联，即快照扁平化操作（BIT 码为 32）。
  
  - `journaling`：是否支持日志 I/O 操作，即是否支持记录镜像的修改操作至日志对象，依赖于 exclusive-lock 特性（BIT 码为 64）。
  
  - `data-pool`：是否支持将镜像的数据对象存储于纠删码存储池，主要用于将镜像的元数据与数据放置于不同的存储池。
  
  - `striping`: 是否支持数据对象间的数据条带化（BIT 码为 2）

- 常用 RBD 镜像相关命令：
  
  ```bash
  $ rbd pool init <pool-name>
  # 初始化指定存储池存储 RBD 镜像
  $ rbd create [--id <name>] \
    --size <num>[M|G] [--object-size <num>[M|G]] \
    --image-feature=<feature1>,<feature2>,... \
    #--image-feature=exclusive-lock,journaling \  #示例
    <pool-name>/<rbd-image-name>
  # 指定 RBD 镜像大小（默认单位 MiB）、chunk 的对象大小以及支持的镜像特性以创建 RBD 镜像
  # 注意：
  #   1. 其中 layering, exclusive-lock, object-map, fast-diff, deep-flatten 为
  #      默认支持的镜像特性
  #   2. 若只需启用特定的镜像特性可使用 --image-feature 选项指定即可
  #   3. 以上示例中的 exclusive-lock 与 journaling 特性在 RBD Mirror 中启用
  
  $ rbd rm [--id <name>] <pool-name>/<rbd-image-name>
  $ rbd ls [--id <name>] --pool <pool-name>
  $ rbd info [--id <name>] <pool-name>/<rbd-image-name>
  $ rbd resize --size <number>[M|G] <pool-name>/<rbd-image-name>
  
  $ rbd map [--id <name>] <pool-name>/<rbd-image-name>
  $ rbd unmap /dev/rbdX
  $ rbd showmapped
  # 注意：
  #   1. RBD 镜像的映射于客户端上执行
  #   2. 客户端需安装 ceph-common 软件包并加载 rbd 内核模块
  
  $ fsfreeze --freeze /path/to/<mount-point>
  $ fsfreeze --unfreeze /path/to/<mount-point>
  $ rbd snap create [--id <name>] <pool-name>/<rbd-image-name>@<snap-name>
  $ rbd snap remove [--id <name>] <pool-name>/<rbd-image-name>@<snap-name>
  $ rbd snap list [--id <name>] <pool-name>/<rbd-image-name>
  $ rbd snap protect [--id <name>] <pool-name>/<rbd-image-name>@<snap-name>
  $ rbd clone [--id <name>] \
    <pool-name>/<rbd-image-name>@<snap-name> <pool-name>/<clone-name>
  # 创建基于 RBD 镜像快照的克隆
  $ rbd flatten <rbd-image-clone-name>
  #创建扁平化克隆
  
  $ blockdev --getro /path/to/device
  ```

- Ceph RBD Mirror 相关命令：
  
  RBD Mirror 的两种模式，包括 `RBD one-way mirroring` 模式（`active-backup mode`）、`RBD two-way mirroring` 模式（`active-active mode`），如下所示：
  
  ![](pictures/rbd-one-way-mirroring.jpg)
  
  ![](pictures/rbd-two-way-mirroring.jpg)
  
  🧪 示例：实现 RBD one-way mirroring 的核心命令
  
  ```bash
  ### RBD Mirror Primary 集群（pool 模式）###
  $ rbd mirror pool enable <pool-name> <mirror-mode>
  # 指定存储池启用 RBD Mirror 的指定模式（pool 模式、image 模式）
  $ rbd mirror pool enable rbd pool  #示例
  # 启用 rbd 存储池启用 RBD Mirror 的 pool 模式（池模式）
  $ rbd info rbd/image1  #示例
    rbd image 'image1':
          ...
          features: exclusive-lock, journaling
          ...
          mirroring state: enabled
          mirroring mode: journal
          mirroring global id: cccdeb7c-1ce5-4ca4-9498-1400f228bf72
          mirroring primary: true
  # 该示例显示 rbd 存储池已启用 pool 模式，并且其中的 image1 镜像可进行同步。
  
  $ rbd mirror pool peer bootstrap create \
    --site-name <site-name> <pool-name> > /path/to/mirror-bootstrap-token
  # 创建引导对等存储集群的 bootstrap token（active-passive 集群模式）
  
  ### RBD Mirror Secondary 集群（pool 模式）###
  $ ceph orch apply rbd-mirror --placement=<fqdn>
  # 指定节点启用 rbd-mirror 守护进程
  $ ceph orch apply rbd-mirror --placement=serverf.lab.example.com  #示例
  $ ceph orch ls  #示例
    ...
    rbd-mirror                   1/1  8m ago     2h   serverf.lab.example.com
    ...
  $ rbd mirror pool peer bootstrap import \
    --site-name <site-name> \
    --direction rx-only <pool-name> \
    /path/to/mirror-bootstrap-token
  # 导入 primary 集群提供的对等集群引导 token，将设置为 secondary 集群。
  # 该集群只能以接收方式（rx-only）备份同步指定存储池中的镜像
  
  $ rbd mirror pool info <pool-name>
  # 查看 RBD Mirror 指定存储池的对等信息
  $ rbd mirror pool info rbd  #示例
    Mode: pool
    Site Name: bup
  
    Peer Sites: 
  
    UUID: 763de4dc-ba66-4672-aaec-582b68cf9cf1
    Name: prod
    Direction: rx-only
    Client: client.rbd-mirror-peer
  $ rbd mirror pool status
    health: OK
    daemon health: OK
    image health: OK
    images: 1 total
        1 replaying
  # 查看 RBD Mirror 的对等状态
  # 注意：primary 集群中无法查询 RBD 镜像同步状态
  
  ### 报错示例 ###
  # RBD Mirror Secondary 集群
  $ rbd rm rbd/image1
  2023-10-07T05:14:13.113+0000 7f6c226f3700 -1 librbd::image::PreRemoveRequest: 0x564199439410 handle_exclusive_lock:
  cannot obtain exclusive lock - not removing
  Removing image: 0% complete...failed.
  rbd: error: image still has watchers
  This means the image is still open or the client using it crashed. Try again after closing/unmapping it or waiting 
  30s for the crashed client to timeout.
  # 由于 primary 集群存储池中镜像设置了分布式锁（exclusive-lock）特性，
  # 因此，secondary 集群中无法删除镜像，只能从 primary 集群中删除。
  ```
  
  🧪 示例：`rbd-nbd` 映射使用已 mirroring 的 RBD 镜像
  
  笔者环境中已部署 RBD Mirror Primary 与 Secondary 集群，两套集群中均已创建 rbd 存储池并启用 pool 模式的 RBD Mirror。primary 集群的 rbd 存储池中已创建 image1 镜像，该镜像已启用 `exclusive-lock` 与 `journaling` 特性。现将该镜像映射给客户端虚拟机作虚拟磁盘使用，如下所示：
  
  ```bash
  ### 客户端虚拟机 ###
  $ sudo dnf install -y ceph-common
  # 安装 rbd 命令
  $ sudo rbd map rbd/image1
    rbd: sysfs write failed
    RBD image feature set mismatch. You can disable features unsupported by 
    the kernel with "rbd feature disable image1 journaling"
    In some cases useful info is found in syslog - try "dmesg | tail".
    rbd: map failed: (6) No such device or address
  # rbd 映射 rbd/image1 镜像内核模块报错
  # 注意：
  #   1. 使用该命令前需同步 primary 集群的 /etc/ceph/{ceph.conf,
  #      ceph.client.admin.keyring} 文件至本地 /etc/ceph/ 目录中
  #   2. rbd 内核模块报错无法映射是由于当前内核不支持 journaling 特性。
  #      但是，mirroring 必须依赖该特性，因此使用 rbd 映射与 journaling 之间存在矛盾。
  #      目前现有的内核版本均不支持该特性！
  #   3. 可使用 rbd-nbd 替代 rbd 以完成镜像的映射。可参看文末参考链接。
  $ sudo uname -r
    4.18.0-305.el8.x86_64
  
  $ sudo dnf install -y rbd-nbd
  # 安装 rbd-nbd 软件包
  $ sudo dnf info rbd-nbd
    ...
    Name         : rbd-nbd
    Epoch        : 2
    Version      : 16.2.0
    Release      : 117.el8cp
    Architecture : x86_64
    Size         : 511 k
    Source       : ceph-16.2.0-117.el8cp.src.rpm
    Repository   : @System
    From repo    : rhceph-5-tools-for-rhel-8-x86_64-rpms
    Summary      : Ceph RBD client base on NBD
    URL          : http://ceph.com/
    License      : LGPL-2.1 and LGPL-3.0 and CC-BY-SA-3.0 and GPL-2.0 and 
                   BSL-1.0 and BSD-3-Clause and MIT
    Description  : NBD based client to map Ceph rbd images to local device
  $ sudo lsmod | egrep 'rbd|nbd'
    nbd                    49152  2
    rbd                   110592  0
    libceph               454656  1 rbd
  # 确认 rbd 与 nbd 内核模块是否加载
  $ sudo rbd-nbd map rbd/image1
    /dev/nbd1
  # 映射 rbd/image1 镜像为 /dev/nbd1
  $ sudo mkfs -t ext4 /dev/nbd1
  $ sudo mkdir /mnt/rbd
  $ sudo mount -t ext4 /dev/nbd1 /mnt/rbd
  $ sudo dd if=/dev/zero of=/mnt/rbd/test-data1 oflag=direct bs=4M count=10
    10+0 records in
    10+0 records out
    41943040 bytes (42 MB, 40 MiB) copied, 0.747552 s, 56.1 MB/s
  $ sudo ls -lh /mnt/rbd/test-data1 
    -rw-r--r--. 1 root root 40M Oct  7 23:05 /mnt/rbd/test-data1
  # 创建测试数据文件
  
  ### RBD Mirror Primary/Secondary 集群 ###
  $ rbd info rbd/image1
    rbd image 'image1':
          size 1 GiB in 256 objects
          order 22 (4 MiB objects)
          snapshot_count: 0
          id: 8569a03bc09a
          block_name_prefix: rbd_data.8569a03bc09a
          format: 2
          features: exclusive-lock, journaling
          op_features: 
          flags: 
          create_timestamp: Sat Oct  7 17:41:54 2023
          access_timestamp: Sun Oct  8 03:05:02 2023
          modify_timestamp: Sun Oct  8 03:05:02 2023  # 镜像中已写入数据
          journal: 8569a03bc09a
          mirroring state: enabled
          mirroring mode: journal
          mirroring global id: c28aca80-f176-49d0-9a7c-0f34f2380f51
          mirroring primary: true
  # 从以上 primary 与 secondary 集群的镜像信息可见数据已写入并同步
  ```
  
  🏷 其他 RBD Mirror 相关命令：
  
  ```bash
  $ rbd mirror pool disable <pool-name>
  # 指定存储池禁用 pool 模式的 RBD Mirror
  
  $ rbd feature enable <pool-name>/<rbd-image-name> <feature-name>
  # 启用 RBD 镜像的指定特性
  $ rbd feature disable <pool-name>/<rbd-image-name> <feature-name>
  # 禁用 RBD 镜像的指定特性
  ```
  
  ![](pictures/rbd-mirror-other-cmds.png)
  
  ✨ RBD Mirror 的故障转移：
  
  当 primary 集群中的 RBD 镜像变为不可用时（非集群不可用），可对 primary 集群执行镜像的降级（demote）操作，而对 secondary 集群执行镜像的升级（promote）操作。
  
  ```bash
  $ rbd mirror image demote <pool-name>/<rbd-image-name>
    Image demoted to non-primary
  # primary 集群节点：降级执行指定的 RBD 镜像
  $ rbd mirror image promote <pool-name>/<rbd-image-name>
    Image promoted to primary
  # secondary 集群节点：升级指定的 RBD 镜像
  ```

- Ceph iSCSI Gateway 相关命令：

## CephFS 文件系统

```bash
$ ceph fs status
```

- `dirty_ratio`：脏页比率
  
  物理内存的缓存数据不能超过物理内存的 `30%`，超过后将写入物理磁盘，提高磁盘性能。

- `dirty_backgroud_ratio`：
  
  物理内存使用超过 `10%` 时，将在后台将数据写入磁盘。

## 参考链接

- [Product Documentation for Red Hat Ceph Storage 5 | Red Hat Customer Portal](https://access.redhat.com/documentation/zh-cn/red_hat_ceph_storage/5)

- ❤ [5.4. 使用 Ceph Manager 负载均衡器模块 Red Hat Ceph Storage 5 | Red Hat Customer Portal](https://access.redhat.com/documentation/zh-cn/red_hat_ceph_storage/5/html/operations_guide/using-the-ceph-manager-balancer-module_ops)

- [SERVICE MANAGEMENT | Ceph Docs](https://docs.ceph.com/en/latest/cephadm/services/?highlight=service_id#)

- [使用 cephadm 搭建 ceph（octopus）过程](https://juejin.cn/post/7160585472538837006)

- [Chapter 2. Management of services using the Ceph Orchestrator Red Hat Ceph Storage 5 | Red Hat Customer Portal](https://access.redhat.com/documentation/en-us/red_hat_ceph_storage/5/html/operations_guide/management-of-services-using-the-ceph-orchestrator#deploying-the-ceph-daemons-using-the-service-specification_ops)

- [Stray daemon tcmu-runner is reported not managed by cephadm - Red Hat Customer Portal](https://access.redhat.com/solutions/6472281)

- [Ceph - mapping rbd image is failing with RBD image feature set mismatch or image uses unsupported features - Red Hat Customer Portal](https://access.redhat.com/solutions/4270092)

- [maillist - rbd map image with journaling](https://lists.ceph.io/hyperkitty/list/ceph-users@ceph.io/thread/377S7XFN74MUYKVSXXRAN534FNZTDICK/)

- [使用命令行界面更改 Ceph 仪表板密码](https://www.ibm.com/docs/zh/storage-ceph/6?topic=ia-changing-ceph-dashboard-password-using-command-line-interface)
