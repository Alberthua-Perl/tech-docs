## OpenStack Newton 版网络模型与架构概述

### 文档说明：

- 该文档基于 `Red Hat OpenStack Platform 10.0.5` 环境进行验证，其上游社区版为 `OpenStack Newton`。

- 该 OpenStack 环境中使用 `Neutron agent` 作为 L2 与 L3 的服务代理，未使用 `OVN` 作为网络控制平面。

### 文档目录：

- Neutron 网络架构

- Neutron HA 架构

- Neutron LBaaS 部署与实现

- OpenStack 的 VLAN 网络类型

- OpenStack 网络模型分类

- OpenStack 网络排查常用命令

- OpenStack Neutron agent 网络拓扑示意

- 使用 VLAN 网络类型的实例与外部网络通信流表分析

- 使用 GRE 隧道网络的实例与外部网络通信流表分析

- Neutron 中心化与分布式虚拟路由器的使用场景

- 分布式虚拟路由器的实现方式

### Neutron 网络架构：

- Neutron 向消息队列中读取与写入消息，这些消息被 OpenStack 中的其他组件使用。

- Neutron 使用在 `Keystone` 中注册（registered）的端点（`endpoint`）为其他的服务与用户来发现 Neutron 服务。

- 🤘 Neutron 核心模型基于由 L2 层网络隔离的实例，通过提供 NAT 服务的 L3 层网关相互联系，这种模型的好处在于对于需连接至外部网络的实例具有连接的内部安全性。

- Neutron 使用不同的插件（`plug-ins`）来提高连通性与集成。

- Neutron 支持的插件包括：
  
  - 🚀 `Neutron ML2 framework` 允许 OpenStack 网络可同时使用多个 L2 层网络技术，如：
    
    - `Open vSwitch`
    
    - Linux bridge
    
    - `OpenDaylight`
    
    - L2 Population
    
    - `SR-IOV`
  
  - `VPNaaS plugin`：允许创建 VPN 隧道
  
  - `FWaaS plugin`：使用 `iptables` 提供防火墙服务
  
  - `LBaaS plugin`：使用 `HAProxy` 提供负载均衡服务

- Neutron 核心服务的各节点分布：
  
  - controller 节点：
    
    `neutron-server`：Neutron Server 核心守护进程提供 API 服务
  
  - network 节点（controller 节点可兼容）：
    
    - `neutron-*-agent`（L2 agent）：
      
      - 默认为 `neutron-openvswitch-agent`
      
      - L2 agent 使用 `RPC` 与 neutron-server 通信
      
      - 允许 Neutron 集成 Open vSwitch 与管理 L2 层网络
    
    - `neutron-dhcp-agent`：
      
      - DHCP agent 由租户（tenant）网络使用 DHCP 服务
      
      - 🚀 DHCP agent 与 OpenStack 网络服务使用 `RPC` 进行通信。
      
      - DHCP agent 通过 L2 agent 与租户网络相连，两者必须同时部署安装。
      
      - `/etc/neutron/dhcp_agent.ini` 配置文件中可定义 `interface_driver` 与 `dhcp_driver`，如下所示：
        
        ```ini
        [DEFAULT]
        interface_driver = neutron.agent.linux.interface.OVSInterfaceDriver
        # 用于管理虚拟接口的驱动
        resync_interval = 30
        # DHCP agent 与 Neutron 间从 RPC 错误恢复的重新同步时间间隔（秒） 
        dhcp_driver = neutron.agent.linux.dhcp.Dnsmasq
        # 管理 DHCP 服务器的驱动
        ...
        ```
      
      - DHCP agent 的高可用示意：
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/neutron-dhcp-agent-ha.jpg)
    
    - `neutron-l3-agent`：
      
      - L3 agent 提供 3 层网络与租户网络间的互连
      
      - 实现 Linux namespace 的管理、NAT 与外部网络互连
  
  - compute 节点：
    
    `neutron-*-agent`（L2 agent）：默认为 `neutron-openvswitch-agent`

- Neutron agent 组件概述：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/neutron-service-elements.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/neutron-architecture-2.png)
  
  👉 `ML2` 插件、`L2 agent`、`L3 agent` 与 `DHCP agent` 之间的关系：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/neutron-component-2.jpg)

- Neutron 配置文件：
  
  - ML2 插件主配置文件：
    
    `/etc/neutron/plugins/ml2/ml2_conf.ini` 位于控制节点、计算节点
    
    ```bash
    $ grep -Ev '^$|^#' /etc/neutron/plugins/ml2/ml2_conf.ini
    [DEFAULT]
    [ml2]
    type_drivers = vxlan,vlan,flat,gre
    tenant_network_types = vxlan,gre,vlan
    mechanism_drivers = openvswitch,l2population
    extension_drivers = qos,port_security
    path_mtu = 0
    [ml2_type_flat]
    flat_networks = datacentre
    # Neutron 中定义外部网络名称，使用 flat 网络类型。
    [ml2_type_geneve]
    [ml2_type_gre]
    tunnel_id_ranges = 1:4094
    [ml2_type_vlan]
    network_vlan_ranges = datacentre:1:1000
    # Neutron 中定义外部网络名称，使用 vlan 网络类型，其 vlan id 范围为 1~1000。
    # 在创建网络时若使用 --provider-network-type vlan 选项，将从该范围中选取 
    # vlan id，由 ML2 机制驱动实现。
    [ml2_type_vxlan]
    vni_ranges = 1:4094
    vxlan_group = 224.0.0.1
    [securitygroup]
    ```
  
  - Neutron 主配置文件：
    
    `/etc/neutron/neutron.conf` 位于控制节点、计算节点
  
  - 插件配置文件示例：
    
    - `/etc/neutron/plugins/ml2/openvswitch_agent.ini` 位于控制节点、计算节点
      
      ```bash
      $ grep -Ev '^$|^#' /etc/neutron/plugins/ml2/openvswitch_agent.ini 
      [DEFAULT]
      [agent]
      tunnel_types = vxlan,gre
      vxlan_udp_port = 4789
      l2_population = True
      arp_responder = True
      enable_distributed_routing = True
      drop_flows_on_start = False
      extensions = qos
      [ovs]
      integration_bridge = br-int
      tunnel_bridge = br-tun
      local_ip = 172.24.2.1
      bridge_mappings = datacentre:br-ex
      # OVS 将 Neutron 中定义的外部网络名称与 ovs br-ex 网桥进行映射
      [securitygroup]
      firewall_driver = neutron.agent.linux.iptables_firewall.OVSHybridIptablesFirewallDriver
      ```
    
    - `/etc/neutron/plugins/ml2/sriov_agent.ini` 位于控制节点、计算节点
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/neutron-conf-plugin-short-name.jpg)
  
  - 示例：
    
    👉 计算节点上使用 `journalctl -ef` 命令在实例创建的实时日志中相关的虚拟网络接口的创建信息如下所示：
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute-node-create-qbr-qvb-qvo.jpg)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute-node-create-tap-device.jpg)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute-node-create-instance-and-vxlan-device.jpg)
    
    以上实时日志信息在 `/var/log/messages` 中可见，而关于 neutron-openvswitch-agent 的日志在 `/var/log/neutron/openvswitch-agent.log` 中可见。
    
    👉 更改指定计算节点的 neutron-openvswitch-agent 配置文件 `/etc/neutron/plugins/ml2/openvswitch_agent.ini` 可在单独节点上测试特定功能，如下所示，更改安全组的防火墙驱动为 openvswitch，该驱动处于技术预览阶段，即将实例的网口直连 OVS br-int 集成网桥，不再通过 Linux 网桥实现安全组，此种方式与 OVN 架构的实例连接方式相似。
    
    ```ini
    $ vim /etc/neutron/plugins/ml2/openvswitch_agent.ini
    ...
    [securitygroup]
    
    #
    # From neutron.ml2.ovs.agent
    #
    
    # Driver for security groups firewall in the L2 agent (string value)
    #firewall_driver = <None>
    #firewall_driver = neutron.agent.linux.iptables_firewall.OVSHybridIptablesFirewallDriver
    firewall_driver = openvswitch
    ...
    
    $ systemctl restart neutron\*
    # 重启 Neutron 相关的服务
    ```
    
    计算节点上实时日志显示只创建与 OVS br-int 集成网桥直连的 tun 设备。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/change-openvswitch-agent-securitygroup-firewall-driver-log.jpg)

### 🤘 Neutron HA 架构：

- 当前 Red Hat OpenStack Platform 中默认的路由（routing）模型为 `L3-HA`，即将路由功能分布到多个网络节点上，以解决单个网络节点的流量负载问题。

- 除了 `L3-HA` 模型外，OpenStack 集群也支持分布式路由器模型（`DVR, distributed router`），即在各个计算节点上部署 `L3 agent` 与路由器（`qrouter`）以降低集中式网络节点的流量压力与流量优化。

- 🔥 L3-HA 模型与 DVR 模型在同一集群中无法同时部署，而集中式网络节点可与 DVR 同时部署！

- 在 DVR 作为默认路由模式时，若要创建集中式路由，可使用如下命令：
  
  ```bash
  $ neutron router-create --distributed False <router_name>
  # 在租户中创建集中式路由器（非 DVR），位于控制节点或网络节点，以 qrouter 的形式运行。
  # 该命令暂不支持 openstack 命令
  ```

### Neutron LBaaS 部署与实现：

- Neutron LBaaS 服务上游项目称为 `Octavia`。

- Neutron LBaaS 服务逻辑组件：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/openstack-octavia-arch-1.jpg)

- Neutron LBaaS 服务的部署拓扑示意：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/openstack-octavia-lbaas-demo.jpg)

- LBaaS 负载均衡服务部署示例：
  
  finance 项目（租户）中存在两个实例，其 IP 分别为 192.168.1.3/24 与 192.168.1.5/24，租户内路由器为 192.168.1.1，现为该项目配置前端 LBaaS 负载均衡器。
  
  ```bash
  # overcloud: root@controller0
  $ systemctl status neutron-lbaasv2-agent.service
  ● neutron-lbaasv2-agent.service - OpenStack Neutron Load Balancing as a Service (API v2.x) Agent
     Loaded: loaded (/usr/lib/systemd/system/neutron-lbaasv2-agent.service; enabled; vendor preset: disabled)
     Active: active (running) since Sat 2022-08-13 02:48:16 UTC; 1 day 13h ago
     Main PID: 880620 (neutron-lbaasv2)
     CGroup: /system.slice/neutron-lbaasv2-agent.service
             ├─479363 sudo neutron-rootwrap-daemon /etc/neutron/rootwrap.conf
             ├─479364 /usr/bin/python2 /usr/bin/neutron-rootwrap-daemon /etc/neutron/rootwrap.conf
             ├─880620 /usr/bin/python2 /usr/bin/neutron-lbaasv2-agent --config-file /usr/share/neutron/neutron-dist.conf --config-file /usr/share/neutron/neutron-lbaas-dist.conf --config...   
             └─943530 haproxy -f /var/lib/neutron/lbaas/v2/366c088c-1a4c-4e23-969d-8f5bf5fa3717/haproxy.conf -p /var/lib/neutron/lbaas/v2/366c088c-1a4c-4e23-969d-8f5bf5fa3717/haproxy.pid...
  ...
  # 控制节点上运行的 neutron-lbaasv2-agent 服务
  
  $ neutron lbaas-loadbalancer-create --name finance-lb1 finance-subnet1
  # 对项目中的子网创建负载均衡器
  $ neutron lbaas-listener-create --loadbalancer finance-lb1 \
    --protocol HTTP --protocol-port 80 --name finance-listener1
  # 创建负载均衡监听器  
  $ neutron lbaas-pool-create --lb-algorithm ROUND_ROBIN \
    --listener finance-listener1 --protocol HTTP \
    --name finance-pool1
  # 指定负载均衡算法创建负载均衡池，可向该池中添加后端的实例。
  $ neutron lbaas-member-create --subnet finance-subnet1 \
    --address 192.168.1.P --protocol-port 80 \
    finance-pool1
  # 将租户子网中的后端实例添加至负载均衡池中  
  $ neutron lbaas-healthmonitor-create \
    --delay 5 --type HTTP --max-retries 3 --timeout 2 \
    --pool finance-pool1 \
    --name finance-health1
  # 创建负载均衡器的监控器
  
  $ neutron lbaas-loadbalancer-show finance-lb1
  +---------------------+------------------------------------------------+
  | Field               | Value                                          |
  +---------------------+------------------------------------------------+
  | admin_state_up      | True                                           |
  | description         |                                                |
  | id                  | 366c088c-1a4c-4e23-969d-8f5bf5fa3717           |
  | listeners           | {"id": "ddef58b4-a596-4089-80af-190d7ce88034"} |
  | name                | finance-lb1                                    |
  | operating_status    | ONLINE                                         |
  | pools               | {"id": "201a9582-d038-46a7-a094-87f04afa386b"} |
  | provider            | haproxy                                        |
  | provisioning_status | ACTIVE                                         |
  | tenant_id           | ac899ae13143483480041d63f0a4c126               |
  | vip_address         | 192.168.1.14                                   |
  | vip_port_id         | 777826fc-a8a1-403d-b7aa-7f349de16f16           |
  | vip_subnet_id       | a4dcf7fd-bd04-4cba-8752-c4f8685a9b76           |
  +---------------------+------------------------------------------------+
  
  $ neutron floatingip-list -c id -c floating_ip_address -c port_id
  # 查看当前可分配的浮动 IP 地址
  $ neutron floatingip-associate <floatingip_id> <vip_port_id>
  # 将浮动 IP 分配至负载均衡器的 VIP 端口上
  $ neutron floatingip-list
  +--------------------------------------+------------------+---------------------+--------------------------------------+
  | id                                   | fixed_ip_address | floating_ip_address | port_id                              |
  +--------------------------------------+------------------+---------------------+--------------------------------------+
  | 0ff05ecc-653c-41b3-9ed9-34f7bbc5158c | 192.168.1.14     | 172.25.250.115      | 777826fc-a8a1-403d-b7aa-7f349de16f16 |  
  ...
  # 查看负载均衡器 VIP 被分配的浮动 IP
  
  $ neutron lbaas-listener-show finance-listener1
  +---------------------------+------------------------------------------------+
  | Field                     | Value                                          |
  +---------------------------+------------------------------------------------+
  | admin_state_up            | True                                           |
  | connection_limit          | -1                                             |
  | default_pool_id           | 201a9582-d038-46a7-a094-87f04afa386b           |
  | default_tls_container_ref |                                                |
  | description               |                                                |
  | id                        | ddef58b4-a596-4089-80af-190d7ce88034           |
  | loadbalancers             | {"id": "366c088c-1a4c-4e23-969d-8f5bf5fa3717"} |
  | name                      | finance-listener1                              |
  | protocol                  | HTTP                                           |
  | protocol_port             | 80                                             |
  | sni_container_refs        |                                                |
  | tenant_id                 | ac899ae13143483480041d63f0a4c126               |
  +---------------------------+------------------------------------------------+    
  # 查看创建的负载均衡监听器的详细信息
  $ neutron lbaas-pool-show finance-pool1
  +---------------------+------------------------------------------------+
  | Field               | Value                                          |
  +---------------------+------------------------------------------------+
  | admin_state_up      | True                                           |
  | description         |                                                |
  | healthmonitor_id    | 85e7292a-8337-48ee-95e0-dee72162604a           |
  | id                  | 201a9582-d038-46a7-a094-87f04afa386b           |
  | lb_algorithm        | ROUND_ROBIN                                    |
  | listeners           | {"id": "ddef58b4-a596-4089-80af-190d7ce88034"} |
  | loadbalancers       | {"id": "366c088c-1a4c-4e23-969d-8f5bf5fa3717"} |
  | members             | eccdc1cc-e5dd-4e6e-82d4-53c2074162a7           |
  |                     | 0ebc9bc6-1d25-4231-9586-f12eb127853f           |
  | name                | finance-pool1                                  |
  | protocol            | HTTP                                           |
  | session_persistence |                                                |
  | tenant_id           | ac899ae13143483480041d63f0a4c126               |
  +---------------------+------------------------------------------------+  
  # 查看创建的负载均衡池的详细信息
  $ neutron lbaas-member-list finance-pool1
  +--------------------------------------+------+-------------+---------------+--------+--------------------------------------+----------------+
  | id                                   | name | address     | protocol_port | weight | subnet_id                            | admin_state_up |
  +--------------------------------------+------+-------------+---------------+--------+--------------------------------------+----------------+
  | eccdc1cc-e5dd-4e6e-82d4-53c2074162a7 |      | 192.168.1.3 |            80 |      1 | a4dcf7fd-bd04-4cba-8752-c4f8685a9b76 | True           |
  | 0ebc9bc6-1d25-4231-9586-f12eb127853f |      | 192.168.1.5 |            80 |      1 | a4dcf7fd-bd04-4cba-8752-c4f8685a9b76 | True           |
  +--------------------------------------+------+-------------+---------------+--------+--------------------------------------+----------------+   
  # 查看负载均衡池中的实例成员
  
  $ curl http://172.25.250.115
  finance-server2
  $ curl http://172.25.250.115
  finance-server1
  $ curl http://172.25.250.115
  finance-server2
  $ curl http://172.25.250.115
  finance-server1
  # 该租户中已运行 2 个后端实例，使用该负载均衡器时将 http 请求以轮询的方式代理至实例。 
  ```

- LBaaS 负载均衡服务的实现：
  
  👉 集群外部数据包请求集群内实例的过程：controller0 节点上的数据流向
  
  `packets -> eth2(br-ex) -> phy-br-ex(br-ex) -> int-br-ex(br-int) -> qg-152e42fb-8f(qrouter) -> iptables(qrouter) -> qr-4041a3c8-fa(qrouter) -> tap777826fc-a8(qlbaas) -> patch-tun(br-int) -> ...`
  
  ```bash
  # overcloud: root@controller0
  $ ip netns exec qrouter-54e7e5d1-1204-4de8-8590-0c0264983982 ip a s
  1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
      link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
      inet 127.0.0.1/8 scope host lo
         valid_lft forever preferred_lft forever
      inet6 ::1/128 scope host 
         valid_lft forever preferred_lft forever
  59: qr-4041a3c8-fa: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1446 qdisc noqueue state UNKNOWN qlen 1000
      link/ether fa:16:3e:e7:78:2d brd ff:ff:ff:ff:ff:ff
      inet 192.168.1.1/24 brd 192.168.1.255 scope global qr-4041a3c8-fa
         valid_lft forever preferred_lft forever
      inet6 fe80::f816:3eff:fee7:782d/64 scope link 
         valid_lft forever preferred_lft forever
  63: qg-152e42fb-8f: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1496 qdisc noqueue state UNKNOWN qlen 1000
      link/ether fa:16:3e:ff:93:7a brd ff:ff:ff:ff:ff:ff
      inet 172.25.250.180/24 brd 172.25.250.255 scope global qg-152e42fb-8f
         valid_lft forever preferred_lft forever
      inet 172.25.250.115/32 brd 172.25.250.115 scope global qg-152e42fb-8f
         valid_lft forever preferred_lft forever
      inet6 2001:db8::180/64 scope global 
         valid_lft forever preferred_lft forever
      inet6 fe80::f816:3eff:feff:937a/64 scope link 
         valid_lft forever preferred_lft forever 
  # 查看指定租户的 qrouter 路由器信息，包括对外的网关地址、分配的浮动 IP 与对内的网关地址等。
  
  $ ip netns exec qrouter-54e7e5d1-1204-4de8-8590-0c0264983982 iptables -t nat -nvL neutron-l3-agent-PREROUTING
  Chain neutron-l3-agent-PREROUTING (1 references)
   pkts bytes target     prot opt in     out     source               destination         
     54  3240 REDIRECT   tcp  --  qr-+   *       0.0.0.0/0            169.254.169.254      tcp dpt:80 redir ports 9697
     13   852 DNAT       all  --  *      *       0.0.0.0/0            172.25.250.115       to:192.168.1.14 
  # 通过 qrouter 路由器中的 DNAT 规则将流量重定向至 192.168.1.14 VIP，该 VIP 位于 qlbaas namespace 中。
  
  $ ip netns exec qlbaas-366c088c-1a4c-4e23-969d-8f5bf5fa3717 ip a s
  1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
      link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
      inet 127.0.0.1/8 scope host lo
         valid_lft forever preferred_lft forever
      inet6 ::1/128 scope host 
         valid_lft forever preferred_lft forever
  73: tap777826fc-a8: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1446 qdisc noqueue state UNKNOWN qlen 1000
      link/ether fa:16:3e:dc:49:5d brd ff:ff:ff:ff:ff:ff
      inet 192.168.1.14/24 brd 192.168.1.255 scope global tap777826fc-a8
         valid_lft forever preferred_lft forever
      inet6 fe80::f816:3eff:fedc:495d/64 scope link 
         valid_lft forever preferred_lft forever 
  # 负载均衡器的 VIP 地址
  ```

### OpenStack 的 VLAN 网络类型：

- OpenStack 中的 VLAN 网络类型：
  
  - `local VLAN` (br-int)：
    
    - 该 VLAN 类型只在本地节点生效用于管理本地不同租户间的 VLAN tag 标签。
    
    - 不同节点间不共享 VLAN tag，各节点具有独立的租户对应的 tag。
  
  - `internal VLAN` (br-vlan)：
    
    跨节点间的 VLAN 通信，通常包含一个或多个租户的 VLAN trunk 流量。
  
  - `external VLAN` (br-ex)：
    
    将内部租户的 VLAN trunk 流量定向至集群外部的交换机上。

- 如下所示，三种 VLAN 网络类型在集群中的分布：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/external-internal-local-boundaries.jpg)

### OpenStack 网络模型分类：

- 1️⃣ 自服务网络（self-service network）：使用隧道网络（overlay network）通信
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/vxlan-self-service-network-on-compute-node.jpg)

- 2️⃣ 供应商网络（provider network）：租户内实例直接使用供应商网络与外部网络连接
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/provider-network-on-compute-node.jpg)

### OpenStack 网络排查常用命令：

- Linux network namespace 故障排查相关命令：
  
  ```bash
  $ virsh list --all
  $ virsh dumpxml <domain>
  # 查看 KVM 虚拟机的具体配置信息
  $ virsh console <domain>
  # 本地打开 KVM 虚拟机的控制台
  
  $ brctl show
  $ vim /usr/share/doc/bridge-utils-1.5/HOWTO
  # 查看 Linux Bridge 的使用文档
  
  $ ip -d link show
  # 查看网卡设备的具体链路层信息
  $ ip netns list
  $ ip netns exec ip a s
  $ ip netns exec iptables -t nat -nvL
  $ ip netns exec ps -ef | grep dnsmasq
  ```

- OpenStack 与 Neutron 故障排查相关命令：
  
  ```bash
  $ openstack server create \
    --image <image_name> \
    --nic net-id=<network_name> \
    --key-name <keypair_name> \
    --flavor <flavor_name> \
    --availability-zone nova:compute0.overcloud.example.com \ 
    --wait <instance>
  # 使用租户（项目）管理员指定计算节点创建实例
  
  $ openstack console url show <instance>
  # 查看实例的 URL 访问地址
  $ openstack console log show <instance>
  # 查看实例的控制台日志输出
  
  $ openstack network create <network_name>
  # （默认）创建 OpenStack 租户网络
  $ openstack subnet create \
    --network research-network1 \
    --subnet-range=192.168.1.0/24 \
    --dns-nameserver=172.25.250.254 \
    --dhcp \
    research-subnet1
  # 创建 OpenStack 租户网络的子网
  # 注意：
  #   --dhcp 选项：
  #     子网中创建 qdhcp namespac（qdhcp-<network_id>）并为实例分配 IP
  #   --no-dhcp 选项：
  #     子网中不创建 qdhcp namespace 且不为实例分配 IP，但在 openstack 命令
  #     行中可查看实例被分配的 IP 地址，但在实例对应的网口上无分配的 IP。
  $ neutron router-create --distributed False <router_name>
  $ neutron router-gateway-set <router_name> <external_network_name>
  $ openstack router add subnet <router_name> <subnet_name>
  # 创建 non-DVR 模式的路由器，并将路由器设置为对外网关，将子网附加至路由器上。
  $ openstack port list --router <router_name>
  # 查看 non-DVR (legacy) 与 DVR 路由器的端口信息
  # 创建 DVR 路由器也将同时创建运行于控制节点的 snat namespace
  $ openstack port list \
    --router <router_name> --device-owner network:router_gateway
  $ openstack port list \
    --router <router_name> --device-owner network:router_centralized_snat
  $ openstack port list \
    --router <router_name> --device-owner network:router_interface_distributed
  # 查看 non-DVR (legacy) 与 DVR 路由器指定端口类型的端口信息
  
  $ openstack network agent list \
    -c 'Agent Type' -c Host -c Alive -c State -c Binary
  +----------------------+-----------------------------------+-------+-------+---------------------------+
  | Agent Type           | Host                              | Alive | State | Binary                    |
  +----------------------+-----------------------------------+-------+-------+---------------------------+
  | L3 agent             | controller0.overcloud.example.com | True  | UP    | neutron-l3-agent          |
  | DHCP agent           | controller0.overcloud.example.com | True  | UP    | neutron-dhcp-agent        |
  | Metadata agent       | compute1.overcloud.example.com    | True  | UP    | neutron-metadata-agent    |
  | Open vSwitch agent   | compute1.overcloud.example.com    | True  | UP    | neutron-openvswitch-agent |
  | Loadbalancerv2 agent | controller0.overcloud.example.com | True  | UP    | neutron-lbaasv2-agent     |
  | Open vSwitch agent   | compute0.overcloud.example.com    | True  | UP    | neutron-openvswitch-agent |
  | L3 agent             | compute0.overcloud.example.com    | True  | UP    | neutron-l3-agent          |
  | Metadata agent       | controller0.overcloud.example.com | True  | UP    | neutron-metadata-agent    |
  | Open vSwitch agent   | controller0.overcloud.example.com | True  | UP    | neutron-openvswitch-agent |
  | L3 agent             | compute1.overcloud.example.com    | True  | UP    | neutron-l3-agent          |
  | Metadata agent       | compute0.overcloud.example.com    | True  | UP    | neutron-metadata-agent    |
  +----------------------+-----------------------------------+-------+-------+---------------------------+
  # 查看 OpenStack 集群中各节点 Neutron agent 的运行状态
  $ openstack network show <network_name> | grep segmentation_id
  # 查看 OpenStack 外部或内部（租户）网络的 segmentation_id 值（VNID）
  # 该值默认以十进制输出，可将其转化为十六进制。
  $ openstack port list
  # 查看 Neutron 管理的端口 ID、MAC 地址、IP 地址与所在的子网
  $ printf "%x\n" <number>
  # 转换十进制为十六进制
  ```

- Open vSwitch 故障排查相关命令：
  
  ```bash
  $ vim /usr/share/doc/openvswitch
  # 查看 Open vSwitch 的使用文档
  $ ovs-vsctl show
  $ ovs-ofctl dump-ports-desc <br-int|br-tun|br-ex>
  # 查看指定 OVS 网桥的端口信息
  
  $ man 7 pcap-filter
  # 查看 tcpdump 抓包的包过滤语法
  $ tcpdump -n -vv -e -i <eth1|vlan20> 'host 172.24.2.12' \
    -w vxlan-capture-test.pcap
  # 指定网口抓包来自于 172.24.2.12 主机的数据包并将其写入指定 .pcap 文件中
  $ tcpdump -n -vv -e -i <eth1|vlan20> proto GRE && proto ICMP
  # 指定网口抓取由外层 GRE 隧道封装的内层 ICMP 数据包 
  
  $ ovs-ofctl dump-flows <br-int|br-tun|br-ex>
  # 查看指定 OVS 网桥的 OpenFlow 规则
  $ ovs-ofctl dump-flows <br-int|br-tun|br-ex> table=<number>
  # 查看指定 OVS 网桥中相应 table 的 OpenFlow 规则
  $ watch -d -i <second> ovs-ofctl dump-flows <br-int|br-tun|br-ex>
  # 实时查看指定 OVS 网桥的 OpenFlow 规则变化（常用于定位具体的 OpenFlow 规则）
  $ ovs-appctl fdb/show br-int
  # 查看 OVS br-int 网桥的转发数据库（forwarding database）
  ```

### ✨ OpenStack Neutron agent 网络拓扑示意：

- finance 项目中创建的中心化与分布式逻辑路由器及与其连接的多个租户内子网，该示意用于下文各流表分析、中心化与分布式路由分析。

- 集群逻辑网络拓扑示意，如下所示：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/neutron-legacy-router-and-dvr-router.jpg)

- OpenStack Neutron agent 虚拟网络连接与流量类型分析示意：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/openstack-neutron-agent-virtual-network-flows-analyze-demo.jpg)

### 🚀 使用 VLAN 网络类型的实例与外部网络通信流表分析：

- 创建使用 VLAN 网络的租户网络与实例：
  
  ```bash
  $ openstack network create \
    --provider-network-type vlan finance-vlan-network1
  # 创建使用 VLAN 网络的租户网络
  $ openstack subnet create 
    --network finance-vlan-network1 \ 
    --subnet-range=192.168.3.0/24 \ 
    --dns-nameserver=172.25.250.254 \ 
    --dhcp finance-vlan-subnet1
  # 创建租户网络内子网 
  $ openstack router add subnet finance-router1 finance-vlan-subnet1
  # 将租户网络内子网附加至租户路由器上
  $ openstack server create \
    --flavor default \
    --key-name example-keypair \
    --nic net-id=finance-vlan-network1 \
    --image rhel7 \
    --wait finance-vlan-server1
  
  $ openstack network show finance-vlan-network1
  # 由于在 ML2 插件与 neutron-openvswitch-agent 中已配置，结果返回包含
  # "provider:network_type vlan"、"provider:physical_network datacentre"、
  # "provider:segmentation_id 99"、"mtu 1496"
  # 
  # 注意：
  #   1. --provider-network-type 为 vlan 时，租户实例的 MTU 值为 1496。
  #   2. --provider-network-type 为 vxlan 时，租户实例的 MTU 值为 1446。
  #   3. --provider-network-type 为 gre 时，租户实例的 MTU 值为 1454。
  #   4. 从实例侧发出的封包采用该 MTU 值，最终可能通过计算节点再次封装发往其他计算节点、
  #      控制节点或网络节点，因此，若实例所在的计算节点对原始的封包再做隧道封装、VLAN header
  #      添加或其他操作将增加 MTU 值的大小，因此在 OpenStack 集群各节点与物理网络设备上
  #      设置正确的 MTU 值，以免造成网络故障。
  ```

- 该示例从上述创建的实例中使用 `ping 172.25.250.254` 命令以追踪数据包从实例、计算节点、控制节点到外部网络的完整通信过程，实例所在的项目中使用的路由器为 `non-DVR`（非分布式虚拟路由器），运行于控制节点的 `qrouter-54e7e5d1-1204-4de8-8590-0c0264983982` namespace 中，分析过程如下所示：
  
  1️⃣ 计算节点：从 finance-vlan-server1 实例发出的数据包通过其连接的 qbr 网桥、qvb 端口、qvo 端口进入 br-int 网桥的流表中进行过滤跳转，最终从 `int-br-ex` 端口转发至 br-ex 网桥。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/OUTPUT_COMPUTE1-packets-from-instance-into-br-int-qvo-port.jpg)
  
  2️⃣ 计算节点：通过 `phy-br-ex` 端口进入 br-ex 网桥的数据包使用如下规则将租户内的 `VLAN tag 3` 修改为 `VLAN id 99`，该值与 br-ex 网桥对接的网络 `172.25.250.0/24` 的 VLAN id 相同（该 VLAN id 由 ML2 插件创建）。 
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/OUTPUT_COMPUTE1-packets-into-br-ex-phy-br-ex-port.jpg)
  
  3️⃣ 计算节点：通过 br-ex 网桥的数据包最终通过 `eth2` 网口出计算节点经由 172.25.250.0/24 网络连接至控制节点，计算节点上对 eth2 网口抓包。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute1-capture-packets-eth2-icmp-request-reply.jpg)
  
  4️⃣ 控制节点：在 eth2 网口上抓包，可确认与计算节点间的 VLAN 99 的流量。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/controller0-capture-packets-eth2-icmp-request-replay.jpg) 
  
  5️⃣ 控制节点：租户路由器的网口与 iptables 信息
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/controller0-finance-router1-qr-port.jpg)
  
  6️⃣ 控制节点：从 eth2 进入的流量进入 br-ex 网桥后再通过 int-br-ex 端口进入 br-int 网桥，其中的 OpenFlow 规则将源数据包的 `VLAN id 99` 修改为租户内 `VLAN tag 8` 后，数据包通过 `qr` 端口进入 qrouter namespace，进入的数据包再通过 `iptables SNAT` 转换后从 `qg` 端口流出，此时数据包带有 `VLAN tag 5`，因此，数据包再次进入 br-ex 网桥时 OpenFlow 规则将匹配 VLAN tag 5。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/INPUT_CONTROLLER0-packets-into-br-int-int-br-ex-port.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/controller0-finance-router1-snat-rules.jpg)
  
  7️⃣ 控制节点：进入 br-ex 网桥的数据包匹配租户内 VLAN tag 5，去除该 tag 后由 eth2 将数据包发往外部目标地址。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/OUTPUT_CONTROLLER0-packets-into-br-ex-phy-br-ex-port.jpg)

### 🚀 使用 GRE 隧道网络的实例与外部网络通信流表分析：

- 创建 GRE 隧道网络与租户内实例：
  
  ```bash
  $ openstack network create \
    --provider-network-type gre finance-gre-network1
  # 创建 GRE 隧道网络，使租户内网络可通过 GRE 隧道通信。
  $ openstack subnet create \ 
    --network finance-gre-network1 \ 
    --subnet-range=192.168.4.0/24 \ 
    --dns-nameserver=172.25.250.254 \ 
    --dhcp finance-gre-subnet1
  # 创建租户网络内子网，连接至该子网的实例将使用 GRE 隧道。
  $ openstack router add subnet finance-router1 finance-gre-subnet1
  # 将租户网络内子网附加至租户路由器上
  
  $ openstack server create \
    --flavor default \
    --key-name example-keypair \
    --nic net-id=finance-gre-network1 \
    --image rhel7 \
    --wait finance-gre-server1
  ```

- 该示例使用的测试方法及租户路由器与上述流表分析的相同，分析过程如下所示：
  
  1️⃣ 计算节点：来自于实例的数据包从 qvo 端口（`in_port=7`）进入 br-int 网桥，经过 OpenFlow 规则处理，最终从 br-int 网桥的 `patch-tun` 端口流出再通过 br-tun 网桥的 `patch-int` 端口进入。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/OUTPUT_COMPUTE0-packets-into-br-int-qvo-output-patch-tun.jpg)
  
  2️⃣ 计算节点：进入 br-tun 网桥的 `patch-int` 端口的数据包，其目标 MAC 地址为租户内路由器对内网关的端口 `MAC` 地址，将去除租户的 `VLAN tag 3` 并添加隧道 ID `27`，再从 `gre` 端口发出通过 `eth1` 发送至控制节点。
  
  ```bash
  $ man 8 ovs-ofctl
    ...
    dl_dst=xx:xx:xx:xx:xx:xx/xx:xx:xx:xx:xx:xx
        Matches an Ethernet destination address specified as 6 pairs of hexadecimal digits delimited by colons (e.g. 00:0A:E4:25:6B:B0), with
        a wildcard mask following the slash. Open vSwitch 1.8 and later support arbitrary masks for source and/or destination.  Earlier  ver‐
        sions only support masking the destination with the following masks:
  
        01:00:00:00:00:00
            Match  only  the  multicast bit.  Thus, dl_dst=01:00:00:00:00:00/01:00:00:00:00:00 matches all multicast (including broadcast)
            Ethernet packets, and dl_dst=00:00:00:00:00:00/01:00:00:00:00:00 matches all unicast Ethernet packets.
  
        fe:ff:ff:ff:ff:ff
            Match all bits except the multicast bit.  This is probably not useful.
  
        ff:ff:ff:ff:ff:ff
            Exact match (equivalent to omitting the mask).
  
        00:00:00:00:00:00
            Wildcard all bits (equivalent to dl_dst=*.)
    ...
  # 查看 OpenFlow 规则的字段定义，如搜索 dl_dst 关键字。
  ```
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/OUTPUT_COMPUTE0-packets-into-br-tun-patch-int-port-output-gre-port-1.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/OUTPUT_COMPUTE0-packets-into-br-tun-patch-int-port-output-gre-port-2.jpg)
  
  3️⃣ 计算节点：在 eth1 网口抓包显示目标地址的 ICMP 响应数据包
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute0-capture-packets-eth1-gre-tunnel.jpg)
  
  4️⃣ 控制节点：通过 gre 端口进入 br-tun 网桥的数据包被去除隧道 ID `27`，并被添加 `VLAN tag 9`，经过 OpenFlow 规则的转发，数据包从 patch-int 端口发出进入 br-int 网桥，在该网桥中带有 VLAN tag 9 的数据包可进入 `qr-f4376fd3-71` 端口，通过租户内路由器的 `iptables SNAT` 规则处理可转发至目标网络 172.25.250.0/24 中的主机。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/INPUT_CONTROLLER0-packets-into-br-tun-gre-port-1.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/INPUT_CONTROLLER0-packets-into-br-tun-gre-port-2.jpg)
  
  5️⃣ 控制节点：使用 `watch` 命令可实时监测使用 ping 命令发送 ICMP 请求与响应的流量影响的具体 OpenFlow 规则，其 `n_packets` 与 `n_bytes` 在实时变化。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/br-int-patch-tun-port-accept-packets-and-int-br-ex-accept-return-packets.jpg)

### Neutron 中心化与分布式虚拟路由器的使用场景：

- 如前文所述，中心化路由与分布式路由可在集群中同时部署，因此，可同时使用中心化与分布式虚拟路由器。

- 中心化虚拟路由器（legacy）：
  
  - 也称为 `non-DVR`，以 `qrouter namespace` 的方式运行于控制节点或网络节点，提供实例间的东西向流量（`east-west`）及实例与集群外部的南北向流量（`north-sourth`）。
  
  - 实例间的东西向流量包括：
    
    - 1️⃣ 相同租户内同一子网间的流量：
      
      - 同一节点上的实例可直接通过 br-int 网桥通信
      
      - 跨节点间的实例必须通过 `VLAN`、`GRE` 或 `VXLAN` 隧道 overlay 网络通信
    
    - 2️⃣ 相同租户内不同子网间的流量：
      
      - 若不同实例所在的子网连接同一个 `non-DVR`，则实例间无论是在同一节点还是跨节点间，实例间的通信必须使用位于控制节点上的 `qrouter namespace`。
      
      - 若不同实例所在的子网连接不同的 `non-DVR`，则实例间的通信需使用 `FIP`。
    
    - 3️⃣ 不同租户间不同子网间的流量：
      
      - 实例必需有 FIP
      
      - 不同实例间的通信需借助 FIP 与 qrouter namespace。
  
  - 实例与集群外的南北向流量：
    
    - 实例必需有 FIP
    
    - 使用 `qrouter namespace` 中的 `DNAT` 与 `SNAT` 来实现通信

- 🚀 分布式虚拟路由器（`distributed virtual router, DVR`）：
  
  - 以 `qrouter namespace` 的方式运行于控制节点与计算节点，路由 `east-west` 流量，只有当租户内运行实例时，计算节点才创建对应的 `DVR`，而控制节点的 DVR 不发挥作用，并且在控制节点上也同时创建 `snat namespace`。
  - 当实例被分配 FIP 时，计算节点上将再创建 `FIP namespace`，与 DVR 路由 `north-sourth` 流量，在 `qrouter namespace` 中为实例执行 `DNAT/SNAT`。
  - `snat namespace` 运行于控制节点，未分配 FIP 的实例若需与外部网络通信必须使用该 namespace，从计算节点转发出的实例数据包将重定向至该 namespace 中，通过 `SNAT` 后经过 br-int 网桥与 br-ex 网桥转发出控制节点至外部网络。

- 如上所述，已提供 `non-DVR` 的创建方法，以下为 `DVR` 的创建方法：
  
  ```bash
  $ openstack network create finance-dvr-network1
  $ openstack subnet create \
    --network finance-dvr-network1 --subnet-range=192.168.5.0/24 \
    --dns-nameserver=172.25.250.254 --dhcp
    finance-dvr-subnet1
  # 创建使用 DVR 的租户网络与子网
  $ neutron router-create --distributed True finance-dvr-router2
  # 创建分布式虚拟路由器 DVR
  $ neutron router-gateway-set finance-dvr-router2 provider-datacentre
  # 通过外部网络 provider-datacentre 将 DVR 设置为租户对外网关
  $ openstack router add subnet finance-dvr-router2 finance-dvr-subnet1
  # 添加 DVR 的租户内子网
  ```
  
  ```bash
  $ openstack server create \
    --flavor default \
    --key-name example-keypair \
    --nic net-id=finance-dvr-network1 \
    --image rhel7 \
    --wait finance-server3
  # 创建租户内实例，同时该实例运行的计算节点上将创建 qrouter namespace。
  ```

### ✨ 分布式虚拟路由器的实现方式：

- 1️⃣ 未分配浮动 IP（FIP）的实例访问外网：
  
  该场景中的实例要使用 `SNAT` 以访问外网，因此，实例请求的数据包使用其所在计算节点的 `DVR` 路由并将数据包通过 `VXLAN` 隧道转发至控制节点的 `snat namespace` 中，最终通过控制节点的 `br-ex` 网桥发送与响应外部网络的数据包，如前文 "OpenStack Neutron agent 虚拟网络连接与流量类型分析示意" 图中所示。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute0-qrouter-dvr-without-fip-1.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute0-qrouter-dvr-without-fip-2.jpg)
  
  一般情况下，`ip route show` 命令只能查看 kernel 中路由策略（routing policy）定义的 `main` 路由表的规则而其他自定义的规则无法查到，因此，可使用 `ip rule list` 命令查看是否具有自定义路由策略。
  
  如上图所示，通过 `qr-fe3b4198-d2` 端口进入 DVR 的数据包根据默认的路由规则已无法确定其下一跳网关的地址，因此查看路由策略中自定义规则 `3232236033`，该规则定义的路由表中包含 `192.168.2.4` 下一跳网关地址，即控制节点上 `snat namespace` 的 `sg-cc29fc83-48` 端口的 IP 地址。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/controller0-qrouter-dvr-snat-namespace-without-fip-1.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/controller0-qrouter-dvr-snat-namespace-without-fip-2.jpg)
  
  `snat namespace` 的 iptables NAT 表中已定义 SNAT 规则，可将数据包转发出控制节点。

- 2️⃣ 已分配浮动 IP（FIP）的实例与外网间的互访：
  
  该场景中实例运行的计算节点上已运行 DVR，并在分配 FIP 的过程中将在同一计算节点上创建 `fip namespace`，DVR 中也将创建自定义路由策略可将数据包转发至 fip namespace，并且也具有 DNAT 与 SANT 规则将实例的租户内地址转换为 FIP，直接可将数据包从计算节点本地的 `br-ex` 网桥实现接收与发送，以此完成分布式路由的功能。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute0-qrouter-dvr-fip-namespace-1.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute0-qrouter-dvr-fip-namespace-2.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute0-qrouter-dvr-fip-namespace-3.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute0-qrouter-dvr-fip-namespace-4.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/openstack-newton-network-architecture/compute0-qrouter-dvr-fip-namespace-5.jpg)
  
  DVR 与 fip namespace 的连接方式如前文 "OpenStack Neutron agent 虚拟网络连接与流量类型分析示意" 图中所示。
