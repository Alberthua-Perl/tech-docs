## DNS 原理与常用配置实现

### 文档说明：

- OS 版本：CentOS Linux release 7.7.1908 (Core)
- Bind 相关软件包版本：
  - bind-9.11.4-9.P2.el7.x86_64 
  - bind-utils-9.11.4-9.P2.el7.x86_64
  - bind-chroot-9.11.4-9.P2.el7.x86_64

### 文档目录：

- DNS 域名解析服务之层次结构
- DNS 域名解析服务之区域
- DNS 查询流程
- DNS 资源记录
- DNS 缓存及实现
- DNS 域名服务器类型
- BIND DNS 实现权威域名解析服务
- dig 命令使用示例
- host 命令使用示例
- nslookup 命令使用示例
- DNS 安全问题
- 参考链接

### DNS 域名解析服务之层次结构：

- 相较于由数字构成的 IP 地址，域名更容易被理解和记忆，所以通常更习惯通过域名的方式来访问网络中的资源。

- 但网络中的计算机之间只能基于 IP 地址来相互识别对方的身份，而且要想在互联网中传输数据，也必须基于外网的 IP 地址来完成。

- 因此，DNS（Domain Name System，域名系统）技术应运而生。

- 该技术解决管理和解析域名与 IP 地址对应关系，即，能够接受用户输入的域名或 IP 地址，然后自动查找与之匹配（或者说具有映射关系）的 IP 地址或域名，如下所示：
  
  - 正向解析：将域名解析为 IP 地址
  
  - 反向解析：将 IP 地址解析为域名

- DNS 域名解析技术的正向解析是最常使用的一种工作模式。

- 鉴于互联网中的域名和 IP 地址对应关系数据太过庞大，DNS 域名解析服务采用了类似目录树的层次结构来记录域名与 IP 地址之间的对应关系，从而形成了一个具有层次结构的大型分布式集群系统。

- DNS 协议运行在 UDP 协议之上，使用端口号 `53`。

- 在 RFC 文档中 `RFC 2181` 对 DNS 有规范说明，`RFC 2136` 对 DNS 的动态更新进行说明，`RFC 2308` 对 DNS 查询的反向缓存进行说明。

- DNS 域名系统层次结构：
  
  ![](pictures/dns-domain-hierarchical-structure.jpg)
  
  - 根域名服务器（Root Domain）：
    
    用 `·`（句点）表示，根域名服务器全球的数量是固定的，为 `13` 组。
  
  - 顶级域名服务器（Top Level Domain, `TLD`）：
    
    指代某个国家/地区或组织使用的类型名称，如 com、cn、edu 等。
  
  - 次级域名服务器（Second Level Domain, `SLD`）：
    
    个人或组织在 Internet 上注册的名称，如 qq.com、gitHub.com 等。
  
  - 三级域名服务器或权威域名服务器（如果有）：Third Level Domain
    
    这层严格来说是次级域名的子域，是二层域名派生的域名，通俗说就是网站名，如 `cs.berkeley.edu`。
  
  - 主机名（如果有）：
    
    主机名称标签，通常在 DNS 域名最左侧，标识网络上的特定计算机，如 `www.berkeley.edu`。

- 13 组根域名服务器的具体信息：
  
  ![](pictures/dns-root-server.jpg)

> 👨‍🏫 注意：为什么根域名服务器全球只有 13 组？
> 
> 1. 主要是基于当时的网络环境，从 UDP 分片上考虑，保证报文能畅通无阻。
> 
> 2. 绝大多数的网络接口类型支持 IP 报文 ≤ 576 字节无需分片自由通行，考虑到以上因素，IETF 决定将 DNS 报文体限制在 512 字节。每一组根域名服务器占用 32 字节，其中包括根域名的名称、IP 地址、TTL (Time To Live) 等参数。
> 
> 3. 13 组根域名服务器一共占用 416 字节，剩余的 96 字节用于包装 DNS 报文头以及其它协议参数。所以从空间上来说，没有多余的空间容纳第 14 组根域名服务器的 32 字节。
> 
> 4. 容易被误解的是，这 13 组根域名服务器并不等于 13 台物理服务器，而是代表着 13 个全球 IP 地址，由 13 个机构来管理，对应有不同的名字，分别为 "A" 到 "M"，其中美国最大电信运营商 Verizon 管理两个根域名全球 IP 地址。
> 
> 5. 截止到（2019.09）为止，全球一共有 1011 台服务器实例遍布 5 大洲 4 大洋。

- 域名后缀一般分为国际域名和国内域名。

- 原则上来讲，域名后缀都有严格的定义，但在实际使用时可以不必严格遵守。

- 目前最常见的域名后缀：
  
  - `.com`：商业组织
  
  - `.org`：非营利组织
  
  - `.gov`：政府部门
  
  - `.net`：网络服务商
  
  - `.edu`：教研机构
  
  - `.pub`：公共大众
  
  - `.cn`：中国国家顶级域名

### DNS 域名解析服务之区域：

- DNS 服务器有一个区域（`zone`）的概念。

- 由于 DNS 服务器要为全球的用户提供查询和数据库的服务，对计算、存储、网络带宽都要求很高，所以就必须组织成域名服务器集群，使它们协同工作，共同提供域名解析服务。

- 每个集群就负责管理相应的域名名字空间，也就是说每个集群负责不同的 DNS 区域。

- 如，每个根域名服务器节点指向的下一级就是其管理的 DNS 区域，同样再往下，cn 顶级域名服务器节点下面就管理中国区的 DNS 区域。

- 通过这种区域管理的方法，就很容易进行 DNS 查询。

- 每一级的 DNS 区域域名服务器会保存下一级的 DNS 区域域名服务器的信息，如果从根域名服务器出发，一级级的往下查找，就可以得到目标域名对应的 IP 信息。

- 严格来说具有三种 DNS 服务器角色不容忽视，如下所示：
  
  - 本地 DNS 服务器：
    
    - 特指内网的 DNS 服务器，提供内网主机之间的域名查询服务，一般作为缓存/转发之用，当在该服务器上找不到相应域名时，它会将请求转发到上级 DNS 服务器作进一步查询。
    
    - 它依赖于两个文件，以 Linux 为例，即 `/etc/hosts` 与 `/etc/resolv.conf`。前者记录了内网主机 hostname 和 IP 之间的映射关系（静态解析），后者记录了外网的DNS 服务器地址（动态解析）。当本地 DNS 服务器在 /etc/hosts 和自身的缓存中都找不到相应的域名时，就会从 /etc/resolv.conf 中记录的地址进行进一步查询。
  
  - ISP DNS 服务器（Internet Service Provider, `ISP`）：
    
    - 若计算机直连运营商（ISP）的网络，则外网 DNS 服务器就是指 ISP DNS 服务器。
    
    - ISP DNS 服务器比本地 DNS 服务器多了递归和迭代向各分级 DNS 服务器查询的功能。
  
  - 公共 DNS 服务器：
    
    - 若不想通过 ISP 的 DNS 服务器查询，也可向公共的 DNS 服务器去查询。
    
    - 现在有很多公共的 DNS 服务器，部分如下所示：
      
      ```bash
      ### Google 提供的公共 DNS：
      8.8.8.8  # 主
      8.8.8.4  # 备
      
      ### 国内三大运营商提供的公共 DNS：
      114.114.114.114
      114.114.115.115
      
      ### 阿里提供的公共 DNS：
      223.5.5.5
      223.6.6.6
      
      ### 百度提供的公共 DNS：
      180.76.76.76
      ```

### DNS 查询流程：

- DNS 查询的两种方式：
  
  - 1️⃣ 递归查询（`recursive query`）：
    
    - 客户端首先在 /etc/hosts 静态文件、本地 DNS 缓存中查找目标主机的 IP 地址。
    
    - 客户端与本地 DNS 服务器间为递归查询。
    
    - 递归查询需逐层查询直接返回结果，而不使用查询提示。
    
    - 查询过程如下所示：
      
      ![](pictures/dns-recursive-query.jpg)
  
  - 2️⃣ 迭代查询（`iterative query`）：
    
    - 本地 DNS 服务器在收到客户端发起的请求时，并不直接返回查询结果，而是向另一台 DNS 服务器发起请求，以获取查询提示。
    
    - 查询过程如下所示：
      
      ![](pictures/dns-iterative-query.jpg)

- 实际中，由于根域名服务器和其他的 TLD 服务器的资源宝贵，需要为全球的用户提供 DNS 解析服务，因此其不执行递归查询，执行 DNS 递归查询会导致全球互联网性能不佳。而且递归查询需要逐层查询才能获得查询结果，当查询具有许多层次的 DNS 结构时效率很低。

- 🚀 实际应用中这两种查询方式一起使用，本地 DNS 服务器和 ISP DNS 服务器可以执行递归查询，而其他层级 DNS 服务器执行迭代查询。

- 🐱‍🏍 DNS 查询的完整示意，如下所示：
  
  以下 3 个图例均结合 DNS 递归与迭代查询以完成 DNS 查询的完整过程
  
  ![](pictures/dns-query-progress-1.jpg)
  
  ![](pictures/dns-query-progress-2.jpg)
  
  ![](pictures/dns-query-progress-3.jpg)

### DNS 资源记录：

- DNS 资源记录的类型：
  
  - `IN` 资源记录：网络记录
  
  - <font color=red>**SOA**</font> 资源记录（Start of Authority Record）：
    
    - 授权起始机构记录，用于在区域的多个 NS 记录中区分哪一台为主 DNS 服务器。
    
    - 💥 每个区域必须正好有 1 个 SOA 记录。
      
      ```bash
      example.com.    86400    IN    SOA    classroom.example.com.  root.classroom.example.com. 2015071700 3600 300 604800 60
      ```
  
  - <font color=red>**NS**</font> 资源记录（Name Server Record）：
    
    - 名称服务器记录，用于将子域名映射到对其 DNS 区域具有权威的 DNS 名称服务器。
    
    - 区域的每个公开权威名称服务器必须具有 NS 记录。
    
    - 💥 NS 记录映射到必须具有 A 和/或 AAAA 记录的名称。
      
      ```bash
      example.com.                       86400   IN  NS    classroom.example.com.
      168.192.ip-addr.arpa.              86400   IN  NS    classroom.example.com.
      9.0.e.1.4.8.4.6.2.e.d.f.ip6.arpa.  86400   IN  NS    classroom.example.com.
      ```
  
  - <font color=red>**SRV**</font> 资源记录：
    
    - 服务资源记录，指定某域名所提供的特定服务，帮助客户端查找支持域的特定服务的主机。
    
    - 格式：`<service_name>.<protocol_type>.<domain_name>`
    
    - 示例：`_ldap._tcp.example.com. 86400 IN SRV 0 100 389 server0.example.com.`
      
      该 SRV 记录表示一个可以使用 TCP 传输协议（_tcp）进行联系的 LDAP 服务器（_ldap），该服务器属于 example.com 域。该服务器为 server0.example.com，正在侦听 389 端口，其优先级为 0，权重为 100（当客户端收到多个 SRV 记录时，它用于控制选取哪一个服务器）。
    
    - 🔥 Kubernetes 与 OpenShift 中 service 的服务发现采用 `SRV` 记录实现。以下示例为 Kubernetes 集群 codeready-workspace 命名空间内的 pod 通过上游 DNS 服务器获得 service 的 SRV 记录的请求查询。
      
      ![](pictures/k8s-pod-dns-resolv-1.png)
      
      ![](pictures/k8s-pod-dns-resolv-2.png)
  
  - `A` 与 `AAAA` 资源记录（Address Record）：
    
    - A 记录：提供主机名到 IPv4 地址的映射
    
    - AAAA 记录：也称为 "四 A" 记录，提供主机名到 IPv6 地址的映射。
      
      ```bash
      host.example.com.       86400   IN      A       172.25.254.254
      a.root-servers.net.     604800  IN      AAAA    2001:503:ba3e::2:30
      ```
  
  - <font color=red>**CNAME**</font> 资源记录（Canonical Name Record）：
    
    - 权威名称记录，允许为主机名创建别名。
    
    - 常用于域名的内部跳转，为服务器配置提供灵活性，用户感知不到。
    
    - 示例：用户访问 `www.baidu.com` 时，实际返回的是 `www.a.shifen.com` 的 IP 地址
    
    - 对内部域名与 IP 的变更带来极大的便利
      
      ![](pictures/dns-cname-1.png)
      
      ![](pictures/dns-cname-2.png)
    
    - 💥 CNAME 资源记录必须最终解析为具有 A 和/或 AAAA 的记录的名称。
    
    > 注意：应避免将 CNAME 记录指向其他 CNAME 记录（CNAME 循环）。
  
  - `MX` 资源记录（Mail Exchange Record）：
    
    邮件交换记录，将域名映射到接受该域的邮箱服务器域名。
    
    ```textile
    example.com.            86400   IN      MX      10 classroom.example.com.
    example.com.            86400   IN      MX      10 mail.example.com.
    example.com.            86400   IN      MX      100 mailbackup.example.com.
    ```
  
  - `PTR` 资源记录（Pointer Record）：
    
    指针记录，提供 IPv4 或 IPv6 地址到主机名的映射，常用于反向 DNS 解析。
    
    ```textile
    4.0.41.198.in-addr.arpa. 785    IN      PTR     a.root-servers.net.
    0.3.0.0.2.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.e.3.a.b.3.0.5.0.1.0.0.2.ip6.arpa. 86400 IN  PTR a.root-servers.net.
    ```
  
  - `TXT` 资源记录（TXT Record）：
    
    - 将名称映射到编码为可打印 `ASCII` 字符的任意文本。它们通常用于提供各种电子邮件身份验证方案（如 SPF、DKIM 和 DMARC 等等）所用的数据，用于验证域所有权（如，针对 Google 与 Facebook），以及用于其他杂项用途。
    
    - 可记录额外的任何信息，也可为空。
      
      ```bash
      lwn.net.   27272   IN  TXT    "google-site-verification: sVlx-LS_z1es5D-fNSUNXrqr3n9Y4F7tOr7HNVMKUGs"
      lwn.net.   27272   IN  TXT    "v=spf1 a:mail.lwn.net  a:prod.lwn.net a:git.lwn.net a:ms.lwn.net -all"
      ```

- 主机与资源记录：
  
  - 无论是客户端还是服务端的主机都将具有以下 DNS 资源记录：
    
    - ⼀个或多个 A 和/或 AAAA IP 地址记录。
    
    - ⽤于将其 IP 地址反向映射到名称的 PTR 记录。
    
    - （可选）⼀个或多个 CNAME 记录，⽤于将备⽤名称映射到其规范主机名。
  
  - 除了区域中的主机的记录之外，DNS 区域还具有以下资源记录：
    
    - 正好⼀个 SOA 记录。
    
    - 其每个权威名称服务器的 NS 记录。
    
    - （可选）⼀个或多个 MX 记录，⽤于将域名映射到邮件交换器，后者接收以域名结尾的地址的电⼦邮件。
    
    - （可选）各种功能（如 SPF 或 Google 站点验证）的⼀个或多个 TXT 记录。
    
    - （可选）⽤于在域中查找服务的⼀个或多个 SRV 记录。

### DNS 缓存及实现：

- 众所周知的 80/20 原则，即用在网站访问上 80% 的时间我们都在看那些 20% 的网站。

- 若将常用的信息缓存起来，那么后面的请求将会大大缩短访问时间。

- 对于 DNS 请求来说，如果中间的各级 DNS 服务器都设有缓存，缓存那些经常会用到的域名信息，那将大大提高 DNS 的解析效率。

- DNS 查询解析涉及的 DNS 缓存：
  
  - 浏览器缓存：
    
    当用户通过浏览器访问某域名时，浏览器首先会在其缓存中查找是否有该域名对应的 IP。
  
  - 系统缓存：
    
    详见下文 "Linux 中 DNS 缓存相关的组件与服务"。
  
  - 本地 DNS 缓存（如果有）：
    
    此处特指硬件服务器，与系统缓存对应。
  
  - 路由器 router 缓存：
    
    一般现在的出口路由器也会设有 DNS 缓存，当以上都查询不到，会在路由器缓存中查询。
  
  - ISP DNS 缓存：
    
    ISP 网络提供的 DNS 服务器缓存。
  
  - 根域名（root）、顶级域名（TLD）、次级域名（SLD）服务器等缓存：
    
    当完成一次完整的查询之后，中间的各 DNS 服务器缓存会将返回的结果一次存入自己的缓存中，以备下次使用提高查询效率。

- DNS 缓存当然能够加快 DNS 查询的效率，但有时也会影响网络问题排查。此时需将 DNS 缓存清除，如下所示：
  
  - Windows 系统：
    
    ```powershell
    > ipconfig /displaydns
    # 查看本地的 DNS 缓存信息
    ```
    
    ![](pictures/win-dns-cache-1.jpg)
    
    ```powershell
    > ipconfig /flushdns
    # 清除本地 DNS 缓存
    ```
    
    ![](pictures/win-dns-cache-2.jpg)
  
  - Linux 系统：
    
    - 若使用 `dnsmasq` 缓存服务器，重启该缓存服务可清除 DNS 缓存。
      
      ```bash
      $ sudo systemctl restart dnsmasq.service
      ```
    
    - 若未使用，则根据各 Linux 发行版各自的缓存服务来清除。如 Ubuntu 中默认使用 systemd 解析的服务来缓存 DNS 条目，如下方式清除 DNS 缓存：
      
      ```bash
      $ sudo systemd-resolve --flush-caches
      ```
  
  - 对于 Web 浏览器的 DNS 缓存，可在地址栏输入 `chrome://net-internals/#dnsChrome`，然后再点击 `clear host cache`，也可以通过 `Ctrl+Shift+Del` 打开清除浏览器数据窗口进行清除。
    
    ![](pictures/chrome-dns-cache-clear.jpg)

- Linux 中 DNS 缓存相关的组件与服务：
  
  - DNS 缓存可使用 Linux 内核模块或缓存名称服务器来实现，通常有 bind、dnsmasq、nscd、unbound 等。
  
  - 💪 缓存名称服务器在本地缓存中存储 DNS 查询结果，并且在 `TTL` 到期后从缓存中删除资源记录，同时，若其自身缓存中无对应的解析条目，也可通过转发（`forward`）方式将 DNS 请求转发给上游 DNS 服务器完成解析后再缓存该条目。通常设置缓存名称服务器以代表本地网络上的客户端执行查询。这降低了公网上的 DNS 流量，从而提高了 DNS 名称解析的效率。随着缓存的增加，缓存名称服务器从其本地缓存中应答越来越多的客户查询，从而提高 DNS 性能。
  
  - `dns_resolver` 内核模块：
    
    dns_resolver 模块是 Linux 内核中用于处理 DNS 查询的模块，它提供了一组函数，用于处理 DNS 查询的过程，包括解析 DNS 名称、返回 IP 地址等。dns_resolver 模块可以直接用于实现域名解析功能，但它并不是专门用于缓存 DNS 查询结果的工具。因此，如果系统需要使用本地 DNS 缓存，可以考虑使用其他工具。
    
    ![](pictures/dns_resolver-module.png)
  
  - 🏷 `dnsmasq` 缓存名称服务器：
    
    关于 dnsmasq 服务的说明可参考 [DNSmasq 详细解析及详细配置](https://cloud.tencent.com/developer/article/1174717)，此篇文章中说明了该服务常用的场景。
  
  - `nscd` 缓存名称服务器（下文详细介绍）：
    
    nscd 是一种常用的缓存工具，它可以实现 DNS 查询结果的缓存。nscd 是一个递归的 DNS 缓存系统，可以缓存域名的 DNS 查询结果，并防止重复的 DNS 查询。nscd 可以用于优化系统的 DNS 查询性能，特别是在需要频繁查询域名的情况下，如网站缓存或邮件缓存等。在使用 nscd 时，需要指定 nscd 的配置文件和缓存大小等参数，以适应系统的需求。
  
  - `unbound` 缓存名称服务器（下文详细介绍）：
    
    unbound 是一个开源的 DNS 解析器，也可以用于实现本地 DNS 缓存。unbound 可以缓存 DNS 查询结果，并支持正向解析和反向解析。unbound 适用于需要解析大量域名或需要频繁查询域名的情况，例如网站解析、DNS 轮询等。在使用 unbound 时，需要指定 unbound 的配置文件和缓存大小等参数，以适应系统的需求。

- nscd 缓存名称服务器使用示例：
  
  - `nscd`（Name Service Cache Daemon）是一种能够缓存 `passwd`、`group`、`hosts` 的本地缓存服务，分别对应三个源 `/etc/passwd`、`/etc/hosts`、`/etc/resolv.conf`。
  
  - 其最为明显的作用是加快 DNS 解析速度，在接口调用频繁的内网环境建议开启。
  
  - 开启 nscd 的 hosts 缓存服务后，每次内部接口请求不会都发起 DNS 解析请求，而是直接命中 nscd 缓存散列表，从而获取对应服务器 IP 地址，这样可在大量内部接口请求时减少接口的响应时间。
  
  - nscd 服务配置文件：`/etc/nscd.conf`
    
    ![](pictures/nscd-conf-1.jpg)
    
    ![](pictures/nscd-conf-2.jpg)
    
    `/var/log/nscd.log` 日志文件中的 timeout 换算：
    
    ```bash
    $ date --date='@<second>'
    ```
  
  - 👉 关于 **`/etc/nsswitch.conf`** 配置文件的说明：该文件定义 hosts 域名解析的查询顺序，若 files（/etc/hosts）查询失败，将查询 dns（/etc/resolv.conf）。
    
    ![](pictures/nsswitch-query-sequence-1.jpg)若该文件中定义的查询路径均无法成功返回，但在 nscd hosts 缓存散列表中存在域名与 IP 的对应关系，查询也将成功返回。
    
    ![](pictures/nsswitch-query-sequence-2.jpg)
    
    💥 /etc/nsswitch.conf 配置文件中对 DNS 解析的类型与顺序非常重要，若配置错误将导致无法解析，即使在静态与动态解析文件均配置正确的情况下。该文件中的 `hosts` 行如下所示：
    
    ```bash
    hosts:    files  dns  myhostname
    # files：本地 /etc/hosts 中查找主机名
    # dns：可根据网口配置文件中的定义或 /etc/resolv.conf 文件中的定义执行 DNS 查找
    # myhostname：若查找失败，则 localhost 和系统的主机名的结果应使用 nss-myhostname (8) 进行解析。
    ```

- unbound 缓存名称服务器使用示例：
  
  - unbound 是开源的默认启用了 `DNSSEC` 验证的更安全的缓存名称服务器。
  
  - 常用的 unbound 服务配置使用如下所示：
    
    ```bash
    ### unbound 服务端 ###
    $ sudo yum install -y unbound
    # 安装 unbound 服务软件包
    
    $ sudo vim /etc/unbound/unbound.conf
      ...
      server:
        interface: 172.25.250.10
        # 指定具有 IPv4 地址的网络接口应答 DNS 请求，默认监听 localhost（127.0.0.1 与 ::1）
        # 指定 0.0.0.0 与 ::0 以绑定所有可用的接口
        interface-automatic: no
        # 关闭网络接口自动选择
        access-control: 172.25.250.0/24 allow
        # 控制可访问服务端的客户端及访问权限
        # 默认所有的客户端为拒绝访问（refuse）
        # 访问权限如下所示：
        #   deny：阻止访问，且不发送任何响应。
        #   refuse：阻止访问，并向客户端发送 REFUSED 错误。
        #   allow：允许递归访问
        #   allow_setrd：允许递归访问，rd bit 需强制启用。
        #   allow_snoop：允许递归与非递归访问
        #   deny_non_local：拒绝所有非本地解析应答的请求
        #   refuse_non_local：拒绝所有非本地解析应答的请求，并向客户端发送 REFUSED 错误。
        domain-insecure: "example.com"
        # 为特定未签名区域跳过 DNSSEC 验证
        # 注意：
        #   1. 默认情况下，unbound 会执行 DNSSEC 验证，以验证是否收到了所有 DNS 响应。
        #      通常希望执行此操作，但有时会有一个未正确签名的内部域，因此无法通过 DNSSEC 验证。
        #   2. 请勿只是为了解决无法解释的 DNS 解析问题⽽禁用 DNSSEC 验证。
        #   3. DNSSEC 失败可能表明已收到了被正确拒绝的欺骗响应。
      forward-zone:
        name: .
        # 转发所有的 DNS 查询请求
        forward-addr: 172.25.250.254
        # 转发 DNS 查询请求至指定的主机，该主机可以是其他缓存名称服务器，也可是上游 DNS 服务器。
        # 也可使用 forward-host 参数指定主机域名
      ...
    ```
    
    ```bash
    ### unbound 服务端 ###
    $ sudo unbound-control-setup
    # 生成 unbound 服务端私钥与证书
    $ sudo unbound-checkconf
    # 检查 /etc/unbound/unbound.conf 是否存在语法错误
    $ sudo firewall-cmd --permanent --add-service=dns
    $ sudo firewall-cmd --reload
    # 启用防火墙允许 DNS 流量（53/TCP 与 53/UDP）
    $ sudo systemctl enable --now unbound.service
    ```
    
    启用 unbound 服务后，在客户端使用 dig 或 nslookup 命令进行测试验证服务端缓存功能是否生效，并且是否可转发 DNS 请求至上游 DNS 服务器完成递归查询。
    
    ```bash
    ### unbound 客户端 ###
    $ dig A serverb.lab.example.com @servera.lab.example.com
    # 使用 servera 节点（unbound 服务端）查询 serverb 节点的 A 记录
    ```
    
    ```bash
    ### unbound 服务端 ###
    $ sudo unbound-control dump_cache
      START_RRSET_CACHE
      ;rrset 64696 1 0 7 3
      lab.example.com.        64696   IN      NS      bastion.lab.example.com.
      ;rrset 596 1 0 8 3
      serverb.lab.example.com.        596     IN      A       172.25.250.11
      ;rrset 596 1 0 3 3
      bastion.lab.example.com.        596     IN      A       172.25.250.254
      END_RRSET_CACHE
      START_MSG_CACHE
      msg serverb.lab.example.com. IN A 33152 1 596 3 1 1 1
      serverb.lab.example.com. IN A 0
      lab.example.com. IN NS 0
      bastion.lab.example.com. IN A 0
      END_MSG_CACHE
      EOF
    # 客户端查询请求在本地服务端中的缓存信息
    # 如上所示，缓存名称服务器将 DNS 请求转发至上游 DNS 服务器 bastion 节点
    ```
  
  - unbound 服务端对缓存的导入与导出：
    
    ```bash
    $ sudo unbound-control dump_cache > /path/to/file
    # 将名称解析缓存重定向至指定文件（默认为标准输出）
    $ sudo unbound-control load_cache < /path/to/file
    # 将指定文件中的缓存信息导入至缓存名称服务器中
    ```

- unbound 服务端对缓存的清除：
  
  管理员需要定期从缓存中清除过期的资源记录。在资源记录上的 `TTL` 到期之前，缓存中的错误和过期资源记录将阻止已更正的对应资源记录供客户端使用。此时，需执行以下命令强制清除过期记录，而不必等待 `TTL` 过期：
  
  ```bash
  $ sudo unbound-control flush <fqdn>
  $ sudo unbound-control flush serverb.lab.example.com
  # 清除指定资源记录的缓存
  
  $ sudo unbound-control flush_zone <zone_name>
  $ sudo unbound-control flush_zone lab.example.com
  # 清除指定 zone 的所有资源记录
  ```

### DNS 域名服务器类型：

- DNS 技术可提供下面三种类型的服务器：
  
  - 主服务器（master）：
    
    在特定区域内具有唯一性，负责维护该区域内的域名与 IP 地址之间的对应关系。
  
  - 从服务器（slave）：
    
    从主服务器中获得域名与 IP 地址的对应关系并进行维护，以防主服务器宕机等情况。
  
  - 缓存服务器（cache）：
    
    通过向其他域名解析服务器查询获得域名与 IP 地址的对应关系，并将经常查询的域名信息保存到服务器本地，以此来提高重复查询时的效率。

- 主服务器是用于管理域名和 IP 地址对应关系的真正服务器，从服务器协助主服务器，可分散部署在各个国家、省市或地区，以便让用户就近查询域名，从而减轻主服务器的负载压力。

- 缓存服务器不太常用，一般部署在企业内网的网关位置，用于加速用户的域名查询请求。

- 🤘 目前可实现 DNS 服务的常用组件：`Bind`、`DNSmasq`、`CoreDNS`

### BIND DNS 实现权威域名解析服务：

> 以下涉及的配置文件可点击 [sc-col/bind9-master-dns](https://github.com/Alberthua-Perl/sc-col/tree/master/bind9-master-dns)、[sc-col/bind9-slave-dns](https://github.com/Alberthua-Perl/sc-col/tree/master/bind9-slave-dns) 以获取。

- `BIND`（Berkeley Internet Name Domain，伯克利因特网名称域）服务是全球范围内使用最广泛、最安全可靠且高效的域名解析服务。

- BIND 可以配置为主（master）或从（slave），以服务于每个区域的 DNS 请求，首先需配置主 DNS 权威服务器，主服务器将作为从 DNS 权威服务器的源。

- 当 BIND 配置为从时，它使用区域传输方法从主服务器获取区域数据的副本。

- DNS 域名解析服务建议在生产环境中安装部署时加上 `chroot`（俗称牢笼机制）扩展包，以便有效地限制 bind 服务仅能对自身的配置文件进行操作，以确保整个服务器的安全。

- bind 服务安装与配置：
  
  - 各服务器节点角色分布：
    
    ![](pictures/named-service-nodes.jpg)
  
  - 各服务器节点物理拓扑示意：
    
    ![](pictures/named-service-arch.jpg)
    
    并非所有 DNS 权威服务器都必须是公共服务器。如，可能决定仅使用主 DNS 权威服务器来管理区域文件，并将区域信息发布到从 DNS 权威服务器。主服务器可以是私有服务器，但从服务器将是面向公共的，为外部客户端提供权威应答。这可保护主服务器免受攻击。
    
    ✨ 此架构中仅使用主服务器与从服务器，未部署缓存域名服务器。若使用缓存域名服务器可将其部署于客户端与从 DNS 权威服务器之间，它一方面缓存客户端的查询解析条目，另一方面可将来自客户端的 DNS 查询请求转发至从服务器上。可参考如下分布：
    
    `client -> unbound-cache-nameserver -> slave-nameserver <-> master-nameserver`

- **主 DNS 权威服务器部署：**
  
  1️⃣ 安装 bind 与相关软件包：
  
  ```bash
  $ sudo yum install -y bind bind-utils
  ```
  
  RHEL 8 中的 bind 软件包默认将服务配置为基本的递归缓存名称服务器，它也被配置为 localhost 以及相关域和地址的主服务器，以从 root 名称服务器卸除负载。此默认配置还将访问权限仅限于本地主机上的程序。它侦听 IPv4 和 IPv6 环回接口 `127.0.0.1` 和 `::1` 的 `53 UDP/TCP` 端口上的连接。
  
  2️⃣ bind 服务程序中三个关键文件：
  
  - `/etc/named.conf` 主配置文件：
    
    包含 `options` 段、`logging` 段、`zone` 段、`include` 指令，还可包含可选的 `view` 段。 
  
  - `/etc/named.rfc1912.zones` 区域配置文件：
    
    - 正/反解记录清单文件
    
    - 定义了域名与 IP 地址解析规则保存的文件位置以及服务类型等内容，而没有包含具体的域名、IP 地址对应关系等信息。
    
    - type 类型有三种：`hint`（根区域）、`master`（主区域）、`slave`（从区域）
    
    - 其中常用的 master 和 slave 指的就是主服务器和从服务器。
    
    - 可将自定义的 zone 区域添加到区域配置文件的最下面，也可将该文件中的原有信息全部清空，而只保留自己的域名解析信息。 
  
  - `/var/named/` 目录：
    
    - 数据配置文件目录（正/反解记录文件）
    
    - 该目录用来保存域名和 IP 地址真实对应关系的数据配置文件
    
    - /var/named/named.ca 保存全球 13 组根域名服务器的主机名与 IP 地址。
    
    - Linux 中 bind 服务的名称为 `named`。
    
    > 若将从区域文件保存在 `/var/named/slaves` 中，那么当从服务器启动时，它会将该区域的缓存版本与主服务器上的当前版本进行比较，若为最新状态，则使用它。若区域不是最新的，或者文件不存在，则 named 会执行区域传送，并将结果缓存到该文件中。
  
  3️⃣ 配置 bind 服务的主配置文件：
  
  ```bash
  $ sudo vim /etc/named.conf 
  ```
  
  ![](pictures/named-conf-demo.jpg)通过配置主 DNS 服务器 /etc/named.conf 文件来实现当主 DNS 服务器上的区域发生更改时通知所有从 DNS 服务器进行区域更新。
  
  ![](pictures/named-conf-notify.jpg)
  
  4️⃣ 配置 bind 服务的区域配置文件：
  
  ```bash
  $ sudo vim /etc/named.rfc1912.zones
  ```
  
  ![](pictures/named-zone-conf.png)
  
  5️⃣ 创建区域数据配置文件：
  
  ```bash
  $ sudo cp -a /var/named/named.localhost /var/named/lab.example.com.zone
  $ sudo cp -a /var/named/named.lookback /var/named/10.197.11.arpa
  # 拷贝正向与反向解析文件模板，并保持原来的所属用户与用户组
  ```
  
  ![](pictures/var-named-zone-data-1.jpg)
  
  ```bash
  $ sudo vim /var/named/lab.example.com.zone
  ```
  
  ![](pictures/var-named-zone-data-2.jpg)
  
  ```bash
  $ sudo vim /var/named/10.197.11.arpa
  ```
  
  ![](pictures/var-named-zone-data-3.jpg)
  
  6️⃣ 重启 bind DNS 服务并设置开机自启动：
  
  ```bash
  $ sudo named-checkconf
  # 检查主配置文件、区域配置文件、正/反解记录文件的语法，若存在错误该服务启动失败。
  $ sudo systemctl enable named.service
  $ sudo systemctl start named.service
  ```

- **从 DNS 权威服务器部署：**
  
  1️⃣ 安装 bind 与相关软件包：
  
  ```bash
  $ sudo yum install -y bind bind-utils
  ```
  
  2️⃣ 配置 bind 服务的主配置文件：
  
  ```bash
  $ sudo vim /etc/named.conf
  ```
  
  ![](pictures/named-conf-slave.png)
  
  3️⃣ 配置 bind 服务的区域配置文件：
  
  ```bash
  $ sudo vim /etc/named.rfc1912.zones
  ```
  
  ![](pictures/named-zone-conf-slave.png)
  
  ```bash
  $ sudo systemctl enable named.service
  $ sudo systemctl start named.service
  # 重启 bind DNS 服务并设置开机自启动
  ```

- **安全加密传输：**
  
  - 互联网中的绝大多数 DNS 服务器（超过95%）都是基于 Bind 域名解析服务搭建的，而 bind 服务为了提供安全的解析服务，已经对 `TSIG`（RFC 2845）加密机制提供了支持。
  
  - TSIG 主要是利用了密码编码的方式来保护区域信息的传输（Zone Transfer），即 TSIG 加密机制保证了 DNS 服务器之间传输域名区域信息的安全性。
  
  - 配置主 DNS 服务器加密传输：
    
    ```bash
    $ sudo yum install -y bind-chroot
    # 安装 bind-chroot 软件包
    
    $ dnssec-keygen -a HMAC-MD5 -b 128 -n HOST master-slave
    # 当前目录中生成 dnssec 公私钥文件
    # 使用 HMAC-MD5 算法加密生成名为 master-slave 的 128 位密钥
    ```
    
    ![](pictures/named-master-sectrans-1.png)
    
    ```bash
    $ sudo vim /var/named/chroot/etc/transfer.key
    # 创建 dnssec 密钥验证文件
    ```
    
    ![](pictures/named-master-sectrans-2.png)
    
    ```bash
    $ sudo chown root:named /var/named/chroot/etc/transfer.key
    $ sudo chmod 0640 /var/named/chroot/etc/transfer.key
    $ sudo ln /var/named/chroot/etc/transfer.key /etc/transfer.key
    # 更改 dnssec 密钥验证文件的所有者、所属组、权限及创建硬链接
    $ sudo ls -lh /var/named/chroot/etc/transfer.key
    ```
    
    ![](pictures/named-master-sectrans-3.png)
    
    ```bash
    $ sudo vim /etc/named.conf
    # 添加如下参数开启 bind 服务的密钥验证功能
    ```
    
    ![](pictures/named-master-sectrans-4.png)
    
    ![](pictures/named-master-sectrans-5.png)
    
    ```bash
    $ sudo systemctl restart named.service
    # 重启 bind 服务使配置生效
    ```
  
  - 配置从 DNS 服务器加密传输：
    
    ```bash
    $ sudo yum install -y bind-chroot
    # 安装 bind-chroot 软件包
    
    $ sudo rm -f /var/named/slaves/*
    # 删除原缓存的 DNS 记录文件
    $ sudo systemctl restart named.service
    # 重启 bind 服务
    # 由于主 DNS 服务器已配置加密传输 DNS 记录，因此在该目录中不再生成新的 DNS 记录。
    
    $ sudo vim /var/named/chroot/etc/transfer.key
    # 创建 dnssec 密钥验证文件
    ```
    
    ![](pictures/named-slave-sectrans-1.png)
    
    ```bash
    $ sudo chown root:named /var/named/chroot/etc/transfer.key
    $ sudo chmod 0640 /var/named/chroot/etc/transfer.key
    $ sudo ln /var/named/chroot/etc/transfer.key /etc/transfer.key
    # 更改 dnssec 密钥验证文件的所有者、所属组、权限及创建硬链接
    
    $ sudo vim /etc/named.conf
    # 添加如下参数开启 bind 服务的密钥验证功能
    ```
    
    ![](pictures/named-slave-sectrans-2.png)
    
    ```bash
    $ sudo systemctl restart named.service
    # 重启 bind 服务即可加密传输同步 DNS 记录文件
    ```

- 验证 DNS 解析服务：
  
  - 使用 `dig` 命令验证DNS正向解析结果：
    
    ![](pictures/dig-test-1.png)
    
    ![](pictures/dig-test-2.png)
  
  - 使用 `dig -x` 命令验证 DNS 反向解析结果。
  
  - 也可使用 `nslookup` 命令验证 DNS 正向与反向解析结果。
    
    ![](pictures/nslookup-test.jpg)

### dig 命令使用示例：

- `bind-utils` 软件包提供 dig、host、nslookup 命令工具。其中 dig 命令为常用的 DNS 查询工具。

- dig 命令输出六段信息，如下所示：
  
  ![](pictures/dig-usage-1.png)
  
  status 状态通常包括：
  
  - `NOERROR` 状态：
    
    表示已成功解析查询
  
  - `SERVFAIL` 状态：
    
    常见原因是 DNS 服务器无法与权威名称服务器进行通信。可能是由于权威名称服务器不可用。还可能是由于网络问题干扰了 DNS 服务器与权威名称服务器之间的通信。
  
  - `NXDOMAIN` 状态：
    
    1️⃣ 表示没有找到与查询的名称相关联的记录。若查询被定向到一个对该名称没有权威的服务器，那么服务器的缓存可能包含名称的 <font color=red>**负缓存**</font>（该缓存对正确查询产生干扰）。用户可以等待服务器使该名称的负缓存到期，或者向服务器管理员提交请求，从服务器的缓存中清空名称。从缓存中删除名称后，服务器将查询权威名称服务器以接收该名称的当前资源记录。如，权威名称服务器已更新至最新的名称解析，但缓存名称服务器的 TTL 尚有余期，因此可清空缓存再进行查询，同时可降低 TTL 时间解决此问题。
    
    2️⃣ 可能出现 NXDOMAIN 返回代码的另一种情况是在查询包含 `orphaned CNAME` 的 `CNAME` 记录时。在 CNAME 记录中，记录右侧的名称（即规范名称）应指向一个包含 A 或 AAAA 记录的名称。如果这些关联的记录不存在或者之后被删除，则 CNAME 记录中的规范名称将变为孤立。发生此情况时，对 CNAME 记录中所有者名称的查询将不再可解析，并且将导致 NXDOMAIN 返回代码。
  
  - `REFUSED` 状态：
    
    表示 DNS 服务器的某个策略限制阻止了其客户端的查询。策略限制通常是在 DNS 服务器上实施，以限制哪些客户端可以进行递归查询和区域传输请求。导致 REFUSED 返回代码的一些常见原因是客户端配置为查询错误的 DNS 服务器，或者 DNS 服务器配置错误导致有效的客户端请求被拒绝（如前文的 unbound 缓存名称服务器配置）。
  
  🏷 flags 标记的说明：
  
  - `aa` 代表权威名称服务器，而源自缓存名称服务器的响应不是权威的，因此不设置 aa 标志。
  
  - 应答来自缓存的另一个迹象是后续查询的响应中资源记录的 TTL 值的减少。缓存数据的 TTL 连续倒数到过期，而权威数据的 TTL 始终为静态。

- 常用命令示例如下所示：
  
  ```bash
  $ dig <domain_name>
  $ dig +short <domain_name>
  # 只查询返回对应域名的 IP 地址（A 记录中的 IP 地址）
  ```
  
  ![](pictures/dig-usage-2.png)
  
  ```bash
  $ dig +trace <domain_name> @<dnsserver_ip>
  # 使用指定的 DNS 服务器查询追踪整个 DNS 分级查询过程 
  ```
  
  ![](pictures/dig-usage-3.png)
  
  ```bash
  $ dig [+short] ns com
  $ dig [+short] ns stackexchange.com
  # 查询不同层级域中的 NS 服务器及对应的 A 与 AAAA 记录
  ```
  
  ![](pictures/dig-usage-4.png)
  
  ```bash
  $ dig +[short|trace] [a|ns|soa|cname|mx] <domain_name> @<dnsserver_ip>
  # 指定 DNS 服务器针对不同的记录类型对域名进行追踪查询或只返回简要结果
  # 其中域名可为 com、github.com、www.baidu.com 等，DNS 服务器可为本地 DNS 服务器或
  # 公共 DNS 服务器。 
  ```

- 💥 关于 DNS 使用的协议类型：
  
  尽管 DNS 大多数使用 `UDP` 协议，但当响应的大小超过 `512` 字节（支持 DNS (`EDNS`) 扩展机制的 DNS 解析器和服务器为 `4096` 字节）时，解析器必须从 UDP 切换到 TCP，再重试查询。若允许 53/UDP 端口上的流量，但不允许 53/TCP 端口上的流量，则在解析器遇到超过其可以通过 UDP 处理的响应时，将看到截断通知，后跟主机无法访问的报错。
  
  ```bash
  $ dig A labhost1.example.com @dns.example.com
    ;; Truncated, retrying in TCP mode.
    ;; Connection to 172.25.1.11#53(172.25.1.11) for labhost1.example.com failed: host unreachable.
  ```
  
  将 `tcp` 或 `vc` 选项与 dig 配合使用，以帮助确定 DNS 查询是否可以通过 TCP 成功进行。这些选项强制 dig 使用 TCP，而非表现如下默认行为：首先使用 UDP，仅当遇到较大响应时才回退为使用 TCP。
  
  ```bash
  $ dig +tcp A workstation.lab.example.com @172.25.250.254
  # 强制使用 TCP 协议查询指定主机的 A 记录
  ```

### host 命令使用示例：

- host 命令的使用方法与 dig 命令非常类似，可视为 dig 命令的简化版。

- 如下所示：
  
  ```bash
  $ host <domain_name>
  # 查看 DNS 正向解析记录
  $ host <ip_address>
  # 查看 DNS 反向解析记录，类似 dig -x 命令。
  ```
  
  ![](pictures/host-usage.png)

### nslookup 命令使用示例：

- nslookup 命令有交互式和非交互式两种工作模式。

- 在命令行中直接输入 nslookup，无需输入任何参数即进入交互模式，由 `>` 提示。

- 交互模式，如下所示：
  
  ![](pictures/nslookup-usage-1.png)
  
  ![](pictures/nslookup-usage-2.png)
  
  ![](pictures/nslookup-usage-3.png)

- 非交互模式，如下所示：
  
  ```bash
  $ nslookup -type=soa <domain_name>
  # 查看相应域名的 SOA 记录，与 dig soa 命令类似。
  ```
  
  ![](pictures/nslookup-usage-4.png)
  
  ```bash
  $ nslookup -type=ns <domain_name>
  # 查看相应域名的 NS 记录，与 dig ns 命令类似。 
  ```
  
  ![](pictures/nslookup-usage-5.png)
  
  ```bash
  $ nslookup -type=a <domain_name>
  # 查看相应域名的 A 记录，与 dig 命令或 dig a 命令类似。
  ```
  
  ![](pictures/nslookup-usage-6.png)

### DNS 安全问题：

- 域名抢注：
  
  有些黄牛会批量抢注大量域名，然后转身高价卖给那些对域名感兴趣的人，谋取暴利。

- DNS 缓存污染：
  
  中间 DNS 服务器将上级服务器返回结果进行修改，给客户端一个改变了（污染）的信息。

- DNS 劫持：
  
  - 与 DNS 缓存污染比较类似，不过这种可以被外部第三者劫持进行恶意修改。
  
  - 有些恶意的域名服务器故意更改一些域名的解析结果，将用户引向一个错误的目标地址，这种称为 DNS 劫持，主要用来阻止用户访问某些特定的网站，或者是将用户引导到广告页面。
  
  - 如下图所示：
    
    ![](pictures/dns-hijack.png)

- DNS 欺骗：
  
  - 使用假的 DNS 应答来欺骗用户，使其相信该假地址，并且抛弃真正的 DNS 应答。
  
  - 在一台主机发出 DNS 请求后，它就开始等待应答，如果此时有一个看起来正确（拥有和 DNS 请求一样的序列号）的应答包，它就会信以为真，并且丢弃稍晚一点到达的应答。
  
  - 如下图所示：
    
    ![](pictures/dns-cheat.png)
  
  - 通常可配置 bind、unbound 缓存名称服务器的 DNSSEC 验证预防 DNS 欺骗的发生。

- DNS 放大攻击：
  
  一种 `DDoS` 攻击，利用 DNS 回复包比请求包大的特点，放大流量，伪造请求包的源 IP 地址为受害者 IP，将应答包的流量引入受害者的服务器。

- 系统上运行的 DNS 服务存在漏洞，导致被黑客获取权限，从而篡改 DNS 信息。

- DNS 设置不当，导致泄漏一些敏感信息，提供给黑客进一步攻击提供有力信息。

### 参考链接：

- [使用 nsswitch 控制 Linux DNS 解析顺序](https://www.cnblogs.com/shengulong/p/7019260.html)
- [Linux 应用 - 使用 nscd 作为本地 DNS 缓存](https://blog.csdn.net/qq_35550345/article/details/107045573?utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~aggregatepage~first_rank_v2~rank_aggregation-1-107045573.pc_agg_rank_aggregation&utm_term=linux+%E6%9C%AC%E5%9C%B0dns%E7%BC%93%E5%AD%98%E6%97%B6%E9%97%B4&spm=1000.2123.3001.4430)
- [8 个问题彻底搞透 DNS 协议](https://mp.weixin.qq.com/s?__biz=MzI1OTY2MzMxOQ==&mid=2247488895&idx=1&sn=41b86262b7e7a689fcb30e4f9f7a6ab4&chksm=ea7421c7dd03a8d1ca18194f3b23b64d6bb020597d5979e322b3ac349102d1786ddadbd88dca&mpshare=1&srcid=&sharer_sharetime=1592995451255&sharer_shareid=da47a1ec8f737b150ddf73eced1d76b6&from=timeline&scene=2&subscene=1&clicktime=1592996648&enterid=1592996648&ascene=2&devicetype=android-28&version=27000f51&nettype=WIFI&abtest_cookie=AAACAA%3D%3D&lang=zh_CN&exportkey=AZaUKR5o%2BgsYr2%2B%2By%2FBAcso%3D&pass_ticket=YnXHtAKfkz6t4LIK%2BVqP%2BmSSHxZrJFJ49y%2F3zM6ki4q%2FGl2BK6PNtO%2FD9F9DvJTk&wx_header=1)
- [DNS 原理入门](http://www.ruanyifeng.com/blog/2016/06/dns.html)
- [使用 Bind 提供域名解析服务](https://www.linuxprobe.com/chapter-13.html)
- [CentOS 7 安装配置本地 DNS（BIND）服务器（Master-Slave）](https://www.kclouder.cn/centos-7-dns-bind/)
- [DNS 域名服务Bind（中）- Bind 配置文件](https://blog.csdn.net/ysdaniel/article/details/6994109)
- [Linux DNS 之 nslookup 命令](https://cloud.tencent.com/developer/article/1083201)
- [4.5. Securing DNS Traffic with DNSSEC](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html-single/security_guide/index#sec-Securing_DNS_Traffic_with_DNSSEC)
- [通过向 DNS 服务发送 SRV 查询请求获取 kubernetes 集群内所有 Service 信息](https://mozillazg.com/2021/11/security-use-dns-srv-to-get-all-service-info.html#)
