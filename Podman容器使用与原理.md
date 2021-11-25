## Podman 容器使用与原理

##### 文档说明：

- 实验用 OS 版本：
  - CentOS 7.9、RHEL 8.0、RHEL 8.2、Ubuntu 20.04.3 LTS
  
- 实验用 kernel 版本：
  - 3.10.0-1160.41.1.el7.x86_64
  - 4.18.0-193.el8.x86_64
  - 5.14.0-1.el7.elrepo.x86_64
  
- 实验用 Podman 版本：1.6.4、3.2.3、3.3.1

- 实验用 podman-compose 版本：0.1.8

- 若未做特殊说明，以下示例均于 `RHEL 8.2`（`4.18.0-193.el8.x86_64`）上执行，Podman 版本为 `3.2.3`。

- 该文档中未涉及 podman 命令的基础使用方法，可参阅 [该文档](https://mp.weixin.qq.com/s/MDi4RB5V60EGl3ii9usD0Q) 加以熟悉。

  

##### 文档目录：

- Podman 的特性概述
- Podman 版本兼容性比较
- Podman 的扩展功能
- Podman 在不同 OS 版本中的安装
- Podman 的网络实现原理（rootfull 与 rootless）
- Podman rootless 容器用户映射实现方式
- Podman 的 macvlan 网络实现
- podman 与 podman-compose 使用示例
- Podman 使用报错示例
- Podman 有待测试功能
- 参考链接



##### Podman 的特性概述：

- LXC、`LXD`（Go 语言开发）、`systemd-nspawn` 均可作为 Linux 容器，但缺少容器跨主机运行与应用打包的能力。
- Docker 与 Podman 可使用容器镜像实现应用打包发布，快速且轻量。
- Docker 与 Podman 都使用 `runC`（Go 语言开发）作为底层 `oci-runtime`。
- Docker 与 Podman 都支持 `OCI Image Fromat`（Go 语言开发），都能使用 DockerHub 上的容器镜像，而 systemd-nspawn 无法使用它们的镜像。
- 👉 Podman 使用 `CNI`（Go 语言开发）作为 rootfull 容器网络底层，实现比 Docker 网络层略微简单但原理相同。 
- 相对于 LXD 与 systemd-nspawn，CNI 可以避免编写大量的网络规则。
- 🚀 为了实现普通用户 rootless 容器网络，Podman 可以使用 `slirp4netns` 程序，避免 `kernel space` 中的大量 `veth pair` 虚拟接口的出现, 并且性能更好。
- Docker 运行容器必须使用守护进程且使用 root 权限，存在系统安全问题，而 Podman 针对此问题使用以下两个特性加以解决，如下所示：
  - Podman 支持无守护进程（`no-daemon`）运行容器。
  - Podman 支持普通用户运行 `rootless` 容器，即，普通用户直接运行容器无需提权具有 root 权限。
- 虽然 Docker 与 Podman 的实现原理不同，但对于使用者而言其 CLI 十分相似，可平滑地从 Docker 过渡至 Podman。
- Podman 的目标不是容器的编排，编排可以使用更加专业的 Kubernetes、OpenShift、Rancher 等，使用 Podman 可以更轻量的运行容器且不受 root 权限的安全问题，即便是 root 用户也无法查看其它普通用户空间下的容器，Podman 通过 `user namespace` 进行隔离。
- 👉 Podman 可使用 `systemd service` 单元文件直接管理容器，实现容器服务随系统启动而启动。
- 👉 Podman 里集成了 `CRIU`，因此 Podman 中的容器可以在单机上热迁移。
- 由于 Kubernetes 将从 `v1.24.x` 版本后放弃使用 `dockershim` 接口层，容器运行时可选择使用 `Containerd` 或者 `CRI-O`，两者虽然均支持 OCI image 规范，但它们不是面向使用者或开发者直接管理容器或镜像的工具，而 Podman 可直接面向使用者或开发者操作容器或镜像。



##### Podman 版本兼容性比较：

- Podman 版本、kernel 版本与 OS 版本的兼容性将直接影响普通用户使用 rootless 容器。

- 如下所示，kernel 不支持 rootless 容器：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/centos79-kernel-not-support-podman-rootless.jpg)

- 普通用户 rootless 容器兼容性比较：

  | Podman 版本 | OS 版本      | kernel 版本                 | 是否支持 rootless |
  | :---------- | :----------- | :-------------------------- | :---------------- |
  | 1.6.4       | CentOS 7.9   | 3.10.0-1160.41.1.el7.x86_64 | no                |
  | 1.6.4       | CentOS 7.9   | 5.14.0-1.el7.elrepo.x86_64  | yes               |
  | 3.2.3       | RHEL 8.0/8.2 | 4.18.0-193.el8.x86_64       | yes               |

  > 📌注意：rootless 容器特性的支持取决于 kernel 的版本，不取决于 OS 与 Podman 的版本。

  - 由于 `user namespace` 特性在 kernel `4.9.0` 之后出现，因此升级 kernel 即可解决 rootless 问题。

  - 关于 rootless 特性在 RHEL 8 中的设置，可 [点击此处](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/building_running_and_managing_containers/assembly_starting-with-containers_building-running-and-managing-containers#proc_setting-up-rootless-containers_assembly_starting-with-containers) 参考 Red Hat 的官方配置说明。



##### Podman 的扩展功能：

- `cockpit-podman` 软件包作为 cockpit 插件可集成于 `Web UI` 中，实现 Web UI 管理容器。

  - cockpit-podman 服务安装与启用：

    ```bash
    $ sudo yum install -y cockpit-podman
    $ sudo systemctl enable --now cockpit.socket
    $ sudo systemctl status cockpit.service
    # 安装 cockpit-podman 软件包，并启用 cockpit 服务。
    $ sudo netstat -tunlp | grep 9090
    # 查看 systemd 监听的 9090 端口是否启用
    ```

  - 在 Web UI 中可查看并管理 podman 容器与镜像：

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/cockpit-podman-1.jpg)

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/cockpit-podman-2.jpg)

- `podman-compose` 旨在使用更轻量的方式实现`单机容器编排`，以用于替换 `docker-compose`，这种方式将不再依赖守护进程与 root 权限，同时可使用 rootless 容器，详细示例见下文。

- podman-compose 使用 `Python` 开发，因此可直接使用 `pip3` 安装该组件，或使用 rpm 软件包方式安装。

- 由于 podman-compose 依然处于 `dev` 阶段，仅作为功能测试使用，暂未受到 GA 环境支持。



##### Podman 在不同 OS 版本中的安装：

- CentOS 7.x/8.x 或 RHEL 7.x/8.x 中：yum 命令使用 podman `rpm` 软件包安装

  ```bash
  $ sudo yum install -y podman-3.2.3-0.11.module_el8.4.0+942+d25aada8.x86_64
  # 安装 podman 最新版本，低版本 podman 存在较多 bug。
  # 注意：
  #   1. 需配置 CentOS 8 的 yum 软件源以安装最新版的 podman 及其依赖软件包
  #   2. yum 安装 podman 时也将安装 containernetworking-plugins 软件包
  ```

- 🤘 Ubuntu 20.04.2 LTS 中：apt-get 命令使用 podman `deb` 软件包安装

  ```bash
  $ . /etc/os-release
  # 查看当前的系统发行版
  $ echo "deb https://download.opensuse.org/repositories/devel:/kubic:/libcontainers:/stable/xUbuntu_${VERSION_ID}/ /" | sudo tee /etc/apt/sources.list.d/devel:kubic:libcontainers:stable.list
  $ curl -L "https://download.opensuse.org/repositories/devel:/kubic:/libcontainers:/stable/xUbuntu_${VERSION_ID}/Release.key" | sudo apt-key add -
  # 添加 podman 软件源与 apt 公钥
  $ sudo apt-get update -y
  $ sudo apt-get upgrade -y
  # 更新系统软件源并升级系统软件包
  $ sudo apt-get install -y podman
    Reading package lists... Done
    Building dependency tree       
    Reading state information... Done
    ...
    The following NEW packages will be installed:
      catatonit conmon containernetworking-plugins containers-common criu crun fuse-overlayfs fuse3 libfuse3-3 libnet1 libprotobuf-c1
      podman podman-machine-cni podman-plugins
    ...
  # 安装 podman 与相关的软件包，包括 conmon、containernetworking-plugins、crun 等。  
  ```
  
  安装参考链接：
  
  - [Podman Doc - installation](https://podman.io/getting-started/installation)
  - [Easy to Install Podman on Ubuntu 20.04](https://www.hostnextra.com/kb/easy-to-install-podman-on-ubuntu-20-04/)
  - [podman from devel:kubic:libcontainers:stable project](https://software.opensuse.org//download.html?project=devel%3Akubic%3Alibcontainers%3Astable&package=podman)



##### Podman 的网络实现原理（rootfull 与 rootless）：

- Podman 支持的容器网络模式如下所示：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/podman-network-mode.jpg)

- root 用户运行 rootfull 容器网络分析：

  - 默认情况下，rootfull 容器使用 bridge 网络模式，并且在未创建任何容器前系统上不会自动创建 `cni-podman0 `网桥，只有创建容器后自动生成。

  - root 用户使用全局范围内的 CNI 插件，podman 默认使用 `bridge`、`portmap` 插件，其配置文件如下：

    ```bash
    $ cat /etc/cni/net.d/87-podman-bridge.conflist
    {
      "cniVersion": "0.4.0",
      "name": "podman",
      "plugins": [
        {
          "type": "bridge",
          "bridge": "cni-podman0",
          "isGateway": true,
          "ipMasq": true,
          "hairpinMode": true,
          "ipam": {
            "type": "host-local",
            "routes": [{ "dst": "0.0.0.0/0" }],
            "ranges": [
              [
                {
                  "subnet": "10.88.0.0/16",
                  "gateway": "10.88.0.1"
                }
              ]
            ]
          }
        },
        {
          "type": "portmap",
          "capabilities": {
            "portMappings": true
          }
        },
        {
          "type": "firewall"
        },
        {
          "type": "tuning"
        }
      ]
    # 该配置文件位于 Podman 源码 cni/87-podman-bridge.conflist
    # Podman 可调用 bridge、portmap 等 CNI 插件
    
    $ sudo podman inspect <container_name> | jq .[0].HostConfig.NetworkMode
      "bridge"
    # root 用户创建的容器网络模式
    ```

  - root 用户创建具有端口映射的容器时，iptables filter 表与 nat 表规则将相应增加：

    ```bash
    # ----- filter 表中创建新容器后的新增规则 -----
    *filter
    -A FORWARD -m comment --comment "CNI firewall plugin rules" -j CNI-FORWARD
    -A CNI-FORWARD -m comment --comment "CNI firewall plugin admin overrides" -j CNI-ADMIN
    -A CNI-FORWARD -d 10.88.0.3/32 -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
    # 新增规则：允许 3 层转发目标地址为 10.88.0.3 的流量（进入容器的流量），conntrack 模块进行连接状态追踪。
    # 当容器通过 MASQUERADE 对外访问，回包再次进入容器宿主机时不再通过 DNAT 转发，而通过 conntrack 
    # 记录的连接状态直接转发至该规则并通过 cni-podman0 网桥进入容器。
    -A CNI-FORWARD -s 10.88.0.3/32 -j ACCEPT
    # 新增规则：允许 3 层转发源地址为 10.88.0.3 的流量（出容器的流量）。
    
    # ----- nat 表中创建新容器后的新增规则 -----
    *nat
    -A PREROUTING -m addrtype --dst-type LOCAL -j CNI-HOSTPORT-DNAT
    -A POSTROUTING -m comment --comment "CNI portfwd requiring masquerade" -j CNI-HOSTPORT-MASQ
    -A POSTROUTING -s 10.88.0.3/32 -m comment --comment "name: \"podman\" id: \"2d2b3521457cb1d9b7ae6657304d05789a854e7a48916276a40da543df9aa217\"" -j CNI-b6c5fb6c593e895d843cb5bd
    # 新增规则：来自于 10.88.0.3 容器的流量转发至 CNI-b6c5fb6c593e895d843cb5bd 链
    -A OUTPUT -m addrtype --dst-type LOCAL -j CNI-HOSTPORT-DNAT
    # 启用 CNI 后即创建的规则，该规则接收来自本地应用的流量并转发至 CNI-HOSTPORT-DNAT 链
    -A CNI-HOSTPORT-SETMARK -m comment --comment "CNI portfwd masquerade mark" -j MARK --set-xmark 0x2000/0x2000
    -A CNI-HOSTPORT-MASQ -m mark --mark 0x2000/0x2000 -j MASQUERADE
    ### 以下 6 条在创建新容器时同时创建 
    -A CNI-HOSTPORT-DNAT -p tcp -m comment --comment "dnat name: \"podman\" id: \"2d2b3521457cb1d9b7ae6657304d05789a854e7a48916276a40da543df9aa217\"" -m multiport --dports 8843 -j CNI-DN-b6c5fb6c593e895d843cb
    # 自定义 DNAT 链，发送至本地 8843 端口的流量转发至 CNI-DN-b6c5fb6c593e895d843cb 链。
    -A CNI-b6c5fb6c593e895d843cb5bd -d 10.88.0.0/16 -m comment --comment "name: \"podman\" id: \"2d2b3521457cb1d9b7ae6657304d05789a854e7a48916276a40da543df9aa217\"" -j ACCEPT
    # 允许转发目标网段为 10.88.0.0/16 的流量（进入容器的流量），该网段为容器所在的网络。
    -A CNI-b6c5fb6c593e895d843cb5bd ! -d 224.0.0.0/4 -m comment --comment "name: \"podman\" id: \"2d2b3521457cb1d9b7ae6657304d05789a854e7a48916276a40da543df9aa217\"" -j MASQUERADE
    # MASQUERADE 出容器流量
    -A CNI-DN-b6c5fb6c593e895d843cb -s 10.88.0.0/16 -p tcp -m tcp --dport 8843 -j CNI-HOSTPORT-SETMARK
    -A CNI-DN-b6c5fb6c593e895d843cb -s 127.0.0.1/32 -p tcp -m tcp --dport 8843 -j CNI-HOSTPORT-SETMARK
    -A CNI-DN-b6c5fb6c593e895d843cb -p tcp -m tcp --dport 8843 -j DNAT --to-destination 10.88.0.3:443
    # 自定义 DNAT 链实现容器宿主机至容器的端口映射
    ```

  - 🚀 示例：外部访问容器内 Web 服务时，涉及的宿主机 iptables：

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/external-access-container-web-service-iptables.jpg)
    
    从外部访问容器内 Web 服务时，流量将通过 PREROUTING 链及自定义链（`CNI-HOSTPORT-DNAT`、`CNI-DN-xxxx`、`DNAT`），经由 FORWARD 链及自定义链（`CNI-FORWARD`）的三层转发与 `cni-podman0` 网桥的二层转发进入容器，容器对外响应的流量将经过 cni-podman0 网桥转发，并经过 CNI-FORWARD 链与 POSTROUTING 链及自定义链（`CNI-HOSTPORT-MASQ`）出容器宿主机。
    
  - 🚀 示例：直接从容器内访问外部时，返回容器的回包将直接使用 conntrack 模块追踪的连接状态，流量通过 `CNI-FORWARD` 链的三层转发与 cni-podman0 的二层转发至容器中，即，回包进入容器宿主机不再通过`CNI-HOSTPORT-DNAT`链。

    如下所示，相关的 DNAT 链无流量通过（蓝框），CNI-FORWARD 链均有流量通过（蓝框）。

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/container-access-external-iptables.jpg)

    > 📌 **Kubernetes 相关问题提示：**
    >
    > 1. 容器或 pod 通过 cni 网桥桥接的方式在 Kubernetes 或 OpenShift3 中需在计算节点（worker node）上配置 `net.bridge.bridge-nf-call-iptables` 与 `net.bridge.bridge-nf-call-iptables6` 内核参数，使 cni 二层网桥可调用 iptables 的 conntrack 模块，以解决前后端 pod 在同一节点上时，由于 pod 直连 cni 二层网桥，而二层网桥只实现二层转发，无法追踪前后端的连接状态，造成后端 pod 向前端 pod 回包时无法处于同一连接链路的问题，可 [点击此处](https://imroc.cc/k8s/faq/why-enable-bridge-nf-call-iptables/) 获得更多帮助。
    > 2. 使用以上内核参数时，需加载 `br_netfilter` 内核模块方能生效。
    
  - 使用 `iperf3` 工具的容器测试不同 rootfull 容器之间的网络性能，如下所示：

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/rootfull-container-to-container-bandwidth.jpg)

- 普通用户运行 rootless 容器网络分析：

  - `slirp4netns` 程序支持 user rootless network namespace，而非通过 `iptables` 与 CNI 实现。

  - 普通用户创建的容器网络模式为 `slirp4netns`（slirp4netns 软件包实现）。

    ```bash
    $ podman inspect <container_name> | jq .[0].HostConfig.NetworkMode
      "slirp4netns"
    # 普通用户创建的 rootless 容器网络模式
    ```
    
  - 每个普通用户运行 rootless 容器都将生成 slirp4netns 进程用于隔离该用户的 `network namespace`，以下分别使用 godev 与 hualf 用户运行 rootless 容器：
  
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/godev-rootless-container.jpg)
  
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/hualf-rootless-container.jpg)
  
  - slirp4netns 实现的网络模式与带宽比较：
  
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/rootless-slirp4netns-networking.jpg)
  
  - 使用 `iperf3` 工具的容器测试不同 rootless 容器之间的网络性能，如下所示：
  
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/rootless-container-to-container-bandwidth.jpg)
  
    对比 rootfull 容器之间的网络性能来看，slirp4netns 实现的 rootless 容器在不同的网络命名空间内的通信性能损耗较大，而 rootfull 容器之间的网络性能相比前者在此次测试中高出近 5 倍。
  
  - 关于 slirp4netns 更加详细的内容，请参考 [Github 项目](https://github.com/rootless-containers/slirp4netns)。



##### Podman rootless 容器用户映射实现方式：  

  - Podman rootless 容器的实现核心在于解决 network namespace（NetNS） 与 user namespace（UserNS） 的问题，前文已介绍 NetNS 的实现方式，后文将介绍 UserNS 的实现方式。
  
  - 若要使用 rootless 容器，需确认 OS 是否开启 user namespace 功能：
  
  ```bash
  $ sudo sysctl -a | grep user\.max_user_namespaces
    user.max_user_namespaces = 47494
  ```
  
  - 系统上每创建一个用户就会在 `/etc/subuid` 与 `/etc/subgid` 中生成对应用户在其用户命名空间中的映射规则，以 /etc/subuid 为例，参数以冒号分隔，每个参数含义如下所示：
  
    - 第一个参数（uid）：用户名称
    
    - 第二个参数（loweruid）：用户命名空间中起始的映射 uid
  
  - 第三个参数（count）：用户命名空间内部与外部可映射 uid 数量（可理解为所有容器普通用户的 uid 数量和）
  
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/rootless-user-namespace-mapping.jpg)
  
  - 以上两个文件允许运行进程的 uid 映射范围，在 `/proc/<pid>/uid_map` 中定义。
  
  - 可过滤容器 `conmon` 进程的 pid 确认每个容器中的 uid 映射情况，参见以下示例。
  
  - 关于以上两个文件的具体说明可参考 `newuidmap` 与 `newgidmap` 命令的 man 手册。
  
  - 可参考 Podman 官方推荐的命令创建 uid 的映射，如下所示：
  
    ```bash
    $ sudo usermod --add-subuids 10000-75535 $(whoami)
    
    # ----- 示例 -----
    $ sudo cat /etc/subuid
      appuser:10000:500
    $ sudo cat /etc/subgid
      appuser:500:50
    # 该用户创建的 user namespace 中可以使用从 10000 开始的 500 个 UID 和从 500 开始的 50 个 GID 的映射。
    ```

- 🚀 示例：

  普通用户 hualf 在 /etc/subuid 中映射为 hualf:165536:65536，说明在该用户的用户命名空间中可嵌套一个或多个用户命名空间（或容器），每个容器中的 root 用户 uid 0 都映射为 hualf 用户的 uid 1001（运行容器进程的用户），而容器中普通用户的 uid 映射至宿主机的 subuid 范围中，对于此例 subuid 范围为 165536~231071，容器中的 uid 1 用户映射为宿主机 uid 165536，因此容器中 admin 用户 uid 1000 映射为宿主机 uid 166535（165536+999）。

  通过容器宿主机上每个普通用户的用户命名空间的 subuid 映射范围，可分配众多 uid 在 rootless 容器中运行应用进程。

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/user-namespace-subuid-mapping-1-edited.png)

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/user-namespace-subuid-mapping-2-edited.png)



##### Podman 的 macvlan 网络实现:

- `macvlan` 作为 CNI 在 Kubernetes 与 OpenShift v4 中作为 `Multus CNI` 支持的额外插件类型使用愈加广泛，集群中除了常规使用的 Flannel、Calico 等作为 `slow path` 的插件外，要求高性能的业务流量可使用 macvlan 直连 pod 宿主机物理网口实现 `fast path`。

- 为后续熟悉以上场景的实现，因此在 Podman `rootfull` 容器中使用 macvlan 网络模式。

- 关于 macvlan 的基础知识可参考 [Linux 虚拟网卡技术：Macvlan](https://mp.weixin.qq.com/s?__biz=MzU1MzY4NzQ1OA==&mid=2247484064&idx=1&sn=ffd745069b6c4aeac0589de00467b2f2&chksm=fbee426dcc99cb7bdf26f5e6a21bbeaebba7ccd384a02f850d4461ea92331ed140edf98ffaec&mpshare=1&scene=1&srcid=03049MKwF55OVgEZ4OCH39wd&sharer_sharetime=1583337046541&sharer_shareid=8eaca72194dae7b3d51d5c708436eee4#rd) 与 [linux 网络虚拟化：macvlan](https://cizixs.com/2017/02/14/network-virtualization-macvlan/) 

- macvlan 特性由 `Linux kernel` 支持，笔者的实验环境满足 macvlan 的要求，请使用如下命令确定：

  ```bash
  $ sudo lsmod | grep macvlan
  # 若无任何返回，说明还未加载 macvlan 内核模块。
  $ sudo modprobe macvlan
  # 加载 macvlan 内核模块，若执行报错，说明 kernel 不支持该特性。
  ```

- podman 与 macvlan 类型网络的集成，如下所示：

  ```bash
  $ sudo podman network create -d macvlan -o parent=ens33 <network_name>
    /etc/cni/net.d/<network_name>.conflist
  # 创建 macvlan 类型网络  
  $ sudo podman network ls
  $ sudo /opt/cni/bin/dhcp daemon
  # 在另一个窗口中启动 dhcp 守护进程供 macvlan 插件调用，为容器网口分配 IP 地址。
  $ sudo podman run -it --rm \
    --name <container_name> --network=<network_name> \
    <container_image>:<tag> /bin/sh
  # 创建支持 macvlan 类型网络的 rootfull 容器  
  ```

- 从与 rootfull 容器在同一广播域的其他节点上 ping 该容器，可正常通信：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/podman-macvlan-network.png)



##### podman 与 podman-compose 使用示例：

- 示例 1：

  👉 使用 podman 命令登录 `Quay` 公共容器镜像仓库并推送镜像：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/podman-push-quay.jpg)

  👉 搜索并拉取 Red Hat 容器镜像仓库中的镜像列表：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/podman-pull-image.jpg)

- 示例 2：

  👉 从头创建 pod 并附加额外的容器：

  ```bash
  $ podman pod create --name <pod_name> [-p <host_port>:<pod_port>]
  # 使用 pause 容器镜像从头创建 pod
  # 若之后需在 pod 中创建使用端口映射的容器，需要在创建 pod 之初指定端口映射关系，无法在创建容器时指定，由于 pod
  # 提供了其中所有容器的共享网络命名空间。
  # 注意：若需指定多个端口，可同时使用多个 -p 选项。
  $ podman run -d --name <container_name> --pod <pod_name> <container_image>:<tag>
  # 创建容器将其附加到 pod 中
  $ podman pod [ps|list|ls]
  # 查看已存在的 pod
  $ podman pod [stop|rm] <pod_name>
  # 停止或删除 pod，将一并删除 pod 中的所有容器。
  ```

  > 📌**注意：**
  >
  > 1. `k8s.gcr.io/pause:3.5` 镜像拉取需要科学上网。
  > 2. 若无法拉取，可先拉取 `registry.aliyuncs.com/google_containers/pause:3.5` 镜像，再更改其 `tag` 即可。

  👉 随创建容器时同时创建 pod：

  ```bash
  $ podman run -d \
    --name <container_name> --pod new:<pod_name> \
    [-p <host_port>:<pod_port>] \
    <container_image>:<tag>
  # 随创建容器时同时创建 pod
  $ podman run -d \
    --name <container_name> --pod <pod_name> \
    <container_image>:<tag>
  # 在 pod 中创建新的容器
  ```

  如下所示，创建名为 nginx-docs 的容器并同时创建名为 docker-docs 的 pod，也可创建其他容器添加至 pod 中，使用该容器即可访问 nginx-docs 容器（两者共享网络命名空间）：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/podman-run-pod-create.jpg)

- 示例 3：

  🤘 部署并使用云原生轻量级对象存储 `MinIO Server`：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/minio-server-cloud-native-object-storage-demo-1.jpg)

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/minio-server-cloud-native-object-storage-demo-2.jpg)

  > 🔊注意：以上示例已将 podman 与 systemd 集成实现普通用户的 rootless 容器开机自启动。

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/minio-server-cloud-native-object-storage-demo-3.jpg)

  关于 MinIO Server 分布式对象存储的详细内容，请 [参考官网](https://min.io/)

- 示例 4：

  🔊 root 用户运行 rootfull 容器：多个容器间通过 `cni-podman0` 网桥互相通信。

  🤘 [部署 loganalyzer 管理集中式日志](https://github.com/Alberthua-Perl/scripts-confs/tree/master/deploy-rsyslog-viewer)

- 示例 5：

  🔊 普通用户或 root 用户运行容器：

  🔊 同一个 `pod` 中的多个容器使用共享网络命名空间，并通过 `link` 链接至指定的容器建立通信。

  🤘 使用 `podman-compose` 部署轻量级 Git 代码版本控制仓库：`Gogs + PostgreSQL`

  - 关于 podman-compose 的安装可参考 [GitHub 项目](https://github.com/containers/podman-compose)

  > 📌注意：可考虑如何使用 podman-compose 部署轻量级 `Gitea + Drone` CI 平台

  - 关于 Gogs 项目的详细内容可参考 [Gogs GitHub 项目](https://github.com/gogs/gogs)

  - Gogs 代码版本控制仓库使用 Golang 语言开发，可与后端 MySQL、PostgreSQL、SQLite3、TiDB 等集成。

  - 此处使用容器化部署 Gogs，并与 PostgreSQL 集成。

  - 部署用主机上必须先安装 podman 与 podman-compose，并拉取相应容器镜像加速部署过程，如下所示：

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/podman-image-list.jps.JPG)

    > 📌**注意：**
    >
    > podman-compose 使用创建 `pod` 将多个容器组建成 pod 的方式进行容器编排，因此必须具有 `pause` 容器镜像提供 pod 的共享网络命名空间与挂载命名空间。

  - 使用普通用户部署，过程如下所示：

    ```bash
    $ mkdir -p gogs-app/gogs-data/{gogs,gogs-logs,postgresql}
    # 创建用于存储 gogs 与 postgresql 数据映射的目录
    $ sudo chown -R 100999:100999 gogs-app/gogs-data/{gogs,gogs-logs}
    # 更改映射目录的属组，否则容器启动权限报错。
    $ getenforce
      Enforcing
    # 确认系统处于 enforcing SELinux 状态，需设置目录映射时的标签。
    # 也可禁用 SELinux，若禁用 SELinux，以下两步可不执行并且去除 podman-compose 定义文件中的 "Z"。
    $ sudo semanage port -a -t http_port_t -p tcp 10800
    $ sudo semanage port -a -t ssh_port_t -p tcp 10022
    # 添加自定义端口至 SELinux 数据库中，否则由于权限问题无法访问并安装 Gogs。
    $ vim gogs-app/gogs-postgres-podman-compose.yaml
    # 如下所示 podman-compose 的 yaml 定义文件
    ```

    ```yaml
    version: "3"
    services:
      postgresql:
        image: docker.io/library/postgres:14.1-bullseye
        container_name: "gogs-postgresql"
        volumes:
          - "./gogs-data/postgresql:/var/lib/postgresql:Z"
        environment:
          - "POSTGRES_USER=gogs"
          - "POSTGRES_PASSWORD=redhat"
          - "POSTGRES_DB=gogs"
        ports:
          - "5432:5432"
    
      gogs:
        image: docker.io/gogs/gogs:0.12
        container_name: "gogs"
        volumes:
          - "./gogs-data/gogs:/data:Z"
          - "./gogs-data/gogs-logs:/app/gogs/log:Z"
        ports:
          - "10022:22"
          - "10800:3000"
        links:
          - postgresql
    ```

    ```bash
    $ podman-compose -f gogs-app/gogs-postgres-podman-compose.yaml --project gogs-app up
    # 启动 Gogs 与 PostgreSQL 容器，并指定项目名称。
    # 若不指定项目名称，项目默认为 yaml 文件所在的目录名称。
    # 首次启动容器时，所有的启动与运行日志将打印至终端屏幕上，该终端不可关闭，直至关闭所有服务容器后将自动退出。
    $ podman-compose -f gogs-app/gogs-postgres-podman-compose.yaml --project gogs-app ps
      using podman version: podman version 3.2.3
      podman ps -a --filter label=io.podman.compose.project=gogs-app
      CONTAINER ID  IMAGE                                     COMMAND               CREATED      STATUS          PORTS                                                                   NAMES
      2bed211ffe60  docker.io/library/postgres:14.1-bullseye  postgres              6 hours ago  Up 3 hours ago  0.0.0.0:10022->22/tcp, 0.0.0.0:10800->3000/tcp, 0.0.0.0:5432->5432/tcp  gogs-postgresql
      2c7d0de4b0a0  docker.io/gogs/gogs:0.12                  /bin/s6-svscan /a...  6 hours ago  Up 3 hours ago  0.0.0.0:10022->22/tcp, 0.0.0.0:10800->3000/tcp, 0.0.0.0:5432->5432/tcp  gogs
      0
    # 查看 podman-compose 管理的容器服务
    $ podman ps
      CONTAINER ID  IMAGE                                     COMMAND               CREATED      STATUS          PORTS                                                                   NAMES
      b6df150a3a49  k8s.gcr.io/pause:3.5                                            6 hours ago  Up 6 hours ago  0.0.0.0:10022->22/tcp, 0.0.0.0:10800->3000/tcp, 0.0.0.0:5432->5432/tcp  c3a10da46f18-infra
      2bed211ffe60  docker.io/library/postgres:14.1-bullseye  postgres              6 hours ago  Up 3 hours ago  0.0.0.0:10022->22/tcp, 0.0.0.0:10800->3000/tcp, 0.0.0.0:5432->5432/tcp  gogs-postgresql
      2c7d0de4b0a0  docker.io/gogs/gogs:0.12                  /bin/s6-svscan /a...  6 hours ago  Up 3 hours ago  0.0.0.0:10022->22/tcp, 0.0.0.0:10800->3000/tcp, 0.0.0.0:5432->5432/tcp  gogs
    # 查看正在运行的容器，包含 infra 容器。
    ```

  - 所有容器正常运行后，使用 `http://<容器宿主机 IP 地址>:10800` 访问 Gogs 安装界面，需填入的值参考如下：

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/gogs-settings.jpg)
    
    - Run User 值：默认 `git`。
    - Domain 值：若要从其他主机连接至 Gogs 仓库，Domian 必须配置为容器宿主机的 IP 地址或主机名。
    - SSH Port 值：podman-compose 定义文件中对外暴露的 SSH 端口号。
    - HTTP Port 值：默认 `3000` 端口。
    
  - Web 页面中最后需设置 Gogs 管理员账号以完成安装。

  - 安装完成后，使用管理员账号登录或重新注册新账号登录与使用。

  - 如下所示，使用 `devops` 用户创建新代码库并完成 commit 提交：

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/gogs-git-repository.jpg)

  - 如需关闭 Gogs 代码仓库，请使用以下方法停止 gogs 与 postgresql 容器服务即可：

    ```bash
    $ podman-compose -f gogs-app/gogs-postgres-podman-compose.yaml --project gogs-app stop gogs postgresql
      using podman version: podman version 3.2.3
      podman stop -t 10 gogs
      gogs
      0
      podman stop -t 10 gogs-postgresql
      gogs-postgresql
      0
    $ podman ps
      CONTAINER ID  IMAGE                 COMMAND     CREATED       STATUS             PORTS                                                                   NAMES
      b6df150a3a49  k8s.gcr.io/pause:3.5              30 hours ago  Up 39 minutes ago  0.0.0.0:10022->22/tcp, 0.0.0.0:10800->3000/tcp, 0.0.0.0:5432->5432/tcp  c3a10da46f18-infra  
    ```

    > 💥**注意：**
    >
    > 切不可直接使用 podman-compose 命令的 `down` 子命令，该子命令将所有相关的容器与 pod 全部删除，pod 删除后无法将其中的各容器映射至宿主机对应的目录中，即使原始数据依然保留于目录中。

  - 重新启动 Gogs 代码仓库的方式，如下所示：

    ```bash
    $ podman-compose -f gogs-app/gogs-postgres-podman-compose.yaml --project gogs-app start gogs postgresql
      using podman version: podman version 3.2.3
      podman start gogs
      gogs
      0
      podman start gogs-postgresql
      gogs-postgresql
      0
    ```

  - 以上 gogs-postgres-podman-compose.yaml 文件可参考 [此处](https://github.com/Alberthua-Perl/dockerfile-s2i-demo/blob/master/gogs-postgres-compose/gogs-postgres-podman-compose.yaml)。
    

##### Podman 报错示例：

- podman 容器镜像仓库的配置方式：
  - 全局配置：/etc/containers/registries.conf
  - 局部配置：$HOME/.config/containers/registroes.conf
  
- 若 podman 安装后在以上配置中未唯一指定的容器镜像仓库，那么在拉取容器镜像时，将交互式提示用户选择容器镜像仓库。

- 示例 1：

  👉 podman v3.2.3 登录 Harbor v1.8.1 身份认证报错：

  ```bash
  $ podman login harbor.domain12.example.com:8880
    Username: admin
    Password: redhat
    Error: authenticating creds for "harbor.domain12.example.com:8880": error pinging docker registry 
    harbor.domain12.example.com:8880: Get "https://harbor.domain12.example.com:8880/v2/": 
    http: server gave HTTP response to HTTPS client
  # Podman 未做任何配置登录 Harbor 报错，该 Harbor 容器镜像仓库未配置 TLS 加密传输。
  # 报错显示 Harbor 响应 HTTP 请求，而 Podman 发送 HTTPS 请求登录。
  # 因此，将 Podman 配置为发送 HTTP 请求的客户端。
  ```

  🤔 解决方式一：

  ```bash
  $ podman login --tls-verify=false harbor.domain12.example.com:8880
    Username: admin
    Password: redhat
    Login Succeeded!
  # Podman 未进行任何配置，直接使用 --tls-verify=false 选项即可认证登录。
  ```

  🤔 解决方式二：

  ```bash
  $ mkdir -p ~/.config/containers/ && cd ~/.config/containers/
  # 创建普通用户 rootless 容器的目录
  $ vim ~/.config/containers/registries.conf
    unqualified-search-registries = ['harbor.domain12.example.com:8880']
  
    [[registry]]
    location = "harbor.domain12.example.com:8880"
    insecure = true
    # If true, unencrypted HTTP as well as TLS connections with untrusted
    # certificates are allowed.
    block = false
  # 配置未加密传输的 Harbor 容器镜像仓库的主机名与端口
  
  $ podman login --log-level=debug harbor.domain12.example.com:8880
    INFO[0000] podman filtering at log level debug
    DEBU[0000] Called login.PersistentPreRunE(podman login --log-level=debug harbor.domain12.example.com:8880)
    DEBU[0000] overlay storage already configured with a mount-program
    DEBU[0000] Merged system config "/usr/share/containers/containers.conf"
    DEBU[0000] overlay storage already configured with a mount-program
    DEBU[0000] Using conmon: "/usr/bin/conmon"
    ...
    DEBU[0000] Using OCI runtime "/usr/bin/runc"
    DEBU[0000] Default CNI network name podman is unchangeable
    INFO[0000] Setting parallel job count to 13
    DEBU[0000] Loading registries configuration "/home/kiosk/.config/containers/registries.conf"
    DEBU[0000] Loading registries configuration "/etc/containers/registries.conf.d/000-shortnames.conf"
    DEBU[0000] Loading registries configuration "/etc/containers/registries.conf.d/001-rhel-shortnames.conf"
    DEBU[0000] Loading registries configuration "/etc/containers/registries.conf.d/002-rhel-shortnames-overrides.conf"
    DEBU[0000] No credentials for harbor.domain12.example.com:8880 found
    Username: admin
    Password: # 交互式输入登录密码
    DEBU[0004] Looking for TLS certificates and private keys in /etc/docker/certs.d/harbor.domain12.example.com:8880
    DEBU[0004] GET https://harbor.domain12.example.com:8880/v2/
    DEBU[0004] Ping https://harbor.domain12.example.com:8880/v2/ err Get "https://harbor.domain12.example.com:8880/v2/": http: 
    server gave HTTP response to HTTPS client (&url.Error{Op:"Get", URL:"https://harbor.domain12.example.com:8880/v2/", 
    Err:(*errors.errorString)(0xc000590030)})
    ...
    DEBU[0004] GET http://harbor.domain12.example.com:8880/service/token?account=admin&service=harbor-registry
    DEBU[0004] GET http://harbor.domain12.example.com:8880/v2/
    DEBU[0004] Stored credentials for harbor.domain12.example.com:8880 in credential helper containers-auth.json
    Login Succeeded!
    DEBU[0004] Called login.PersistentPostRunE(podman login --log-level=debug harbor.domain12.example.com:8880)
  # Podman 默认使用 TLS 加密传输
  # 以上配置文件将使 Podman 以 HTTP 方式认证登录 Harbor。
  ```
  
- 示例 2：

  👉 podman v3.2.3 推送容器镜像至 Harbor v1.8.1 中显示 "不完整"：

  ```bash
  $ podman push harbor.domain12.example.com:8880/library/apache-rhce8.2-alpine:1.0
    Getting image source signatures
    Copying blob 551db21ded82 skipped: already exists
    Copying blob 8213d0880f11 skipped: already exists
    Copying blob e2eb06d8af82 skipped: already exists
    ...
    Copying blob 05e56f8d5aae skipped: already exists
    Copying blob 631e8a8040bb skipped: already exists
    Copying blob dedba5c062fc skipped: already exists
    Copying blob 0e609f35aa06 [--------------------------------------] 0.0b / 0.0b
    Copying config 34f32c2e7a [======================================] 10.0KiB / 10.0KiB
    Writing manifest to image destination
    Storing signatures
  ```
  
  从推送的返回结果显示，具有 2 层容器镜像层似乎未推送成功，但将该镜像从 Harbor 中拉取并重新运行容器后，容器能正常提供服务，因此最后 2 层镜像层实际推送成功。
  
- 示例 3：

  👉 容器镜像无任何运行或退出状态容器占用，但依然无法删除镜像，可尝试使用 `--force` 选项将其强制删除。

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/podman-rmi-error-no-container-use.jpg)
  
- 示例 4：

  👉 由于从 `dockerbub` 上直接拉取的镜像为 `docker image format`，无法使用 `podman commit` 命令提交为新的容器镜像，该命令对于 `-m` 选项不能对 docker image format 镜像生效，默认只支持 `OCI image format`，因此使用 -m 选项对容器执行提交时需强制指定 `-f docker` 才能生效。

  > 📌注意：可使用 `skopeo` 工具转换 docker image format 与 OCI image format。

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/podman-commit-warning.jpg)

- 示例 5：

  👉 podman 运行 rootfull 或 rootless busybox 容器后，`ping` 外网报错权限问题无法 ping 通外网，但使用其他工具可与外网通信，通过 [该文档](https://www.redhat.com/sysadmin/container-networking-podman) 中可知，ping 命令对`capability`敏感，容器可能缺少 `CAP_NET_RAW `capability 无法通过宿主机 ping 通外网。

  👉 当然，运行容器时指定 `--privileged` 选项可使容器获得与宿主机 root 用户同样的与宿主机交互的权限能力，但赋予的权限过高，应当压制该权限，更好的选择是对运行容器添加适当的 `Linux capabilities`。

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/podman-arch-usage/podman-busybox-capability.jpg)



##### Podman 有待测试功能：

- 前文所述，使用 podman pod 命令或使用 podman-compose 组件单机编排容器，而且 Podman 支持 `podman play kube` 命令基于 YAML 资源定义文件创建 pod，该方法类似 Kubernetes 或 OpenShift，有待测试。
- Podman 日志驱动目前只支持 `k8s-file`、`journald` 与 `none`，暂时不支持容器日志的 `JSON `格式输出，因此不能与日志收集引擎 `fluentd` 集成，由其实现 ELK/EFK 集中式的存储、索引等。
- Podman 与 `Linux capabilities` 的关系与应用在最后一个示例中有所提及，但更全面的关系有待测试。



##### 参考链接：

- [Reintroduction of Podman](https://projectatomic.io/blog/2018/02/reintroduction-podman/)
- [Using pods with Podman on Fedora](https://fedoramagazine.org/podman-pods-fedora-containers/)
- [Configuring container networking with Podman](https://www.redhat.com/sysadmin/container-networking-podman)
- [RedHat docs - Building, running, and managing Linux containers on Red Hat Enterprise Linux 8](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/building_running_and_managing_containers/index)
- [容器安全拾遗 - Rootless Container初探](https://developer.aliyun.com/article/700923)
- [Documentation for /proc/sys/user/](https://www.kernel.org/doc/html/latest/admin-guide/sysctl/user.html)
- [docker docs - Overview of Docker Compose](https://docs.docker.com/compose/)
- [CNI docs - firewall plugin](https://www.cni.dev/plugins/current/meta/firewall/)
- [CNI docs - Port-mapping plugin](https://www.cni.dev/plugins/current/meta/firewall/)
- https://fossies.org/linux/podman/docs/tutorials/basic_networking.md

