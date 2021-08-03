## OpenShift v3.x 基础架构与命令汇总

##### 文档说明：

- 该文档根据 Red Hat OpenShift Container Platform 3.9 版本创建
- 以下文档中将 Red Hat OpenShift Container Platform 简称为 OCP（或 OCP 3.x）
- 其中涉及的选项与参数在绝大部分 OCP 3.x 中适用，部分版本略有不同，参考使用时以实际版本为准，此处提供思路与方法！

##### 文档目录：

- OCP 3.x 概要
- OCP 3.x 集群的部署方法
- OCP 集群资源（resource）类型概述说明
- OCP 常用命令汇总
- 参考链接

##### OCP 3.x 概要：

- OCP 的设计哲学：一切皆容器、一切皆对象

- OCP 客户端命令行工具 `oc` 命令取代 K8s 的 kubectl 命令，两者使用方法与参数选项基本保持一致。

- OCP 使用 `source-to-image`（`S2I`）技术，可将源代码注入构建镜像（`S2I builder image`）构建为应用镜像，实现快速部署应用，并支持版本回滚、快速迭代与 DevOps 的功能。

- OCP 3.x 集群架构概述：

  ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-arch.png)

- Origin 项目与 OCP 3.x 项目的对应关系：

  ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-origion-developer.jpg)

##### OCP 3.x 集群的部署方法：

- 生产环境：

  1. OCP 3.4、3.5 集群部署使用 RPM 软件包方式，OCP 3.9、3.11 集群部署使用容器镜像方式。 

  2. OCP 3.x 中使用 Ansible 部署 OCP。

- 开发与测试环境：

  1. all-in-one：`AIO`（本地单节点集群）

  2. OCP 二进制执行程序快速启动与部署

  3. `minishift` 工具部署含 all-in-one 集群的虚拟机，与 `minikube` 非常类似。

- minikube 安装报错：

  ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/minikube-error-1.jpg)

  ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/minikube-error-2.jpg)

  ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/minikube-error-3.jpg)

  ```bash
  $ minikube addons list
  # 查看 minikube Kubernetes 支持的插件列表
  ```

##### OCP 集群资源（resource）类型概述说明：

1. master 节点：

   1）OCP 集群或 K8s 集群的控制节点

   2）生产环境中建议使用 3 个或奇数个 master 节点确保高可用性，并将 etcd 数据库集群单独分开。

   3）OCP 3.4、3.5 集群的 master 节点：不运行 pod，核心组件以 `systemd` 服务的方式运行。

   4）OCP 3.9、3.11 集群的 master 节点：可运行 pod，核心组件也以 `pod` 的方式运行。

   5）master 节点执行的服务包括：

      a. apiserver（authentication/authorization）：

   ​      接收、处理、响应来自 master 节点与 node 节点各组件的 `Restful API` 请求。

   ​      处理 OCP 集群内的用户与服务的身份认证/授权服务（`oAuth`）。

      b. controller-manager：

   ​      控制管理器，包含复制控制器（replication controller），用于实现无状态 pod 的多副本高可用性。

      c. scheduler：

   ​      调度器，用于实现 pod 在各个 node 节点上的分配调度。

      d. data-store：

   ​      `etcd` 分布式键值型数据库，用于服务配置发现，OCP 集群中的数据存储与核心。
   
   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-master-pod.jpg)

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-master-etcd.jpg)

2. compute 节点：

   1）OCP 集群与 K8s 集群的计算节点

   2）compute 节点用于运行 pod 提供服务

   3）compute 节点的 docker 守护进程异常而导致 **`atomic-openshift-node`** 服务报错。

   > ✅注意：atomic-openshift-node 服务已集成 kubelet 功能。

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/atomic-openshift-node-error-1.jpg)
   
   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/atomic-openshift-node-error-2.jpg)

3. project：项目

   1）也称为命名空间（namespace），OCP 集群使用项目来隔离资源，区别于 Linux namespace。

   2）若未将 `self-provisioner` 角色从指定用户去除，使用指定用户创建的项目，该用户即为项目的项目管理员。

   3）default 项目与 openshift 项目能被所有用户使用，但只能由 `system:admin` 用户或具有 `cluster-admin` 角色的用户管理。

4. image stream（is）、image stream tag（istag）：镜像流、镜像流标签

   1）应用源代码注入时所使用的构建镜像（S2I builder image）的镜像流默认为 openshift 项目中的镜像流。

   > 💥注意：
   >
   > 若 openshift 项目中存在 imagestream 但不存在相应镜像，或不存在相应的 imagestream 时，需导入容器镜像至 imagestream 或手动创建 imagestream。

   2）镜像流为容器镜像导入 OCP 集群中的镜像元数据。

   3）openshift 项目中的所有 image stream 可以被所有项目用户使用。

   4）image stream 的容器镜像可以来自 `OCP internal registry` 或 `OCP external registry`。

   > ✅注意：
   >
   > 可作为 OCP internal registry 的组件：docker-registry pod、quay pod
   >
   > 可作为 OCP external registry 的组件：Harbor、Red Hat Quay、docker-distribution

   5）构建镜像根据 image stream tag 从 OCP external registry 中下载相应镜像。如在 openshift 项目中 image stream tag 为 php:5.5，即将从 OCP external registry 中下载 php 5.5 的构建镜像。

   6）构建镜像（builder image）的组成：

      基本的操作系统运行环境、编程语言环境、相应的编程语言框架等

   7）OCP internal registry（docker-registry pod）只存储由 S2I 构建的应用镜像，实现一次构建多次部署。

   8）新构建的应用镜像将存储于 default 项目中的 `docker-registry pod` 中。

   9）该 pod 根据其资源定义文件中的 pvc 将应用镜像存储于后端存储中，默认为 NFS 存储，也可集成 Ceph RBD。

   10）openshift 项目中的 image stream 与 template 可被 OCP 项目用户查看，但只由具有 `cluster-admin` 角色的管理员用户管理。

   11）image stream 使用报错示例：

      a. 若已存在的 image stream 报错 `! error`，可将其删除后再从外部容器镜像仓库导入。

      b. 删除已存在的 image stream 需指定 `tag`： 

   ```bash
   $ oc tag -d <imagestream_name>:[tag] -n <project>
   # 删除指定项目中 imagestream 中的 imagestreamtag，使其可重新上传。
   ```

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/imagestream-error-1.jpg)

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/imagestream-error-2.jpg)

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/imagestream-error-3.jpg)

5. build config（`bc`）：构建配置

   1）OCP 集群使用 bc 通过 S2I 与 imagestream 创建应用镜像，其中包含触发（trigger）构建的条件信息。

   2）使用 `oc new-app` 命令默认生成 `List` 资源定义文件，包含dc的资源定义。   

   3）支持通过 Webhook 与外部的持续集成（CI）与持续开发（CD）系统集成。

   4）S2I 可为 OpenShift 提供完整的 CI/CD 管道，可参考链接 "基于 S2I 的 Golang 应用构建与部署示例"。

   > ✅注意：S2I 更多的使用于构建编译前端应用容器镜像。

6. build：构建

   1）build 资源对象以 pod 的方式运行，即，应用源代码通过 S2I 注入构建镜像后运行的 pod。

   2）该对象用于跟踪 bc 中 S2I 生成新的应用镜像的过程，成功运行后将部署应用 pod，触发部署应用 pod 的过程可阻断。

7. deployment config（`dc`）：部署配置

   1）replication controller 与 pod 的资源定义已嵌入至 dc 中，可根据触发条件（trigger）进行部署。

   2）使用 oc new-app 命令默认生成 List 资源定义文件，包含 dc 的资源定义。

   3）支持通过 Webhook 与外部的持续集成（CI）与持续开发（CD）系统集成。

8. deploy：部署

   1）deploy 资源对象以 pod 的方式运行。

   2）该对象用于跟踪 dc 生成新的 pod 的过程。

   3）若新部署的 pod 无法正确运行，删除 deploy pod 后，将自动删除正在由 deploy pod 部署的其他 pod。

9. replication controller（`rc`）：复制控制器

   1）复制控制器保证运行的 pod 的高可用性，已嵌入至 dc 中。

   2）若 pod 由于某些原因故障停止，rc 将根据配置的 pod 数量（replicas）重新部署 pod 保证不间断服务，此类 pod 为无状态应用居多。

10. service：服务

    1）由于 pod 经常因某些故障而重启，每次重启后其 IP 地址都将改变，因此使用 service 将一个或一组相同的 pod 间进行关联。

    2）service 为 pod 提供统一的入口 IP 地址，该入口地址默认为 service 的虚拟 IP 地址（`ClusterIP`）。

    3）service 资源对象由 `kube-proxy` 组件实现，其虚拟 IP 地址存在于每个节点的 iptables NAT 表中，使用 `iptables -t nat -nvL` 命令即可查看指定的 ClusterIP。
    
    <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/kube-proxy-service-code.jpg" style="zoom: 67%;" />

    4）service 有反向代理与负载均衡的功能，默认以 Round Robin 轮询的方式将流量转发至 pod。

    > 💥注意：
    >
    > 无论 OCP 集群使用 `ovs-subnet` 或 `ovs-multitenent` SDN 插件，同一项目的 pod 始终在同一个 flat 网络中，pod 间可直接通信，无需使用 service！
    > 
    > <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-network-plugin.jpg" style="zoom:80%;" />    

    6）若使用 ovs-multitenant SDN 插件，只是使用 `VNID` 实现不同项目间的 pod 二层隔离。

    7）OCP 3.x OVS 网络拓扑示意：

    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-ovs-1.png)

    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-ovs-2.png)

    8）同一节点上 pod 间的通信示意： 
    
    <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-ovs-3.png" style="zoom: 80%;" />

    9）OCP 3.x OVS 流表分析示意：

    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-ovs-openflow-1.jpg)

    10）访问使用 `NodePort` service 类型的 pod 跨节点流量分析：iptables NAT 表与 OVS 流表

    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/NodePort-service-iptables-nat-ovs-analyze.jpg)

    11）service 通过 `selector` 与具有相同 `label` 的 pod 关联，将固定的 IP 地址与 pod 解耦，提高 pod     部署的灵活性，即 OCP 可根据 scheduler 调度器将 pod 部署至不同的 node 节点上，根据 rc 部署相应副本数量的 pod，保证 pod 的服务高可用。

    12）service 作为前端 pod 访问后端 pod 的入口点，实现服务发现。

    13）前端应用 pod 使用后端应用的 service `环境变量`来发现后端应用 pod，也可通过 OCP 中内嵌的 `SkyDNS` 的 `SRV 记录`实现前端应用对后端应用的服务发现。

    14）对于项目内的每个 service，将自动定义环境变量，并注入到同一项目中的所有 pod 中。

    15）service 环境变量的服务发现方式包括：

    ​    <svc_name>_SERVICE_HOST：service 的 IP 地址

    ​    <svc_name>_SERVICE_PORT：service 的 TCP 端口号

    > 💥注意：
    >
    > 使用 service 环境变量实现服务发现时，必须先创建后端 service，再创建启动前端 pod，才能实现后端 service 环境变量的注入。

    16）SkyDNS 服务发现方式：

    ​    a. 从 pod 发现 service 的另一种方式是使用 OCP 集群内部的 DNS 服务器来实现，该服务器仅对 pod 可见。

    ​    b. 每个 service 被动态分配一个 `FQDN` 格式的 `SRV记录`：

    ​       <svc_name>.<project_name>.svc.cluster.local

    > ✅注意：在应用 pod 中使用 DNS 查询来实现服务发现，可在 pod 启动后再查找创建的 service。

    17）service 的虚拟 IP 地址与 pod 的 IP 地址面向 OCP 集群内部，OCP 集群外部不可访问，若使外部能够访问，需要使用 `route` 资源对象进行暴露。

    18）service 的类型：ClusterIP、NodePort、LoadBalancer、ExternalIPs

    > 💥注意：
    >
    > 使用 NodePort 类型 service 的资源定义文件更改后再创建 ClusterIP 类型 service 时，需删除其中的 **`spec.externalTrafficPolicy`** 字段属性，否则创建失败！
    >
    > ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/change-service-type-error.jpg)

    19）OCP 中建议将 service 整合入 dc 中，而 K8s 中建议将 service 定义在 DeploymentSet 中。

    20）OCP 集群默认使用 Open vSwitch（`OVS`）来实现集群的 SDN 网络。

    21）使用原生 kube-proxy 实现的 service 与自研未使用 service 解决方案的响应对比：
    
    <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/service-performance.jpg"  />
    
    22）因此，目前开源社区使用 `eBPF` 技术为基础，开发的 `Cilium` CNI 插件可不使用 service 以实现其功能，在流量转发方面性能得到极大的提升。

11. route：路由

    1）可借助 service 实现 OCP 集群内前后端 pod 间的通信，而 OCP 集群外部对内部 pod 的访问默认需要使用 default项目的 `router pod` 来实现。

    2）OCP 集群外部通过泛域名解析指向特定 `infra` 节点，router pod 需指定在该 infra 节点上运行，其 IP 地址与 infra 节点绑定。

    3）router pod 以 `HAProxy` 的方式实现。
    4）默认情况下，OCP 集群中的所有项目的 route 规则都将注入到 default 项目中的 router pod 中。

    5）router pod 可将外部流量直接转发到 OCP 集群中的应用 pod 上。

    6）router pod 只使用相应的 service 在 etcd 中查询对应 pod 的 `endpoint`，直接转发流量至 pod。

    7）router 路由原理架构示例：

    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-route-infra.jpg)

12. pod：

    1）pod 是 K8s 集群与 OCP 集群中容器运行的原子单位（最小粒度），单个 pod 中可以运行单个或多个容器，共享 `network namespace` 与 volume。

    2）在 pod 中可存储临时数据，但在 pod 重启后将丢失全部数据，因此 pod 需使用永久存储。

13. label：标签

    1）基于等值关系的标签
    2）OCP 集群中的各种资源使用 label 标签进行匹配

14. persistent volume（pv）：持久卷

    1）持久卷属于 OCP 集群资源，必须使用 system:admin 管理员用户或具有 `cluster-role` 角色的用户进行管理、     创建与删除。

    2）pv 资源定义中默认使用 NFS 服务端提供 NFS 存储，可为 pod 提供永久存储。

    3）pv 的访问模式：`NFS` 均支持以下三种模式
    <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/pv-access-mode.jpg" style="zoom:67%;" />

    4）持久卷存储等级（persistent volume storage class）定义后端存储的类型与等级，由 `storageClassName` 属性定义。

    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/storageClassName.jpg)

    5）pv 回收策略：`PersistentVolume.spec.persistentVolumeReclaimPolicy` 属性

    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/pv-recycle-policy.jpg)

    ​    a. `retain`（默认方式）：pv 中的数据将保留，管理员需手动处理该卷。

    ​       若需清除 pv 中的数据，可执行以下操作：

    ​       👉 手动删除 pv。

    ​       👉 手动清理后端存储卷中数据，以免数据被重用。

    ​       👉 手动删除后端存储卷。

    ​       👉 创建新的 pv 以重用之前的数据。

    ​    b. `recycle`：

    ​       👉 通过执行 rm -rf 命令删除卷上所有数据，使得卷可被新 pvc 使用。

    ​       👉 目前只有 NFS 与 hostPath 支持该回收模式。

    >  💥注意：pv 与 pvc 可绑定成功，但不代表 pv 使用的后端存储可正常使用！     

15. persistent volume claim（pvc）：持久卷声明

    1）持久卷声明属于项目（或命名空间）资源，使用项目用户即可管理 pvc。

    2）pv 与 pvc 通过访问模式（`accessMode`）与存储大小（`storage`）进行匹配。

    3）若存在相同访问模式与存储大小的 pv，pvc 在使用时将随机使用 pv，但第一次匹配成功使用后将持久使用该 pv。

    4）pv 与 pvc 的使用方法：

       a. NFS 存储卷共享（属组与权限）

       b. pv 资源定义与创建

       c. pvc 资源定义与创建

       d. 更改 dc 或 pod 资源定义的 `persistentVolumeClaim.claimName` 属性值以创建 pod 资源

    5）OCP 部署过程中定义 NFS 存储作为 OCP internal registry 的存储后端：
    <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-internal-registry-pvc-1.jpg" style="zoom: 80%;" />
    
    6）OCP 集群 default 项目中定义的 pv 与 pvc 的关系：
    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-internal-registry-pvc-2.jpg)
    
    <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-internal-registry-pvc-3.jpg"  />

16. secret：

    1）该资源保存 OCP 集群中的敏感信息，如密码、token 凭据、配置文件等。

    2）该资源将敏感信息与 pod 解耦，可使用 `volume plug-in` 将 secret 挂载至 pod 中。

    3）secret 资源可在命名空间中共享。

    4）service account（sa）与 secret 的关联：

       a. 必须指定 sa 以运行 pod，若未指定将使用 default sa。

       b. sa 中包含两个 secret，并且每个 secret 分别具有一个 token。

       c. token分别用于：

    ​      👉 pod 与 apiserver 间的认证通信

    ​      👉 从 OCP internal registry 中拉取已构建的容器镜像

    ​      ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/service-account-secret-1.jpg)

       d. pod 运行后将 secret 挂载至 /var/run/secrets/kubernetes.io/serviceaccount/ 目录中。
    
       e. 该目录中的 token 即为 sa 中的 secret 对应的 token。
    
       <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/service-account-secret-2.jpg" style="zoom: 67%;" />
    
       <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/service-account-secret-3.jpg" style="zoom: 50%;" />

17. OCP 中特有的资源对象：bc、dc、route、template

18. 外部访问 OCP 集群内 pod 的方式：NodePort、Route、port-forward

19. OCP 集群资源对象与工作流程：

    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-resource-workflow.jpg)

##### OCP 常用命令汇总：

1. all-in-one OCP 集群命令使用：

   ```bash
   $ oc cluster [status|down]
   # 查看或关闭 all-in-one OCP 集群
   
   $ oc cluster up [-h|--help]
   # 查看启动本地 all-in-one OCP 集群的使用方法
   # 常用命令选项：
     --use-existing-config    使用已有的 OCP 集群配置，若配置不存在，将创建新的配置。
     --public-hostname        指定 OCP 集群 Web console 的域名地址
     --route-suffix           指定 route 路由的泛域名后缀（wild-card DNS），使其始终
                              指向同一个公网 IP 地址（指定 node 计算节点的 IP 地址），
                              外部通过该地址访问 OCP 集群中的 pod。
     --host-config-dir        指定 OCP 集群的配置目录
                              默认路径：/var/lib/origin/openshift.local.config
     --host-data-dir          指定 etcd 数据的存储目录，若不指定该目录，主机关机后将
                              丢失集群所有的 etcd 数据。
                              默认路径：/var/lib/origin/etcd
     --host-volumes-dir       指定 OCP 集群的卷存储目录
                              默认路径：/var/lib/origin/openshift.local.volumes
     --image                  指定 OCP 集群使用的容器镜像名称
     --version                指定容器镜像的版本
   ```

2. 集群部署与登录相关：

   ```bash
   # ----- 密码与字符串编码 -----
   $ authconfig --test | grep hashing
   # 查看系统支持的密码加密算法（每种系统发行版存在差异）
   
   $ openssl passwd -6 -salt <salt_value> <password>
   # 根据 salt 值通过 SHA512 哈希算法对明文密码加密，生成相应的哈希值。
   # 生成的密码与 /etc/shadow 中相应用户的密码相同
   $ openssl passwd -apr1 <password>
   # 根据 apr1 算法生成明文密码的哈希值
   $ openssl rand -base64 16 | tr -d '+=' | head -c 16
   # 对数字 16 生成 base64 编码的随机数，并取前 16 个字节。
   
   $ echo "<string>" | base64
   # 使用 base64 加密算法对字符串加密
   $ echo "<hash>" | base64 -d
   # 使用 base64 加密算法对哈希解密
   
   $ python -c \
     "import crypt, getpass, pwd; print crypt.crypt('<password>', '\$6\$<salt_vaule>\$')"
   # 根据 salt 值通过 SHA512 哈希算法对明文密码加密，生成相应的哈希值。
   # crypt 模块只可在 python2 环境中使用
   $ perl -e 'print crypt("<password>", "\$6\$<salt_value>\$")."\n"'
   # 使用 perl 生成明文密码的 SHA512 哈希值
   
   # ----- OCP 集群登录 -----
   $ oc login \
     https://<hostname_of_ocp_apiserver>:8443 \
     -u <ocp_username> \
     -p <password> \
     --insecure-skip-tls-verify=true
   # 使用 OCP 集群管理员或项目用户远程登录 OCP 集群，成功登录后即可管理项目与应用。
   # oc 命令缓存用户与集群域名的凭据，并且 Web Console 控制台在 master 节点上运行。
   $ oc login https://ocp.lab.example.com:8443/console -u developer -p developer
   # 使用 OCP 集群项目用户远程登录
   # 成功登陆后用户的认证令牌（token）将保存在登录用本地用户（student）的家目录中，即 $HOME/.kube/config 中。
   
   $ oc login -u system:admin
   # OCP 集群管理员用户本地登录 master 节点，提高安全性。
   
   $ oc whoami
   # 查看当前登录 OCP 集群的项目用户
   
   $ oc logout
   # 登出 OCP 集群
   
   $ docker tag registry.lab.example.com/nginx:latest \
     docker-registry-default.apps.lab.example.com/webapp/nginx:latest
   # 更改外部容器镜像 tag 为 OCP 内部容器镜像 tag，将其推送至 OCP 内部容器镜像仓库。
   ```

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/system-admin-logout.jpg)

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/docker-registry-route.jpg)

3. 命令使用帮助相关：

   ```bash
   $ oc version
   # 查看 OCP 与 K8s 版本信息
   
   $ oc <command> --help
   # 查看 oc 子命令的使用方法
   
   $ oc options
   # 查看 oc 命令行可用的选项
   
   $ oc types
   # 查看 OCP 集群的概念与类型说明
   
   $ oc explain <resource_object>
   # 查看 OCP 集群指定资源对象的详细说明
   ```

4. 构建与部署应用：

   ```bash
   $ oc projects
   # 查看所有已存在的可用项目，当前所在的项目前具有 "*" 标识。
   # 该命令只能由 master 节点的 system:admin 管理员用户或具有 cluster-role 角色的用户执行。
   
   $ oc project
   # 查看当前所在的项目
   
   $ oc project <project_name>
   # 切换至指定的项目
   
   $ oc new-project <project_name> --description='<description_string>'
   # 创建项目与相应描述内容
   # OCP 使用项目进行资源隔离，即项目为命名空间（namespace）。
   
   $ oc status -v
   # 查看当前所在项目的详细状态，包括 bc 与 dc 状态等。
   
   $ oc import-image <imagestream>[:<tag>] \
     --confirm --from <container_registry_for_imagestream> [--insecure] \
     -n <project_name> 
   # 将容器镜像仓库中的镜像作为 imagestream 上传至指定项目中
   # 需确认容器镜像仓库是否使用 SSL 连接
   # 注意：
   #   openshift 项目中 imagestream 无法使用相应 tag 的容器镜像报错处理：
   #   1. 查看集成的 OCP 外部镜像仓库中是否具有相应 tag 的容器镜像
   #   2. 删除报错的 imagestream，报错信息如 "! error: Import ..."。
   #      $ oc tag -d <imagestream>:<tag> -n openshift 
   #   3. 重新导入 OCP 外部镜像仓库中的容器镜像的 metadata 至 imagestream 中。
   #      $ oc import-image apache-httpd:2.5 \
   #        --confirm --from registry.lab.example.com:5000/do288/apache-httpd \
   #        --insecure -n openshift      
   ```

   1）使用容器镜像创建应用 pod 流程：

   ```bash
   $ oc new-app --list
   # 查看 OCP 集群中已存在的模板（template）与镜像流（imagestream）
   
   $ oc new-app \
     --name=<application_name> \
     --docker-image=<container_registry_url>/<repository>/<image_name>:[tag]
   # 使用已有的容器镜像创建应用，并指定应用名称。
   ```

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/oc-new-app-container-image-stratgy-1.jpg)

   <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/oc-new-app-container-image-stratgy-2.jpg" style="zoom:150%;" />

   2）使用 Dockerfile 构建应用容器镜像并创建 pod：

   ```bash
   $ oc new-app \
     --name=<application_name> \
     --strategy docker \
     <git_repository_url_of_dockerfile> [--insecure-registry]
   # 以 Dockerfile 所在的 Git 仓库的方式创建应用
   # 注意：
   #   1. Dockerfile 中 FROM 指令指向的容器镜像仓库，需注意是否使用 SSL 协议，即是否使用
   #      --insecure-registry 命令行选项。
   #   2. 如下所示：
   #      $ oc new-app --name=echo --strategy docker \
   #        --insecure-registry http://services.lab.example.com/rhel7-echo
   ```

   3）使用应用源代码以 S2I 的方式注入构建镜像创建应用 pod：

   ```bash
   # ----- 三种使用 S2I 构建应用的方法 -----
   ### 方法1 ###
   $ oc new-app \
     --name=<application_name> \
     <imagestream_name>:[tag]~<git_repository_url_of_source> \
     --labels=app=<application_name>
   # 以 S2I 源代码注入的方式创建应用
   
   $ oc new-app \
     --name=hello \
     php:5.5~http://workstation.lab.example.com/php-helloworld \
     --labels=app=hello \
     -o json > /tmp/app-s2i.json
   # 可将其定义的资源模板重定向至指定的 JSON 文件而不创建资源
   
   $ oc new-app \
     --name=hello \
     workstation.lab.example.com:5000/openshift3/php-55-rhel7:latest~
     http://workstation.lab.example.com/php-helloworld \
     --labels=app=hello
   # 使用外部容器镜像仓库的镜像与源代码创建应用
   
   # oc new-app 命令常用选项：
     --name                        指定构建的应用及其相关资源的名称
     --strategy [docker|source]    指定 S2I 构建过程中使用的构建策略，
                                   包括 Dockerfile 文件（docker）、源代码注入（source）。
     --docker-image                指定构建应用所使用的容器镜像    
     -i, --image-stream            指定构建应用时的 imagestream名称
     --code                        使用源代码方式构建应用
     --labels                      指定构建的应用具有的标签
     --template                    使用模板文件创建各应用资源，见下图示例。
     -p, --param                   指定模板文件中定义的环境变量，该变量可覆盖模板文件
                                   中定义的变量。
     --insecure-registry           指定使用的容器镜像仓库未使用 SSL 连接
   
   ### 方法2 ###
   $ oc new-app \
     --name=<application_name> \
     --strategy source \
     -i <imagestream_name>:[tag] \
     --code <git_repository_url_of_source> 
   # 以 S2I 源代码注入的方式创建应用
   
   ### 方法3 ###
   $ oc new-app \
     --name=<application_name> \
     --strategy source \
     <git_repository_url_of_source> 
   # 以 S2I 源代码注入的方式创建应用
   # 未指定 imagestream 时，将自动识别 openshift 项目中相应的 S2I 构建镜像（builder image）。 
   # 该命令将尝试根据 Git 代码仓库根目录中存在的特定文件来确定使用哪一种语言的构建镜像。
   ```

   👉 使用 S2I 源代码注入创建应用 pod 流程：
   
   <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/s2i-application-build.jpg" style="zoom:67%;" />

   > ✅注意：
   >
   > 1. oc new-app 命令使用 `List` 资源定义文件，该文件定义 is、bc、dc、service。   
   >
   > 2. 该文件不定义 route 资源，必须手动定义 route资源定义文件并创建 route 资源，或手动暴露 service 来创建 route资源。   
   >
   > 3. 模板（`template`）与 `Web Console` 中已嵌入 route 资源，因此可直接创建。
   >
   > 4. <imagestream_name>:[tag]：
   >
   >    a. 使用 imagestreamtag 名称，默认为 OCP 集群中 `openshift` 项目提供。
   >
   >    b. 也可为 OCP external registry 提供，需指定 tag。
   >
   > 5. <git_repository_url>：指定远程 Git 代码仓库地址
   > 6. 使用非 openshift 项目中标准的 imagestream 时，只能使用第一种 "~" 方法创建应用！

   👉 使用 template 模板定义文件创建各应用资源：

      a. 指定项目与模板资源名称     

      b. `-p` 选项：指定模板文件中的参数，命令行中定义的参数将覆盖模板文件中定义的参数。

      ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/oc-new-app-template.jpg)

5. 操作资源对象：

   ```bash
   $ oc get nodes
   # 查看节点的概要信息（system:admin 用户或具有 cluster-admin 角色的用户执行）
   
   $ oc get all [-n <project>]
   # 查看项目中所创建的所有资源的重要信息
   # 可指定项目名称，若不指定，则默认使用所在的项目。
   
   $ oc get <resource_type> <resource_name> -o yaml [-n <project>]
   # 查看指定资源对象的详细信息
   
   $ oc describe <resource_type> <resource_name> [-n <project>]
   # 查看指定资源的详细信息
   
   $ oc export <resource_type> <resource_name> [-n <project>] -o json > <filename>.json
   # 以 JSON 格式输出指定资源的资源定义文件
   # 输出的资源定义文件可进行修改，再用于创建相同类型的资源。
   
   $ oc create -f <resource_defination_file>.json [-n <project>]
   # 使用修改的资源定义文件创建新的资源
   # oc create 命令常与 oc export 命令一起使用
   
   $ oc update -f <resource_defination_file>.json [-n <project>]
   # 更新指定资源的资源定义文件（较小改动）
   
   $ oc edit <resource_type> <resource_name> [-n <project>]
   # 启用 vi 缓冲区以编辑指定资源的资源定义文件，编辑后即时生效。
   
   # ----- 创建不同类型的 route 路由资源 -----
   $ oc expose svc <service_name> \
     --name=<route_name> --port=<service_port> \
     --hostname=<custom_name>.<wildcard_domain> [-n <project>]
   # 将 service 通过 route 暴露给 OCP 集群外部，使外部能够访问前端 pod。     
   $ oc expose svc temp-cvt --name=ocp
   # --name        指定 route 的名称
   #               若不指定 route 的名称，则使用 application_name 代替 route_name。
   # --hostname    指定对外的公网域名
   #               默认的对外公网域名：<route_name-project_name>.<wildcard_domain>
   
   $ oc create route edge \
     --service=<service_name> --hostname=<exposed_fqdn_url> \
     --key=<ca_trusted_private_key>.key --cert=<ca_trusted_certificate>.crt
   # 使用 CA 私钥与 CA 签名的证书为 service 创建安全的边界型路由规则（secure edge-terminated）
   ```
   
   > 💥注意：
   >
   > OCP 3.9 版本删除 route 并重建后无法生效，报错 `HostAlreadyClaimed`，Bugfix 请详见参考链接。
   
   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-delete-route-error-1.jpg)
   
   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-delete-route-error-2.jpg)
   
   ```bash
   $ oc port-forward <pod> <localhost_port>:<pod_port> [-n <project>]
   # 将本地节点的端口映射至远程 pod 的端口，不局限于 80 与 443 端口，可供开发人员使用调试。
   # 本地节点可以为 OCP 集群外节点，该方法提供了从 OCP 集群外访问 pod 的方式。
   
   $ oc exec <pod> [-n <project>] -- <command>
   # 直接在 pod 中执行命令并返回结果 
   
   $ oc exec -it <pod> /bin/bash [-n <project>]
   # 以交互模式进入 pod 运行环境中
   
   $ oc exec -it -p <pod> -c <container> /bin/bash [-n <project>]
   # 以交互模式进入指定 pod 的指定容器运行环境中
   
   $ oc rsh <pod> bash -c '<command>' [-n <project>]
   # 使用远程 shell 会话在 pod 中运行命令，pod 中必须具有 shell 运行环境。
   
   $ oc rsh -t <pod> [-n <project>]
   # 使用远程 shell 会话交互方式进入 pod 运行环境
   
   $ oc cp <path_of_file> <pod>:<path_of_file> [-n <project>]
   # 拷贝本地文件至 pod 中
   # 本地文件路径与 pod 中的文件路径都必须具有文件名称，并且容器镜像中必须具有 tar 命令。 
   # 拷贝本地文件至 pod 中，也可将 pod 中的文件拷贝至本地，如下所示：
   #   $ oc cp /home/developer/quote.sql quotesdb-1-fzrgd:/tmp/quote.sql 
   #   $ oc cp quotesdb-1-fzrgd:/tmp/quote.sql /home/developer/quote.sql  
   
   $ oc env pod <pod> --list [-n <project>]
   # 查看 pod 中容器的环境变量
   
   $ oc volume pod <pod> [-n <project>]
   # 查看 pod 中容器的挂载点与 pvc 的对应关系
   
   $ oc volume dc <deploymentconfig> [-n <project>]
   # 查看部署配置中的 volume 信息（pvc）
   
   $ oc delete project <project>
   # 删除项目及其所有资源
   
   $ oc delete all --labels=<label>
   # 删除项目中所有相应标签的资源
   # 可在创建各项资源时添加标签，便于删除相应资源。
   ```
   
6. 使用 template 模板资源：

   ```bash
   $ oc get templates [-n <project>]
   # 查看指定项目中的 OCP 资源定义模板，使用 oc get all 命令无法获取。
   
   $ oc create -f <template>.json [-n <project>]
   # 上传 OCP 资源定义模板至指定项目中
   # 企业生产环境中常使用 template 模板部署应用
   
   $ oc describe templates <template> [-n <project>] 
   # 查看指定项目中 OCP 资源定义模板的参数定义
   
   $ oc process -f <template>.json | oc create -f -
   # 使用 template 模板文件部署应用资源
   # 注意：template 模板文件中可直接定义 pvc 与 route
   ```

7. 日志与事件：

   ```bash
   # ----- 应用构建日志与事件 -----
   $ oc logs <resource_type> <resource_name> [-n <project>]
   # 查看指定资源的日志信息，该日志信息不输出至 /var/log/messages。
   
   $ oc get builds
   # 查看 build 的构建状态
   
   $ oc logs build/<build_name>
   # 查看指定构建的日志
   
   $ oc logs <builder_pod> [-n <project>]
   # 查看构建 pod 的日志
   $ oc logs hello-1-build
   # 查看 builder pod 的构建编译过程，包括新生成的应用容器镜像上传至 OCP 集群内部镜像仓库的过程。
   
   $ oc get buildconfig
   # 查看已存在的构建配置
   
   $ oc start-build bc/<buildconfig> [-n <project>]
   # 重新进行 S2I 应用容器镜像的构建
   
   $ oc logs -f bc/<buildconfig> [-n <project>]
   # 查看构建配置过程日志
   ```

   > ✅注意：
   >
   > 1. 由于应用源代码的更改或构建镜像（builder image）的改变将触发应用 pod 的重新部署。
   >
   > 2. 若应用源代码存在错误，并自动进行构建与部署，将导致 pod 的报错，因此需确认源代码无误后，手动进行构建。
   >
   > 3. 构建成功后将触发新的自动部署，直至 pod 的正常运行。
   >
   > 4. start-build 构建报错示例：
   >
   >    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-start-build-error-1.jpg)
   >
   >    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/ocp3-start-build-error-2.jpg)
   >
   > 5. 更改 buildconfig 中的 `BUILD_LOGLEVEL` 环境变量（env）可调整构建配置的日志等级，如该值为 5。

   ```bash
   $ oc logs -f dc/<deploymentconfig> [-n <project>]
   # 查看部署过程日志
       
   $ oc logs <pod> [-n <project>]
   # 查看 pod 的运行日志
   
   $ oc get [events|ev] [-n <project>]
   # 查看 OCP 集群的事件信息，常用于 troubleshooting 排错。
   # 也可在 Web Console 的 Monitoring > Events 中查看事件信息
   ```
   
8. secret 与 configmap：

   ```bash
   $ oc create secret generic <secret_name> \
     --from-literal='<key1>'='<value1>' ... --from-literal='<keyN>'='<valueN>'
   # 创建 secret 资源对象，使敏感数据与 pod 解耦。
   
   $ oc create secret generic mysql \
     --from-literal='database-user'='mysql' \
     --from-literal='database-password'='redhat' \
     --from-literal='database-root-password'='do285-admin'
   # 创建 secret 资源对象以存储 MySQL 相关的用户名与密码，该 secret 可被其他资源所引用。
   ```

   ```bash
   $ oc create configmap <configmap_name> \
     --from-literal='<key1>'='<value1>' ... --from-literal='<keyN>'='<valueN>' \
     [-n <project>]
   # 创建 configmap，该资源定义环境变量，并将其注入 deploymentconfig 中。
   ```

   ```bash
   $ oc set env dc/<deploymentconfig_name> \
     --from=configmap/<configmap_name> [-n <project>]
   # 将 configmap 定义的环境变量注入 deploymentconfig 中
   ```

   > 💥注意：
   >
   > 以上操作由于改变了 dc 配置，将触发新的 dc，部署全新的 pod。
   >
   > ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/configmap-trigger-dc.jpg)

   ```bash
   $ oc rollout latest dc/<deploymentconfig_name> [-n <project>]
   # dc 将根据 REVISION 中的版本更新至最新版本，pod 将重新部署至最新版本。
   
   $ oc rollback 
   # oc rollout/rollback 都是针对 dc 来实现
   ```
   
9. pod 的资源调度：

   ```bash
   $ oc label node <node_fqdn> <key>=<value> [--overwrite]
   # 设置（覆盖）已存在的 node 节点标签
   
   $ oc label node node2.lab.example.com region=app --overwrite 
   # 设置（覆盖）已存在的节点标签 region 为 app
   # 设置的节点标签可被 pod 的节点选择器 Pod.spec.nodeSelector 使用，使其调度至该节点。
   ```

   > ✅注意：region 为地理概念，zone 为不同的机柜/架或机房（故障恢复域）。

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/node-label.jpg)

   ```bash
   $ oc adm manage-node --schedulable=false <node_fqdn>
   # 设置 node 节点为 pod 不可调度状态
   ```

   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/node-unscheduleable.jpg)

   ```bash
   $ oc adm manage-node <node_fqdn> --evacuate --pod-selector='<key>'='<value>
   # 指定 pod 标签从 node 节点上迁移指定的 pod
   ```
   
   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/pod-evacuate-1.jpg)
   
   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/pod-evacuate-2.jpg)

   ```bash
   $ oc adm drain <node_fqdn> [--delete-local-data]
   # 从 node 节点上撤离所有运行的 pod
   # 若 pod 中已挂载使用相应的 pvc，在撤离时将报错，无法卸载已使用的 pvc！
   ```
   
   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/evacuate-delete-local-data-1.jpg)
   
   ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/evacuate-delete-local-data-2.jpg)
   
10. OCP 用户（user）与基于角色的访问控制（RBAC）：

    ```bash
    $ oc adm policy add-cluster-role-to-user cluster-admin admin
    # 为 admin 用户添加 cluster-role 集群管理员角色
    ```

    > ✅注意：
    >
    > 1. OCP 的权限模型：用户与组（users and groups）、角色（roles）
    >
    > 2. OCP 中用户分类：普通用户（regular user）、系统用户（system user）、服务账户（service account）   
    >
    > 3. 基于角色的访问控制（`RBAC`）：用户与组通过绑定（binding）与角色（roles）相关联
    >
    > 4. OCP 定义的两种策略：角色是策略的集合
    >
    >    a. 集群策略（cluster policy）：administration-related
    >
    >    b. 本地项目策略（local policy）：project-related
    >
    >    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/project-level-role.jpg)
    >
    > 5. 用户与组可同时绑定一个或多个本地项目角色与集群角色。

    ```bash
    $ oc adm policy remove-cluster-role-from-group \
      self-provisioner \
      system:authenticated:oauth
    # 从集群角色中删除自调配角色，使已认证的 oAuth 用户与组无法调配创建新项目。
    
    $ oc adm policy add-cluster-role-from-group \
      self-provisioner \
      system:authenticated \
      system:authenticated:oauth
    # 集群角色中添加自调配角色，使已认证的 oAuth 用户与组能调配创建新项目。
    
    $ oc get clusterrole
    # 查看集群角色信息
    ```

    ```bash
    $ oc describe clusterrole system:<role>
    # 查看集群角色的详细信息
    ```
    
    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/clusterrole-demo.jpg)
    
    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/verbose-examples.jpg)

    ```bash
    $ oc describe clusterrole self-provisioner
    # 查看自调配角色的详细信息
    $ oc get clusterrolebinding.rbac -n default
    # 查看集群角色绑定的信息
    $ oc get rolebinding.rbac -n <project_name>
    # 查看指定项目中用户的本地项目角色绑定信息
    ```
    
    ![](https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/self-provisioner-desc.jpg)

    ```bash
    $ oc create serviceaccount <serviceaccount_name> [-n <project>]
    # 在指定项目中创建服务账户，该账户可用于pod与api-server的通信认证。
    # 注意：服务账户必须由具有项目管理员角色的用户创建
    $ oc create serviceaccount wordpress -n farm
    ```
    
    <img src="https://github.com/Alberthua-Perl/scripts-confs/blob/master/docs/images/serviceaccount-wordpress.jpg" style="zoom:150%;" />

    ```bash
    $ oc adm policy \
      add-scc-to-user anyuid -z <serviceaccount_name> -n <project> 
    # 使用 system:admin 用户或具有 cluster-admin 角色的用户为指定项目的服务账户添加 anyuid 的安全上下文（SCC）
    # 该安全上下文可使 pod 中运行应用的用户提权至 root 权限
    ```

#### 参考链接：

- OpenShift 项目：

  https://github.com/openshift/
  
- OKD 3.11 官方文档：

  https://docs.okd.io/3.11/welcome/index.html 

- 基于 S2I 的 Golang 应用构建与部署示例：

  https://github.com/Alberthua-Perl/dockerfile-s2i-demo/tree/master/golang-s2i

- SkyDNS 项目：

  https://github.com/skynetservices/skydns

- Bugfix: route HostAlreadyClaimed

  https://bugzilla.redhat.com/show_bug.cgi?id=1660598

  
