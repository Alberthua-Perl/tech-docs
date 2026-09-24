# DO285 Learning Notes (by Sreejith)

> 基于 DO285 v4.2 课程笔记整理

## DO180 部分

### Day 1

- 有些应用**完全不适合**容器化部署，例如：
  - 传统数据库（如 Oracle）
  - 任何需要访问容器所在宿主机硬件细节的应用
    - 例如网络包入侵检测软件，因为它需要访问 VM/宿主机的网卡（NIC）
  - 任何 CPU/内存/磁盘监控工具
- 提到 Oracle 数据库时，是否意味着其他数据库也不适合？
- 容器工具/运行时**必须**确保应用以安全方式部署，并且应用不会耗尽宿主机所有资源（CPU/内存/磁盘/网络）。
- 在虚拟化世界中，Hypervisor 曾负责提供这种保障。
- 在 Linux 容器技术中，我们使用三个内核功能来确保容器内的安全性、隔离性和资源限制：
  - **安全**：SELinux（Security Enhanced Linux）
  - **隔离**：Linux Namespaces
  - **资源限制**：Control Groups（Cgroups）

**容器运行时**

- 容器运行时为 **CRI-O**（底层技术是 runC）。
- RHEL8 引入了新的容器工具集：
  1. **Podman** —— 连接容器运行时的 CLI
  2. **Buildah** —— 构建容器镜像的 CLI
  3. **Skopeo** —— 管理容器镜像仓库（Container Registry）中的镜像

**Podman**

- Podman 管理整个**容器**生命周期：
  - 从镜像实例化容器 → 确保容器健康运行（运维人员使用 podman 连接运行中的容器）→ 终止容器。
- Podman 的 CLI 与相关 Docker 命令具有**命令行对等性**，方便从 Docker 世界过渡过来的用户。
- 注意：DO180 教材中没有提及 Buildah 或 Skopeo。

**镜像仓库（Registries）**

- 存在存放镜像的公共门户，通常称为**仓库（Registry）**。
- **Red Hat Container Catalog** 是 Red Hat 托管的镜像仓库，提供来自 Red Hat 及其合作伙伴的认证和支持镜像。
- Podman 可帮助从这些仓库下载和上传镜像。

**版本对比**

- OpenShift 3 = Kubernetes + Docker
- OpenShift 4 = Kubernetes + CRI-O

**OpenShift（OCP）**

- OpenShift 是企业级 Kubernetes，具有社区版（DIY）Kubernetes 没有的一些额外功能：
  - **Source-to-Image（S2I）**——可从源代码创建容器镜像
  - **ImageStreams**——跟踪镜像变更以实现持续部署
- `dnf install container-tools` 可在 RHEL8 上安装所有容器工具（podman、buildah、skopeo）。
- 所有容器都以**非 root 用户**运行，也可以由非 root 用户启动（RHEL8 和 RHEL7.7）。
- 本练习中使用 sudo 是因为 **RHEL7.6 尚不支持 rootless 容器**。
- 配置文件 `/etc/containers/registries.conf` 包含该服务器配置为推送/拉取镜像的默认镜像仓库信息。

### Day 2

- **镜像** = 精简操作系统（Just Enough OS）+ 应用二进制文件 + 依赖项。
- **UBI** = Red Hat Universal Base Image 7。
- `podman build` 实际上会在后台调用 buildah 库来执行实际构建。
- **Red Hat CoreOS（RHCOS）** 是专用于容器的操作系统，只做一件事：承载容器。
  - 它是 RHEL 的精简/稳定/剥离版，配置为作为 OpenShift 节点（master 或 worker 节点）运行。
  - RHCOS 遵循与 RHEL 相同的硬件认证标准。
  - 它使用与 RHEL 相同的内核——因此一旦硬件通过 RHEL 认证，认证同样适用于 RHCOS。
- RHCOS 是可信的容器宿主机，可用于企业 Kubernetes 平台——即 OpenShift。
- 使用 CRI-O 时，我们以非 root 用户运行**无守护进程（daemonless）**容器，完全依赖操作系统的功能和安全机制（如 SELinux、Seccomp、Namespaces、Cgroups 等）。
  - 这是一个巨大的变革：底层操作系统（而非守护进程）承担起安全责任。
- 如果以 root 用户 SSH 登录 Red Hat CoreOS 机器，服务器会**自动重启**。
  - 因此 RHCOS 通常被称为"**不可变基础设施（Immutable Infrastructure）**"。
- 在 OpenShift 早期版本中，底层 OS（Atomic 或 RHEL）由系统管理员自行配置（如打补丁、安装代理等）。
- 但在 RHCOS 中，所有变更都通过 OpenShift Kubernetes API 本身经由 **Operator Framework** 完成。
- 使用 OpenShift 4，集群内的一切都由 Operator Framework 管理。
- 你可以通过 **Custom Resource Definitions（CRD）** 创建自己的自定义 Operator。

**集群架构**

- master 节点承担运行"数据存储"组件 **Etcd** 的重任。
- 为使 master 组件高可用，至少需要 **3 个 master 节点**。
- 整个 master 节点组件统称为"**控制平面（Control Plane）**"。
- **3 个 master 节点 + N 个 worker 节点** 的组合称为 OpenShift 集群。
- Etcd 是基于**仲裁（quorum）**的集群，为维持仲裁需要至少 3 个节点。

**其他对象**

- 还有一个名为 **ImageStream** 的对象——它可以跟踪源代码或构建器镜像（builder image）的变更。
- **BuildConfig** = 持续集成（Continuous Integration）
- **DeploymentConfig** = 持续部署（Continuous Deployment）

### Day 3

- 在 Kubernetes/OpenShift 中，项目/命名空间下的**环境变量在所有 Pod 之间共享**。
- 项目/命名空间充当团队/用户所用资源的逻辑分组。
- 你可以为项目设置配额（quota）和限制/资源约束。
- 在多 Pod 应用中，比如前端、后端和数据库，部署在项目内的三个 Pod 都能看到项目内所有 Pod 的环境变量。
  - 因此前端能够发现后端和数据库中的环境变量……
- Kubernetes 使用许多映射到 Pod 服务 IP 的环境变量。
- 任何由 `oc new-app` 或 Web 界面创建的 Pod，都会由 Kubernetes 自动分配一个服务 IP。

**控制平面与数据平面**

- **控制平面（Control Plane）**：Kubernetes API / 认证 / 调度器 / 复制控制器——必须部署在 Red Hat CoreOS（RHCOS）之上。
- **数据平面（Data Plane）**：可以是 RHCOS 或 RHEL。
- 数据平面称为 worker，你的容器化应用将部署在这里，并由 master 监控。
- 在 OCP4 中，使用新的部署工具（使用 Golang 构建），名为 **openshift-install**。

**RHCOS 要点回顾**

- RHCOS 是专为容器部署而设计的目的性/精简/稳定操作系统。
- 遵循与 RHEL 相同的硬件认证。
- 在 OCP3 中，如果要扩展基础设施（添加更多 worker 节点），需要在 OCP 之外完成：部署新的 RHEL 主机，然后运行基于 Ansible 的安装工具。
- 但在 OCP4 中，要扩展集群时，OpenShift 自带 Operator 工具，也可以管理底层 RHCOS，按需扩展（添加/移除）RHCOS。
- 因此称之为"**不可变基础设施**"。
- RHCOS 通过 OCP 本身进行管理——不使用传统管理方式进行人工管理。

**Operator 与认证**

- Red Hat 已认证许多 Operator，可通过 OLM 部署，连接到 OperatorHub.io。
- 控制平面组件（API、调度器、etcd）全部作为 Pod 在集群内运行。
- **CRI-O 和 kubelet** 作为 systemd 服务实例化。
- **Kubelet** 是 Kubernetes master 的代理，运行在所有 worker 节点上。
- 注意：master 节点同时也是 worker 节点，但不用于调度客户应用容器。

**认证与授权**

- Kubernetes 默认**没有任何认证**！
- 但 OpenShift 支持更丰富的认证方案，可与现有的集中式认证机制集成，如 LDAP/AD/HTTP/Radius 等。
- Kube-api 会将认证请求代理给 openshift-api，后者配置了 OAuth 机制。
- OpenShift 不自己做认证，而是交给 OAuth——由 OAuth 连接认证源。
- 但 OpenShift 使用 RBAC 进行**授权**。

**版本对应关系**

- OpenShift 3 的次版本与上游 Kubernetes 版本紧密对应。
- OCP 3.11 —— K8s 1.11
- OCP 4.0 —— K8s 1.12
- OCP 4.2 —— K8s 1.14
- OCP 4.3 —— K8s 1.15

**Operator 详解**

- Operator 在 OCP4 中默认存在。
- 任何需要在 OCP 中部署的应用都可以通过 Operator 部署和管理。
- 传统上我们只能创建 Kubernetes API 支持的对象。
- 如果需要部署自己的对象类型怎么办？
- Operator 本质上**扩展**了 Kubernetes API 以支持自定义对象。
- 你可以通过部署相应资源的 Operator 来创建自定义资源（Custom Resources）。
- 作为管理员，你首先部署 Operator 本身，然后使用该 Operator 部署 Operator 中定义的自定义资源。
- 自定义资源需要在语法上进行定义，该语法通常称为 **Custom Resource Definition（CRD）**。
- OpenShift 默认自带 **OLM（Operator Lifecycle Manager）**，可帮助你部署 Operator。
- 需要在 Operator 中部署的自定义资源，其语法定义称为 CRD。
- CRD 通常遵循 YAML/JSON 语法——与标准 Kubernetes 对象类似。
- 部署 OCP 时会默认获得一个 Operator 目录（Operator Catalog）。
  - 通过该目录可以浏览经 Red Hat 认证的 Operator。
  - 可以部署目录中任何想要的 Operator，然后通过该 Operator 部署应用。
- Operator 部署后以 Pod 形式运行在它们自己的项目中！
- 谁能部署 Operator？只有**集群管理员**或运维团队（Ops Team）。
- 运维团队会将 Operator 部署到该 Operator 的专属项目中！
- 与 OpenShift 集群相关的 Operator 位于以 `openshift-*` 开头的项目下。

**开发自己的 Operator**

- 可以使用 **Operator SDK** 创建自己的 Operator。
- Operator SDK 帮助你使用 **Go 语言**开发自己的 Operator。
- 如果不会 Go，还有基于 **Ansible** 的 Operator 开发工具包，使用 playbook 来开发 Operator。

**部署方式**

- OpenShift 4 有两种部署方式：
  1. **FullStack Automation**（全栈自动化）
  2. **Pre-Existing Infrastructure**（预置基础设施）
- 部分文档中同样的部署方式被称为：
  - **IPI（Installer Provided Infrastructure）**——安装程序提供基础设施
  - **UPI（User Provided Infrastructure）**——用户提供基础设施
- 可以让安装程序创建所有基础设施组件（如 master、worker 节点，然后在其上部署 OCP）——这就是 FullStack Automation。
- 如果使用云提供商或虚拟基础设施提供商，可以使用全栈自动化。
- 支持全栈自动化的提供商：AWS、Azure、GCP、VMware、OpenStack。

**ImageStreams**

- ImageStreams 将相似镜像分组，通过简称让开发者的工作更轻松，同时确保它们跟踪源镜像/配置的变更。
- 如果镜像或配置发生变化，ImageStreams 会自动触发新构建。
- ImageStreams 实际上会触发新构建——从而使**持续集成**成为可能。
- 此外，OpenShift 还添加了其他功能，如 annotations，帮助实现更丰富的 **RBAC**——例如针对用户/项目的资源限制等。
- OpenShift 内置了集成镜像仓库——用于在 BuildConfig 过程中推送新构建的镜像以供复用。

### Day 4

**RBAC（基于角色的访问控制）**

- RBAC 对用户和组强制执行，映射到身份（identities）时发现的才有资格分配 RBAC 策略。
- OCP 内的授权组件使用身份资源来映射 RBAC 策略。
- **角色（Role）** = 权限的组合——应用于 OCP 集群中的对象
- **规则（Rule）** = 按设计划分的单个权限
- 角色通过称为"**绑定（binding）**"的过程应用到用户。
- 将角色绑定到用户/组。
- 创建项目会创建服务账户（service accounts），可用于提升权限，如"构建 Pod"、"部署 Pod"等。
- `oc get rolebinding.rbac` 会给出适用于该项目的本地策略详情。
- 每当用户创建项目时，该用户默认成为该项目的 **admin**。

**Code Ready Containers（CRC）**

- OCP4 不再有 minishift。
- 取而代之的是 **Code Ready Containers（CRC）**——可在笔记本电脑上部署单节点 OCP4 环境。
- 如果有 RHEL8 或最新版 Fedora，可以使用 libvirt/kvm 将 CRC 部署为虚拟机，并演示部分 OCP 任务。
- CRC 虚拟机本身至少需要 **8G 内存**，所以只在有 16G 内存的笔记本电脑上尝试。

**多租户**

- OpenShift 是**多租户 PaaS 平台**。
- 意味着多个租户将使用同一集群，互不干扰。
- 即 tenant1 只能访问自己创建的对象/资源。

**Secret 与 ConfigMap**

- Secret 会被解码并提供给应用，以便引用 secret 内部的键值对。
- 要使 secret 更安全，需要使用基于 etcd 的加密，使其在文件系统中真正安全。
- 任何你想从容器镜像中外部化的内容，都可以使用 ConfigMap。
- 顾名思义，ConfigMap = 配置映射（Configuration Mappings）。
- 回忆一下我们使用的 MYSQL 镜像，在部署数据库时通过 CLI 提供了 `MYSQL_USER`、`MYSQL_DATABASE`、`MYSQL_ROOT_PASSWORD` 等变量。
  - 对于这些值，可以使用 ConfigMap，将这些环境变量添加到部署中。
- 也可以选择使用"模板（Templates）"并将这些值作为参数传入——但 ConfigMap 是向部署提供动态变化值的另一种方式。

**S2I 与 Dockerfile 的取舍**

- 默认情况下，大多数客户会使用基于 Dockerfile 的镜像构建，并尝试在 OCP 中使用这些镜像，主要将 OCP 用作部署平台（可以说是仅用于持续部署）。
  - 换句话说，他们不会使用 Source-to-Image，因为镜像构建已由专门团队管理，使用传统的 Dockerfile 方法。
- 但在 Docker 中，大多数在线参考和文档都建议新构建的镜像以 root 用户运行。
- 如前所述，**OCP 不允许** Dockerfile 中将 `USER root` 指定为运行用户的镜像。
- 一种解决方法是重新设计 Dockerfile，将 `USER root` 改为 `USER 1001`。
  - **UID 1001** 是 OpenShift 部署 Pod 时使用的默认 UID。
- 如果使用 **Security Context Constraints（SCC）**，OCP 允许灵活地将 Pod 的权限提升为**以任意用户 ID 运行**！
- SELinux 在后台用于启用此权限提升，允许任何进程以任何 UID 运行并访问任何对象（如端口、文件等）。
- SCC 和 Role 可以添加到用户、组和/或服务账户。
- 除了允许用户/组获得提升权限外，还可以将角色/SCC 分配给服务账户，然后在部署应用时专门**使用**该服务账户。
  - 这样**只有**该应用才被允许拥有该权限。

**软件定义网络（SDN）**

- 顾名思义，软件定义网络就是使用软件组件定义网络。
- 如果你了解 OSI 模型，SDN 主要处理 OSI 模型的第 2 层和第 3 层，即网络层和 IP 层。
- 在软件中创建网络/子网（第 2 层）和创建路由器（第 3 层），运行在通用 x86 硬件上。
- 而不是使用传统意义上昂贵的专有硬件。
- OpenShift Container Platform 使用的 SDN 插件名为 **Open vSwitch（OVS）**。
- OVS 帮助我们在 OCP 环境内创建新项目时按需创建虚拟网络和路由。
- 注意，OCP 是多租户环境。
  - 意味着一个租户的网络不应与其他租户的网络产生冲突。
  - 因此所有租户创建时都会被放入一个第 2 层子网，并拥有自己唯一的 VxLAN ID，使离开该项目的任何数据包都会带有该 VxLAN 标签。
- **ClusterNetwork CIDR** 定义了集群内可创建的 Pod 总数。
- **HostNetworkCIDR** 定义了单个节点上可创建的 Pod 数量！
- 可以使用不同的 SDN Open vSwitch 实现来启用 flat、multi-tenant 或 networkpolicy 策略。
- Open vSwitch 在 OCP 集群中以 Pod 形式运行。
- 新项目总会被分配一个唯一的 VxLAN ID，在该项目中创建的每个 Pod 发出的数据包都会带有该 VxLAN ID！
  - 这样在网络栈层面一个项目与其他项目隔离。
- 如果需要，可以通过创建网络策略（Network Policies）让一个项目与另一个项目通信，实现双向或单向通信。
  - 要实现这一点，**必须**使用 `ovs-networkpolicy` SDN 插件。
- 在 OCP3 中，默认插件是 `ovs-subnet`，这是一个扁平网络，默认允许项目间的 Pod-Pod 通信。
  - 但如果使用 `ovs-multitenant` 插件，会给不同项目分配不同的 VxLAN，并且**不允许**项目间的 Pod-Pod 通信。
  - 因此 `ovs-networkpolicy` 是 OCP4.X 的默认 OVS 插件，允许创建灵活的策略，以选择性的方式实现项目间的 Pod-Pod 通信。

**路由（Routes）与外部访问**

- Kubernetes 在最初实现中没有路由（route）的概念。
- 如果需要从外部访问 Pod，必须使用 Pod 所在节点（主机）的 IP 地址。
  - 这种实现称为 **NodePort**！
  - 顾名思义，NodePort 是节点上的端口。
- 这种方式效率低下：即使 Pod 只运行在少数几个节点上，所有节点的端口都必须监听。
  - 例如有 20 个节点，应用有 3 个 Pod 运行在 3 个不同节点上，虽然应用只用了 3 个 Pod，但**所有节点**都必须监听该端口（并配置防火墙规则允许访问）。
- 随着 OCP 中路由的实现，OpenShift 缓解了此问题。
- 这促使 Kubernetes 也接受了 **Ingress**，它本质上是为外部客户端访问 Pod 创建完全限定域名（FQDN）。
- 可能还记得，我讨论过 OpenShift 服务希望访问 OpenShift 外部的服务！
  - 这可以通过在 OpenShift 内创建**外部服务（external service）**来实现。

**常用命令**

```bash
oc get clusternetworks
oc get hostsubnets
oc get machineset
oc debug nodes <node_name>
oc get machines -n openshift-machine-api
```

- 路由用于为运行在 OCP 中的 Pod 提供外部访问。
- 路由是 FQDN（SSL 或 HTTP）。
- K8s 在 1.9 版本引入 Ingress 路由器之前没有任何类似路由的原生功能。
- Ingress 与路由非常相似——它会创建基于 DNS 的主机名（FQDN）来访问 Pod。
- OpenShift 现在也支持基于 Ingress 的路由：创建 Ingress 对象后会动态创建路由对象，并实时保持路由与 Ingress 的同步更新。
- `oc expose <service>` 作为路由暴露时，只会创建**基于 HTTP 的未加密路由**。
- OpenShift 路由也支持启用 TLS/SSL 的访问，有三种终止方式：
  1. **Edge Termination（边缘终止）**——外部客户端到 OCP 应用的访问在请求到达 OCP 路由器之前是安全的；之后连接为非 SSL。
  2. **Passthrough（透传）**——外部客户端的请求将端到端加密，直到运行在 OCP 集群内部的 Pod。由应用本身负责通过 SSL 证书的公钥/私钥进行通信的解密/加密。
  3. **Re-encrypt（重新加密）**——外部客户端到 OCP 路由器使用一个 SSL 证书；路由器到 Pod 使用另一个 SSL 证书。主要用于客户要求不仅在防火墙外、而且在组织内部也具备严格高级安全性的场景，大多数银行类应用属于此类。

**区域（Regions）与可用区（Availability Zones）**

- Region 是部署**完整集群**的地理位置。
- 一个 Region 内可以有多个数据中心，称为"**可用区（Availability Zones）**"。
- 通常部署 OCP 时，每个 master 部署在自己的可用区中。
- 可用区（AZ）是**允许故障**的。
  - 所以如果一个 AZ 故障，仍有其他两个 AZ 提供冗余。
- 根据 OCP 部署情况，可以选择在 OCP 内使用 region 和 zone 概念对 worker 节点分类。
  - 键值对如 `Env: Production` 或 `Env: Test`
  - 或使用 `region: tokyo`、`region: jakarta`
- 然后使用 `oc label` 命令为 worker 节点打标签，将这些键值映射到 region/zone 标签。
- 这些标签可在 Pod 规约（YAML 文件）中使用，Kubernetes 调度器会检查规约，然后将该 Pod 调度到匹配这些标签的节点。
- 还可以使用 OpenShift 中的 annotations，将项目中的所有 Pod 放置到满足某标签的子集节点上。
- 甚至可以更进一步，让特定用户的所有 Pod 放置在匹配标签的节点子集上。
- 如果要更进一步，可以修改 kubernetes 的 scheduler.json 文件，设置自己的优先级（priorities）/谓词（predicates）来影响放置。
  - 有权重（weights）的概念，可以优先让某些 worker 节点总是优先接收 Pod。
- 注意，我们讨论的是集群——集群意味着**相同**节点的集合。
- 应用放置可通过编辑 DeploymentConfig / Deployment，在 Pod 部分添加标签来控制。

### Day 5

**资源限制与配额**

- 默认情况下，K8s 或 OpenShift 对容器/Pod/Deployment 或项目/用户**不强制执行任何限制**。
  - 即用于限制资源的控制组（Cgroups）在 OCP 中默认未强制执行。
- 可以为整个项目或 Pod 内运行的容器强制执行资源限制。
- 限制也可以延伸到用户/组，以在用户/组级别实施细粒度的配额限制。
- 惯例是**不允许**用户自行创建项目——而是让新入职用户成为已有项目的成员，该项目已强制执行限制！
- 限制可以是 Pod、service、route 等的**数量**，以及所创建资源内部的资源使用限制（例如可以限制某个 Deployment 本身的资源）。
- 资源使用非常关键；基于使用统计，如果集群中配置了指标（Prometheus），可以配置集群和应用自动扩缩容。
- `oc scale` 和 `oc autoscale` 可根据用户请求动态扩缩容器化应用。
- 结合 RBAC 和配额限制，可以有效管理集群内的资源。
- OCP4 安装过程本身已集成 metrics（无需单独安装）。

**配额与限制对象**

- 对项目的配额由 **ResourceQuota** 对象强制执行。
- 资源的限制由 **LimitRanges** 对象强制执行。
- `oc create` 命令由**管理员**使用，用于限制项目的配额以及 Pod/容器/镜像/卷的资源使用限制。
- 这些限制/配额针对运行在项目中的应用强制执行。
- 实际上**不会**对临时资源（如构建 Pod / 部署 Pod 等）强制执行。
- 配额基本上作为占位符，结合硬件资源消耗（如 CPU/内存/存储）和可用对象数量（如 Pod/服务/复制控制器）。
- 配额在项目级别强制执行。
- 限制对象则针对单个资源使用强制执行——例如最大 CPU、内存等。
- 基本上可以对单个资源（如 Pod、镜像、容器、持久卷等）强制执行"多少"CPU/内存/存储。
- 这些资源限制可以与应用的可扩展性结合。
- 必须有监控工具检查项目内资源的真实容量和利用情况——即应用的资源消耗。
- 基于此，可以指示 OCP 检查应用是否始终得到保证的**最小**副本数，或在需要时增加副本以满足增长的需求，直至某个最大限制。
- 这通过 `oc autoscale` 命令完成，在其中为 deployment/deploymentconfig 设置最小和最大 Pod 数。
- 也可以随时执行 `oc scale`——但那是手动操作。
- 永远记住一件事：`oc scale` 和 `oc autoscale` 是在实际创建 Pod 的对象上执行的——即 Deployment 或 DeploymentConfig。
- **Replication Controller** 是 K8s 对象；OpenShift 使用 Deployment 或 DeploymentConfig。

**自动扩缩容（Autoscaling）**

- 默认情况下，自动扩缩容**仅使用 CPU** 作为指标来判断是否达到 CPU 限制。
- 但在 OCP4 中，可以选择定义自己的自定义指标，从而不仅监控 CPU，还能监控内存、网络连接数等其他使用情况。
- 说到扩缩容，就是添加/移除额外的 Pod。
- 这些 Pod 被添加到新的 worker 节点上，因此是以水平方式扩展。
- 有一个名为 **Horizontal Pod Autoscaler（HPA）** 的对象。
- 执行 autoscale 时，项目中会创建 HPA 对象来跟踪资源使用情况，并通过将 Pod 放置到不同的 worker 节点上来扩展应用。
- 借助 OCP4 的 Operator Framework，不仅可以扩缩容器化应用，还可以扩缩容器基础设施本身。
- 部署了 Operator 来根据 ClusterAutoScaler Operator 中配置的最大节点数来扩缩 worker 节点。
- 与容器化应用的副本数非常相似，有两个 Operator 负责扩缩部署在提供商上的 OCP worker 节点。
- OCP4 中定义了 **Machine** 和 **MachineSet** 对象，分别对应一个 worker 节点和一组 worker 节点。
- 这些对象实际上用于根据应用工作负载的需求扩缩 worker 节点。
- 因此根据部署 OCP 的提供商，machineset 默认按可用区配置。
  - 例如选择 AWS 且区域为新加坡，则每个 AZ 都会部署单独的 machineset，作为 OCP 安装的一部分。
- 这是 OCP4 的关键差异化优势：Operator 也用于扩缩底层集群基础设施。
- 配合后面讨论的自动扩缩容，集群可以根据动态请求自动扩展和收缩。
- 要对 worker 节点自动扩缩容，需要一个顶层资源 **ClusterAutoScaler**。
  - ClusterAutoScaler 定义集群可拥有的最大机器数、累计总 CPU 和内存。
- 要扩缩 machineset 中的机器，需要将 **MachineAutoScaler** 资源映射到每个 machineset。
  - 在 MachineAutoScaler 中定义 MachineSet 中机器的最小和最大数量。
  - 如果未定义 ClusterAutoScaler，MachineAutoScaler 将无法工作。

扩缩容层级关系：

```
ClusterAutoScaler -> MachineAutoScaler -> MachineSet -> Machines
```

- 注意：每个可用区（AvailabilityZone）对应一个 MachineSet。

**其他命令与提示**

```bash
sha256sum <filename>
```

- 使用新加坡区域总是安全的，因为新加坡的 AWS 通常拥有所有最新功能。
- 也可以选择将 worker 改为 2 个、master 改为 1 个。
- 注意：**master 必须有 16G 内存**，worker 可以是 8G。
- Amazon 使用 EBS。
