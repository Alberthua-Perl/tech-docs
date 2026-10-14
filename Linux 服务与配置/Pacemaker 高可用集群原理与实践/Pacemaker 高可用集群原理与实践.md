# Pacemaker 高可用集群原理与实践

## 环境说明

| 主机名 | IPv4 地址 | Linux 发行版 | vCPU | 内存 |
| ----- | ----- | ----- | ----- | ----- |
| nodea.private.example.com | 192.168.0.11/24 | RHEL8.3 | 2 | 2GiB |
| nodeb.private.example.com | 192.168.0.12/24 | RHEL8.3 | 2 | 2GiB |
| nodec.private.example.com | 192.168.0.13/24 | RHEL8.3 | 2 | 2GiB |
| noded.private.example.com | 192.168.0.14/24 | RHEL8.3 | 2 | 2GiB |

以上 192.168.0.101~104 地址为支持 IPMI 协议的带外管理口 IPv4 地址。

## 文档目录

- [Pacemaker 高可用集群原理与实践](#pacemaker-高可用集群原理与实践)
  - [环境说明](#环境说明)
  - [文档目录](#文档目录)
  - [部署配置 Pacemaker 集群](#部署配置-pacemaker-集群)
    - [放行三节点高可用防火墙](#放行三节点高可用防火墙)
    - [启用订阅频道、安装 Pacemaker 与 Fence 代理软件包](#启用订阅频道安装-pacemaker-与-fence-代理软件包)
    - [启动 Pacemaker 守护进程](#启动-pacemaker-守护进程)
    - [设置 hacluster 系统用户密码](#设置-hacluster-系统用户密码)
    - [认证集群节点](#认证集群节点)
    - [启动名为 cluster1 的三节点集群](#启动名为-cluster1-的三节点集群)
    - [创建 stonith 隔离设备](#创建-stonith-隔离设备)
    - [查看 stonith 资源状态](#查看-stonith-资源状态)
  - [添加新的集群节点](#添加新的集群节点)
  - [相关重要命令](#相关重要命令)
  - [Corosync 仲裁 vs Ceph 仲裁对比](#corosync-仲裁-vs-ceph-仲裁对比)
  - [Corosync 仲裁算法基础](#corosync-仲裁算法基础)
  - [Corosync 仲裁计算](#corosync-仲裁计算)
    - [仲裁计算选项详解](#仲裁计算选项详解)
    - [仲裁计算选项设置](#仲裁计算选项设置)
  - [隔离集群故障节点](#隔离集群故障节点)
  - [参考链接](#参考链接)

## 部署配置 Pacemaker 集群

此文档用于构建三节点 nodea、nodeb、nodec 的 Pacemaker 高可用基础集群。

### 放行三节点高可用防火墙

```bash
[root@nodea ~]# firewall-cmd --permanent --add-service=high-availability
[root@nodea ~]# firewall-cmd --reload

[root@nodeb ~]# firewall-cmd --permanent --add-service=high-availability
[root@nodeb ~]# firewall-cmd --reload

[root@nodec ~]# firewall-cmd --permanent --add-service=high-availability
[root@nodec ~]# firewall-cmd --reload
```

### 启用订阅频道、安装 Pacemaker 与 Fence 代理软件包

```bash
[root@nodea ~]# subscription-manager repos --enable=rhel-8-for-x86_64-baseos-rpms --enable=rhel-8-for-x86_64-appstream-rpms --enable=rhel-8-for-x86_64-highavailability-rpms
[root@nodea ~]# dnf install pcs fence-agents-ipmilan

[root@nodeb ~]# subscription-manager repos --enable=rhel-8-for-x86_64-baseos-rpms --enable=rhel-8-for-x86_64-appstream-rpms --enable=rhel-8-for-x86_64-highavailability-rpms
[root@nodeb ~]# dnf install pcs fence-agents-ipmilan

[root@nodec ~]# subscription-manager repos --enable=rhel-8-for-x86_64-baseos-rpms --enable=rhel-8-for-x86_64-appstream-rpms --enable=rhel-8-for-x86_64-highavailability-rpms
[root@nodec ~]# dnf install pcs fence-agents-ipmilan
```

### 启动 Pacemaker 守护进程

```bash
[root@nodea ~]# systemctl enable --now pcsd
[root@nodeb ~]# systemctl enable --now pcsd
[root@nodec ~]# systemctl enable --now pcsd
```

### 设置 hacluster 系统用户密码

```bash
[root@nodea ~]# echo "redhat" | passwd --stdin hacluster
[root@nodeb ~]# echo "redhat" | passwd --stdin hacluster
[root@nodec ~]# echo "redhat" | passwd --stdin hacluster
```

### 认证集群节点

```bash
[root@nodea ~]# pcs host auth nodea.private.example.com nodeb.private.example.com nodec.private.example.com
Username: hacluster
Password: redhat
nodea.private.example.com: Authorized
nodeb.private.example.com: Authorized
nodec.private.example.com: Authorized
```

### 启动名为 cluster1 的三节点集群

```bash
[root@nodea ~]# pcs cluster setup cluster1 --start nodea.private.example.com nodeb.private.example.com nodec.private.example.com
[root@nodea ~]# pcs cluster enable --all
nodea.private.example.com: Cluster Enabled
nodeb.private.example.com: Cluster Enabled
nodec.private.example.com: Cluster Enabled
[root@nodea ~]# pcs status
#类似：pcs cluster status 查看集群状态
```

### 创建 stonith 隔离设备

在 Pacemaker 集群中创建一个名为 fence_nodea 的 STONITH 隔离设备，类型为 fence_ipmilan（通过 IPMI 带外管理口对节点 nodea 执行强制断电/重启），用于在判定 nodea 故障时将其 "杀死"，保证资源能安全迁移、防止脑裂。

```bash
[root@nodea ~]# pcs stonith create fence_nodea fence_ipmilan pcmk_host_list=nodea.private.example.com ip=192.168.0.101 username=admin password=password lanplus=1 power_timeout=180
[root@nodea ~]# pcs stonith create fence_nodeb fence_ipmilan pcmk_host_list=nodeb.private.example.com ip=192.168.0.102 username=admin password=password lanplus=1 power_timeout=180
[root@nodea ~]# pcs stonith create fence_nodec fence_ipmilan pcmk_host_list=nodec.private.example.com ip=192.168.0.103 username=admin password=password lanplus=1 power_timeout=180
```

| 参数 | 值 | 含义 |
| ----- | ----- | ----- |
| `fence_nodea` | — | stonith 资源在 Pacemaker 中的 **自定义名字** |
| `fence_ipmilan` | — | 使用的 **fence agent**（隔离代理程序），通过 IPMI-over-LAN 协议操作服务器的 BMC（如 iDRAC/iLO/iBMC 等带外管理控制器）。 |
| `pcmk_host_list` | `nodea.private.example.com` | 指定 **该设备负责隔离哪些节点**。Pacemaker 决定隔离 nodea 时，只会调用配置了它名字的 fence 设备。名字必须与 `corosync.conf` / `pcs status` 中显示的节点名一致，否则隔离时会报 "not found"。 |
| `ip` | `192.168.0.101` | nodea 的 **BMC/带外管理口 IP 地址**（不是业务网卡 IP）。fence agent 通过这个地址连接 IPMI 服务执行电源操作 |
| `username` | `admin` | **BMC 带外管理口的登录用户名**，不是操作系统账号。 |
| `password` | `password` | **BMC 的登录密码**（示例中是弱口令，生产环境应改强密码并注意该密码会以明文存于集群配置库 CIB 中，权限要控制好）。 |
| `lanplus=1` | — | 使用 **IPMI v2.0（RMCP+ / LANplus）**，加密且支持更长的密码。现代 BMC 基本必须开；若设为 0 则走 IPMI v1.5，很多新硬件会直接连不上。 |
| `power_timeout=180` | 180 秒 | **电源操作超时时间**。执行 power off/reset 后，agent 会轮询确认节点电源状态，180 秒内未确认操作生效就判定失败，返回错误给 Pacemaker。 |

### 查看 stonith 资源状态

```bash
[root@nodea ~]# pcs stonith status
  * fence_nodea  (stonith:fence_ipmilan):   Started nodea.private.example.com
  * fence_nodeb  (stonith:fence_ipmilan):   Started nodeb.private.example.com
  * fence_nodec  (stonith:fence_ipmilan):   Started nodec.private.example.com
[root@nodea ~]# pcs stonith config
 Resource: fence_nodea (class=stonith type=fence_ipmilan)
  Attributes: ip=192.168.0.101 lanplus=1 password=password pcmk_host_list=nodea.private.example.com power_timeout=180 username=admin
  Operations: monitor interval=60s (fence_nodea-monitor-interval-60s)
 Resource: fence_nodeb (class=stonith type=fence_ipmilan)
  Attributes: ip=192.168.0.102 lanplus=1 password=password pcmk_host_list=nodeb.private.example.com power_timeout=180 username=admin
  Operations: monitor interval=60s (fence_nodeb-monitor-interval-60s)
 Resource: fence_nodec (class=stonith type=fence_ipmilan)
  Attributes: ip=192.168.0.103 lanplus=1 password=password pcmk_host_list=nodec.private.example.com power_timeout=180 username=admin
  Operations: monitor interval=60s (fence_nodec-monitor-interval-60s)
 Resource: fence_noded (class=stonith type=fence_ipmilan)
  Attributes: ip=192.168.0.104 lanplus=1 password=password pcmk_host_list=noded.private.example.com power_timeout=180 username=admin
  Operations: monitor interval=60s (fence_noded-monitor-interval-60s)

[root@nodea ~]# fence_ipmilan --lanplus --ip=192.168.0.103 --username=admin --password=password -o status
Status: ON
#手动验证 fence_ipmilan 代理是否能连接远程带外管理口
#类似的测试方法：ipmitool -H 192.168.0.103 -p 623 -U admin -P password -I lanplus chassis power status
[root@nodea ~]# fence_ipmilan --lanplus --ip=192.168.0.103 --username=admin --password=password --power-timeout=180
Success: Rebooted
#使用 fence_ipmilan 代理手动重启节点，此代理的默认行为是重启节点。
# 注意：此命令的 action 选项
# -o, --action=[action]          Action: on, off, reboot (default), status, list,
#                                        list-status, monitor, metadata, manpage,
#                                        validate-all or diag
```

## 添加新的集群节点

将 noded 节点添加为新的集群节点：

```bash
[root@noded ~]# firewall-cmd --permenant --add-service=high-availability
[root@noded ~]# firewall-cmd --reload
[root@noded ~]# subscription-manager repos --enable=rhel-8-for-x86_64-baseos-rpms --enable=rhel-8-for-x86_64-appstream-rpms --enable=rhel-8-for-x86_64-highavailability-rpms
[root@noded ~]# dnf install -y pcs fence-agents-ipmilan
[root@noded ~]# systemctl enable --now pcsd
[root@noded ~]# echo "redhat" | passwd --stdin hacluster

[root@nodea ~]# pcs host auth -u hacluster -p redhat noded.private.example.com
[root@nodea ~]# pcs cluster node add noded.private.example.com
[root@nodea ~]# pcs cluster auth
[root@nodea ~]# pcs cluster enable noded.private.example.com
[root@nodea ~]# pcs cluster start noded.private.example.com
[root@nodea ~]# pcs cluster
[root@nodea ~]# pcs stonith create fence_noded fence_ipmilan pcmk_host_list=noded.private.example.com ip=192.168.0.104 username=admin password=password lanplus=1 power_timeout=180
[root@nodea ~]# pcs stonith config
```

## 相关重要命令

pcs cluster 子命令用于操作集群，而 pcs status 子命令用于操作不同资源。

```bash
$ pcs cluster auth
#确认节点间认证情况
$ pcs cluster sync
#手动同步集群配置

$ pcs status cluster
#确认集群整体状态
$ pcs status nodes
#确认集群节点状态
$ pcs status corosync
#确认 corosync 状态
```

## Corosync 仲裁 vs Ceph 仲裁对比

| 对比维度 | Corosync 仲裁（votequorum） | Ceph 仲裁 |
| ----- | ----- | ----- |
| **解决的问题** | 集群节点故障时，**谁有权继续运行资源/服务**（防止脑裂） | 分布式存储中，**哪份数据视图/写入是权威的**（保证数据一致性） |
| **所在层** | 集群资源管理层（Pacemaker 的基础设施） | 存储系统内部，贯穿 MON、OSD、MDS |
| **算法基础** | 多数派投票（majority，> n/2 票数） | Monitor 用 **Paxos**（基于法定人数共识）；OSD 副本组用**过半数副本确认** |
| **投票主体** | 每个集群节点 1 票（可配置权重） | Monitor 集群成员（通常 3 或 5 个 MON）；OSD 则是每个副本（replica）1 票 |
| **仲裁单位** | 整个集群一个 quorum | 多层独立仲裁：**MON 层**（集群地图 monmap）+ **PG 层**（每个 PG 副本组独立选 acting set） |
| **典型规模** | 2~32 节点的小型 HA 集群 | 可达上千 OSD、少量 MON（3/5 个） |
| **脑裂防护** | 失去 quorum 的分区**自我下线/拒绝服务** | 失去多数副本的 PG 进入 **incomplete/error，拒绝 I/O**；双分区时只有有 majority 的一边可写 |
| **特殊机制** | **QDevice**：2 节点集群引入第三方投票避免平局；last man standing | **tiebreaker MON**：恰好 2 个 MON 时对 monmap 变更起决胜作用；**stretch mode** 跨站点仲裁（tiebreaker site） |
| **MON 选举** | 无（quorum 达成后由 Pacemaker 再选 DC） | 有，Paxos 选举 leader（leader 持有 paxos.lease，处理 monmap/osdmap 变更） |
| **数据一致性含义** | 不涉及数据，只决定 "资源在哪运行" | 强相关：PG 日志（pg\_log）以 majority 副本为准做 peering，保证副本间日志收敛 |
| **失去仲裁的表现** | 节点停资源、集群拒绝启动新资源；有 fence 时触发 stonith | MON 失去 quorum → 集群管理面冻结；PG 失去 majority → 该 PG 的 I/O 阻塞（inactive） |
| **恢复方式** | 节点重新加入，票数恢复过半即自动恢复 quorum | MON 恢复多数后自动恢复；OSD 副本修复（recovery/backfill）使 PG 回到 active+clean |

- Corosync 仲裁："谁能干活" —— 票数过半的一方才有权调度资源，服务于防脑裂。
- Ceph 仲裁："哪份数据算数" —— MON 用 Paxos 维护集群地图，每个 PG 用副本过半决定权威日志，服务于数据一致性。
- 两者都是 "多数派" 思想，但 Corosync 是 **<font color=red>集群可用性仲裁</font>**，Ceph 是 **<font color=red>分布式数据一致性仲裁</font>**，解决的问题层次不同，且常在生产中共存（Ceph 节点本身可跑在 Pacemaker/Corosync 管理的 HA 集群上）。

## Corosync 仲裁算法基础

Corosync 基于 **票数统计的多数派裁决**（majority quorum calculation），corosync_votequorum 的核心逻辑满足以下计算：

```plain
quorum = floor(expected_votes / 2 + 1)
```

> 注意：具体源码可参考 [exec/votequorum.c](https://github.com/corosync/corosync/blob/main/exec/votequorum.c) 中的 1033 行 `static void are_we_quorate(unsigned int total_votes)` 函数实现

`expected_votes` 为集群中应到总票数，quorum 为法定线（通过上述计算过程动态获得）。**<font color=orange>默认情况下，expected_votes 与 quorum 在集群配置中固定不改变，因此，当集群中有节点无法连接而被 fence 隔离后，只要 total_votes (每次成员变化时实时统计的在线票数总和) ≥ quorum (法定线)，那么就能完成仲裁，集群资源继续可用，但如果 total_votes < quorum，那么无法完成仲裁且无法使用集群资源。</font>**

## Corosync 仲裁计算

### 仲裁计算选项详解

- /etc/corosync/corosync.conf 配置文件扩展模式选项：
  - `two_node: 1`：2 节点集群要求 2 票全到才给 quorum（特例）
  - `wait_for_all: 1`：
    - 集群冷启动时必须等所有节点都加入过一次才授予 quorum，防止启动脑裂（split-brain）。
    - 此设置可有效防止集群启动时发生 **围栏竞态**（fence race）。
    - 若未启用此设置，一旦达到 quorum，所有未加入集群或已正常关闭的节点将被自动围栏隔离。

    > **fence race（围栏竞态）**：集群发生脑裂/通信中断时，两个节点同时互相执行 fence（隔离/踢掉对方），谁先下手谁 "赢"，甚至出现双方都成功的局面。</br>
    > **典型场景**：以 2 节点集群为例，心跳网络中断后，双方都看不到对方，都认为对方死了，双方各自启动 fence 流程，即 A 去 fence B，B 同时去 fence A，结果取决于 fence 设备（IPMI/iLO 等）哪个指令先生效 —— 这是个**时间竞态**。</br>
    > **具体危害**：最坏情况是双方都 fence 成功，两台同时被断电重启，整个集群服务全停，即使只有一方成功，"赢" 的判定也是碰运气，不可预测。fence 动作本身有延迟（IPMI 几秒到几十秒），竞态窗口很真实。

  - `auto_tie_breaker: 1`：
    - 仅需集群节点的 50% 处于在线状态，即可达到就绪条件，而无需默认的 50% 加 1 票。
    - 在出现 50% 对 50% 脑裂的情况时，**最低节点 ID** 的主机将获得就绪权。
  - `last_man_standing: 1`：**LMS**
    - 系统默认每 10 秒重新计算一次预期的投票数（expected_votes），重新计算的时间间隔由 last_man_standing_window 选项设置，默认设置为 10000 毫秒（即 10 秒）。
    - 设置此选项后，节点逐个离开时动态下调 quorum，允许最后剩下的少数节点保留 quorum。
    - 这使系统管理员能够逐个断开集群节点，直到仅剩两个节点在集群中活跃参与。
    - 💥 请注意，如果断开集群节点过快，即使有两台或更多节点，也可能导致失去多数同意（quorum）。
    - 比如，5 台集群节点，quorum 为 3，last_man_standing 启用的状态下，可能有以下 3 种情况出现：
      - inquorate 不足法定人数：在 10 秒内（last_man_standing_window 默认为 10 秒）一次性关闭 3 台节点，这样导致 expected_votes 依然维持在 5，quorum 依然为 3，而此时的 total_votes 为 2，导致 **total_votes < quorum**，无法完成仲裁导致集群资源不可用。
      - quorate 满足法定人数：在 10 秒内，3 台节点重新上线，expected_votes 为 5，quorum 为 3，total_votes 为 5，**total_votes > quorum**，集群保持原来配置，不再仲裁，集群资源可用。
      - quorate 满足法定人数：在 10 秒后，3 台节点被隔离，expected_votes 动态下调至 2，quorum 动态下调至 2，此时 **total_votes = quorum**，集群可完成仲裁，集群资源重新可用。
- 扩展模式选项搭配：
  - wait_for_all + last_man_standing：防止多个分区（集群的子集）同时获取多数投票权
  - auto_tie_breaker + last_man_standing：可将集群降级为 **单个节点**
- 集群中每个节点的 `/etc/corosync/corosync.conf` 文件内容必须相同。
- 自动更新配置文件：`pcs quorum update` 命令会先在本地更新该文件，然后自动与所有节点同步，即使集群已停止运行。
- 手动编辑配置文件：Red Hat 不建议这样做，然后使用 `pcs cluster sync` 命令将文件复制到所有节点。
- 同步完 corosync 配置文件后，可再次使用 `pcs cluster start --all` 启动集群。

### 仲裁计算选项设置

```bash
$ pcs cluster stop --all
#设置仲裁计算选项时必须停止集群

$ pcs quorum config
Options:
#查看集群仲裁配置信息

$ pcs quorum update wait_for_all=1 last_man_standing=1
  Checking corosync is not running on nodes...
  nodeb.private.example.com: corosync is not running
  nodec.private.example.com: corosync is not running
  nodea.private.example.com: corosync is not running
  noded.private.example.com: corosync is not running
  Sending updated corosync.conf to nodes...
  nodeb.private.example.com: Succeeded
  nodec.private.example.com: Succeeded
  nodea.private.example.com: Succeeded
  noded.private.example.com: Succeeded

$ pcs quorum config
  Options:
    last_man_standing: 1
    wait_for_all: 1

$ pcs cluster start --all
  nodeb.private.example.com: Starting Cluster...
  nodec.private.example.com: Starting Cluster...
  noded.private.example.com: Starting Cluster...
  nodea.private.example.com: Starting Cluster...

$ pcs quorum status
#查看集群仲裁状态（集群必须运行）
  Quorum information
  ------------------
  Date:             Mon Oct 12 09:08:10 2026
  Quorum provider:  corosync_votequorum
  Nodes:            4
  Node ID:          1
  Ring ID:          1.27
  Quorate:          Yes

  Votequorum information
  ----------------------
  Expected votes:   4
  Highest expected: 4
  Total votes:      4
  Quorum:           3  
  Flags:            Quorate WaitForAll LastManStanding 

  Membership information
  ----------------------
      Nodeid      Votes    Qdevice Name
           1          1         NR nodea.private.example.com (local)
           2          1         NR nodeb.private.example.com
           3          1         NR nodec.private.example.com
           4          1         NR noded.private.example.com

  $ corosync-quorumtool status
  #以 corosync 子命令直接查询 pacemakers 的规则

  $ corosync-cfgtool -s
  #查看 corosync 环（ring）状态和各节点通信是否正常
  ```

## 隔离集群故障节点

STONITH（全称 Shoot The Other Node In the Head）fence 隔离故障节点依赖 fence 设备与 fence 代理。

```bash
$ pcs stonith list
#查看 fence 代理状态
$ pcs stonith describe <fence_agent>
#查看指定 fence 代理的具体参数
$ pcs stonith describe fence_ipmilan
fence_ipmilan - Fence agent for IPMI

fence_ipmilan is an I/O Fencing agentwhich can be used with machines controlled by IPMI.This agent calls support software ipmitool (http://ipmitool.sf.net/). WARNING! This fence agent might report success before the node is powered off. You should use -m/method onoff if your fence device works correctly with that option.

Stonith options:
  auth: IPMI Lan Auth type.
  cipher: Ciphersuite to use (same as ipmitool -C parameter)
  hexadecimal_kg: Hexadecimal-encoded Kg key for IPMIv2 authentication
  ip: IP address or hostname of fencing device
  ipport: TCP/UDP port to use for connection with device
  lanplus: Use Lanplus to improve security of connection
  method: Method to fence
  ...
#查看名为 fence_ipmilan 代理的具体参数
```

```bash
### 创建 fence 设备 ###
$ pcs stonith create <fence_device_name> <fencing_agent> [fencing_parameters]    #每种不同的 fence 代理具有不同的参数
# fence_device_name：指定的 STONITH fence 设备
# fencing_agent：指定 fence 设备使用的 fence 代理
# fencing_parameters：指定 fence 代理参数

### 更新 fence 设备 ###
$ pcs stonith update <fence_device_name> <fencing_agent> [fencing_parameters]
#指定 fence 代理参数更新 fence 设备

### 删除 fence 设备 ###
$ pcs stonith delete <fence_device_name>

### 测试 fence 设备 ###
$ pcs stonith fence <fence_node_name>
$ pcs stonith fence noded.private.example.com    #fence_ipmilan 代理：默认重启节点，查看节点是否可自动重新加入集群。
Node: noded.private.example.com fenced
#将 noded.private.example.com 节点围栏隔离，具体隔离的方式取决于 fence 代理的设置。
# 注意：隔离指定的节点时，不能在此节点上执行此命令，需要在其他节点上运行。

$ pcs stonith status
    * fence_custom        (stonith:fence_rh436):   Started nodeb.private.example.com
#查看 stonith fence 设备状态
$ pcs stonith config [<fencing_device_name>]
pcs stonith config fence_custom
 Resource: fence_custom (class=stonith type=fence_rh436)
  Attributes: ip=192.168.0.100 password=password pcmk_host_map=nodea.private.example.com:1;nodeb.private.example.com:2;nodec.private.example.com:3 power_timeout=180 username=admin
  Operations: monitor interval=60s (fence_custom-monitor-interval-60s)
#查看 stonith fence 设备的具体配置信息
```

## 参考链接

- [Corosync](https://corosync.github.io/corosync/)
- [The Corosync Cluster Engine | GitHub](https://github.com/corosync)