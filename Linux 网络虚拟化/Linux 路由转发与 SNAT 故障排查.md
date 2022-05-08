## 📌 Linux 路由转发与 SNAT 故障排查

### 故障场景：

- 各节点 OS 版本：Red Hat Enterprise Linux release 8.2 (Ootpa)

- 故障描述：内网的 server 节点需要访问外部 Internet，但在多级路由转发过程中发现无法访问外部 Internet，因此进入到各个 Linux 路由节点分析内核静态路由表（kernel static route table）以及对指定网口进行抓包分析。

### 排查过程：

- 👉 通过该示例进一步理解 `firewalld`、`iptables` 与 `nftables` 的相互关系，以及配置 SNAT 与 MASQUERADE 的方法。

- 故障节点的架构与抓包示意：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-firewall-route-troubleshoot/linux-gateway-snat-or-masquerade.jpg)

- 如上所示，server 节点访问 Internet 需要通过 Linux-router1 节点、Linux-router2 节点与 foundation 节点的路由转发。

- Linux-router1 节点排查：
  
  > 👉 使用 firewalld 配置 SNAT 规则。
  
  - 该节点的 IP 与路由表，如下所示：
    
    ```bash
    $ ip -br a s
      lo               UNKNOWN        127.0.0.1/8 ::1/128
      eth0             UP             172.25.250.254/24 fe80::3a50:cfa:5e7:a805/64   
      eth1             UP             172.25.252.250/24 fe80::783e:7d62:966a:66b9/64
    
    $ route -n
    Kernel IP routing table
    Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
    0.0.0.0         172.25.252.254  0.0.0.0         UG    101    0        0 eth1
    172.25.250.0    0.0.0.0         255.255.255.0   U     100    0        0 eth0
    172.25.252.0    0.0.0.0         255.255.255.0   U     101    0        0 eth1
    ```
  
  - 该节点由 Linux 实现路由转发，因此从该节点 eth0 接收的 `S:172.25.250.15, D:192.168.110.1` 的数据包通过 iptables 的 `FORWARD` 链经由 kernel 转发（该过程需要启用 `net.ipv4.ip_forward = 1` 内核参数），再通过路由决定（routing decision）与 `POSTROUTING` 链后至 eth1，最后数据包经由 eth1 发出。
    
    ```bash
    $ sysctl --value net.ipv4.ip_forward
      1
    # 该内核参数位于 /proc/sys/net/ipv4/ip_forward
    ```
  
  - 若 eth1 不配置 SNAT，那么从 eth1 发出的数据包依然为 `S:172.25.250.15, D:192.168.110.1`，在 Linux-router1 与 Linux-router2 之间三层互通的情况下，Linux-router2 的 eth0 也接收到 `S:172.25.250.15, D:192.168.110.1` 数据包。
  
  - 尝试从 Linux-router1 的 eth1 抓包验证，如下所示：
    
    ```bash
    $ tcpdump -n -i eth1 -p icmp
    tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
    listening on eth1, link-type EN10MB (Ethernet), capture size 262144 bytes
    01:17:10.718270 IP 172.25.250.15 > 192.168.110.1: ICMP echo request, id 43854, seq 17, length 64
    01:17:11.742314 IP 172.25.250.15 > 192.168.110.1: ICMP echo request, id 43854, seq 18, length 64
    01:17:12.767261 IP 172.25.250.15 > 192.168.110.1: ICMP echo request, id 43854, seq 19, length 64
    01:17:13.790068 IP 172.25.250.15 > 192.168.110.1: ICMP echo request, id 43854, seq 20, length 64
    01:17:14.812036 IP 172.25.250.15 > 192.168.110.1: ICMP echo request, id 43854, seq 21, length 64
    01:17:15.838103 IP 172.25.250.15 > 192.168.110.1: ICMP echo request, id 43854, seq 22, length 64
    ```
    
    💥 故障点：抓包结果提示，只有从 172.25.250.15 不断发出的请求包（`request`），而无任何目标回包的响应（`reply`）。
  
  - 因此，在该路由节点上需配置 `SNAT` 或 `MASQUERADE` 将源地址从 `172.25.250.15` 转换为 eth1 的 `172.25.252.250` 后再发出，可使用如下命令：
    
    ```bash
    $ systemctl is-active firewalld.service
      active
    # 查看 firewalld 守护进程的状态
    
    ### 方法 1 ###
    $ firewall-cmd --permanent --zone=trusted --change-interface=eth0
    # 将 eth0 永久添加至 trusted zone（允许所有的进入流量）中
    $ firewall-cmd --permanent --zone=external --change-interface=eth1
    # 将 eth1 永久添加至 external zone 中
    # 注意：
    #   1. 将网络接口添加至指定的 zone 中，该 zone 即可激活（active）。
    #   2. external zone 中默认启用 MASQUERADE（masquerade: yes），因此从该网口发出的
    #      数据包将进行 SNAT。
    
    ### 方法 2 ###
    $ firewall-cmd --permanent --zone=trusted --change-interface=eth0
    # 将 eth0 永久添加至 trusted zone（允许所有的进入流量）中
    $ firewall-cmd --permanent --zone=external --change-interface=eth1
    $ firewall-cmd --permanent --zone=external --remove-masquerade
    # 尝试使用单独的 SNAT，因此禁用 MASQUERADE。
    $ firewall-cmd --permanent --direct --passthrough ipv4 \
      -t nat -A POSTROUTING -s 172.25.250.0/24 -j SNAT --to-source 172.25.252.250
    # 使用 iptables 的类似的语法添加 SNAT 规则 
    ```
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/linux-firewall-route-troubleshoot/firewall-cmd-man-doc.jpg)
    
    通过上述命令其中任意一种方法即可实现 SNAT。由于在 RHEL 8 中默认使用 `firewalld` 守护进程作为 `nftables` 的前端管理程序，nftables 默认作为用户空间层（user space）firewalld 的后端实现（内核空间层 kernel space 由 `netfilter` 实现），因此，在 nftables 的 NAT 表中可查看 firewalld 配置的 SNAT 规则，如下所示：
    
    ```bash
    $ nft list table nat
    table ip nat {
        ...
        chain POSTROUTING {
            type nat hook postrouting priority srcnat; policy accept;
            ip saddr 172.25.250.0/24 counter packets 8 bytes 608 snat to 172.25.252.250
        }
        ...
    }
    # 以上的规则通过方法 2 实现的 SNAT 规则的添加
    ```

- Linux-router2 节点排查：
  
  > 👉 使用 iptables 配置 SNAT 规则。
  
  - 该节点的 IP 与路由表，如下所示：
    
    ```bash
    $ ip -br a s
      lo               UNKNOWN        127.0.0.1/8 ::1/128 
      eth0             UP             172.25.254.254/24 172.25.253.254/24 172.25.252.254/24 fe80::5054:ff:fe00:fe/64
      eth1             UP
    
    $ route -n
      Kernel IP routing table
      Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
      0.0.0.0         172.25.254.250  0.0.0.0         UG    0      0        0 eth0
      172.25.252.0    0.0.0.0         255.255.255.0   U     0      0        0 eth0
      172.25.253.0    0.0.0.0         255.255.255.0   U     100    0        0 eth0
      172.25.254.0    0.0.0.0         255.255.255.0   U     100    0        0 eth0
    ```
  
  - 根据路由表所示，从 Linux-router1 的 eth1 发出的 `S:172.25.252.250, D:192.168.110.1` 数据包可被 Linux-router2 的 eth0 接收，经过路由决定后需进行 SNAT 才能将其转换为 `S:172.25.254.254, D:192.168.110.1` 再转发至 foundation 节点，因此该节点上必须配置 SNAT 规则，经查该节点上已存在相应规则，无故障点出现：
    
    ```bash
    $ nft list table nat
    table ip nat {
        ...
        chain POSTROUTING {
            type nat hook postrouting priority srcnat; policy accept;
            ip saddr 172.25.252.0/24 counter packets 14 bytes 1176 snat to 172.25.254.254
        }
        ...
    } 
    # 查看 nftables 的 NAT 表 POSTROUTING 链中的规则
    ```
  
  - 若需配置上述规则，可使用如下方法：
    
    ```bash
    $ iptables -t nat -A POSTROUTING -s 172.25.252.0/24 -j SNAT --to-source 172.25.254.254
    # 使用 iptables 添加 SNAT 规则
    ```
  
  > 🤘 注意：iptables 与 nftables 均使用 kernel 的 netfilter，因此两种方式均可配置与查看相应的规则！

- foundation 节点排查：
  
  - 该节点的 IP 与路由表，如下所示：
    
    ```bash
    $ ip -br a s
    lo               UNKNOWN        127.0.0.1/8 ::1/128 
    ens160           UP
    ens224           UP
    br2              UP             192.168.110.239/24 fe80::20c:29ff:fe84:7c70/64
    br0              UP             172.25.254.250/24 172.25.0.250/24 fe80::f078:6fff:fef9:21a/64 
    br3              DOWN           fe80::a808:13ff:fee9:35a2/64
    ...
    
    $ route -n
    Kernel IP routing table
    Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
    0.0.0.0         192.168.110.1   0.0.0.0         UG    0      0        0 br2
    172.25.0.0      0.0.0.0         255.255.255.0   U     425    0        0 br0
    172.25.0.0      172.25.254.254  255.255.0.0     UG    425    0        0 br0
    172.25.250.0    0.0.0.0         255.255.255.0   U     0      0        0 privbr0
    172.25.254.0    0.0.0.0         255.255.255.0   U     425    0        0 br0
    192.168.110.0   0.0.0.0         255.255.255.0   U     426    0        0 br2
    192.168.122.0   0.0.0.0         255.255.255.0   U     0      0        0 virbr0
    192.168.123.0   0.0.0.0         255.255.255.0   U     0      0        0 br1
    ```
  
  - 可通过前文相同的方法，查看该节点是否存在相应的 SNAT 规则，如下所示，无故障点出现：
    
    ```bash
    $ nft list table nat
    table ip nat {
        ...
        chain POSTROUTING {
            ...
            oifname "br2" ip saddr 172.25.254.0/24 counter packets 1809 bytes 136485 masquerade
        }
        ...
    }
    ```

### 总结：

- 除 Linux-router1 节点上未配置 SNAT 规则外其余节点均配置正确。

- 故障点排除后可重新从 server 节点对外发起访问，并在各个节点的指定网口进行抓包即可获得与图中一致的结果。
