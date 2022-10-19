## 🐳 容器运行时 runtime 演进与架构概述

### 文档说明：

- 该文档未说明 `Kata Container`、`gVisor` 等强隔离的容器 runtime。

- 该文档中具有较多概念与技术点，难免存在疏忽与纰漏，欢迎交流与指正！

### 文档目录：

- Docker 架构的技术演进

- Containerd 简要概述

- 容器运行时 runtime 分类

- Kubernetes 中 Docker 容器 runtime 示例

- 容器发展史简要回顾

- OCI、runC 与 CRI 概述

- Kubernetes 中 Containerd 及 CRI-O 为 runtime 的架构演进

- 容器运行时 runtime 总览

- 参考链接

### Docker 架构的技术演进：

- Docker 项目早先已由 DotCloud 公司贡献于社区成立 `Moby` 项目（Docker CE）。

- 从 Docker 1.11 版本开始至 `Docker 17.x` 版本及以上版本逐步将 Docker Daemon 解耦成 docker 与`containerd` 两个独立的项目，拆分 Docker 的功能。

- 以下以 Docker 1.13.x 与 Docker 17.x 版本及以上版本为例说明各组件关系。

- Docker 1.13.x 组件关系示意：

  包含的组件：docker、`dockerd`、`docker-containerd`、docker-containerd-shim、`docker-runc`

  <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/docker-1.13.x-dockerd-dockerd-containerd.jpg" style="zoom:150%;" />

- Docker 17.x 及以上版本组件关系示意：

  - 包含的组件：docker、`dockerd`、`containerd`、containerd-shim、`runc`

  - dockerd 守护进程与 containerd 实现解耦拆分：

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/docker-17.x-dockerd-containerd-1.jpg)

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/docker-17.x-dockerd-containerd-2.jpg)

- Docker 18.09.0 与 19.03.13 进程关系示意：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/dockerd-containerd-1.jpg)

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/dockerd-containerd-2.jpg)

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/dockerd-containerd-3.jpg)

  > 📌 注意：各版本之间的 dockerd 与 containerd 进程的调用存在差异！

### Containerd 简要概述：

- 2016 年 12 月 14 日，DotCloud 公司宣布将 containerd 从 Docker Engine 中分离，并捐赠到一个新的开源社区独立发展和运营，"一个工业标准的容器运行时，注重简单、健壮性、可移植性"。

- Docker 1.11 的 Docker Engine 里就包含了 containerd，而现在则是把 containerd 从 Docker Engine 里彻底剥离出来，作为一个独立的开源项目独立发展，目标是提供一个更加开放、稳定的容器运行基础设施。

- 需要独立的 containerd 的原因：

  - 继续从整体 Docker Engine 中分离（开源项目的思路）
  
  - 可以被 Kubernetes CRI 等项目使用（通用化）
  
  - 为广泛的行业合作打下基础（像 runC 一样）

- Docker 对容器的管理和操作基本都是通过 containerd 完成的。

- containerd 可以在宿主机中管理完整的容器生命周期：

  - 拉取、推送容器镜像
  
  - 存储管理：管理镜像及容器数据的存储
  
  - 管理容器网络接口及网络：使用 CNI 容器网络接口
  
  - 管理容器的生命周期：从创建容器到销毁容器
  
  - 调用 runC 运行容器

- containerd 并不是直接面向开发人员或终端用户使用，而是主要用于集成到更上层的系统里，如 Swarm、Kubernetes、Mesos 等容器编排系统。

- containerd 以守护进程的形式运行在系统上，通过 `unix domain socket` 暴露很低层的 `gRPC API`，上层系统可以通过这些 API 管理机器上的容器。

- 每个 containerd 只负责一台主机，拉取镜像，对容器的操作（启动、停止等），网络、存储都由 containerd 完成。

- 具体运行容器由 runC 负责，实际上只要是符合 OCI 规范的容器都可以支持。

- containerd 架构示意：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/containerd-arch.png)

- containerd 的应用：

  - docker 镜像和 containerd 镜像通用，但组织方式和存放目录不同，导致 `docker` 与 `ctr` 命令不通用，各自管理自己的镜像和容器！

  - containerd 的默认配置文件：`/etc/containerd/config.toml`

  - 配置文件语法使用 [参考链接](https://github.com/containerd/containerd/blob/main/docs/ops.md)

  - ctr 命令行使用示例：

    ```bash
    $ sudo ctr images ls
    # 查看 containerd 镜像
    $ sudo ctr images pull docker.io/library/busybox:latest
    # 拉取 containerd 镜像
    # 注意：必须使用镜像的完整名称，否则报错！
    $ sudo ctr run -t docker.io/library/nginx:latest mynginx
    # 运行 containerd 容器
    $ sudo ctr plugins ls
    # 查看 containerd 支持的插件
    ```

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/ctr-command-1.jpg)

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/ctr-command-2.jpg)

    > 📌 注意：
    >
    > 1. containerd 1.2.x 无法拉取 containerd 镜像的 issue，升级 containerd 后可正常使用。
    > 2. GitHub issue [参考链接](https://github.com/containerd/containerd/issues/1818)：
    >
    > ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/ctr-pull-image-error-bug.jpg)

- 关于 Containerd 更加详实具体的文档可查看文末的参考链接。

### 容器运行时 runtime 分类：

- [CNCF 云原生计算基金会的 Landscape 中容器 runtime 项目](https://github.com/cncf/landscape)：

  <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/cncf-container-runtime.jpg" style="zoom:33%;" />

### Kubernetes 中 Docker 容器 runtime 示例：

- 以 `kubelet` 集成 Docker 容器 runtime 为例，解释 kubelet 如何创建容器。

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/kubelet-dockershim-dockerd-containerd-runc.jpg)

- 步骤 1：
  
  - kubelet 通过 `CRI` 接口（`gRPC`）调用 `dockershim`，请求创建容器。
  
  - CRI 即容器运行时接口（container runtime interface），这步中，kubelet 可视作简单的 `CRI Client`，而 dockershim 就是接收请求的 `CRI Server`。
  
  - 目前 dockershim 的代码内嵌在 kubelet 中，所以接收调用的就是 kubelet 进程。

- 步骤 2：

  - dockershim 收到请求后，转化成 Docker Daemon 可接收的请求，发送到 Docker Daemon 并请求创建容器。

- 步骤 3：

  - Docker Daemon 早在 1.11 版本中就已经将针对容器的操作移到另一个守护进程，即 containerd 中，因此 Docker Daemon 仍然不能创建容器，而需请求 `containerd` 创建容器。

- 步骤 4：

  - containerd 收到请求后，并不会直接去操作容器，而是创建 `containerd-shim` 的进程，让 containerd-shim去操作容器。
  
  - 因为容器进程需要一个父进程来做诸如收集状态、维持 `STDIN` 等 `fd` 打开等工作。

- 步骤 5：

  - 创建容器需要设置 `namespaces` 和 `cgroups`，挂载 `rootfs` 等操作，而这些操作需通过 `OCI`（开放容器标准，open container initiative）。
  
  - OCI 的一个参考实现称为 `runC`。
  
  - containerd-shim 需要调用 runc 命令行启动容器。

- 步骤 6：

  - runc 启动完容器后本身会直接退出，`containerd-shim` 则会成为容器进程的父进程，负责收集容器进程的状态，上报给 containerd。
  
  - 在容器中 PID 为 1 的进程退出后接管容器中的子进程进行清理，确保不会出现僵尸进程。

### 容器发展史简要回顾：

- 容器生态中的各类组件与名词在技术演进的过程中形成，因此，了解容器发展史有助于对各类组件与名词的理解，如下所示。

- Kubernetes 最开始的 runtime 架构远没这么复杂，kubelet 要创建容器直接调用 Docker Daemon 即可，而那时也不存在 containerd，Docker Daemon 自己调用 `libcontainer` 库运行起来，整个过程即完成。

- 然而，随着容器圈的一系列政治斗争，先是 Google 与 Red Hat 等认为运行时标准不能被 Docker 一家公司控制，于是就创建了 OCI 开放容器标准。

- 👉 Docker 公司则将 libcontainer 库封装成 runC，将其贡献作为 OCI 的参考实现。

- 接着 `rkt` 想从 Docker 那分一杯羹，希望 Kubernetes 原生支持 rkt 作为运行时，而且 `PR` 还真将其合并。

- 💥 但是，这样也造成在同时处理两种 runtime 的兼容性问题。

- 👉 因此，负责维护 kubelet 的 `sig-node` 小组在 `Kubernetes v1.5` 推出 CRI，只要能实现该接口，即能成为 runetime。

- 👉 不过 CRI 本身只是 Kubernetes 推的标准，当时的 Kubernetes 尚未达到如今的主导地位，容器运行时当然不能跟 Kubernetes 绑死只提供 CRI 接口，于是就有了 `shim`（垫片）。

- 🚀 shim 的职责就是作为接头将各种容器运行时本身的接口适配到 CRI 接口上。

- 接下来 Docker 开发 Swarm 进军 PaaS 市场，将 Docker 做架构切分，把容器操作都转移到单独的 containerd 守护进程中，使 Docker Daemon 专门负责上层的封装编排。

- 👉 可惜 Swarm 惨败于 Kubernetes，之后 Docker 公司将 containerd 项目捐给 CNCF。

- 最后 Docker 公司专注于开发 Docker 企业版。

- 时至今日，尽管已经有 `CRI-O`、`containerd` 这种更精简轻量的 runtime，dockershim 作为经受最多生产环境考验的方案，迄今为止仍是 Kubernetes 默认的 runtime 实现。

  > 📌 注意：
  >
  > dockershim 将在 Kubernetes v1.20 版本及更高版本中逐步废弃，不再使用 Docker 而全面使用 Containerd。

### OCI、runC 与 CRI 概述：

- OCI：开放容器标准，其实就是一系列文档，其中主要规定了两点。
  - `ImageSpec`：

    - 规定容器镜像
    
    - 主要定义一个 `OCI image`，该镜像由 manifest、image index（optional）、filesystem layers、以及 configuration 组成。
    
    - 该规范的目的在于确保构建一套不同容器引擎间可互操作的工具，用于镜像的构建、传输，以及镜像运行准备工作。
  
  - `RuntimeSpec`：
    
    - 主要定义如何配置容器、执行环境以及容器生命周期中需要能接收哪些指令，这些指令的行为是什么等。
    
    - 其中的大致内容为容器要能执行 create、start、stop、delete 等命令，且行为要规范。

- runC：OCI 的一种参考实现

  - 由 libcontainer 库演变而来，并且由 Docker 捐献给 Linux 基金会。
  
  - libcontainer 库直接使用 Linux kernel 提供的相关隔离技术，如 namespace、cgroups。
  
  - 它能按照标准将符合标准的容器镜像运行起来。

  > 📌 注意：
  >
  > 1. 标准的好处就是方便搞创新，只要能符合标准，都能与生态圈中的其它工具一起工作。
  > 2. 那么镜像就可以用任意的工具去构建，容器就不一定非要用 namespace 和 cgroups 来隔离。
  > 3. 这就让各种虚拟化容器（Kata Container 等）可以更好地参与到生态圈当中。

- CRI：单纯是一组 `gRPC` 接口，可归纳为几套核心接口。

  - 针对镜像操作的接口，包括拉取镜像、删除镜像等。

  - 针对容器操作的接口，包括创建、启停容器等。

  - 针对 `PodSandbox`（容器沙箱环境）的操作接口。

    - Pod and container lifecycle management：

      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/cri-grpc-runtimeservice-1.jpg)

    - exec/attach/port-forward requests：

      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/cri-grpc-runtimeservice-2.jpg)

  - CRI 的工作原理：

    - CRI 定义了一组关于 Pod、Container 与 Image 的 gRPC 接口，若 `cri-runtime` 与其对接，只需     开发出一套相应的 cri-runtime 连接 `shim` 即可，就能被 kubelet 所调用。

    - 如图所示：

      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/how-cri-works.jpg)

- 因此，可以找到很多符合 OCI 标准或兼容 CRI 接口的项目，而这些项目大体构成了整个 Kuberentes 的 runtime 生态：

  - OCI compatible：

    runC、Kata（以及其前身 runV 和 Clear Containers）、gVisor、railcar（Rust）

  - CRI compatible：

    Docker（借助 dockershim）、containerd、CRI-O、frakti

- 💥 OCI 和 CRI 的区别与联系，最容易让人产生混淆的一大原因就是社区里`糟糕的命名`。

- 👉 以上的项目统统可以称为容器运行时（container runtime），彼此之间区分的办法就是给`容器运行时`这个词加上各种定语和从句来进行修饰。

- 👉 显然，container runtime 就不是好名字，更准确的说法：`cri-runtime`、`oci-runtime`

- 通过这个粗略的分类，其实可以总结出整个 runtime 架构不变的三层抽象：

  > Orchestration API --> Container API --> Kernel API

- 其中 Kuberentes 已经是 Orchestration API 的事实标准，而 Container API 的接口标准就是 CRI，由 cri-runtime 实现，Kernel API 的规范是 OCI，由 oci-runtime 实现。

### Kubernetes 中 Containerd 及 CRI-O 为 runtime 的架构演进：

- 之前使用 Docker 的架构确实有点复杂，而复杂是万恶之源（其本质就是替代 Docker）。

- 于是就促成直接使用 containerd 作为 cri-runtime 的方案。

- 当然，containerd 还需要适配系统以对接 CRI，该适配工作交由一个 `shim` 来完成。

- `Containerd 1.0` 中，对 CRI 的适配通过一个单独的进程 `CRI-containerd` 来完成。

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/cri-containerd-containerd-1.jpg)

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/cri-containerd-containerd-2.jpg)

- `Containerd 1.1` 中直接去除 CRI-containerd 进程，将适配逻辑作为插件放进 Containerd 主进程中，调用链如下所示：

  kubelet --> CRI-plugin（在 containerd 主进程中）--> containerd

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/containerd1.1.jpg)

- 但与此同时，社区就已经有了一个更为专注的 cri-runtime，即 `CRI-O`。

- 它非常纯粹，兼容 CRI 和 OCI，作为 Kuberentes 与 OpenShift 专用的容器运行时。

- 其中 `conmon` 对应 `containerd-shim`，大体意图是一致的。

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/cri-o-runtime-1.jpg)

- Docker 与 Containerd 架构演进总结：

  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/docker-containerd-runtime.jpg)

- CRI-O 与 Podman 架构示意：

  - kubelet 与 CRI-O 的集成：

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/cri-o-runtime-2.jpg)

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/cri-o-runtime-3.jpg)

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/cri-o-runtime-4.jpg)

  - Podman 运行容器的方式：

    - Podman 曾经是 CRI-O 项目中的一部分，后来被分离出成为一个独立的项目，即 libpod。
    
    - Podman（Pod Manager）的目标是提供与 Docker 相似的 container CLI，提供給使用者创建与运行容器。
    
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/podman-arch.jpg)
    
    - 关于 Podman 更加详实具体的文档可查看文末的参考链接。

### 容器运行时 runtime 总览：

- 以下是从高层次与低层次的视角对容器运行时的总览：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/container-runtime-introduce/high-or-low-level-runtime.jpg)

### 参考链接：

- [容器技术生态概览](https://www.jianshu.com/p/453021b7c1ff?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)

- [GitHub Doc - Moby](https://github.com/moby/moby)

- [Containerd 简述](https://www.cnblogs.com/embedded-linux/p/10850491.html)

- [初识 Containerd](https://mp.weixin.qq.com/s?__biz=MzA3MDM1NjE0NA==&mid=2247483826&idx=1&sn=8fefccc0dabf9aac5650b7a14263fae0&chksm=9f3f5c6da848d57b17917a9983e97a57e33e5ca596b8becca7c0c581ea6a1098efe27be9358a&mpshare=1&scene=24&srcid=0504zxZIFifH8YLeoc6PdG9O&sharer_sharetime=1620116918211&sharer_shareid=6f52fcf5de1f55a8e68dc61bb30e3bc1#rd)

- [Containerd 的前世今生和保姆级入门教程 - 米开朗基杨](https://mp.weixin.qq.com/s/lyccCunbaKCsgotrl2lLug)

- [一文搞定 Containerd 的使用 - 阳明](https://mp.weixin.qq.com/s/--t74RuFGMmTGl2IT-TFrg)

- [可以像 Docker 一样方便的使用 Containerd 吗？- 阳明](https://mp.weixin.qq.com/s/1o4ec6m4LANHt1wgPrwZDA)

- [GitHub Doc - Containerd](https://github.com/containerd/containerd)

- [白话 Kubernetes Runtime](https://mp.weixin.qq.com/s/PpKz9FBIo_GCnxquh9F5ow)

- [容器运行时概述](https://insujang.github.io/2019-10-31/container-runtime/)

- [Kubernetes 官方文档中对 CRI 的说明](https://kubernetes.io/blog/2016/12/container-runtime-interface-cri-in-kubernetes/)

- [GitHub 中对 CRI 的说明](https://github.com/kubernetes/kubernetes/blob/242a97307b34076d5d8f5bbeb154fa4d97c9ef1d/docs/devel/container-runtime-interface.md)

- [GitHub Doc - CRI-O](https://github.com/cri-o/cri-o)

- [Podman 容器使用与原理 - Alberthua](https://github.com/Alberthua-Perl/tech-docs/blob/master/Podman%20%E5%AE%B9%E5%99%A8%E4%BD%BF%E7%94%A8%E4%B8%8E%E5%8E%9F%E7%90%86.md)

- [Podman 官网](https://podman.io/)

- [GitHub Doc - Podman](https://github.com/containers/podman)

- [GitHub Doc - OCI](https://github.com/opencontainers)
