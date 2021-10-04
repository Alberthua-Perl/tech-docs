## Linux NAT 与 Overlay 网络的实现

##### 文档说明：

- OS 版本：CentOS Linux release 7.9.2009 (Core)

- 该文档中涉及的 Linux NAT 技术与 Overlay 网络技术为理解 Flannel 与 Calico 的网络模式提供思路。

  

##### 文档目录：

- Linux kernel 原生支持的网络技术
- iptables NAT 表的 DNAT、SNAT 与 MASQUERADE
- IPIP 隧道原理与 Linux 的原生实现
- GRE 隧道原理与 Linux 的原生实现
- VXLAN 隧道原理与 Linux 的原生实现
- 参考链接



##### Linux kernel 原生支持的网络技术：

- Linux kernel 原生支持的虚拟网络设备：
  - veth pair（veth 对）：`veth` 内核模块实现
  - tun/tap：`tun` 内核模块实现
  - bridge：`bridge` 内核模块实现
  - vtep：`vxlan` 内核模块实现

- Linux kernel 原生支持的隧道技术（Overlay 网络）：
  - IPIP 隧道：`ipip` 内核模块实现
  - GRE 隧道：`ip_gre` 内核模块实现
  - VXLAN 隧道：`vxlan` 内核模块实现
  - SIT 隧道：`sit` 内核模块实现
  - GENEVE 隧道：`geneve` 内核模块实现

  > 📌**注意：**
  >
  > 1. $ man ip-link：查看与配置网络设备
  >
  >    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\man-ip-link.png)
  >
  > 2. 以上内核模块均需要 `tunnel4.ko` 模块所支持！
  > 3. GENEVE 隧道的实现与 VXLAN 隧道类似。

- Linux kernel 原生支持的隧道技术可通过 ip 命令与 kernel module 实现。

- 支持隧道技术的开源实现：
  - Flannel：
    - UDP 网络模式
    - VXLAN overlay 网络模式
    - `host-gw` 网络模式
  - Open vSwitch（OVS）：
    - GRE 网络模式
    - VXLAN 与 `Geneve` overlay 网络模式

- Linux kernel 内核网络栈包括：物理或虚拟网卡、lo 回环设备、路由表、iptables 防火墙规则表

  > 📌**注意：**刘超（网易云计算研究院）
  >
  > 只要是在网络上传输的数据包，都是完整的，可以有下层协议没上层协议，但绝对不可能有上层协议没下层协议！ 



##### iptables NAT 表的 DNAT、SNAT 与 MASQUERADE：

- DNAT、SNAT 与 MASQUERADE 都是 NAT（网络地址转换）。
- `MASQUERADE`（地址伪装）是 SNAT 的一个特例。
- DNAT 在 `PREROUTING` 链上进行，SNAT 在 `POSTROUTING` 链上进行。



- Linux 主机作为 NAT 网关设备实现 `DNAT`：
  
  - 即，从外部客户端访问内部应用。
    
  - 该 Linux 主机需配置 DNAT 规则，将外部 IP 地址转换为内部网络 IP。
  
  - 另外，需添加 SNAT 规则转发 DNAT 转换后的数据包，否则无法访问后端应用（回包）！
  
  - `net.ipv4.ip_forward` 内核参数作用于 `FORWARD` 链，POSTROUTING 链定义数据包从网口发出的规则。
  
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\linux-nat-gateway-dnat.jpg)
  
    ```bash
    $ iptables -t nat -A PREROUTING \
      -d 10.0.1.2 -p tcp --dport 4000 -j DNAT --to-destination 172.16.17.19
    # 10.0.1.2 相当于对外网络地址，端口 4000 为后端应用暴露的端口，172.16.17.19 相当于访问的后端应用的 IP 地址。
    
    $ iptables -t nat -A POSTROUTING \
      -s 10.0.1.0/24 -j SNAT --to-source 172.16.17.18
    # 10.0.1.0/24 为对外网络地址段，172.16.17.18 为对外暴露的 IP 地址（对内网络地址），该 IP 地址位于 NAT 网关接口上。
    
    $ iptables -t nat -nvL <chain_name> --line-number
    # 查看 iptables NAT 表中指定链的信息并显示规则行号
    $ iptables -t <table_name> -D <chain_name> <line_number>
    # 删除 iptables 表中指定链的规则
    $ service iptables save
    # 保存配置的 iptables 规则，否则重启失效。
    ```
  
  - 客户端访问应用测试，该应用为提供 docker 官方文档的 nginx 容器（外部 -> 内部）。
  
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\linux-nat-gateway-dnat-client.jpg)



- Linux 主机作为 NAT 网关设备实现 SNAT：

  - 即，从内部客户端访问外部网络。

  - 可使用 SNAT 将 Linux 主机作为连接外部网络与内部网络的网关设备。

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\linux-nat-gateway-snat.jpg)

    ```bash
    $ iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
    # 使用地址伪装实现 SNAT，以访问外部网络，常用于 NAT 网关对外 IP 地址不固定的场景。  
    
    # 注意：
    #   1. 可在 iptables POSTROUTING 链中添加如上的 MASQUERADE 或 SNAT 的规则将 Linux 主机
    #      部署成为内部网段通往外部的网关。
    #   2. 若从该 Linux 主机无法 SSH 远程连接至内部其他主机，需检查该主机的 POSTROUTING 链
    #      是否存在相应的 MASQUERADE 或 SNAT 的规则，影响远程连接！
    ```

  - 客户端将该 NAT 设备作为默认网关即可对外访问（内部 -> 外部）。

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\linux-nat-gateway-snat-client.jpg)



- SNAT 与 MASQUERADE 的联系与区别：

  - 使用 SNAT 时，出口 IP 可 NAT 成一个地址或多个地址，如下所示：

    ```bash
    $ iptables -t nat -A POSTROUTING \
      -s 10.8.0.0/255.255.255.0 -o eth0 -j SNAT --to-source 192.168.5.3
    # 将所有 10.8.0.0/24 网段的数据包 SNAT 成 192.168.5.3 的 IP 然后发送出去
    
    $ iptables -t nat -A POSTROUTING \
      -s 10.8.0.0/255.255.255.0 \
      -o eth0 -j SNAT \
      --to-source 192.168.5.3-192.168.5.5
    # 将所有 10.8.0.0/24 网段的数据包 SNAT 成 192.168.5.3、192.168.5.4、192.168.5.5 等
    # 几个 IP 然后发送出去
    ```

  - 对于 SNAT 而言，无论是几个地址，必须明确的指定 SNAT 的 IP。

  - 若当前系统用的是 ADSL 动态拨号方式，那么每次拨号，出口 IP 都会改变，且改变幅度很大。

  - 此时若按照现有方式配置 iptables 将出现问题，因为每次拨号后，服务器地址都会变化，而 iptables 规则内的 IP 是不会随着自动变化的。

  - 因此，每次地址变化后都必须手工修改一次 iptables，把规则中的固定 IP 改成新的 IP，这显得十分不便。

  - MASQUERADE 就是针对这种场景而设计的，其作用是从服务器的网卡上，自动获取当前 IP 地址实现 SNAT：

    ```bash
    $ iptables -t nat -A POSTROUTING -s 10.8.0.0/255.255.255.0 -o eth0 -j MASQUERADE
    ```

  - 因此 eth0 出口获得了怎样的动态 IP，MASQUERADE 会自动读取 eth0 现在的 IP 地址，然后实现 SNAT 发送出去，这样就实现很好的动态 SNAT 地址转换。

  - MASQUERADE 对每个匹配的包都要查找可用的 IP，而不像 SNAT 使用的 IP 是配置好的。

  - MASQUERADE 在 IP 不固定的场合下使用，如拨号网络或通过 DHCP 随机分配 IP。



- iptables 各个链（chain）与表（table）的关系：**四表五链**
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\netfilter-packet-flow.png)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\iptables-for-all.jpg)
  
  > 📌**注意：**各个 OS 版本之间的 iptables 表类型存在细微差别！



##### IPIP 隧道原理与 Linux 的原生实现：

- IPIP 需要 `ipip` 内核模块，该方式最为简单。
- 但不能通过 `IP-in-IP` 隧道转发广播或者 IPv6 数据包。
- 该隧道只是连接了两个一般情况下无法直接通讯的 IPv4 网络而已。
- 至于兼容性，这部分代码已经有很长一段历史，它的兼容性可以上溯到 1.3 版的内核。
- Linux 的 IP-in-IP 隧道不能与其他操作系统或路由器互相通讯。



- IPIP 隧道在 Linux 上的实现：

  - IPIP 隧道实验网络拓扑：
    <img src="D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\ipip-tunnel.png" style="zoom:80%;" />

  - docker-arch 与 podman-arch 作为两端内部网络中的主机。
  
  - bastion0 与 bastion1 作为两端内部网络的路由器网关。
  
  - 两端内部网络中的主机设置对应的路由器网关，如下所示：
  
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\docker-arch-host-route-table.png)
  
  - IPIP 隧道创建：
  
    ```bash
    # 以 bastion0 为例，bastion1 配置类似。
    $ sudo modprobe ipip
    $ sudo ip tunnel add neta \
      mode ipip remote 172.16.17.19 local 172.16.17.18 ttl 255
    $ sudo ip link set dev neta up
    $ sudo ip address add 10.0.1.2 dev neta
    $ sudo ip route add 10.0.2.0/24 dev neta
    # IPIP 隧道创建与 GRE 隧道创建类似
    ```
  
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\ipip-tunnel-demo-1.jpg)
  
  - 在 docker-arch 主机上对 podman-arch 持续发送 ICMP 数据包，使用 tcpdump 对 bastion1 的 eth1 抓包，查看 IPIP 数据包信息。
  
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\ipip-tunnel-demo-2.jpg)



##### GRE 隧道原理与 Linux 的原生实现：

- GRE 隧道简介：

  - GRE 隧道全称通用路由封装协议（generic routing encapsulation），最初由Cisco开发。

  - GRE 是一种 `IP-in-IP` 的隧道，可以对某些网络层协议的数据报进行封装，使这些被封装的数据报能够在IPv4/IPv6 网络中传输。
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\gre-tunnel.jpg)
  
    
  
  - 隧道（tunnel）是一个虚拟的点对点（point to point）的连接，提供了一条通路使封装的数据报文能够在这个通路上传输，并且在一个 tunnel 的两端分别对数据报进行封装及解封装。
  
    > 📌**注意：**GRE 隧道建立在隧道两端主机之间的`路由器`上，而不是主机上，隧道对两端的主机透明！
  
  - 要在 Linux 上创建GRE隧道，需要 `ip_gre` 内核模块，它是 GRE 通过 IPv4 隧道的驱动程序。
  
  - GRE 隧道可传输广播与多播数据包、IPv6 数据包与创建基于 IPv6 的隧道。
  
  - 因此，GRE 隧道可支持虚拟机实例、容器或 Pod 的跨主机间通信。

- GRE 隧道在 Linux 上的实现：

  - GRE 隧道实现拓扑示意：

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\gre-tunnel-demo-1.jpg)

  - 两端内部网络中主机网络信息：docker-arch 主机、podman-arch 主机

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\gre-tunnel-demo-2.jpg)

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\gre-tunnel-demo-3.jpg)

  - 配置 R1 路由器 GRE 隧道：R2 路由器配置与 R1 配置类似

    ```bash
    ### 使用 Linux 实现 R1 与 R2 路由器：bastion0 作为 R1、bastion1 作为 R2
    $ sudo modprobe ip_gre
    $ sudo ip tunnel add neta \
      mode gre remote 172.16.17.19 local 172.16.17.18 ttl 255
    # 创建名称为 neta 的 GRE 隧道
    $ sudo ip link set dev neta up
    $ sudo ip address add 10.0.1.1 dev neta
    # 为该 GRE 隧道接口分配 IP 地址
    $ sudo ip route add 10.0.2.0/24 dev neta
    # 添加对端内部网络的路由网络地址，若不添加，无法实现 GRE 隧道互通。
    
    $ sudo sysctl -a | grep net.ipv4.ip_forward
    # 启用 Linux kernel 数据包转发功能（针对于 iptables 的 FORWARD 链）
    $ sudo iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE
    # 将转发的数据包从 eth1 接口发出，并执行地址伪装。
    # 若该 iptables 防火墙规则未配置，来源于一端内部网络的数据包无法转发至对端网络，数据包将被丢弃（drop）。
    ```

  - R1 与 R2 路由器的 GRE 隧道与路由表信息：

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\gre-tunnel-demo-4.jpg)

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\gre-tunnel-demo-5.jpg)

  - 由于 GRE 隧道建立在路由器之间，因此在 docker-arch 主机上持续发送 ICMP 包，使用 tcpdump 在 R2 路由器上抓包，查看 GRE 数据包信息。

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\gre-tunnel-demo-6.jpg)



##### VXLAN 隧道原理与 Linux 的原生实现：

- Linux 对 VXLAN 协议的支持时间并不久，2012 年 Stephen Hemminger 才把相关的工作合并到 kernel 中，并最终出现在 `kernel 3.7.0` 版本。

- 为了稳定性和很多的功能，某些软件推荐在 3.9.0 或者 3.10.0 以后版本的 kernel 上使用 VXLAN。

- 在 `Linux kernel 3.12.x` 后该技术趋于完善，支持单播与组播、IPv4 与 IPv6。

  > 📌**注意：**如果可以，尽量使用比较新版本的 kernel，以免出现因为内核版本太低导致功能或者性能上的问题。

- 需要 VXLAN 的原因：
  - VLAN 的数量限制：`4096` 个 VLAN 网段远不能满足大规模云计算数据中心的需求
  - 物理网络基础设施的限制：基于 IP 子网的区域划分限制了需要二层网络连通性的应用负载的部署
  - TOR 交换机 MAC 表耗尽：虚拟化以及东西向流量导致更多的 MAC 表项
  - 多租户场景：IP 地址重叠
  
- VXLAN 协议报文简介：

  - VXLAN 报文：

    虚拟可扩展局域网（VXLAN，virtual extensible LAN）是一种 overlay 网络技术，使用 `MAC-in-UDP` 的方法进行封装，共 `50` 字节的封装报文头。

  - 报文格式如下所示：

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-packet.png)
    
    - VXLAN 报文头（VXLAN Header）：
    
      共计 8 个字节（byte），目前使用的是 Flags 中的一个 8 bit 的标识位和 24 bit 的 `VNI`（VXLAN network identifier），其余部分没有定义，但是在使用的时候必须设置为 0x0000。
    
    - 外层 UDP 报文头（UDP Header）：
    
      - 目的端口使用 `4789` 端口，但是可以根据需要进行修改。
      - 同时 UDP 的校验和（checksum）必须设置成全 0（*0x0000*）。
    
    - 外层 IP 报文头（Outer IP Header）：
    
      - 目的 IP 地址可以是`单播地址`，也可以是`多播地址`。
      - 单播情况下，目的 IP 地址是 VXLAN tunnel end point（VTEP）的 IP 地址。
      - 多播情况下，引入 VXLAN 管理层，利用 VNI 和 IP 多播组的映射来确定 VTEPs。
      - 协议（protocol）：设置值为 `0x11`，显示说明为 UDP 数据包。
      - 外层源 IP（outer source ip）：源 *VTEP_IP*
      - 外层目的 IP（outer destination ip）：目的 *VTEP_IP*
    
    - 外层 MAC 报文头（Outer Mac Header）：
    
      - 目的 VTEP 的 MAC 地址，即为本地下一跳的地址，通常是网关 MAC 地址。
      - VLAN：VLAN type 设置为 `0x8100`，并可以设置 `VLAN id tag`（VXLAN 的 VLAN标签）。
      - ether type（以太网类型）：设置值为 `0x8000`，指明数据包为 IPv4 类型。

  > 📌**注意：**VTEP 的作用
  >
  > 1. 用于对 VXLAN 报文进行封装与解封装，包括 ARP 请求报文和正常的 VXLAN 数据报文。
  > 2. 在一段封装报文后通过隧道向另一端 VTEP 发送封装报文，另一端 VTEP 接收到封装的报文解封装后根据封装的MAC 地址进行转发。
  > 3. VTEP 可由支持 VXLAN 的硬件设备或软件来实现。
  > 4. 从封装的结构上来看，VXLAN 提供了将二层网络 overlay 在三层网络上的能力，VXLAN header 中的 VNI 有24 个 bit，数量远远大于 4096，并且 UDP 的封装可以穿越三层网络，比 VLAN 有更好的扩展性。

- VXLAN 的控制平面和数据平面：
  - 控制平面：**改进的二层协议**
    - VXLAN 不会在虚拟机之间维持一个长连接，所以 VXLAN 需要一个控制平面来记录对端地址可达情况。
    
      > 📌**注意：**控制平面的映射表：VNI、内层源 MAC、外层源 VTEP 所在主机的 IP 地址
    - VXLAN 学习地址的时候仍然保存着二层协议的特征，节点之间不会周期性的交换各自的路由表，对于不认识的 MAC 地址，VXLAN 依靠组播来获取路径信息（如果有 SDN Controller，可以向 SDN 单播获取）。
    - 另一方面，VXLAN 还有自学习的功能，当 VTEP 收到一个 UDP 数据包后，会检查自己是否收到过这个虚拟机的数据，如果没有，VTEP 就会记录源 VNI、源外层 IP、源内层 MAC 对应关系，避免组播学习。
  - 数据平面：**隧道机制**
    - VTEP 为虚拟机的数据包加上报文头，这些新的报文头在数据到达目的 VTEP 后才被去掉。
    - 中间路径的网络设备只会根据外层报文头内的目的地址进行数据转发，对于转发路径上的网络来说，一个 VXLAN 数据包跟一个普通 IP 包相比，除了包个头大一点外没有区别。
    - 由于 VXLAN 的数据包在整个转发过程中保持了内部数据的完整，因此 VXLAN 的数据平面是基于隧道的数据平面。
  
- VXLAN ARP 请求与应答：

  - VXLAN 初始化：
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-arp-request-1.png)
    
    - VM1 和 VM2 连接到 VXLAN 网络 VNI 100，两个 VXLAN 主机加入 IP 多播组 239.119.1.1。
    - VTEP 的实现：Linux 原生的 VXLAN 设备、Flannel VXLAN 设备、OVS 网桥
  - ARP 请求（request）：
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-arp-request-2.png)
    
    - VM1 以广播的形式发送 ARP 请求。
    - VTEP1 封装报文，VNI 为 100，外层 IP 头 DA 为 IP 多播组 239.119.1.1，SA 为 VTEP1_IP。
    - VTEP1 在多播组内进行多播。
    - VTEP2 解析接收到多播报文，填写自身维护的映射表，包括 VNI、内层源 MAC 地址、外层源 VTEP 所在主机的IP 地址，并在本地 VXLAN 标识为 100 的范围内广播。
    - VM2 对接收到的 ARP 请求进行响应。
  - ARP 应答（response）：
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-arp-response.png)
    - VM2 准备 ARP 响应报文后向 VM1 发送响应报文。
    - VTEP2 接收到 VM2 的响应报文后把它封装在 IP 单播报文中（VNI 依然为 100），然后向 VM1 发送单播。     
    - VTEP1 接收到单播报文后，学习内层源 MAC 到外层源 VTEP_IP 地址的映射，解封装并根据被封装内容的目的MAC 地址转发给 VM1。
    - VM1 接收到 ARP 应答报文，ARP 交互结束。

- VXLAN 数据传输过程：

  - ARP 请求应答之后，VM1 知道了 VM2 的 MAC 地址，并且要向 VM2 通信（VM1 以 TCP 的方法向 VM2 发送数据）。     VTEP1 收到 VM1 发送数据包，用 MAC 地址从映射表中检查 VM1 与 VM2 是否属于同一个 VNI。两个 VM 不但位于同一个 VNI 中（不在同一个 VNI 中出网关），并且 VTEP1 已经知道 VM2 的所有地址信息（MAC 和 VTEP2_IP）。     VTEP1 封装新的数据包，然后交给上联交换机。

  - 上联交换机收到服务器发来的 UDP 包，对比目的 IP 地址和自己的路由表，然后将数据包转发给相应的端口。

  - 目的 VTEP 收到数据包后检查其 VNI，如果 UDP 报文中 VNI 与 VM2 的 VNI 一致，则将数据包解封装后交给 VM2 进一步处理，至此一个数据包传输完成。

  - 整个 VXLAN 相关的行为（可能穿越多个网关），对虚拟机而言是透明的，虚拟机不会感受传输的过程。

    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-communication-progress.png)

    > 📌**注意：**
    >
    > 1. 虽然 VM1 与 VM2 之间启动了 TCP 来传输数据，但数据包一路上实际是以 UDP 的形式被转发，        两端的 VTEP 并不会检查数据是否正确或者顺序是否完整，所有的这些工作都是在 VM1 和 VM2 在接收到解封装的 TCP 包后完成的。
    > 2. 也就是说如果被 UDP 封装的是 TCP 连接，那么 UDP 和 TCP 将做为两个独立的协议栈各自工作，相互之间没有交互。

- VXLAN 网关（VXLAN gateway）：
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-gateway.png)
  
  - 如果需要 VXLAN 网络和非 VXLAN 网络连接，必须使用 `VXLAN 网关`才能把 VXLAN 网络和外部网络进行桥接和完成VXLAN ID 和 VLAN ID 之间的映射和路由，和 VLAN 一样，VXLAN 网络之间的通信也需要三层设备的支持，即 VXLAN 路由的支持。
  - 同样 VXLAN 网关可由硬件和软件来实现。

- VXLAN 部署：

  - 纯 VXLAN 部署场景：

    对于连接到 VXLAN 内的虚拟机，由于虚拟机的 VLAN 信息不再作为转发的依据，虚拟机的迁移也就不再受三层网关的限制，可以实现跨越三层网关的迁移。

  - VXLAN 与 VLAN 混合部署：

    - 为了实现 VLAN 和 VXLAN 之间互通，VXLAN 定义了 VXLAN 网关。
    - VXLAN 网关上同时存在两种类型的端口：VXLAN 端口、普通端口
    - 当收到从 VXLAN 网络到普通网络的数据时，VXLAN 网关去掉外层包头，根据内层的原始帧头转发到普通端口上。     
    - 当有数据从普通网络进入到VXLAN网络时，VXLAN 网关负责打上外层包头，并根据原始 VLAN ID 对应到一个VNI，同时去掉内层包头的 VLAN ID 信息。
    - 相应的如果 VXLAN 网关发现一个 VXLAN 包的内层帧头上还带有原始的二层 VLAN ID，会直接将这个包丢弃。     
    - 之所以这样，是 `VLAN ID` 是一个本地信息，仅仅在一个地方的二层网络上起作用，`VXLAN` 是隧道机制，并不依赖 VLAN ID 进行转发，也无法检查 VLAN ID 正确与否。
    - 因此，VXLAN 网关连接传统网络的端口必须配置 `ACCESS` 口，不能启用 `TRUNK` 口。



- VXLAN 隧道在 Linux 上的实现：

  - 以单播模式创建 Linux 原生 VXLAN 隧道：

    - 实现的网络拓扑：
    
      <img src="D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-singlecast-demo-1.jpg"  />
      
    - 实现命令如下所示：
    
      ```bash
      ### 以 VM1 的 VXLAN 隧道创建为例
      $ ip link add vxlan1 \
        type vxlan id 1 remote 172.16.17.19 dstport 4789 dev eth0
      # 创建 VXLAN 隧道封装接口 vxlan1（VTEP 设备）、VNI 为 1、设置远程单播地址与 UDP 监听端口、数据包从 eth0 发出。
      $ ip link set dev vxlan1 up
      $ ip address add 10.1.0.106/24 dev vxlan1
      # 启用 vxlan1 接口并分配内层网段 IP 地址
      ```
    
      ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-singlecast-demo-2.jpg)
    
      > 📌**注意：**ip link add 命令选项说明
      >
      > 1. id：VNI，该值范围在 `1~2^24` 之间。
      >
      > 2. remote：
      >
      >    1）对端 VTEP 所在主机的 IP 地址，VTEP 设备用于封装与解封装 VXLAN 报文（添加与去除前 50 bytes），需要知道将封装的 VXLAN 报文发送至哪个对端 VTEP。
      >
      >    2）Linux 上可以利用 group 指定组播组地址，或者利用 remote 指定对端单播地址，此处利用remote 指定点对点的对端 IP 地址为 172.16.17.19。
      >
      > 3. dstport：
      >
      >       1）指定目的 UDP 端口为 `4789`。
      >
      >       2）Linux kernel 3.7.0 版本首次实现 VXLAN 时，UDP 端口还并没有规定下来。
      >
      >       3）很多厂商利用了 8472 这个端口，Linux 也采用了相同的端口。
      >
      >       4）后来 IANA 分配了 4789 作为VXLAN的目的 UDP 端口。
      >
      >       5）若需要使用 IANA 分配的 4789 端口，需要用 dstport 指定。
      >
      >       <img src="D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-singlecast-demo-3.jpg"  />
      >
      > 4. dev：指定 VTEP 通过哪个物理设备来通信，此处使用 eth0。 
      
    - 两端 VXLAN 隧道配置完成后，在 VM1 上持续发送 ICMP 包，并在 VM2 上使用 tcpdump 抓取相应网络接口的数据包，如下所示：
    
      ```bash
      $ tcpdump -i <nic> host <remote_ip_address> -vvv -w <capture_name>.pcap
      ```
    
      ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-singlecast-demo-4.jpg)
    
      
    
  - 以多播模式创建 Linux 原生 VXLAN 隧道：
  
    - 单播模式点对点的 VXLAN 网络只能两两通信，实际用处不大。
  
    - 若 VXLAN 要使用多播模式，那么底层的网络结构需要支持多播的功能。
  
    - 要组成同一个 VXLAN 网络，VTEP 必须能感知到彼此的存在。
  
    - 多播组本来的功能就是把网络中的某些节点组成一个虚拟的组，所以 VXLAN 最初想到用多播来实现。
  
    - 这个实验和前面一个非常相似，只不过主机之间不是点对点的连接，而是通过多播组成一个虚拟的整体，最终的网络架构也很相似，为了简单图中只有两个主机，但该模型可容纳多个主机组成 VXLAN 网络。
  
    - 实现的网络拓扑：
  
      <img src="D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-multicast-demo-1.png"  />
  
      - 使用 veth 对中的 veth0 接口模拟虚拟机的网络接口，将 veth1 与 vxlan100 网络接口桥接至 Linux 网桥 br0 上，以测试虚拟机通过 VXLAN 隧道的跨主机间通信。
      - 以上网络接口设备均工作于 `Linux kernel space`！
  
    - 实现命令如下所示：
  
      - 该脚本的 GitHub URL：
  
        https://github.com/Alberthua-Perl/scripts-confs/blob/master/ovn-arch/deploy-native-vxlan-tunnel.sh
  
        ```bash
        #!/bin/bash
        
        #  Deploy Linux native VXLAN tunnel.
        #  Linux kernel version should be >= 3.7.x to support vxlan.
        #
        #  This script implements one NIC to test communication 
        #  through vxlan tunnel like vms between different nodes.
        #  veth0 of veth pair could be used as one NIC in vm.
        #
        #  You should run this script on two different nodes to 
        #  deploy vxlan tunnel through replace ip address.
        #  
        #  Created on 2020-10-10 by hualf (lhua@redhat.com).
        
        ### Modify variables if deploy vxlan in different environment.
        BRIDGE=br0
        #BRIDGE_IP=192.169.0.6
        #BRIDGE_PREFIX=24
        VNI=100
        NIC=eth1
        VETH_IP=192.167.0.6
        VETH_PREFIX=24
        
        function deploy_native_vxlan_tunnel() {
          echo -e "[\033[1;32m*\033[0m] Create linux bridge to attach link device ..."
          sudo brctl addbr ${BRIDGE}
          sudo ip link set ${BRIDGE} up
          #sudo ip address add ${BRIDGE_IP}/${BRIDGE_PREFIX} dev ${BRIDGE}
          # Linux bridge could be just as ethernet link device to forward packets without address.
          
          echo -e "[\033[1;32m*\033[0m] Create vxlan interface through multicast group ..."
          sudo ip link add vxlan${VNI} type vxlan id ${VNI} group 239.1.1.1 dstport 4789 dev ${NIC}
          # NIC as VTEP device to forward final packets to another VTEP device.
          sudo ip link set vxlan${VNI} up
          sudo brctl addif ${BRIDGE} vxlan${VNI}
          # vxlan interface as link device to attach linux bridge
         
          echo -e "[\033[1;32m*\033[0m] Create veth pair to implement one NIC like vm ..."
          sudo ip link add type veth
          sudo ip link set veth0 mtu 1450
          # Packets will be add 50 bytes vxlan header through vxlan interface.
          # So mtu of veth0 should be set 1450. 
          sudo ip link set veth0 up
          sudo ip address add ${VETH_IP}/${VETH_PREFIX} dev veth0
          sudo ip link set veth1 mtu 1450
          sudo ip link set veth1 up
          sudo brctl addif ${BRIDGE} veth1
          # attach veth1 of veth pair to linux bridge
          echo -e "\n[\033[1;32m*\033[0m] Link device and address showed as follows ..."
          sudo ip address show
          echo -e "\n[\033[1;32m*\033[0m] Route table as follows ..."
          sudo ip route show
          echo -e "[\033[1;32m*\033[0m] Deploy complete ..."
        }
        
        function destroy_vxlan_tunnel() {
          echo -e "[\033[1;32m*\033[0m] Destroy vxlan tunnel ..."
          sudo ip link set veth0 down 
          sudo ip link set veth1 down
          sudo ip link del veth0
          # veth pair will be deleted together.
          sudo ip link set vxlan${VNI} down
          sudo ip link del vxlan${VNI}
          sudo ip link set ${BRIDGE} down
          sudo ip link del ${BRIDGE}
        }
        
        deploy_native_vxlan_tunnel
        #destroy_vxlan_tunnel
        ```
  
    - 分别于 VM1 与 VM2 上运行以上脚本创建 VXLAN 隧道（需更改其中的 VETH_IP 变量值）。
  
    - 两端 VXLAN 隧道配置完成后，在 VM1 上持续向 VM2 发送 ICMP 包，并在 VM2 上使用 tcpdump 抓取 eth1与 vxlan100 网络接口的数据包，如下所示：
  
      - 通过 eth1 的数据包使用 VXLAN 隧道通信，数据包以 UDP 的方式发送。
  
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-multicast-demo-2.png)
  
      - 通过 vxlan100 的数据包去除了 VXLAN 报文头，只具有原始的 ICMP 报文。
  
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-multicast-demo-3.png)
  
    - 可将 tcpdump 命令的抓包结果写入 `.pcap` 结尾的文件中，以便该文件可使用 Wireshark 进一步分析。
  
    - 首次 ping 对端 veth0 的 IP 地址（内部 IP 地址）时，需进行 ARP 广播以获取对端 veth0 的 MAC 地址，并且该 ARP 广播包需通过 IGMP 发送至多播地址（239.1.1.1）。
  
      > 注意：
      >
      > 1. 主机间的 IP 地址在同一网段内，但首次通信未知对端 MAC 地址，需发送 ARP 请求广播获取对端MAC 地址！
      > 2. 使用 VXLAN 协议封装的 UDP 数据包在 Wireshark 中未能解析为 VXLAN 数据包时，可执行 `Analyze -> Decode As`，选择 VXLAN 即可转换。
  
      ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-multicast-demo-4.png)
  
      ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-multicast-demo-5.png)
  
    - UDP 数据包转换为 VXLAN 封装的数据包后，可直接查看 VXLAN 所封装的内层原始载荷数据，如 ARP 请求与ICMP 报文。
  
      - ARP 请求报文结构：目的地址为多播组地址
  
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-multicast-demo-6.png)
  
      - ICMP 报文结构：目的地址为单播地址
  
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-multicast-demo-7.png)
  
    - 多播其实就相当于 VTEP 之间的广播，报文会发给所有的 VTEP，但是只有一个会做出应答。
  
    - VTEP 会通过接收到的报文学习 `MAC-VNI-VTEP IP` 的信息，减少后续不必要的多播报文。
  
    - 对于 Overlay 网络中的通信实体来说，整个通信过程对它们是透明的，它们认为自己的通信和经典网络没有区别。
  
    - VM1 与 VM2 通信建立后，可分别查看 `fdb` 表项与 `ARP` 缓存：
  
      ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\SDN\pictures\Linux NAT与Overlay网络的实现\vxlan-multicast-demo-8.png)



##### 参考链接：

- NAT 学习（DNAT与SNAT）：

  https://blog.csdn.net/nmjhehe/article/details/85475310

- iptables 中 DNAT、SNAT 与 MASQUERADE 的理解：

  https://www.cnblogs.com/fyc119/p/7489052.html

- Linux IPIP 隧道及其实现：

  https://www.cnblogs.com/weifeng1463/p/6805856.html

- GRE Tunneling：

  https://tldp.org/HOWTO/Adv-Routing-HOWTO/lartc.tunnel.gre.html

- VXLAN 原理：

  https://www.cnblogs.com/wipan/p/9220615.html

- VXLAN 协议：

  https://www.cnblogs.com/wipan/p/9220615.html

- Linux 上实现 VXLAN 网络：蚂蚁金服

  https://cizixs.com/2017/09/28/linux-vxlan/
