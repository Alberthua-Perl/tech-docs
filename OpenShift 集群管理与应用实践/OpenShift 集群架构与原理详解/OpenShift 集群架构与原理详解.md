# ⭕ OpenShift 集群架构与原理详解

## 文档说明

- 该文档作为笔者在学习与实践 OpenShift 过程中的总结与感悟，难免存在纰漏，望不吝赐教。
- 该文档主要根据 `Red Hat OpenShift Container Platform v3.9` 与 `v4.14` 环境实践。
- 以下分别称为 `OCP3` 与 `OCP4`。
- 其中涉及的选项与参数在绝大部分版本中适用，但部分版本可能略有不同，请参考实际使用版本。
- 以上两个版本在架构与资源对象概念上存在诸多相同点，若未特别说明，即两者均适用。

## 文档目录

- [⭕ OpenShift 集群架构与原理详解](#-openshift-集群架构与原理详解)
  - [文档说明](#文档说明)
  - [文档目录](#文档目录)
  - [1. OpenShift 基础架构概述](#1-openshift-基础架构概述)
  - [2. OpenShift 集群部署方法说明](#2-openshift-集群部署方法说明)
  - [3. OpenShift 帮助与登录](#3-openshift-帮助与登录)
  - [4. CRI-O 容器运行时相关命令](#4-cri-o-容器运行时相关命令)
  - [🔥 5. OpenShift 资源对象详解](#-5-openshift-资源对象详解)
    - [5.1 Master 类型节点](#51-master-类型节点)
    - [5.2 Worker 类型节点](#52-worker-类型节点)
    - [5.3 Project 资源对象](#53-project-资源对象)
    - [📦 5.4 ImageStream (is) 与 ImageStreamTag (istag) 资源对象](#-54-imagestream-is-与-imagestreamtag-istag-资源对象)
      - [5.4.1 基础概念](#541-基础概念)
      - [👨‍💻 5.4.2 **示例：从 OCP external registry 导入外部容器镜像至 OCP 集群**](#-542-示例从-ocp-external-registry-导入外部容器镜像至-ocp-集群)
      - [5.4.3 OCP internal registry 镜像缓存说明](#543-ocp-internal-registry-镜像缓存说明)
      - [5.4.4 image stream 资源使用](#544-image-stream-资源使用)
      - [💎 5.4.5 OCP4 internal registry 说明](#-545-ocp4-internal-registry-说明)
      - [5.4.6 image stream 使用报错示例](#546-image-stream-使用报错示例)
    - [⚙️ 5.5 BuildConfig (bc) 与 Build 资源对象](#️-55-buildconfig-bc-与-build-资源对象)
      - [5.5.1 基础概念](#551-基础概念)
      - [5.5.2 构建触发器（build trigger）类型](#552-构建触发器build-trigger类型)
      - [5.5.3 build 构建与 S2I 说明](#553-build-构建与-s2i-说明)
    - [5.6 DeploymentConfig (dc) 与 Deploy 资源对象](#56-deploymentconfig-dc-与-deploy-资源对象)
    - [💎 5.7 Deployment 资源对象](#-57-deployment-资源对象)
    - [5.8 ReplicationController (rc) 与 ReplicaSet 资源对象](#58-replicationcontroller-rc-与-replicaset-资源对象)
    - [5.9 Pod 资源对象](#59-pod-资源对象)
    - [5.10 Label 资源对象](#510-label-资源对象)
    - [🔥 5.11 Service (svc) 资源对象](#-511-service-svc-资源对象)
      - [🚀 5.11.1 OCP3 \& OCP4 的网络模型类型](#-5111-ocp3--ocp4-的网络模型类型)
      - [5.11.2 service 资源对象处理的场景](#5112-service-资源对象处理的场景)
      - [5.11.3 service 在 Kubernetes 中的实现方式](#5113-service-在-kubernetes-中的实现方式)
      - [5.11.4 service 的类型](#5114-service-的类型)
      - [5.11.4 service 与 pod 的关联方式](#5114-service-与-pod-的关联方式)
      - [5.11.5 service 与服务发现](#5115-service-与服务发现)
      - [5.11.6 service 与实现跨节点通信的 CNI 插件的关系](#5116-service-与实现跨节点通信的-cni-插件的关系)
    - [5.12 Route 资源对象](#512-route-资源对象)
    - [5.13 PersistentVolume (pv) 资源对象](#513-persistentvolume-pv-资源对象)
    - [5.14 PersistentVolumeClaim (pvc) 资源对象](#514-persistentvolumeclaim-pvc-资源对象)
    - [5.15 Secret 资源对象](#515-secret-资源对象)
      - [5.15.1 基础概念](#5151-基础概念)
      - [5.15.2 创建与使用 secret 资源对象](#5152-创建与使用-secret-资源对象)
      - [5.15.3 提取与更新 secret](#5153-提取与更新-secret)
      - [👨‍💻 **5.15.4 示例：OCP 4.6 中使用 secret 拉取外部私有容器镜像**](#-5154-示例ocp-46-中使用-secret-拉取外部私有容器镜像)
      - [5.15.5 serviceaccount 与 secret 的关联](#5155-serviceaccount-与-secret-的关联)
    - [5.16 ConfigureMap (cm) 资源对象](#516-configuremap-cm-资源对象)
      - [5.16.1 基础概念](#5161-基础概念)
      - [5.16.2 创建与使用 configmap 资源](#5162-创建与使用-configmap-资源)
  - [🧪 OpenShift 资源对象使用](#-openshift-资源对象使用)
  - [OpenShift 用户与访问控制](#openshift-用户与访问控制)
    - [OpenShift 用户认证（Authentication）](#openshift-用户认证authentication)
    - [OpenShift 用户授权（Authorized）](#openshift-用户授权authorized)
  - [OpenShift Pod 的调度](#openshift-pod-的调度)
  - [OpenShift 服务与路由使用](#openshift-服务与路由使用)
  - [OpenShift 日志与事件](#openshift-日志与事件)
  - [参考链接](#参考链接)

## 1. OpenShift 基础架构概述

- 上游 `Origin` 项目与 `OpenShift` 项目的发展对应关系：
  
  ![ocp3-origion-developer](images/ocp3-origion-developer.jpg)
  
  2014 年 Kubernetes 诞生以后，Red Hat 决定对 OpenShift 进行重构（原先的架构不依托于 Kubernetes），正是该决定，彻底改变了 OpenShift 的命运以及后续 `PaaS` 市场的格局。  
  2015 年 6 月，Red Hat 推出了基于 `Kubernetes 1.0` 的 `OpenShift 3.0`。  
  🚀 2018 年 6 月，Red Hat 推出了基于 `Kubernetes 1.13` 的 `OpenShift 4.1`，在 OCP4 架构中引入及增强了 OCP3 中新功能。

- OpenShift 的客户端命令行工具 `oc` 命令取代 Kubernetes 的 `kubectl` 命令，相同版本下两者的使用方法与参数选项基本保持一致。
- OCP3 集群架构：
  
  ![ocp3-arch](images/ocp3-arch.png)

- OCP4 集群架构：
  
  ![ocp4-arch](images/ocp4-arch.jpg)
  
  OCP4 中已将 `Kubernetes service` 与 `OpenShift service` 进行解耦实现松耦合设计，使 OpenShift 集群自身的资源由自身的 service 进行管理，此处的 service 指控制平面（control plan）的服务组件与集群中的 service 资源相区别。
  
  ![kubernetes-service-in-ocp4](images/kubernetes-service-in-ocp4.jpg)
  
  ![openshift-service-in-ocp4](images/openshift-service-in-ocp4.jpg)

- OCP4 集群网络拓扑示例：  
  OCP3 与 OCP4 在集群 SDN 的选型上存在差异，如 OCP3 使用 `OVS` 插件实现 SDN 并且不能支持单 Pod 具有多个虚拟网络接口，而 OCP4 可使用 `OVS` 插件或 `OVN-Kubernetes` 插件并且支持单 Pod 具有多个虚拟网络接口，如下所示，OCP4 南北向流量与东西向流量拓扑。
  
  ![ocp4-sourth-north-east-west-traffic](images/ocp4-sourth-north-east-west-traffic.png)

## 2. OpenShift 集群部署方法说明

- OCP3 集群部署方法：  
  - 生产环境：
    - OCP 3.4、3.5 集群部署使用 RPM 软件包，OCP 3.9、3.11 集群部署使用容器镜像。
    - 👉 OCP3 中使用 `Ansible` 部署 OpenShift。  
  - 开发与测试环境：
    - all-in-one：`AIO`（本地单节点集群），即 CRC 开发与测试环境。
    - OCP 二进制执行程序快速启动与部署
    - 👉 `minishift` 工具部署含 all-in-one 集群的虚拟机，与 `minikube` 非常类似。
  - 如下所示，minikube 在 RHEL 8.0 中的安装报错：

    ![minikube-error-1](images/minikube-error-1.jpg)

    ![minikube-error-2](images/minikube-error-2.jpg)

    ![minikube-error-3](images/minikube-error-3.jpg)

    ```bash
    $ minikube addons list
    # 查看 minikube Kubernetes 支持的插件列表
    ```

- OCP4 集群部署方法：
  - CRC 开发与测试环境：[Red Hat OpenShift Local (CRC) v2.35 部署与管理](https://github.com/Alberthua-Perl/tech-docs/blob/master/Red%20Hat%20OpenShift%20Container%20Platform/Red%20Hat%20OpenShift%20Local%20v2.35%20%E9%83%A8%E7%BD%B2%E4%B8%8E%E7%AE%A1%E7%90%86.md)
  - MicroShift 单节点集群环境（边缘计算场景）：[基于 RHEL9.3 的 Red Hat MicroShift v4.15 部署与管理](https://github.com/Alberthua-Perl/tech-docs/blob/master/Red%20Hat%20OpenShift%20Container%20Platform/%E5%9F%BA%E4%BA%8E%20RHEL9.3%20%E7%9A%84%20Red%20Hat%20MicroShift%20v4.15%20%E9%83%A8%E7%BD%B2%E4%B8%8E%E7%AE%A1%E7%90%86/%E5%9F%BA%E4%BA%8E%20RHEL9.3%20%E7%9A%84%20Red%20Hat%20MicroShift%20v4.15%20%E9%83%A8%E7%BD%B2%E4%B8%8E%E7%AE%A1%E7%90%86.md)

## 3. OpenShift 帮助与登录

- 帮助命令：
  
  ```bash
  $ oc version
  # 查看 OCP 与 K8s 版本信息
  
  $ oc <command> --help
  # 查看 oc 子命令的使用方法
  
  $ oc options
  # 查看 oc 命令行可用的选项
  
  $ oc api-resources
  # 查看当前集群中受支持的 API 资源，默认不区分是否在命名空间中。

  $ oc api-resources --namespaced=[true|false]
  # 查看命名空间内（true）或全局非命名空间（false）内的 API 资源

  $ oc api-resources --api-group=<api_group_name>
  # 指定 API 组查看其中支持的 API 资源
  $ oc api-resources --api-group=''
  # 查看核心 API 资源
  $ oc api-resources --api-group=operator.openshift.io
  # 查看 operator.openshift.io/v1 API 组的 API 资源

  $ oc api-resources --namespaced=true --api-group apps --sort-by name
  # 查看命名空间内，指定 API 组中支持的 API 资源，并根据 name 列排序。

  $ oc types
  # 查看 OCP 集群的概念与类型说明
  # 💥 注意：该子命令已在 OCP4 中不再使用！
  
  $ oc explain <resource_object>
  # 查看 OCP 集群指定资源对象的详细说明
  ```

  以下为 OCP4 (v4.14) 集群中获取的 operator.openshift.io/v1 的 API 资源：

  ![ocp4-api-resources-operator](images/ocp4-api-resources-operator.png)

- 密码与字符串编码：
  
  ```bash
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
  $ echo "<hash>" | base64 [-d|--decode]
  # 使用 base64 加密算法对哈希解密
  
  $ python -c \
    "import crypt, getpass, pwd; print crypt.crypt('<password>', '\$6\$<salt_vaule>\$')"
  # 根据 salt 值通过 SHA512 哈希算法对明文密码加密，生成相应的哈希值。
  # crypt 模块只可在 python2 环境中使用
  $ perl -e 'print crypt("<password>", "\$6\$<salt_value>\$")."\n"'
  # 使用 perl 生成明文密码的 SHA512 哈希值
  ```

- 集群登录：
  
  ```bash
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
  
  ![system-admin-logout](images/system-admin-logout.jpg)
  
  ![docker-registry-route](images/docker-registry-route.jpg)

## 4. CRI-O 容器运行时相关命令

- 虽然流行的容器运行时包括 Docker、Containerd、Podman 等，但在 OpenShift 集群中的各节点上使用 CRI-O 容器运行时运行容器与 Pod。从 OCP4 开始，集群中的容器运行时均使用 CRI-O，不再使用 OCP3 中的 Docker。
- CRI-O 提供一个命令行接口可使用 `crictl` 工具管理容器与 Pod。
- 由于 OpenShift 课程环境的集群设置，可使用如下方法登录集群节点：

  ![login-ocp-course-lab-node](images/login-ocp-course-lab-node.png)

- 以上方法登录集群节点具有特殊性，而使用 `oc debug` 子命令将在指定的集群节点上启用 debug pod，通过此 pod 可对其宿主节点进行调试。

  ![oc-debug-chroot](images/oc-debug-chroot.png)

- crictl 工具使用示例：

  ```bash
  $ sudo crictl pods
  # 列举集群节点上的 pod 列表信息

  $ sudo crictl images
  # 查看集群节点上的容器镜像列表

  $ sudo crictl ps -o [json|yaml]
  # 查看集群节点上运行的容器

  $ sudo crictl inspect <container_id> | jq .info.pid
  # 获取指定容器中运行进程的 PID

  $ sudo crictl exec <container_id> <command> <arg1> <arg2> ... <argN>
  # 交互式地在指定的容器中运行命令
  $ sudo crictl exec -it <container_id> /bin/bash
  bash-4.4$ 

  $ sudo crictl logs <container_id>
  # 查看指定容器的标准输出与标准错误日志
  ```

  如下所示，crictl exec 子命令可交互式进入容器内部查看进程状态，同样也可利用 crictl 命令获取容器内进程 PID，再使用 `lsns` 与 `nsenter` 命令获取进程命名空间中的进程状态：

  ![crictl-get-container-info-1](images/crictl-get-container-info-1.png)

  ![crictl-get-container-info-2](images/crictl-get-container-info-2.png)

## 🔥 5. OpenShift 资源对象详解

### 5.1 Master 类型节点

- OCP 集群或 Kubernetes 集群的控制节点
- 生产环境中建议使用 3 个或奇数个 master 节点确保控制平面（control plane）的高可用性，推荐将 etcd 数据库集群单独分开。
- OCP 3.4、3.5 的 master 节点：不运行 pod，核心组件以 `systemd` 服务的方式运行。
- OCP 3.9、3.11 的 master 节点：可运行 pod，核心组件也以 `pod` 的方式运行。
- master 节点执行的服务包括：
  - `apiserver`（包括 authentication/authorization）：
    - 接收、响应来自集群内部与外部的 `Restful API` 请求。
    - 处理 OCP 集群内的用户与服务的身份认证/授权服务（`OAuth`）。
  - `controller-manager`：
    控制管理器，用于实现无状态 pod 与有状态 pod 的控制管理。
  - `scheduler`：
    调度器，用于实现 pod 在各个 node 节点上的分配调度。
  - `data-store`：
    `etcd` 分布式键值型数据库，用于服务配置发现，OCP 集群中的数据存储与核心。

    ![ocp3-master-pod](images/ocp3-master-pod.jpg)

    ![ocp3-master-etcd](images/ocp3-master-etcd.jpg)

### 5.2 Worker 类型节点

- OCP 集群与 Kubernetes 集群的计算节点
- compute 节点用于运行 pod 提供服务
- compute 节点的 docker 守护进程异常而导致 `atomic-openshift-node` 服务报错。
- 每个节点上的 `atomic-openshift-node` 服务已集成 `kubelet` 功能。

![atomic-openshift-node-error-1](images/atomic-openshift-node-error-1.jpg)  

![atomic-openshift-node-error-2](images/atomic-openshift-node-error-2.jpg)

### 5.3 Project 资源对象

- 中文名称：项目，也称为命名空间（namespace），OCP 集群使用项目来隔离资源（硬隔离），区别于 Linux namespace。
- 若未将 `self-provisioner` 角色从指定用户去除，使用指定用户创建的项目，该用户即为项目的项目管理员。
- default 项目与 openshift 项目能被所有用户使用，但只能由 `system:admin` 用户或具有 `cluster-admin` 角色的用户管理。

### 📦 5.4 ImageStream (is) 与 ImageStreamTag (istag) 资源对象

#### 5.4.1 基础概念

- 中文名称：镜像流、镜像流标签

  ```bash
  $ oc get imagestream -n openshift -o name
  # 查看 openshift 项目中所有的镜像流名称
  ```
  
- image stream 是容器镜像在 OCP 集群中的镜像元数据拷贝，它可存储当前的与过去的容器镜像层，用于加速查询与显示容器镜像。
- 一个 image stream 为一系列 iamge stream tags 提供默认的配置。
- 每个 image stream tag 引用一个容器镜像流（container image stream）。
- 每个 image stream tag 使用对应的镜像 ID（image ID），其使用 `SHA-256` 哈希唯一地标识一个不可变的容器镜像，并且不能修改镜像，相反，需创建一个具有新 ID 的新容器镜像。
- image stream tag 保存了从 OCP external registry 或 OCP internal registry 获取的最新 image ID 的历史记录。

  ![oc-describe-is](images/oc-describe-is.jpg)
  
- image stream tag 中当前引用的容器镜像前带 `*` 号，通常位于镜像列表的第一个。
- 可使用 image stream tag 内的历史记录回滚到以前的镜像，如，若一个新的容器镜像导致部署错误，可回滚至之前的镜像版本。
- 在 OCP external registry 中更新容器镜像不会自动更新 image stream tag。
- image stream tag 将引用它获取的最后一个 image ID，这种行为对于扩展应用程序至关重要，因为它将 OpenShift 与 OCP external registry 上发生的更改隔离开来。
- 因此，OCP external registry 中的容器镜像发生更新，需同时更新 image stream tag 以使用对应的 image ID，否则 image stream tag 依然使用原始的 image ID。
- OpenShift 确保新部署的 pod 使用的 image ID 与第一个部署的 pod 的 image ID 是相同的。
- `openshift` 项目中保存 S2I 使用的构建镜像（S2I builder image）的 image stream。
- 构建镜像的组成：基础 OS 运行环境、编程语言环境、应用依赖、编程语言框架等
- **image stream 引用的容器镜像可来自 `OCP external registry` 或 `OCP internal registry`。**
- 以下可作为 OCP external registry：
  - docker.io
  - registry.access.redhat.com
  - registry.redhat.io
  - quay.io
  - 公司或组织内部的 Harbor、Red Hat Quay、registry v2、docker-distribution 等

    > 👉 关于 Red Hat Quay 的容器镜像仓库的原理与部署可参考 [此 GitHub 链接](https://github.com/Alberthua-Perl/tech-docs/blob/master/Red%20Hat%20Quay%20v3%20registry%20%E5%8E%9F%E7%90%86%E4%B8%8E%E5%AE%9E%E7%8E%B0.md)。
  
- 以下可作为 OCP internal registry：
  - OCP 集群内的 docker-registry pod
  - OCP 集群内的 quay pod

  > ✅ external 与 internal 相对于 OCP 集群内外而言
  
#### 👨‍💻 5.4.2 **示例：从 OCP external registry 导入外部容器镜像至 OCP 集群**

- 1️⃣ 方式 1：</br>
  `skopeo` 命令拷贝已从 OCP external registry 中拷贝的 `OCI` 格式目录至 OCP internal registry 中成为指定项目的 image stream，该 image stream 在项目中会自动创建（见如下补充中的示例）。
- 2️⃣ 方式 2：</br>
  💥 若 openshift 项目中存在 image stream tag 但不引用任何镜像时，需导入 image stream tag 至容器镜像的引用。
  - oc import-image 命令创建 `image stream tag` 引用外部 `公共` 容器镜像。

    ```bash
    $ oc import-image <imagestream>:[<tag>] --confirm \
      --from <container_registry_for_imagestream> [--insecure] \
      -n <project_name>
    # 在指定项目中创建 image stream tag 引用外部容器镜像
    # 需确认容器镜像仓库是否使用 TLS 连接
    # 注意：
    #   openshift 项目中 image stream 无法使用相应 tag 的容器镜像报错处理：
    #   1. 查看集成的 OCP 外部镜像仓库中是否具有相应 tag 的容器镜像
    #   2. 删除报错的 image stream，报错信息如 "! error: Import ..."。
    #      $ oc tag -d <imagestream>:<tag> -n openshift 
    #   3. 重新导入 OCP 外部镜像仓库中的容器镜像的 metadata 至 image stream。
    #      $ oc import-image apache-httpd:2.5 --confirm \
    #        --from registry.lab.example.com:5000/do288/apache-httpd \
    #        --insecure -n openshift
        
    $ oc import-image <imagestream>[:<tag>] --confirm
    # 指定当前存在的 image stream 更新其引用至 OCP external registry 中最新的 image ID
    ```

  - oc import-image 命令创建 `image stream tag` 引用外部 `私有` 容器镜像。引用外部私有容器镜像时需提供 `access token` 作为 secret，这与通过外部私有容器镜像仓库中的镜像部署应用的方式相同，只是不需要链接（link）任何 service account，oc import-image 命令能在当前项目中搜寻匹配外部私有容器镜像仓库名称的 secret，如下所示：

    ![oc-import-image-private-is](images/oc-import-image-private-is.jpg)

  - 以上示例如下：

    ```bash
    $ podman login -u alberthua quay.io
    $ oc create secret generic privateis \
      --from-file .dockerconfigjson=/run/user/1000/containers/auth.json \
      --type kubernetes.io/dockerconfigjson
    $ oc import-image nginx-ssl:1.0.1 --confirm \
      --from quay.io/alberthua/nginx-ssl:1.0.1
    ```

#### 5.4.3 OCP internal registry 镜像缓存说明

- 🚀 默认情况下，oc import-image 命令使用 `--reference-policy=source` 选项，创建 image stream tag 引用 OCP external registry，不将容器镜像缓存至 OCP internal registry 中，因此，删除外部私有镜像仓库后将无法构建应用镜像。
  - 未删除外部私有镜像时，可被正确拉取构建，追踪 BUILD_LOGLEVEL=4 的 build 日志，如下所示：

    ![project-imagestream-reference-external-private-image-exist](images/project-imagestream-reference-external-private-image-exist.jpg)

  - 删除外部私有镜像后，将无法从外部拉取，如下所示：

    ![project-imagestream-reference-external-private-image-not-exist](images/project-imagestream-reference-external-private-image-not-exist.jpg)

- 🚀 而 `--reference-policy=local` 将 OCP external registry 中的容器镜像缓存至 OCP internal registry，相当于在集群本地实现了 `镜像缓存`，可加速应用构建，即使删除外部私有镜像后也不影响应用镜像的构建，如下所示：

  ![import-image-reference-policy-local-test](images/import-image-reference-policy-local-test.jpg)

  skopeo 命令验证是否存在镜像缓存：

  ![skopeo-internal-registry-cache-layer](images/skopeo-internal-registry-cache-layer.jpg)

- 👨‍💻 **示例：使用 S2I 方式与导入的外部私有构建镜像构建 NodeJS 应用**

  > 该示例说明以上 --reference-policy=local 选项的使用

  ```bash
  $ podman login -u alberthua quay.io
  $ oc create secret generic quaypriv \
    --from-file .dockerconfigjson=/run/user/1000/containers/auth.json \
    --type kubernetes.io/dockerconfigjson
  $ oc import-image nodejs:12-latest \
    --confirm \
    --reference-policy local \
    --from quay.io/alberthua/nodejs-12-rhel7:latest
  # 创建 image stream tag，将外部私有镜像缓存至 OCP internal registry 中。
      
  ### 注意：去删除外部私有镜像测试是否在集群内缓存了外部私有镜像 ###
      
  $ oc secrets link builder quaypriv
  # 将名为 quaypriv 的 secret 链接至名为 builder 的 service account 上
  # 注意：若不进行链接，将无法验证身份与拉取构建镜像！
  $ oc new-app --name myapp \
    --build-env npm_config_registry=http://${RHT_OCP4_NEXUS_SERVER}/repository/nodejs \
    --build-env BUILD_LOGLEVEL=4 \
    nodejs:12-latest~https://github.com/alberthua-perl/DO288-apps#app-config \
    --context-dir app-config
  # 指定构建日志级别进行应用镜像构建与应用部署
  $ oc logs -f buildconfig/myapp
  # 可追踪查看构建过程详细日志，即可得到以上示意图。
  ```

#### 5.4.4 image stream 资源使用
  
- 在 openshift 项目中的 image stream 与 template 资源在各个项目中均可共享，但只由具有 `cluster-admin` 角色的管理员用户管理。  
- 自 OCP4 开始可使用 `Samples operator` 管理 openshift 项目并可删除由手动添加的资源。
- **示例：使用来自另一个项目的 image stream（基于 private registry）构建与部署应用**
  - 1️⃣ 方式 1：</br>
    在每个使用 image stream 的项目中创建包含可访问私有 OCP external registry 的 access token 的 secret，并将其 link 至每个项目中的 service account。
  - 2️⃣ 方式 2：</br>
    仅仅在创建 image stream 的项目中创建包含可访问私有 OCP external registry 的 access token 的 secret，并配置 image stream 具有本地引用策略（`local reference policy`），即，将外部容器镜像缓存于 OCP internal registry，并为每个使用 image stream 的项目中使用 image stream 的 `service account` 授权。

    ![oc-import-image-shared-is-between-project](images/oc-import-image-shared-is-between-project.jpg)

  > ✅ 注意：可为 service account 添加 scc 与 role，也可将相关的 secret 链接至 service account。
  
#### 💎 5.4.5 OCP4 internal registry 说明

- OCP internal registry 只存储由 `S2I` 构建或 `Dockerfile/Containerfile` 构建的应用镜像，以实现一次构建多次部署。
- OpenShift 安装程序将 internal registry 配置为仅仅集群内部可被集群管理员、各项目用户或各个组件等访问。
- 使用集群管理员（`cluster-admin`）权限即可暴露 internal registry，对外暴露的路由信息可被集群外部访问，如下所示：

  ```bash
  $ oc patch config.imageregistry cluster -n openshift-image-registry \
    --type merge -p '{"spec":{"defaultRoute":true}}'
  ```

  ![ocp4-operator-oc-patch-internal-registry-expose-route](images/ocp4-operator-oc-patch-internal-registry-expose-route.jpg)

- internal registry 位于 `openshift-image-registry` 项目中，以 `image-registry pod` 的形式运行，并可通过 `cluster-image-registry-operator` 控制。

  ![ocp4-internal-registry-operator](images/ocp4-internal-registry-operator.jpg)

  > ✅ 默认 OpenShift 不允许常规用户访问 `openshift-*` 项目中的任何资源！

- 若要使用 Podman、Skopeo 来登录暴露的 internal registry，必须获得登录 OpenShift 集群用户的 `token`。

  ```bash
  $ TOKEN=$(oc whoami -t)
  # 获得登录 OpenShift 集群用户的 token
      
  $ INTERNAL_REGISTRY=$(oc get route default-route \
    -n openshift-image-registry -o jsonpath='{.spec.host}')
  # 查看 internal registry 对外暴露的 route 信息
      
  $ podman login -u <ocp_project_user> -p ${TOKEN} ${INTERNAL_REGISTRY}
  # 使用 OpenShift 集群用户与 token 登录内部镜像仓库
      
  $ skopeo inspect --creds=<ocp_project_user>:${TOKEN} \
    docker://${INTERNAL_REGISTRY}/<project>/<imagename>
      
  ### 示例 ###
  $ skopeo inspect --creds=iwootz:${TOKEN} \
    docker://${INTERNAL_REGISTRY}/iwootz-app-config/myapp
      
  $ skopeo copy --dest-creds=iwootz:${TOKEN} \
    oci:/home/student/DO288/labs/expose-registry/ubi-info \
    docker://${INTERNAL_REGISTRY}/iwootz-common/ubi-info:1.0
  # 通过认证 OpenShift 用户拷贝本地 OCI 格式目录（存储镜像的目录）至
  # internal registry 的 iwootz-common 项目目录中
  # 注意：同时在对应项目中也将自动创建同名的 image stream 引用该镜像
  ```

- 访问 internal registry 内镜像的角色（role）：
  - `registry-viewer` and `system:image-puller`：允许用户从 internal registry 中拉取与查看容器镜像
  - `registry-editor` and `system:image-pusher`：允许用户推送并 tag 镜像至 internal registry 中
  - `registry-*` 角色为 registry 管理提供了更全面的功能，这些角色授予额外的权限，如创建新项目，但不授予其他权限，如构建和部署应用程序。OCI 标准没有指定如何管理 registry，所以管理 OpenShift internal registry 的用户需要知道 OpenShift 管理概念和 oc 命令，这使得 registry-* 不那么友好。
  - `system:*` 角色提供了将镜像拉取与推送到 internal registry 所需的最小功能，已经在项目中拥有 admin 或 edit 角色的 OpenShift 用户不需要这些 system:* 角色。

#### 5.4.6 image stream 使用报错示例

- 若已存在的 image stream 报错 `! error`，可将其删除再从外部容器镜像仓库导入。
- 删除已存在的 image stream 需指定 `tag`：

  ```bash
  $ oc tag -d <imagestream_name>:[tag] -n <project>
  # 删除指定项目中 image stream 的 image stream tag，使其可重新上传。
  ```

  ![imagestream-error-1](images/imagestream-error-1.jpg)

  ![imagestream-error-2](images/imagestream-error-2.jpg)

  ![imagestream-error-3](images/imagestream-error-3.jpg)

### ⚙️ 5.5 BuildConfig (bc) 与 Build 资源对象

#### 5.5.1 基础概念

- 中文名称：构建配置、构建
- 创建 buildconfig 的方法：
  - 直接使用 `oc new-app` 命令创建或通过 template 模板中的参数化定义创建
  - `oc create -f` 命令使用 buildconfig 的 JSON 或 YAML 资源定义文件创建
- `oc new-app` 命令使用 `List` 资源定义文件，该文件定义 image stream、buildconfig、deploymentconfig、service，但不定义 route，必须手动定义 route 资源定义文件，或手动暴露 service 来创建 route 资源对象。
- buildconfig 资源中重要的字段说明：

  | 参数名 | 说明 |
  | ----- | ----- |
  | **spec.runPolicy** | 构建策略，默认值为Serial。有3种类型：<br> 1. Serial：按顺序执行构建 <br> 2. SerialLatestOnly：执行最新的构建 <br> 3. Parallel：并行执行构建 |
  | **spec.triggers** | 构建触发器的配置。可指定3种类型的触发器：<br> 1. Webhook：支持以向 OpenShift 发送 HTTP 请求的方式触发构建，支持 GitHub、GitLab、BitBucket、Generic 4种 Webhook <br> 2. ImageChange：镜像更新时触发构建 <br>3. ConfigChange：配置更新时触发构建 |
  | **spec.source** | 构建输入源。支持5种类型的输入源：<br> 1. Dockerfile：Dockerfile 文件内容 <br> 2. Images：从指定镜像获取构建所需文件或目录 <br> 3. Git：源代码仓库地址 <br> 4. Binary：二进制资源 <br> 5. configmap/secret：以 ConfigMap 或者 Secret 传入拉取构建所需内容的凭据 |
  | **spec.strategy** | 构建策略。支持3种构建策略：<br> 1. Source策略：S2I 策略，基于源代码和基础镜像来构建应用镜像 <br> 2. Docker 策略：通过docker build 命令使用 Dockerfile 文件构建应用镜像 <br> 3. JenkinsPipeline 策略：通过 Jenkinsfile 文件生成应用自动集成流水线 |
  | **spec.output** | 镜像构建完成后，将镜像推送到镜像仓库。有两种类型：<br> 1. ImageStream：OpenShift 内部镜像仓库，创建对应的ImageStreamTag <br> 2. DockerImage：将镜像推送到公有或私有的外部镜像仓库 |
  | **spec.postCommit** | 构建过程中的钩子脚本。镜像构建完成后，推送至内部镜像仓库前，需要在执行构建的容器中运行的脚本 |
  
#### 5.5.2 构建触发器（build trigger）类型

- OpenShift 可通过 buildconfig 手动触发构建，也可自动触发构建。
- 在 OpenShift中，可定义构建触发器，以允许平台根据特定事件（event）自动启动新的构建。
- buildconfig 中支持的三种构建触发器：
  - 1️⃣ **ImageChange trigger：**
    - 根据 image stream tag 引用的 S2I 构建镜像发生改变
    - Dockerfile/Containerfile 中 FROM 指令指定的父镜像发生改变而触发构建
    - S2I 构建镜像的更改将 `自动` 触发构建过程，如下所示：

      ![imagechange-trigger-auto-build](images/imagechange-trigger-auto-build.jpg)

  - 2️⃣ **Webhook trigger：**
  
    > 关于不同类型的 webhook 触发器的使用可参考 [Chapter 8. Triggering and modifying builds](https://access.redhat.com/documentation/en-us/openshift_container_platform/4.6/html-single/builds/index#triggering-builds-build-hooks)

    - 根据 `VCS`（version control system）中构建所使用的应用源代码的改变而触发构建，可通过手动触发与自动触发两种。
    - 通过 webhook 可与外部的代码仓库、持续集成（CI）与持续开发（CD）系统集成。
    - 支持的 webhook 类型：
      - 1️⃣ Generic：
        - 该 webhook 类型为 buildconfig 触发构建的默认方式，可通过以下命令 `手动` 触发构建：

          ```bash
          $ curl -i -X POST -k https://<url_for_generic_webhook>
          ```

        - 也可通过 oc start-build 命令手动触发构建，如下所示：

          ```bash
          $ oc start-build <build_name> -F [-n <project>]
          # 手动启动新的 S2I 应用容器镜像的构建，并显示构建的日志信息。
          ```

          构建成功后在项目中自动创建 `image stream tag` 引用已推送至 OCP internal registry 的应用容器镜像。
          若 deploymentconfig 中也定义了该 image stream tag，那么将自动触发应用部署，直至应用 pod 的正常运行。

          ```bash
          $ oc cancel-build <build_name> [-n <project>]
          # 在应用容器镜像构建失败前取消构建
          ```

          处于 `running` 或 `pending` 状态的 build 构建可被取消。
          取消构建意味着 `build pod` 的终止，不推送新镜像至 OCP internal registry 中，deploymentconfig 也不被触发。

          > 失败的 build 构建状态：failed、canceled、error

        - 💥 `oc start-build` 构建报错示例：

          ![ocp3-start-build-error-1](images/ocp3-start-build-error-1.jpg)

          ![ocp3-start-build-error-2](images/ocp3-start-build-error-2.jpg)

      - 2️⃣ GitHub：
        - 配置 GitHub 的 webhook 功能可在代码更新时向 OpenShift 推送代码以 `自动` 触发构建。
        - 👨‍💻 **示例：`Java` 应用 GitHub webhook 配置过程**
          oc describe bc 获得 github 类型的 `webhook URL`，其中的 `<secret>` 字段可被 oc get bc 中 triggers 的 github `secret` 字段所替换而获得完整的 URL。

          ![buildconfig-webhook-github-5](images/buildconfig-webhook-github-5.jpg)

          对应用源代码所在的仓库设置 `Webhooks`（只需更改以下两处即可）：

          ![buildconfig-webhook-github-1](images/buildconfig-webhook-github-1.jpg)

          更改代码后提交 commit：

          ![buildconfig-webhook-github-2](images/buildconfig-webhook-github-2.jpg)

          通过 webhook 自动将代码推送至集群，若推送成功无任何报错，推送的事件信息可在 webhook 中查看到详细信息：

          ![buildconfig-webhook-github-3](images/buildconfig-webhook-github-3.jpg)

          根据源代码的更改而自动触发构建：

          ![buildconfig-webhook-github-4](images/buildconfig-webhook-github-4.jpg)

      - 3️⃣ GitLab：</br>
        该类型常用于公司或组织内部离线的代码仓库，与以上 GitHub 配置方式类似，也可 `自动` 触发构建。
      - 4️⃣ BitBucket
    - oc new-app 命令默认创建了 `generic` 和 `github` webhook，这两种自动创建的触发器默认会创建对应的 secret。

      ![buildconfig-triggers-jq](images/buildconfig-triggers-jq.jpg)

    - OpenShift 也可为 buildconfig 添加多种 webhook 触发器，如 GitLab 等，要在 buildconfig 中添加其他类型的 webhook，需使用如下命令：

      ```bash
      $ oc set triggers bc/<buildconfig_name> --from-gitlab
      # 添加来自于 GitLab 的 buildconfig 自动构建的触发器
      $ oc set triggers bc/<buildconfig_name> --from-gitlab --remove
      # 移除来自于 GitLab 的 buildconfig 自动构建的触发器
      ```

    - 对于所有添加的 webhook，必须用一个名为 `WebHookSecretKey` 的 key 定义一个 `secret`，该值为调用 webhook 时提供的值。
    - 然后，webhook 定义必须引用这个 secret，该 secret 确保了 URL 的唯一性，防止其他人触发构建。
    - 该 key 的值（secret）将与 webhook 调用时提供的 secret 进行比较。
      > 💥 默认创建的 github webhook 不使用 WebHookSecretKey！

  - 3️⃣ **Configuration change trigger：**</br>
    buildconfig 中定义的构建用环境变量（`--build-env` 选项指定）可被注入于 build pod 中，若使用如下命令更改 buildconfig 中的环境变量，可手动触发新的构建：

    ```bash
    $ oc set env bc/<buildconfig_name> <env_var_key>=<env_var_value>
    # 设置 buildconfig 的构建用环境变量
    $ oc set env bc/<buildconfig_name> --list
    # 列出 buildconfig 中定义的环境变量
    $ oc start-build <build_name> -F 
    ```

#### 5.5.3 build 构建与 S2I 说明

- build 资源对象以 pod 的方式运行，即，应用源代码通过 S2I 注入构建镜像后运行的 pod，用于创建 buildconfig 中 S2I 构建新的应用镜像的环境，成功构建后将触发部署应用 pod。
- 根据一个 buildconfig 可进行 `多次 build 构建`，因此，在删除 build 时需指定 build 的名称。

  ```bash
  $ oc delete build/<build_name>
  ```
  
- S2I 或 Dockerfile/Containerfile 应用构建日志追踪：

  ```bash
  $ oc get buildconfig
  # 查看已存在的构建配置
    
  $ oc logs -f bc/<buildconfig_name> [-n <project>]
  # 查看构建配置过程日志
    
  $ oc get builds
  # 查看 build 的构建状态
    
  $ oc logs build/<build_name>
  # 查看指定构建的日志
    
  $ oc logs <builder_pod> [-n <project>]
  # 查看构建 pod 的日志
    
  $ oc logs -f dc/<deploymentconfig> [-n <project>]
  # 查看部署过程日志
  ```
  
- 默认情况下，oc get builds 保存最近 5 次的构建历史（无论是成功还是失败），可使用 `successfulBuildsHistoryLimit` 属性（默认值为 5）与 `failedBuildsHistoryLimit` 属性（默认值为 5）限制成功构建与失败构建的次数。
- 如下所示，构建 `PHP` 应用的 buildconfig 资源定义文件：

  ![buildconfig-pruning-builds](images/buildconfig-pruning-builds.jpg)
  
- build 构建按照时间排序，最老的构建首先被删除。
- 出于调试与故障排除目的，可调整 build 构建过程的构建日志级别，通过向 S2I 构建镜像运行的 `build pod` 中注入 `BUILD_LOGLEVEL` 环境变量来实现，也可通过更改 buildconfig 环境变量的形式来实现。
- 日志级别在 `0~5` 之间，0 为默认日志级别具有最少的日志信息，5 能显示最多的日志信息。
- 调整构建日志级别的方式：
  - 方式 1：使用 oc new-app 命令创建应用时指定 `--build-env BUILD_LOGLEVEL=<number>` 选项，即可调整构建日志级别。

    ![oc-new-app-build-loglevel](images/oc-new-app-build-loglevel.jpg)

  - 方式 2：对已有的 buildconfig 设置 BUILD_LOGLEVEL 环境变量，当触发重新构建时，可查看更详细的日志信息。

    ```bash
    $ oc set env bc/<buildconfig_name> BUILD_LOGLEVEL="4"
    ```

  - 如下所示，该环境变量在 `NodeJS` 应用的 buildconfig 资源中的定义：

    ![buildconfig-build-loglevel](images/buildconfig-build-loglevel.jpg)
  
- 在 OpenShift 中，S2I 可为集群提供完整的原生应用持续构建与部署的过程。
- 🚀 关于 `S2I` 的详细内容，请参考 [S2I 基本原理与应用构建部署示例](https://github.com/Alberthua-Perl/dockerfile-s2i-demo/tree/master/golang-s2i)。
- 常用于构建应用的语言包括但不限于 `HTML`、`PHP`、`Ruby`、`NodeJS`、`Java`、`Golang` 等，S2I 更多的使用于构建前端应用容器镜像，如各种编程语言构建的应用、`Apache HTTPD`、`Nginx` 等应用。
- buildconfig 支持 `post-commit build hook`：
  - post-commit build hook 功能来执行构建过程中测试应用的任务，仅仅用于验证构建的应用镜像，而不更改应用镜像本身。
  - 在推送构建的应用镜像至 OCP internal registry 中与启动新的应用部署之前，build pod 中运行 post-commit build hook 定义的命令或 shell 脚本可返回退出状态码（exit code），若返回非 0 状态码，说明构建失败且不推送镜像至 internal registry，新的应用部署也不被触发，若返回 0 状态码，说明构建成功并推送镜像至 internal registry，触发新的应用部署。
  - 可使用 `oc logs bc/<buildconfig_name>` 追踪 post-commit build hook 日志。
  - 使用 post-commit build hook 的常见场景：
    - 通过 HTTP API 将构建与外部应用程序集成
    - 验证应用的非功能性需求（non-functional requirement），如应用的性能、安全性、可用性或者兼容性。
    - 向开发团队发送电子邮件，通知他们新的构建。
  - 配置 post-commit build hook 的方法：
    - 使用简单的命令行模式
    - 使用 shell 脚本模式：
      使用 `/bin/sh -ic` 命令配合脚本来实现所有的功能，如参数扩展、重定向等，并且 build pod 必须可提供 `sh` 的 shell 解释器。

### 5.6 DeploymentConfig (dc) 与 Deploy 资源对象

- 中文名称：部署配置、部署
- 在 Kubernetes 1.0 中并不像现在如此方便可快速部署应用，而是需要繁复的手动配置才能满足要求，而在 OpenShift 3.0 中 Red Hat 开发了 `deploymentconfig`，以提供参数化部署输入、执行滚动部署、启用回滚至先前部署状态，以及通过触发器（`trigger`）以驱动自动部署等（buildconfig 构建配置完成后触发 deploymentconfig）。  
- 由于 buildconfig 中 imagestreamtag 的改变，deploymentconfig 或 deployment 中可探测到 imagestreamtag 的改变针对新构建应用镜像的自动重新部署。
  - 👉 OCP3 中 deploymentconfig 的 `imagestreamtag trigger`：

    ![ocp3-imagestreamtag-trigger-dc-deploy](images/ocp3-imagestreamtag-trigger-dc-deploy.jpg)

  - 👉 OCP4 中 deployment 的 `imagestreamtag trigger`：

    ![ocp4-imagestreamtag-trigger-deployment-deploy](images/ocp4-imagestreamtag-trigger-deployment-deploy.jpg)
  
- deploymentconfig 中的许多功能最终成为 `Kubernetes deployment` 功能集的一部分。
  > ✅ 目前 OCP4 同时兼容 deployment 资源与 deploymentconfig 资源。
- 使用 oc new-app 命令默认生成 `List` 资源定义文件，包含 deploymentconfig 的资源定义。
- 支持通过 webhook 与外部的持续集成（CI）与持续开发（CD）系统集成。
- deploy 资源对象以 pod 的方式运行。
- 该对象用于跟踪 deploymentconfig 生成新的 pod 的过程。
- 若新部署的 pod 无法正确运行，删除 deploy pod 后，将自动删除正在由 deploy pod 部署的其他 pod。
- 🔎 关于 deploymentconfig 资源 deprecated 状态的说明，可参考 [DeploymentConfig API is being deprecated in Red Hat OpenShift Container Platform 4.14](https://access.redhat.com/articles/7041372)。

### 💎 5.7 Deployment 资源对象

- OCP4 中建议使用 Deployment 以代替 DeploymentConfig，而此资源对象为保证兼容性依然得以保留。
- OCP4 中 deploymentconfig 集成 replication controller，该控制器支持基于等值类型的标签选择器（`equality-based selector`），而 deployment 中集成 `replicaset`，该控制器支持基于集合类型的标签选择器（`set-based selector`），两者均通过与 pod 的特定标签与 pod 进行关联，实现 pod 副本数的高可用。
- 🔎 可参考官方文档 [Understanding Deployment and DeploymentConfig objects](https://docs.openshift.com/container-platform/4.6/applications/deployments/what-deployments-are.html)

```bash
$ kubectl create deployment <deployment_name> --image <imagename> \
  -o yaml --save-config --dry-run=client > /path/to/file.yml
# 生成 deployment 模版：保留 kubectl.kubernetes.io/last-applied-configuration 注释，并保存客户端对象。

$ oc rollout restart deployment <deployment_name>
# 根据新更新的 deployment 重新部署相关资源
```

- 🎯 **`kubectl create` 与 `kubectl apply` 的区别：**

  | 序号 | kubectl create | kubectl apply |
  | ----- | ----- | ----- |
  | 1 | 首先删除集群中现有的所有资源，然后重新根据 yaml 文件（必须是完整的配置信息）生成新的资源对象。 | 根据 yaml 文件中包含的字段（yaml 文件可以只写需要改动的字段），直接升级集群中的现有资源对象。 |
  | 2 | yaml 文件必须是完整的配置字段内容 | yaml 文件可以不完整，只写需要的字段。 |
  | 3 | kubectl create 工作在 yaml 文件中的所有字段 | kubectl apply 只工作在 yaml 文件中的某些改动过的字段 |
  | 4 | 在没有改动 yaml 文件时，使用同一个 yaml 文件执行命令 kubectl replace，将不会成功（fail 掉），因为缺少相关改动信息。 | 在只改动了 yaml 文件中的某些声明时，而不是全部改动，可以使用 kubectl apply。 |

### 5.8 ReplicationController (rc) 与 ReplicaSet 资源对象

- 前者已集成至 deploymentconfig 中，而后者集成至 deployment 中。
- 该资源对象保证运行的 pod 的高可用，使其当前的数量趋近于 desired 数量。
- 若 pod 由于某些原因故障停止，该资源对象将根据配置的 pod 数量（replicas）重新部署 pod 保证不间断服务，此类 pod 为无状态应用居多。

### 5.9 Pod 资源对象

- pod 是 Kubernetes 集群与 OCP 集群中容器运行的原子单位（最小粒度）
- 单个 pod 中可以运行单个或多个容器，它们共享 `network namespace` 与 `volume`。
- 在 pod 中可存储临时数据（ephemeral storage），但在 pod 重启后将丢失全部数据，因此 pod 在某些场景下需使用永久存储（persistent storage）。
- 🔥 pod 中的 UIDs 与 GIDs 分配方式：
  - 使用 OpenShift 默认安全策略，常规集群用户（regular cluster users）不能为他们的容器选择 `USER` 或者 `UIDs`。当常规集群用户创建 pod，OpenShift 忽略容器镜像中的 `USER` 指令。取而代之的是，OpenShift 为容器内的用户从项目 `annotation` 中的识别范围中分配一个 `UID` 与一个补充 `GID`（supplemental GID）。用户的 GID 总是 `0`，这就意味着用户属于 root 组。容器进程可以写的任何文件或目录必须具有读和写的权限 (使用 `GID=0` 或具有 root 组)。虽然在容器中的用户属于 root 用户组，这个用户是一个非特权账户。
  - 与此相反的是，当集群管理员创建 pod，容器镜像中的 `USER` 指令可被处理。比如，如果容器镜像的 USER 指令被设置为 0，然后容器中的用户是 root (特权账户)，其 UID 为 0。使用特权账户执行容器具有安全风险。在容器内的特权账户对容器的主机系统具有未加限制地访问。未加限制地访问意味着容器能修改或删除系统文件，安装软件包，或对主机的其他危害。RedHat 建议使用 `rootless` 用户运行容器，或者使用仅具有必要特权的非特权用户来运行容器。

    ![openshift-project-uid-gid-assignment](images/openshift-project-uid-gid-assignment.png)

  - 如果两个容器的 UID 相同，那么在一个容器内的进程可能访问另一个容器中的资源与文件。
  - 依靠为每个项目分配一个可区别的 UIDs 与 GIDs 范围，OpenShift 确保在不同项目中的应用运行不使用相同的 UID 或 GID。
  - 当未定义安全上下文创建 pod 时，Kubernetes Pod 安全准入控制器（pod security admission controller）发出一个告警。而 OpenShift 使用安全上下文限制控制器（security context contraints controller）为 pod 安全提供安全默认值。
- pod 日志处理方式：
  - 容器化的应用应将其日志发送至标准输出（standard output），若容器化的应用将日志发送至日志文件，该方式与非容器化应用一致。
  - 保存于容器临时存储中的日志将随容器的销毁而丢失。
  - OpenShift 集群提供可选的基于 `EFK`（Elasticsearch、Fluentd、Kibana） 的日志子系统，该日志子系统提供了长期存储与检索 OpenShift 集群节点与应用日志的能力。
  - 应用应该充分利用 EFK 的优势，将其日志以标准输出的形式发送给 EFK，以达到收集与处理日志的能力。

    > 🤔 是否可以采用 Fluentd + Loki 的轻量级日志采集方案？
    >
    > 📝 Kubernetes 官方推荐的日志架构可参考 [此链接](https://kubernetes.io/zh/docs/concepts/cluster-administration/logging/)。

### 5.10 Label 资源对象

- 中文名称：标签
- 类型：基于等值类型的标签 + 基于集合类型的标签（OCP4 已支持）
- OCP 集群中的各种资源使用 label 标签进行匹配

### 🔥 5.11 Service (svc) 资源对象

#### 🚀 5.11.1 OCP3 & OCP4 的网络模型类型

- 1️⃣ 高耦合的 pod 内部容器间（container-to-container）的通信
- 2️⃣ pod 与 pod 间（pod-to-pod）的通信（同节点或跨节点）：`CNI plugins`
- 3️⃣ pod 与 service 间（pod-to-service）的通信：`iptables`、`ipvs`、`OpenFlow rules`、`Cillium eBPF`
- 4️⃣ 集群外部与 service 间（external-to-service）的通信：`openshift route`、`ingress`
- OCP4 集群提供 `Cluster Network Operator (CNO)` 配置集群网络，通过 CNO 加载与配置 CNI 插件：

  ```bash
  $ oc get deployment/network-operator -n openshift-network-operator
  # 查看 CNO 部署的 deployment 与 pod
  ```

  ![ocp4-network-dns-ingress-operator](images/ocp4-network-dns-ingress-operator.png)

- OCP4 中使用如下命令获取全局 pod 的子网与全局 service 的子网：

  ```bash
  $ oc get network.config.openshift.io cluster -o yaml
  $ oc get network.config/cluster -o yaml
  # 以上命令等效
  ```

  ![ocp4-pod-service-subnet](images/ocp4-pod-service-subnet.png)

#### 5.11.2 service 资源对象处理的场景

- 由于 pod 经常因某些故障而重启，每次重启后其 IP 地址都将改变，因此使用 service 将一个或一组相同的 pod 进行关联。
- service 为 pod 提供统一的入口 IP 地址，该入口地址默认为 service 的虚拟 IP 地址（`ClusterIP`）。
- service 提供反向代理与负载均衡的功能，默认以 `Round Robin` 轮询的方式将流量转发至 pod。  

#### 5.11.3 service 在 Kubernetes 中的实现方式

- service 提供三种代理模式：
  - `userspace`：
    - 早期的代理模式，由于性能较差该模式已废弃不再使用。
  - `iptables`：
    - OpenShift 当前默认的代理模式
    - 但是，该模式只能支持简单的负载均衡能力，若需实现复杂的负载均衡算法需引入其他方式。
  - `ipvs`：
    - 该模式自 `Kubernetes v1.11` 版本开始已达到 GA（一般可用性）水平，并在之后的版本中成为默认的 service 代理模式。
    - Kubernetes 既可使用 iptables 实现流量的转发规则，也可使用 ipvs 实现流量的负载均衡能力。
- service 资源由 `kube-proxy` 组件实现，其虚拟 IP 地址存在于每个节点的 iptables NAT 表中，使用 `iptables -t nat -nvL` 命令即可查看指定的 ClusterIP。
- 使用原生 `kube-proxy` 实现的 service 与自研的 service 解决方案的响应对比：

  ![service-performance](images/service-performance.jpg)

- 因此，目前开源社区使用 `eBPF` 技术为基础，开发的 `Cilium` CNI 插件可不使用 service 以实现其功能，在流量转发方面性能得到极大的提升。
- OCP3 中的 service 网段与 pod 网段的定义示意：

  ![ocp3-network-plugin](images/ocp3-network-plugin.jpg)

#### 5.11.4 service 的类型

- 4大类型：
  - 1️⃣ `ClusterIP`
  - 2️⃣ `NodePort`
  - 3️⃣ `LoadBalancer`：这种类型构建在 NodePort 类型之上，其通过 cloud provider 提供的负载均衡器将服务暴露到集群外部，因此 LoadBalancer 一样具有 NodePort 和 ClusterIP。
  - 4️⃣ `ExternalIPs`

![K8s-service-type](images/K8s-service-type.jpg)

- 💥 使用 NodePort 类型 service 的资源定义文件更改后再创建 ClusterIP 类型 service 时，需删除其中的 `spec.externalTrafficPolicy` 字段，否则创建失败！
  
  ![change-service-type-error](images/change-service-type-error.jpg)

#### 5.11.4 service 与 pod 的关联方式

service 通过 `selector` 与具有相同 `label` 的 pod 关联，将固定的 IP 地址与 pod 解耦，提高 pod 部署的灵活性，OCP 可根据 scheduler 调度器将 pod 部署至不同的 node 节点上，根据 ReplicationController (OCP3) 或 Deployment (OCP4) 部署相应副本数量的 pod，保证 pod 的服务高可用，此类 pod 应用一般为无状态类型服务。

![ocp-service-selector-match-pod-label](images/ocp-service-selector-match-pod-label.jpg)

> 💥 无论 OCP 集群使用 `ovs-subnet` 或 `ovs-multitenent` 类型的 SDN 插件，同一项目的 pod 间可直接通信，无需使用 service！

#### 5.11.5 service 与服务发现

- service 作为前端 pod 访问后端 pod 的入口点，实现服务发现。
- 服务发现的实现方式：
  - 1️⃣ service 环境变量：
    - 前端应用 pod 使用后端应用的 `service 环境变量` 来发现后端应用 pod
    - 对于项目内的每个 service，将自动定义环境变量，并注入到同一项目中的所有 pod 中。
    - service 环境变量的服务发现方式：
      - *svc_name*_SERVICE_HOST：service 的 IP 地址
      - *svc_name*_SERVICE_PORT：service 的 TCP 端口号

    > 💥 使用 service 环境变量实现服务发现时，必须先创建后端 service，再创建启动前端 pod，才能实现后端 service 环境变量的注入。

  - 2️⃣ SkyDNS（OCP4 中已淘汰）：
    - OCP3 中通过 `SkyDNS` 的 `SRV 记录` 实现前端应用对后端应用的服务发现。
    - `SkyDNS` 服务发现方式：
      - SkyDNS 进程集成于 OpenShift master 与 node 进程中，无需进一步额外配置。
      - SkyDNS 将每个 service 动态分配一个 `FQDN` 格式的 `SRV 记录`：*svc_name*.*project_name*.svc.cluster.local

    > ✅ 在 pod 中使用 DNS 查询来实现服务发现，可在 pod 启动后再查找相应的 service。

  - 3️⃣ CoreDNS：
    - OCP4 中通过名为 dns 的 `ClusterOperator` 部署 `CoreDNS`，CoreDNS 是 CNCF 毕业的成熟的云原生领域域名解析系统。
    - 集群中各个 pod 可通过其 `/etc/resolv.conf` 中的 nameserver 实现服务发现，其中 nameserver 的 IP 地址为 CoreDNS pod 的 `service IP`。
    - 每个 pod 都可通过 CoreDNS 查询动态分配的 service 与 `FQDN` 格式的 `SRV 记录`，每条 SRV 记录中的 FQDN 对应一条 A 记录，而 A 记录中的 IP 地址即为一组 pod 的 service IP。其中 SRV 记录的 FQDN 格式为 *svc_name*.*project_name*.svc.cluster.local。通过这种方式不同 pod 间可发现其他 pod 中的服务。以下示例为 web-server pod 可通过 DNS 查询发现 gcw 应用的 service IP：

    ![ocp4-coredns-service-discovery-demo](images/ocp4-coredns-service-discovery-demo.png)

    ![coredns-pod-resolv-2](images/coredns-pod-resolv-2.png)

#### 5.11.6 service 与实现跨节点通信的 CNI 插件的关系

- service 的虚拟 IP 地址与 pod 的 IP 地址面向 OCP 集群内部，OCP 集群外部不可访问，若使外部能够访问，需要使用 `route` 资源进行暴露。
- OCP3 中建议将 service 整合入 DeploymentConfig 中，而 Kubernetes 中建议将 service 定义在 Deployment 中。
- 💥 service 从逻辑层面解决了 service 与 pod 间的网络通信问题，而 pod 与 pod 间的跨节点间通信必须使用 CNI 插件加以解决。
- OCP3 中默认使用 `OVS` 作为 SDN 插件，其共有 3 种工作类型，包括 `ovs-subnet`、`ovs-multitenent` 与 `ovs-networkpolicy`。
- 💎 补充：
  - OCP4 中依然默认使用 `openshift-sdn` 插件（OVS 插件）的 `ovs-networkpolicy`，以实现更加细粒度的网络资源隔离，可基于 `namespace` 级别以及 `pod` 级别。
  - ovs-subnet 将所有项目的 pod 置于扁平化（flat）网络中，彼此之间均能通信。
  - ovs-multitenant 使用 `VNID` 实现不同项目间的 pod 二层隔离，其使用 VXLAN 隧道打通 pod 间在各节点之间的联系。
  - OCP4 也可以使用 `OVN-Kubernetes` 作为 SDN 插件，但需要在集群规划与部署前确定具体使用哪个 SDN 插件，一旦部署完成不可更改，并且可同时使用 `Multus CNI` 调用其他 CNI 插件使单 pod 同时具备 2 个网口，以同时满足集群的网络流量与需要较高网络性能的业务流量。
    - 可参考官方文档 [Understanding multiple networks](https://docs.openshift.com/container-platform/4.6/networking/multiple_networks/understanding-multiple-networks.html)

- 🚀 OCP3 OVS 网络拓扑示意：

  ![ocp3-ovs-1](images/ocp3-ovs-1.png)

  ![ocp3-ovs-2](images/ocp3-ovs-2.png)
  
- OCP3 同一节点上 pod 间的通信示意：

  ![ocp3-ovs-3](images/ocp3-ovs-3.png)

- 🚀 OCP3 OVS 流表分析示意：

  ![ocp3-ovs-openflow-1](images/ocp3-ovs-openflow-1.jpg)
  
- 🚀 OCP3 中访问使用 `NodePort` service 类型的 pod 跨节点流量分析：
  以下为 iptables NAT 表与 OVS 流表部分条目，并且 OCP3 集群使用 ovs-subnet SDN 插件，无 `networkpolicy` 策略存在。

  ![NodePort-service-iptables-nat-ovs-analyze](images/NodePort-service-iptables-nat-ovs-analyze.jpg)

### 5.12 Route 资源对象
  
- 中文名称：路由
- 可借助 service 实现 OCP 集群内前后端 pod 间的通信，而 OCP 集群外部对内部 pod 的访问默认需要使用 default 项目的 `router pod` 来实现。
- OCP 集群外部通过泛域名解析（wildcard）指向特定 `infra` 节点（前端可加负载均衡与高可用），router pod 需指定在该 infra 节点上运行，其 IP 地址与 infra 节点绑定。
- router pod 以 `HAProxy` 的方式实现。
- 默认情况下，OCP 集群中的所有项目的 route 规则都将注入到 default 项目中的 router pod 中。
- router pod 可将外部流量直接转发到 OCP 集群中的应用 pod 上。
- router pod 只使用相应的 service 在 `etcd` 中查询对应 pod 的 `endpoint`，直接转发流量至 pod。
- router 路由原理架构示例：

  ![ocp3-route-infra](images/ocp3-route-infra.jpg)

- 💎 补充：
  - OCP4 中名为 `ingress` 的 ClusterOperator 提供 `ingress controller`。Kubernetes 中的 Ingress 包含两个重要组件，分别为 ingress controller 与 ingress，而在 OpenShift 中 ingress controller 对应为 HAProxy router pod，而 ingress 对应为 route。

### 5.13 PersistentVolume (pv) 资源对象

- 中文名称：持久卷
- 持久卷属于 OCP 集群资源，必须使用 `system:admin` 管理员用户或具有 `cluster-role` 角色的用户进行管理、创建与删除。
- pv 资源定义中默认使用 `NFS` 服务端提供 NFS 存储，可为 pod 提供永久存储。
- pv 的访问模式：

  > 注意：NFS 均支持以下三种模式

  ![pv-access-mode](images/pv-access-mode.jpg)
  
- 持久卷存储等级（persistent volume storage class）定义后端存储的类型与等级，由 `storageClassName` 属性定义。

  ![storageClassName](images/storageClassName.jpg)

- pv 回收策略：`PersistentVolume.spec.persistentVolumeReclaimPolicy` 属性

  ![pv-recycle-policy](images/pv-recycle-policy.jpg)

  - `retain`（默认方式）：pv 中的数据将保留，管理员需手动处理该卷。</br>
    若需清除 pv 中的数据，可执行以下操作：</br>
    👉 手动删除 pv。</br>
    👉 手动清理后端存储卷中数据，以免数据被重用。</br>
    👉 手动删除后端存储卷。</br>
    👉 创建新的 pv 以重用之前的数据。</br>
  - `recycle`：</br>
    👉 通过执行 rm -rf 命令删除卷上所有数据，使得卷可被新 pvc 使用。</br>
    👉 目前只有 NFS 与 hostPath 支持该回收模式。</br>

    > 💥 pv 与 pvc 可绑定成功，但不代表 pv 使用的后端存储可正常使用！

### 5.14 PersistentVolumeClaim (pvc) 资源对象

- 中文名称：持久卷声明  
- 持久卷声明属于项目（或命名空间）资源，使用项目用户即可管理 pvc。  
- pv 与 pvc 通过访问模式（`accessMode`）与存储大小（`storage`）进行匹配。  
- 若存在相同访问模式与存储大小的 pv，pvc 在使用时将随机使用 pv，但第一次匹配成功使用后将持久使用该 pv。  
- pv 与 pvc 的使用方法：
  - NFS 存储卷共享（属组与权限）
  - pv 资源定义与创建
  - pvc 资源定义与创建
- 更改 dc 或 pod 资源定义的 `persistentVolumeClaim.claimName` 属性值以创建 pod 资源
- OCP 部署过程中定义 NFS 存储作为 OCP internal registry 的存储后端：

  ![ocp3-internal-registry-pvc-1](images/ocp3-internal-registry-pvc-1.jpg)
  
- OCP 集群 default 项目中定义的 pv 与 pvc 的关系：

  ![ocp3-internal-registry-pvc-2](images/ocp3-internal-registry-pvc-2.jpg)

  ![ocp3-internal-registry-pvc-3](images/ocp3-internal-registry-pvc-3.jpg)

### 5.15 Secret 资源对象

#### 5.15.1 基础概念

- 该资源对象保存 OCP 集群中的敏感数据，如密码、token 凭据等，将敏感数据与 pod 解耦。
- 数据使用 `base64` 编码（encode）存储在 secret 资源对象中。
- secret 资源对象可在命名空间中共享。
- 当来自 secret 的数据被注入到容器中时，可采用以下两种方式实现：
  - 1️⃣ 作为环境变量（env）注入到容器中
  - 2️⃣ 作为卷（volume）挂载：指定容器内挂载路径，容器映射目录中的各个文件名为 secret 的各个 key，文件的内容为对应 key 的值。
- secret 的类型：

  ![k8s-ocp-secret-type](images/k8s-ocp-secret-type.png)

#### 5.15.2 创建与使用 secret 资源对象

若使用 Web 控制台创建 secret 资源对象，由于使用 `base64` 编码该资源对象，需对其解码才能注入 secret 的值，而使用 CLI 创建的方式如下所示：

```bash
$ oc create secret --help
# 查看创建 secret 的方法
```

![oc-create-secret](images/oc-create-secret.jpg)

```bash
$ oc create secret generic <secret_name> \
  --from-literal='<key1>'='<value1>' ... --from-literal='<keyN>'='<valueN>'
# 指定 key 与 value 的值创建 secret 资源，使敏感数据与 pod 解耦。
    
$ oc create secret generic <secret_name> \
  --from-file <key1>=/path/to/file1 ... --from-file <keyN>=/path/to/fileN
# 指定 key 与文件的内容创建 secret 资源

$ oc create secret tls <secret_name> \
  --cert /path/to/certification-file --key /path/to/certification-key
# 指定 CA 证书文件与 CA 私钥文件创建 secret
    
### 示例 ###
$ oc create secret generic mysql \
  --from-literal='database-user'='mysql' \
  --from-literal='database-password'='redhat' \
  --from-literal='database-root-password'='do285-admin'
# 创建 secret 资源以存储 MySQL 相关的用户名与密码，该 secret 可被其他资源所引用。

$ oc create secret generic <secret_name> \
  --from-file .dockerconfigjson=<access_token_file> \
  --type kubernetes.io/dockerconfigjson
# 使用登录用 token 创建可访问外部私有镜像仓库的 secret 资源

$ oc create secret generic quayio \
  --from-file .dockerconfigjson=/run/user/1000/containers/auth.json
  --type kubernetes.io/dockerconfigjson
# 使用 podman 登录 Quay 的认证 token 创建 secret 资源，可使用该资源链接至 serviceaccount 以拉取私有镜像。

### 示例：secret 通过链接（link）与 serviceaccount 关联 ###
$ oc secrets link <serviceaccount_name> <secret_name> --for=pull
# 将拉取外部私有镜像所需的 secret（包含拉取所需的 token）链接至项目中指定的
# serviceaccount（默认为 default），该 serviceaccount 在创建 pod 时即可
# 拉取镜像，否则 pod 创建失败。
```

💎 补充：OCP3 中若应用已部署，但需将创建的 secret 资源对象注入应用 pod 中，可参考如下命令，而在 OCP4 中使用 `deployment` 资源对象代替 `deploymentconfig` 资源对象即可：

```bash
$ oc create secret generic myappfilesec \
  --from-file ~/DO288-apps/app-config/myapp.sec
# 使用指定的文件创建 secret 资源，其中资源定义的 data 字段中 key 为文件的名称，value 为文件中的内容。
    
$ oc set volume dc/myapp \
  --add --type secret \
  --secret-name myappfilesec \
  --mount-path /opt/app-root/secure \
  --name myappsec-vol
# OCP3 中以卷挂载的方式将 secret 资源挂载至容器的 /opt/app-root/secure/ 目录中，
# 由于 deploymentconfig 中 ConfigChange 将触发应用 pod 的重新部署

$ oc set volume deployment/<name> \
  --add --type secret \
  --secret-name <secret_name> \
  --mount-path /path/to/directory
# OCP4 中以卷挂载的方式将 secret 资源挂载至容器的指定挂载点上（OCP3 与 OCP4 的区别在于 dc 与 deployment）
```

除了以上 CLI 方式外，还可使用 YAML 文件定义的方式创建 secret 资源对象，但在 YAML 文件中标准的 `data` 字段需使用 `base64` 编码的值，因此，该标准方法不能用于 `template` 模板中，可使用 `stringData` 字段替换 data 字段，并且使用明文的值替换 base64 编码的值，但是该替代语法永远不会保存在 OpenShift 的 `etcd` 数据库中。

#### 5.15.3 提取与更新 secret

以下示例为 `HTPasswd` 作为 `Identity Provider` 的场景中，通过提取的文件更新集群中的用户：

```bash
$ oc extract secret/htpasswd-secret -n openshift-config --to /tmp --confirm
# 提取 openshift-config 项目中名为 htpasswd-secret 的 secret 至 /tmp 目录中以文件的形式保存
# 注意：提取的文件中存储用户的用户名与加密的哈希

$ oc set data secret/htpasswd-secret -n openshift-config --from htpasswd=/tmp/htpasswd
# 使用更新后 htpasswd 文件设置 secret
# 注意：使用 htpasswd 命令更新 /tmp/htpasswd 文件中的用户信息

$ oc get pods -w -n openshift-authentication
# authentication ClusterOperator (OAuth operator) 在上述 secret 更新后将重新部署名为 oauth-openshift 的 pod
```

#### 👨‍💻 **5.15.4 示例：OCP 4.6 中使用 secret 拉取外部私有容器镜像**

由于需在 OpenShift 集群中使用 Quay.io 中的私有镜像 `quay.io/alberthua/ubi-sleep:1.0`，若不使用登录用户认证将导致应用部署失败，如下所示：

![oc-new-app-fail-for-no-secret](images/oc-new-app-fail-for-no-secret.jpg)

因此，在项目中创建 secret 并将其链接至名为 default 的 `service account`，实则是为 service account 添加对应的 `imagePullSecrets`，如下所示：

![oc-secretes-link-help](images/oc-secretes-link-help.jpg)

```bash
$ echo ${XDG_RUNTIME_DIR}
  /run/user/1000
# 默认的容器镜像仓库认证文件路径前缀
    
$ oc create secret generic quayio \
  --from-file .dockerconfigjson=${XDG_RUNTIME_DIR}/containers/auth.json \
  --type kubernetes.io/dockerconfigjson
# 使用 podman 登录容器镜像仓库的 token 创建 secret
    
$ oc secrets link default quayio --for=pull
    
$ oc new-app --name sleep \
  --docker-image=quay.io/${RHT_OCP4_QUAY_USER}/ubi-sleep:1.0
# 成功拉取镜像并部署
```

![oc-get-serviceaccounts-default-imagepullsecrets](images/oc-get-serviceaccounts-default-imagepullsecrets.jpg)

![oc-get-secret-quayio](images/oc-get-secret-quayio.jpg)

secret 中通过 base64 编码的数据可通过 `echo <base64_string> | base64 -d` 命令进行解码查看原始数据。

#### 5.15.5 serviceaccount 与 secret 的关联

每个项目中 default serviceaccount（sa）与 secret 的关联：

- 必须指定 sa 以运行 pod，若未指定将使用 default sa。
- default sa 中包含两个 secret，并且每个 secret 分别具有一个 token。
- token分别用于：
  - 👉 pod 与 apiserver 间的认证通信
  - 👉 从 OCP internal registry 中拉取已构建的应用镜像

  ![service-account-secret-1](images/service-account-secret-1.jpg)

- pod 运行后将 secret 挂载至 `/var/run/secrets/kubernetes.io/serviceaccount/` 目录中。
- 该目录中的 token 即为 sa 中的 secret 对应的 token。

  ![service-account-secret-2](images/service-account-secret-2.jpg)

  ![service-account-secret-3](images/service-account-secret-3.jpg)

### 5.16 ConfigureMap (cm) 资源对象

#### 5.16.1 基础概念

- 该资源对象类似于 secret 资源对象，但它们存储的是不敏感的数据。
- configmap 资源对象可用于存储细粒度（`fine-grained`）信息，如独立的属性，或粗粒度（`coarse-grained`）信息，如整个配置文件和 JSON 数据。
- 可使用 OpenShift CLI 或 Web 控制台创建 configmap 与 secret 资源，可在 pod 规范和 OpenShift 中自动引用它们。
- secret 资源对象与 configmap 资源对象的特点：
  - 均可以独立地被定义（definition）与被引用（referenced）
  - 出于安全原因，为这些资源挂载的卷由临时文件存储（tmpfs）支持，不存储在节点上。
  - 它们均作用于所在的命令空间（namespace）

#### 5.16.2 创建与使用 configmap 资源

```bash
$ oc create configmap --help
# 查看创建 configmap 的多种方式
```

![oc-configmap-create](images/oc-configmap-create.jpg)

```bash
$ oc create configmap <configmap_name> \
  --from-literal='<key1>'='<value1>' ... --from-literal='<keyN>'='<valueN>'
# 创建 configmap 资源，并可使用于 deploymentconfig 中。
    
### 示例 ###
$ oc create configmap myappconf \
  --from-literal APP_MSG="Test Message"
```

```bash
$ oc set env dc/<deploymentconfig_name> --from=configmap/<configmap_name>
# OCP3 中将 configmap 资源定义的配置以环境变量的方式通过 deploymentconfig 
# 注入至应用 pod 中
    
### 示例 ###
$ oc set env dc/myapp --from=configmap/myappconf
# OCP3 中通过 deploymentconfig 向已部署的应用 pod 中注入 configmap，在 pod 中
# 以环境变量的方式存在。
```

```bash
$ oc rollout latest dc/<deploymentconfig_name>
# OCP3 中 dc 将根据 REVISION 中的版本更新至最新版本，pod 将重新部署至最新版本。
    
$ oc rollback 
# OCP3 中 oc rollout/rollback 都是针对 dc 来实现
```
  
- 如下所示，由于注入 configmap 与更改 dc 配置导致两次触发 dc 部署全新的 pod：

  ![configmap-trigger-dc](images/configmap-trigger-dc.jpg)
  
- 🚀 将 secret 与 configmap 资源注入应用 pod 的方式：
  - OpenShift 将其作为环境变量（`environment variables`）注入到容器中，在容器中以环境变量的形式存在。
  - OpenShift 通过挂载卷（`volume`）的方式将其注入到容器中，在容器中以挂载卷的形式存在。

    > 还可将正在运行的应用程序的部署配置（deploymentconfig）更改为引用 configmap 资源或 secret 资源，然后 OpenShift 自动重新部署应用程序并使数据对容器可用。

- 💎 补充：
  OCP4 中将 secret 与 configmap 资源对象注入应用 pod 的方式：

  ![oc-set-env-or-volume](images/oc-set-env-or-volume.jpg)

  使用卷挂载的方式注入 secret 或 configmap 资源对象中的应用数据时，需指定 `-m` 选项对应的 pod 中挂载的路径。  
- 选取应用 pod 环境变量或卷挂载方式的参考：
  - 若应用只有少数可以从 `环境变量` 中读取或通过命令行传递的简单配置变量，那么可以使用环境变量从 configmap 和 secret 资源对象中注入数据，环境变量是注入数据的首选方法。
  - 另一方面，若应用有大量的配置变量，或正在迁移大量使用配置文件的遗留应用，则使用 `卷挂载` 方法，而不是为每个配置变量创建一个环境变量。
  - 如，若应用需要从本地节点文件系统上的特定位置获得一个或多个配置文件，则应该使用配置文件创建 secret 或 configmap，并将它们挂载到容器临时文件系统中应用所需的路径下。

- OCP 中特有的资源对象：BuildConfig、DeploymentConfig、Route、Template
- 外部访问 OCP 集群内应用的方式：NodePort、Route、Ingress（OCP4 已支持）、port-forward
- OCP 资源之间的关系与工作流程：
  
  ![ocp3-resource-workflow](images/ocp3-resource-workflow.jpg)

## 🧪 OpenShift 资源对象使用

- 常规操作命令：
  
  ```bash
  ### 获取资源对象状态 ###
  $ oc get nodes
  # 查看节点的概要信息（system:admin 用户或具有 cluster-admin 角色的用户执行）

  $ oc get pods --selector <key>=<value>
  # 根据 label 标签筛选指定的 pod

  $ oc get pod <pod_name> -n <project> | yq r - 'status.podIP'
  # 使用 yq 工具解析 pod 被分配的 IP 地址
  # 注意：新版本的 yq 工具与老版本存在兼容性问题！
  # 下载链接：https://mikefarah.gitbook.io/yq/ 与 https://kislyuk.github.io/yq/，两者不兼容！

  $ oc get pods \
    -o custom-columns=NameSpace:"metadata.namespace",\
    PodName:"metadata.name",\
    ContainerName:"spec.containers[].name",\
    Phase:"status.phase",\
    IP:"status.podIP",\
    HostIP:"status.hostIP",\
    Ports:"spec.containers[].ports[].containerPort"
  # -o custom-columns 选项指定输出格式

  $ oc get pods \
    -o jsonpath='{range .items[]}{"Pod Name: "}{.metadata.name}
    {"IP: "}{.status.podIP}
    {"Ports: "}{.spec.containers[].ports[].containerPort}{"\n"}{end}'
  # 使用 JSONPath 表达式指定输出格式

  $ oc get pods \
    -o go-template='{{range .items}}{{.metadata.name}}{{"\n"}}{{end}}'
  # 使用 Go 模版指定输出格式  
  
  $ oc get all [-n <project>]
  # 查看项目中所创建的所有资源的重要信息
  # 可指定项目名称，若不指定，则默认使用所在的项目。
  
  $ oc get <resource_type> <resource_name> -o [yaml|json] [-n <project>]
  # 查看指定资源对象的详细信息
  
  $ oc describe <resource_type> <resource_name> [-n <project>]
  # 查看指定资源的详细信息
  
  ### 创建与编辑资源对象 ###
  $ oc create -f <resource_defination_file>.json [-n <project>]
  # 以命令式 API 使用修改的资源定义文件创建新的资源
  # oc create 命令常与 oc export 命令一起使用
  
  $ oc edit <resource_type> <resource_name> [-n <project>]
  # 启用 vi 缓冲区以编辑指定资源的资源定义文件，编辑后即时生效。
  
  ### 删除资源对象 ###
  $ oc delete project <project>
  # 删除项目及其所有资源
  
  $ oc delete all --labels=<label>
  # 删除项目中所有相应标签的资源
  # 可在创建各项资源时添加标签，便于删除相应资源。

  $ oc delete pod <pod> --grace-period=<seconds>
  # pod 支持优雅终止，即在 Kubernetes 强制终止 pod 之前，pod 首先尝试终止它的进程。
  # --grace-period 选项指定 pod 被 Kubernetes 强制终止前的时间间隔

  $ oc delete pod <pod> [--grace-period=1|--now]
  # 立即删除指定的 pod

  $ oc delete pod <pod> --force
  # 强制删除指定的 pod
  # 注意：
  #   1. 若强制删除指定的 pod，Kubernetes 不等待 pod 中进程终止的确认，这将保留 pod 中的进程运行直至所在节点侦测到进程已被删除。
  #   2. 因此，强制删除 pod 可能导致不一致或数据丢失。
  ```

- 常用资源调试命令：
  
  ```bash
  $ oc exec <pod> [-c <container>] [-n <project>] -- <command> arg1 arg2 ... argN
  # 直接在 pod 中的容器内执行命令并返回结果
  # 💥 注意：
  #   1. 若忽略容器名称，Kubernetes 使用 pod 中的 `kubectl.kubernetes.io/default-container: <value>` 注释来选择容器。
  #   2. 否则，oc exec 子命令默认进入 pod 中的第一个容器执行命令，若 pod 中运行多容器，可使用 `-c, --container= ` 选项指定容器。
  
  $ oc exec <pod> [-c <container>] [-n <project>] -it -- /bin/bash
  # 以交互模式进入 pod (的指定容器) 运行环境中

  $ oc attach <pod> [-c <container>] [-n <project>] -it
  # 以交互模式进入 pod (的指定容器) 运行环境中
  
  $ oc port-forward <pod> <localhost_port>:<pod_port> [-n <project>]
  # 将本地节点的端口映射至远程 pod 的端口，不局限于 80 与 443 端口，可供开发人员使用调试。
  # 本地节点可以为 OCP 集群外节点，该方法提供了从 OCP 集群外访问 pod 的方式。
  
  $ oc rsh <pod> [-n <project>] bash -c '<command>' 
  # 使用远程 shell 会话在 pod 中运行命令，pod 中必须具有 shell 运行环境。
  
  $ oc rsh -t <pod> [-n <project>]
  # 使用远程 shell 会话交互方式进入 pod 运行环境
  # pod 中必须具有 shell 运行环境
  
  $ oc cp <path_of_file> <pod>:<path_of_file> [-n <project>]
  # 拷贝本地文件至 pod 中
  # 本地文件路径与 pod 中的文件路径都必须为文件名称，并且容器镜像中必须具有 tar 命令。 
  # 若不存在 tar 命令，oc cp 命令执行失败！
  # 
  # 拷贝本地文件至 pod 中，也可将 pod 中的文件拷贝至本地，如下所示：
  #   $ oc cp /home/developer/quote.sql quotesdb-1-fzrgd:/tmp/quote.sql 
  #   $ oc cp quotesdb-1-fzrgd:/tmp/quote.sql /home/developer/quote.sql
  # 
  # 使用场景：可用于将 pod 中的应用临时日志拷贝至本地节点的目标文件中

  $ oc volume pod <pod> [-n <project>]
  # 查看 pod 中容器的挂载点与 pvc 的对应关系
  
  $ oc volume dc <deploymentconfig> [-n <project>]
  # 查看部署配置中的 volume 信息（pvc）  
  ```

  ![oc-exec-it-diff](images/oc-exec-it-diff.png)

## OpenShift 用户与访问控制

### OpenShift 用户认证（Authentication）

- OCP 中的用户与组相关的资源：
  - 用户（User）：使用 `oc get users.user.openshift.io` 命令获取集群中的所有用户
  - 身份（Identity）：使用 `oc get identities.user.openshift.io` 命令获取集群中的身份信息

  ![openshift-user-identity-demo](images/openshift-user-identity-demo.png)

  - 服务账户（Service Account）
  - 组（Group）
  - 角色（Role）
- 认证的 API 请求：
  - 当用户向 API 发起请求，该 API 将用户与请求相关联。成功完成认证后，授权层既能接受也能拒绝该 API 请求。授权层使用基于角色的访问控制 (RBAC) 策略来决定用户的权限。
  - OpenShift API 处理认证请求的两种方式：
    - OAuth 访问 tokens
    - X.509 客户端证书
- 认证 Operator（Authentication Operator）：OCP 提供认证 Operator，它运行一个 OAuth server (openshift-authentication 项目中的 `oauth-server`)。当用户向 API 发起认证，OAuth server 为用户提供 OAuth 访问 tokens。身份提供者必须被配置，并且对 OAuth server 是可用的。OAuth server 使用一个身份提供者来验证请求者的身份。该服务器使用身份和解用户，并且为用户创建 OAuth 访问 token。在用户成功登录集群后 OpenShift 自动创建身份 (identity) 与用户 (user) 资源。

> 注意：身份作为用户与 OAuth 访问 token 的中间层，它可对接集群外部身份提供者。

- 身份提供者（Identity Providers）：
  - HTPasswd
  - Keystone v3
  - LDAP
  - GitHub or GitHub Enterprise
  - OpenID Connect
- 新安装的 OpenShift 集群使用集群管理员权限提供两种方法来认证 API 请求：
  - 1️⃣ 使用 `kubeconfig` 文件：已嵌入了永不过期的 `X.509` 客户端证书

    ```bash
    ### 方法1 ###
    $ export KUBECONFIG=/path/to/kubeconfig
    # 使用 KUBECONFIG 环境变量指定文件
    $ oc get nodes

    ### 方法2 ###
    $ oc --kubeconfig /path/to/kubeconfig get nodes
    ```

  - 2️⃣ 使用 `kubeadmin` 虚拟用户进行认证：成功认证后获取 OAuth 访问 token

    ```bash
    $ oc login -u <user_with_clusteradmin_role> -p <password> https://<apiserver_url>:<port>
    # 使用具有集群管理员权限的用户（默认为 kubeadmin）登录集群
    # 注意：若 $HOME/.kube/config 文件不存在，那么在执行该命令后将在此目录中生成该文件，其中包含集群信息、上下文信息与非集群管理员角色用户的 token 信息等。
    ```

    ![home-kube-config-demo](images/home-kube-config-demo.png)

    kubeadmin 集群管理员用户的密码保存在 `kube-system` 命名空间中名为 `kubeadmin` 的 `secret` 中。因此，可使用以下方法验证密码的准确性：

    ```bash
    $ SECRET_DATA=$(oc get secret kubeadmin -n kube-system -o jsonpath='{range .items[]}{.data.kubeadmin}{"\n"}')
    # 返回以 base64 编码的 kubeadmin 集群管理员用户的加密密码
    # 注意：此密码不是明文的密码，而是通过 bcrypt 加密算法加密返回的哈希值，可根据明文密码与该哈希值进行验证。

    $ echo $SECRET_DATA | base64 -d; echo
    # 解开 base64 编码获取 bcrypt 加密过的哈希值，该哈希值可用如下程序验证。运行此程序可验证 kubeadmin 用户的密码。
    ```

    ```python
    ### file: verify_kube_passwd.py
    #!/usr/bin/env python3

    import bcrypt

    hashed_password = b'<hashed_password_from_secret>'  # 转换字符串类型为字节类型
    password_to_check = b'<kubeadmin_password>'

    if bcrypt.checkpw(password_to_check, hashed_password):
        print("kubeadmin password matched!")
    else:
        print("kubeadmin password NOT matched!")
    ```

- ✨ 多个 `kubeconfig` 配置文件的说明：
  - `$HOME/.kube/config` 文件：
    - 该 kubeconfig 文件位于集群用户的主目录下，通常是在开发者或者管理员的本地机器上。
    - 它用于用户的 `kubectl` 命令行工具与 Kubernetes 集群 API 服务器进行通信，进行资源的管理和操作。
    - 用户可以手动编辑这个文件来切换不同的 Kubernetes 集群或者更新访问凭证。
    - `$HOME/.kube/config` 文件中的凭证通常具有较为受限的权限，取决于用户的角色和权限设置。
    - 该文件是用户与 Kubernetes 集群交互的主要方式，可以通过 `kubectl config view` 命令来查看当前的配置。

    > 注意：OCP4 中若使用虚拟用户 token 的方式登录集群的话，$HOME/.kube/config 文件可不存在。当虚拟用户登录集群后将自动生成此文件。但此文件不包含集群认证证书，而包含虚拟用户的认证 token，与此文件的常规形式不同！

  - `/etc/kubernetes/kubeconfig` 文件：
    - 该 kubeconfig 文件通常存在于 Kubernetes 集群的控制平面节点（如 master 节点）上。
    - 它用于集群组件之间的通信，比如 `kube-apiserver`、`kube-controller-manager`、`kube-scheduler` 和 `etcd` 之间的认证和授权。
    - 该文件通常由集群安装工具在初始化集群时自动生成，并且不应该被手动编辑。
    - 该文件中的凭证通常具有较高的权限，因为它需要访问集群的所有资源以进行管理和调度操作。
    - 该文件的路径可能会根据集群的安装方式和配置有所不同。
  - `/etc/kubernetes/kubeconfig` 主要用于集群内部组件的通信，而 `$HOME/.kube/config` 用于用户与集群的交互。

### OpenShift 用户授权（Authorized）

- 授权的过程由规则（rules）、角色（roles）与绑定（bindings）管理：
  - 规则（Rule）：允许对对象与组的行为
  - 角色（Role）：规则的集合。用户与组能与多个角色关联。
  - 绑定（Binding）：分配角色至用户与组。
- 基于角色的访问控制（`Role-based Access Control`, `RBAC`）：
  - OpenShift 3.0 的发布已提供了基于角色的访问控制（RBAC），而 `Kubernetes 1.6` 版本才提供该功能。
  - 用户与组通过绑定（binding）与角色（roles）相关联
- RBAC 作用域（RBAC Scope）：
  - 集群角色（Cluster Role）：具有此角色水平的用户或组能管理 OpenShift 集群
  - 项目角色（Project Role）：具有此角色水平的用户或组仅仅能管理项目水平的资源
- OCP 中的默认角色：
  - `admin`：具有此类角色的用户能管理所有项目资源，包括为其他用户提权以访问项目。
  - `basic-user`：具有此类角色的用户对项目有读取访问权限。
  - `cluster-admin`：具有此类角色的用户对集群资源具有超级用户的访问权限。这些用户能在集群上执行任何动作，对所有项目有完全的控住权。
  - `cluster-status`：具有此类角色的用户能获取集群状态信息。
  - 🧪 `edit`：具有此类角色的用户在项目中能创建、更改和删除常规应用资源，如 services 与 deployments。这些用户不能管理如 limitranges 与 quotas 等资源，并且不能管理对项目的访问权限。开发者用户常使用此类角色。
  - `self-provisioner`：具有此类角色的用户能创建项目。它是一种集群角色，而不是项目角色。
  - `view`：具有此类角色的用户能查看项目资源，但不能修改项目资源。
- OCP 中用户分类：
  - 普通用户（regular user）
  - 系统用户（system user）
  - 服务账户（service account）：
    - 服务账户是和项目相关的系统用户，是 Kubernetes 资源。工作负载能使用此系统账户来调用 Kubernetes APIs。
    - 默认情况下，服务账户没有角色。授予角色给服务账户来启用工作负载以使用指定的 APIs。
    - 服务账户代表在 pod 中运行应用的一种身份。
    - ✨ 为了授权应用访问 Kubernetes API，可执行以下内容：
      - 创建应用服务账户
      - 授权服务账户访问 Kubernetes API
      - 分配服务账户至应用 pod
    - 如果 pod 定义未指定服务账户，那么 pod 使用 `default` 服务账户。OpenShift 不为 default 服务账户授权权限。
    - 💥 不推荐为 default 服务账户授权额外的权限，因为这将为项目中的所有 pod 授权那些额外的权限，这可能不是有意的。  
- 若根据用户访问不同级别资源的权限划分，可分为：
  - 集群管理员（cluster administrator）：集群的最高权限管理员
  - 项目管理员（project administrator）：项目的最高权限管理员
  - 开发者（developer）：
    - 管理项目资源的子集
    - 资源的子集包括：buildconfig, deploymentconfig, pvc, service, secret, route
    - 该类型的用户不能为其他用户对资源进行提权，也不能管理项目级别（project-level）的资源。
- 👉 没有身份验证或身份验证无效的 API 请求由匿名系统用户（anonymous system user）作为请求进行身份验证。
- 👉 身份验证成功后，策略确定授权用户执行的操作。
- 用户与组可同时绑定一个或多个本地项目角色与集群角色。

- RBAC 常用命令：

  ```bash
  ### 普通用户 ###
  $ oc adm policy add-cluster-role-to-user cluster-admin admin
  # 为 admin 用户添加 cluster-role 集群管理员角色

  $ oc adm policy remove-cluster-role-from-user cluster-admin admin
  # 为 admin 用户移除 cluster-role 集群管理员角色

  $ oc adm policy remove-cluster-role-from-group \
    self-provisioner \
    system:authenticated:oauth
  # 从集群角色中删除自调配角色，使已认证的 OAuth 用户与组无法调配创建新项目。
  
  $ oc adm policy add-cluster-role-from-group \
    self-provisioner \
    system:authenticated \
    system:authenticated:oauth
  # 集群角色中添加自调配角色，使已认证的 OAuth 用户与组能调配创建新项目。
  ```

  ```bash
  $ oc policy add-role-to-user <role> <username> -n <project>
  # 指定项目为用户添加角色

  $ oc policy add-role-to-user basic-user developer -n wordpress
  # 为 developer 用户在 wordpress 项目中添加 basic-user 角色
  # 注意：basic-user 角色是集群角色，指定项目可限制在该项目中。
  ```

  ```bash
  $ oc adm groups new <group_name>
  # 创建新用户组
  $ oc adm groups add-user <group_name> <user_name>
  # 添加用户至指定的组中
  ```
  
  ```bash
  $ oc get clusterrole
  # 查看集群角色信息    
  $ oc describe clusterrole system:<role>
  # 查看集群角色的详细信息
  ```
  
  ![clusterrole-demo](images/clusterrole-demo.jpg)
  
  ![verbose-examples](images/verbose-examples.jpg)
  
  ```bash
  $ oc describe clusterrole self-provisioner
  # 查看自调配角色的详细信息
  $ oc get clusterrolebinding.rbac -n default
  # 查看集群角色绑定的信息
  $ oc get rolebinding.rbac -n <project_name>
  # 查看指定项目中用户的本地项目角色绑定信息
  ```
  
  ![self-provisioner-desc](images/self-provisioner-desc.jpg)

- OCP 中的 ServiceAccount 与 SCC 的关系：

  ```bash
  $ oc get deployment <deployment_name> -o yaml | \
    oc adm policy scc-subject-review [-u <username>] -f -
  # 根据 deployment 查询指定用户的安全上下文约束

  $ oc create serviceaccount <serviceaccount_name> [-n <project>]
  # 在指定项目中创建服务账户，该账户可用于pod与api-server的通信认证。
  # 注意：服务账户必须由具有项目管理员角色的用户创建
  $ oc create serviceaccount wordpress -n farm

  $ oc adm policy \
    add-scc-to-user anyuid -z <serviceaccount_name> -n <project> 
  # 使用 system:admin 用户或具有 cluster-admin 角色的用户为指定项目的服务账户添加 anyuid 安全上下文约束（SCC）
  # 该安全上下文约束可使 pod 中运行应用的用户提权至 root 权限

  $ oc set serviceaccount deployment/<deployment_name> <serviceaccount_name> -n <project>
  # 将项目中的 serviceaccount 与 deployment 关联
  # 比如通过此种方式可将关联有 SCC 的 serviceaccount 与 deployment 关联
  ```

  ![serviceaccount-wordpress](images/serviceaccount-wordpress.jpg)

- 在不同命名空间中访问 Kubernetes API 资源：
  - 使用服务账户与不同角色的绑定可实现不用命名空间中应用 pod 对其他命名空间中资源的访问。
  - 如，使用以下步骤完成 project1 命名空间中应用 app-pod 访问 project2 命名空间中的 secret 资源：
    - project1 命名空间中创建名为 app-sa 的服务账户
    - 为 project1 命名空间中 app-pod 被分配 app-sa 服务账户
    - 在 project2 命名空间中创建 `RoleBinding`，即 `system:serviceaccount:project1:app-sa` 服务账户与 `secret-reader` 集群角色绑定。

    ![serviceaccount-rolebinding](images/serviceaccount-rolebinding.jpg)

  ```bash
  $ oc adm policy add-cluster-role-to-user <cluster_role> <serviceaccount_name>
  # 为服务账户添加集群管理员角色

  $ oc adm policy add-role-to-user <role> -z <serviceaccount_name> -n <project>
  # 为指定项目中服务账户添加角色
  ```

## OpenShift Pod 的调度

- 为 OCP 集群计算节点添加 label 标签：
  
  ```bash
  $ oc label node <node_fqdn> <key>=<value> [--overwrite]
  # 设置（覆盖）已存在的 node 节点标签
  
  $ oc label node node2.lab.example.com region=app --overwrite 
  # 设置（覆盖）已存在的节点标签 region 为 app
  # 设置的节点标签可被 pod 的节点选择器 Pod.spec.nodeSelector 使用，使其调度至该节点。
  ```
  
  ![node-label](images/node-label.jpg)

> ✅ region 为地理概念，zone 为不同的机柜/架或机房（故障恢复域）。

- 管理计算节点的 pod 可调度性：
  
  ```bash
  $ oc adm manage-node --schedulable=false <node_fqdn>
  # 设置 node 节点为 pod 不可调度状态
  ```
  
  ![node-unscheduleable](images/node-unscheduleable.jpg)
  
  ```bash
  $ oc adm manage-node <node_fqdn> --evacuate --pod-selector='<key>'='<value>'
  # 指定 pod 标签从 node 节点上迁移指定的 pod
  ```
  
  ![pod-evacuate-1](images/pod-evacuate-1.jpg)
  
  ![pod-evacuate-2](images/pod-evacuate-2.jpg)
  
  ```bash
  $ oc adm drain <node_fqdn> [--delete-local-data]
  # 从 node 节点上撤离所有运行的 pod
  # 若 pod 中已挂载使用相应的 pvc，在撤离时将报错，无法卸载已使用的 pvc！
  ```
  
  ![evacuate-delete-local-data-1](images/evacuate-delete-local-data-1.jpg)
  
  ![evacuate-delete-local-data-2](images/evacuate-delete-local-data-2.jpg)

## OpenShift 服务与路由使用

- 使用 oc 命令行方式创建 service：

  ```bash
  $ oc expose deployment/<deployment_name> \
    --selector <key>=<vaule> \
    --port <port> --target-port <port> --protocol [TCP|UDP] \
    --name <svc_name>
  # 使用 selector 选择器时对应的 label 必须与 pod 中的相互匹配  

  ### 示例 ###
  $ oc expose deployment/golang-codeready-workspace \
    --selector app=golang-codeready-workspace \
    --port 8080 --target-port 8080 --protocol TCP \
    --name gcw  
  ```

- 方式 1：指定 route 路由名称、对应 service 的端口号与对外暴露的 URL 以创建
  
  ```bash
  $ oc expose svc <service_name> \
    --name=<route_name> --port=<service_port> \
    --hostname=<custom_name>.<wildcard_domain> [-n <project>]
  ```

- 方式 2：直接指定 route 路由名称创建
  
  ```bash
  $ oc expose svc temp-cvt --name=ocp
  # --name        指定 route 的名称
  #               若不指定 route 的名称，则使用 application_name 代替 route_name。
  # --hostname    指定对外的公网域名
  #               默认的对外公网域名：<route_name-project_name>.<wildcard_domain>
  # 注意：可使用相同的 service 创建不同的 route 资源，而之前的 route 可不删除！
  ```

- 🚀 方式 3：创建安全边界终结型路由
  
  ```bash
  $ oc create route edge \
    --service=<service_name> --hostname=<exposed_fqdn_url> \
    --key=<ca_trusted_private_key>.key --cert=<ca_trusted_certificate>.crt
  # 使用 CA 私钥与 CA 签名的证书为 service 创建安全的边界型路由规则（secure edge-terminated）
  
  $ oc get route <route_name> [-n <project>] -o jsonpath='{..spec.host}{"\n"}'
  # 解析返回暴露的路由对应的应用 URL
  ```

- 模板（`template`）与 `Web Console` 中已嵌入 route 资源，因此可直接创建。
- 🪡 OCP 3.9 版本中删除 route 并重建后无法生效，报错 `HostAlreadyClaimed`，Bugfix 请详见 [Bug 1660598 - HostAlreadyClaimed route issue on path based route](https://bugzilla.redhat.com/show_bug.cgi?id=1660598)。
  
  ![ocp3-delete-route-error-1](images/ocp3-delete-route-error-1.jpg)
  
  ![ocp3-delete-route-error-2](images/ocp3-delete-route-error-2.jpg)

- 💎 补充：
  在 OCP4 集群中默认情况下普通用户无法访问 `openshift-console` 项目中的资源，可设置相应项目的 rolebindings 使普通用户可访问。

## OpenShift 日志与事件

- 容器日志是容器的标准输出（stdout）与标准错误（stderr）
- 常规日志与事件查看：
  
  ```bash
  $ oc logs <resource_type> <resource_name> [-n <project>]
  # 查看指定资源的日志信息，该日志信息不输出至 /var/log/messages。
  
  $ oc logs <pod> [-n <project>] [-f|--follow] [--tail=N] [-c|--container=<container_name>] [-p|--previous=true]
  # 查看 pod 的运行日志
  # 重要选项：
  #   -f, --fllow 选项：追踪容器的输出日志
  #   --tail=N 选项：指定输出容器的最后几行日志
  #   -c, --container=<container_name> 选项：指定 pod 内的容器
  #   -p, --previous=[true|false] 选项：指定是否输出 pod 内前一个容器的日志
  
  $ oc get [events|ev] [-n <project>]
  # 查看 OCP 集群的事件信息，常用于 troubleshooting 排错。
  # 也可在 Web Console 的 Monitoring > Events 中查看事件信息
  ```

## 参考链接

- Architecture
  - [Red Hat OpenShift Container Platform 4.6 Architecture](https://access.redhat.com/documentation/en-us/openshift_container_platform/4.6/html-single/architecture/index)
- CRI
  - [Container Runtime Interface (CRI) CLI](https://github.com/kubernetes-sigs/cri-tools/blob/master/docs/crictl.md#container-runtime-interface-cri-cli)
  - [GitHub - cri-api](https://github.com/kubernetes/cri-api)
- Pod
  - [Kubernetes Doc - Pods](https://kubernetes.io/docs/concepts/workloads/pods/)  
- Network
  - [Kubernetes Doc - Cluster Networking](https://kubernetes.io/docs/concepts/cluster-administration/networking/)
  - [Red Hat OpenShift v3.11 東西南北向網路探討](https://blog.pichuang.com.tw/20190404-openshift-network-traffic-overview/)
  - [Red Hat OpenShift v4 東西南北向網路流](https://blog.pichuang.com.tw/20200413-openshift4-network-traffic-overview/)
  - [Chapter 5. Cluster Network Operator in OpenShift Container Platform](https://docs.redhat.com/en/documentation/openshift_container_platform/4.14/html-single/networking/index#cluster-network-operator)
  - 🔥 [About the OVN-Kubernetes network plugin](https://docs.openshift.com/container-platform/4.14/networking/ovn_kubernetes_network_provider/about-ovn-kubernetes.html)
  - 🔥 [Chapter 24. OVN-Kubernetes network plugin](https://docs.redhat.com/en/documentation/openshift_container_platform/4.14/html-single/networking/index#about-ovn-kubernetes)
- Service Discovery
  - [GitHub Doc - SkyDNS](https://github.com/skynetservices/skydns)
  - [GitHub Doc - CoreDNS](https://github.com/coredns/coredns)
- Configure Reloader
  - ✨ [GitHub Doc - Reloader: Reloader can watch changes in ConfigMap and Secret and do rolling upgrades on Pods with their associated DeploymentConfigs, Deployments, Daemonsets Statefulsets and Rollouts.](https://github.com/stakater/Reloader)
