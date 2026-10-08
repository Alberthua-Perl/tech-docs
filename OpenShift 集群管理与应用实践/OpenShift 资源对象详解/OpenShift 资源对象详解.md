# ⭕ OpenShift 资源对象详解

## 文档目录

- [⭕ OpenShift 资源对象详解](#-openshift-资源对象详解)
  - [文档目录](#文档目录)
  - [1. Master 类型节点](#1-master-类型节点)
  - [2. Worker 类型节点](#2-worker-类型节点)
  - [3. Project 资源对象](#3-project-资源对象)
  - [📦 4. ImageStream (is) 与 ImageStreamTag (istag) 资源对象](#-4-imagestream-is-与-imagestreamtag-istag-资源对象)
    - [4.1 基础概念](#41-基础概念)
    - [👨‍💻 4.2 **示例：从 OCP external registry 导入外部容器镜像至 OCP 集群**](#-42-示例从-ocp-external-registry-导入外部容器镜像至-ocp-集群)
    - [4.3 OCP internal registry 镜像缓存说明](#43-ocp-internal-registry-镜像缓存说明)
    - [4.4 image stream 资源使用](#44-image-stream-资源使用)
    - [💎 4.5 OCP4 internal registry 说明](#-45-ocp4-internal-registry-说明)
    - [4.6 image stream 使用报错示例](#46-image-stream-使用报错示例)
  - [⚙️ 5. BuildConfig (bc) 与 Build 资源对象](#️-5-buildconfig-bc-与-build-资源对象)
    - [5.1 基础概念](#51-基础概念)
    - [5.2 构建触发器（build trigger）类型](#52-构建触发器build-trigger类型)
    - [5.3 build 构建与 S2I 说明](#53-build-构建与-s2i-说明)
    - [6. DeploymentConfig (dc) 与 Deploy 资源对象](#6-deploymentconfig-dc-与-deploy-资源对象)
  - [💎 7. Deployment 资源对象](#-7-deployment-资源对象)
  - [8. ReplicationController (rc) 与 ReplicaSet 资源对象](#8-replicationcontroller-rc-与-replicaset-资源对象)
  - [9. Pod 资源对象](#9-pod-资源对象)
  - [10. Label 资源对象](#10-label-资源对象)
  - [🔥 11. Service (svc) 资源对象](#-11-service-svc-资源对象)
    - [🚀 11.1 OCP3 \& OCP4 的网络模型类型](#-111-ocp3--ocp4-的网络模型类型)
    - [11.2 service 资源对象处理的场景](#112-service-资源对象处理的场景)
    - [11.3 service 在 Kubernetes 中的实现方式](#113-service-在-kubernetes-中的实现方式)
    - [11.4 service 的类型](#114-service-的类型)
    - [11.5 service 与 pod 的关联方式](#115-service-与-pod-的关联方式)
    - [11.6 service 与服务发现](#116-service-与服务发现)
    - [11.7 service 与实现跨节点通信的 CNI 插件的关系](#117-service-与实现跨节点通信的-cni-插件的关系)
  - [12. Route 资源对象](#12-route-资源对象)
  - [13. PersistentVolume (pv) 资源对象](#13-persistentvolume-pv-资源对象)
  - [14. PersistentVolumeClaim (pvc) 资源对象](#14-persistentvolumeclaim-pvc-资源对象)
  - [15. Secret 资源对象](#15-secret-资源对象)
    - [15.1 基础概念](#151-基础概念)
    - [15.2 创建与使用 secret 资源对象](#152-创建与使用-secret-资源对象)
    - [15.3 提取与更新 secret](#153-提取与更新-secret)
    - [👨‍💻 **15.4 示例：OCP 4.6 中使用 secret 拉取外部私有容器镜像**](#-154-示例ocp-46-中使用-secret-拉取外部私有容器镜像)
    - [15.5 serviceaccount 与 secret 的关联](#155-serviceaccount-与-secret-的关联)
  - [16. ConfigureMap (cm) 资源对象](#16-configuremap-cm-资源对象)
    - [16.1 基础概念](#161-基础概念)
    - [16.2 创建与使用 configmap 资源](#162-创建与使用-configmap-资源)

## 1. Master 类型节点

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

## 2. Worker 类型节点

- OCP 集群与 Kubernetes 集群的计算节点
- compute 节点用于运行 pod 提供服务
- compute 节点的 docker 守护进程异常而导致 `atomic-openshift-node` 服务报错。
- 每个节点上的 `atomic-openshift-node` 服务已集成 `kubelet` 功能。

![atomic-openshift-node-error-1](images/atomic-openshift-node-error-1.jpg)  

![atomic-openshift-node-error-2](images/atomic-openshift-node-error-2.jpg)

## 3. Project 资源对象

- 中文名称：项目，也称为命名空间（namespace），OCP 集群使用项目来隔离资源（硬隔离），区别于 Linux namespace。
- 若未将 `self-provisioner` 角色从指定用户去除，使用指定用户创建的项目，该用户即为项目的项目管理员。
- default 项目与 openshift 项目能被所有用户使用，但只能由 `system:admin` 用户或具有 `cluster-admin` 角色的用户管理。

## 📦 4. ImageStream (is) 与 ImageStreamTag (istag) 资源对象

### 4.1 基础概念

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
  
### 👨‍💻 4.2 **示例：从 OCP external registry 导入外部容器镜像至 OCP 集群**

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

### 4.3 OCP internal registry 镜像缓存说明

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

### 4.4 image stream 资源使用
  
- 在 openshift 项目中的 image stream 与 template 资源在各个项目中均可共享，但只由具有 `cluster-admin` 角色的管理员用户管理。  
- 自 OCP4 开始可使用 `Samples operator` 管理 openshift 项目并可删除由手动添加的资源。
- **示例：使用来自另一个项目的 image stream（基于 private registry）构建与部署应用**
  - 1️⃣ 方式 1：</br>
    在每个使用 image stream 的项目中创建包含可访问私有 OCP external registry 的 access token 的 secret，并将其 link 至每个项目中的 service account。
  - 2️⃣ 方式 2：</br>
    仅仅在创建 image stream 的项目中创建包含可访问私有 OCP external registry 的 access token 的 secret，并配置 image stream 具有本地引用策略（`local reference policy`），即，将外部容器镜像缓存于 OCP internal registry，并为每个使用 image stream 的项目中使用 image stream 的 `service account` 授权。

    ![oc-import-image-shared-is-between-project](images/oc-import-image-shared-is-between-project.jpg)

  > ✅ 注意：可为 service account 添加 scc 与 role，也可将相关的 secret 链接至 service account。
  
### 💎 4.5 OCP4 internal registry 说明

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

### 4.6 image stream 使用报错示例

- 若已存在的 image stream 报错 `! error`，可将其删除再从外部容器镜像仓库导入。
- 删除已存在的 image stream 需指定 `tag`：

  ```bash
  $ oc tag -d <imagestream_name>:[tag] -n <project>
  # 删除指定项目中 image stream 的 image stream tag，使其可重新上传。
  ```

  ![imagestream-error-1](images/imagestream-error-1.jpg)

  ![imagestream-error-2](images/imagestream-error-2.jpg)

  ![imagestream-error-3](images/imagestream-error-3.jpg)

## ⚙️ 5. BuildConfig (bc) 与 Build 资源对象

### 5.1 基础概念

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
  
### 5.2 构建触发器（build trigger）类型

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

### 5.3 build 构建与 S2I 说明

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

### 6. DeploymentConfig (dc) 与 Deploy 资源对象

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

## 💎 7. Deployment 资源对象

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

## 8. ReplicationController (rc) 与 ReplicaSet 资源对象

- 前者已集成至 deploymentconfig 中，而后者集成至 deployment 中。
- 该资源对象保证运行的 pod 的高可用，使其当前的数量趋近于 desired 数量。
- 若 pod 由于某些原因故障停止，该资源对象将根据配置的 pod 数量（replicas）重新部署 pod 保证不间断服务，此类 pod 为无状态应用居多。

## 9. Pod 资源对象

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

## 10. Label 资源对象

- 中文名称：标签
- 类型：基于等值类型的标签 + 基于集合类型的标签（OCP4 已支持）
- OCP 集群中的各种资源使用 label 标签进行匹配

## 🔥 11. Service (svc) 资源对象

### 🚀 11.1 OCP3 & OCP4 的网络模型类型

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

### 11.2 service 资源对象处理的场景

- 由于 pod 经常因某些故障而重启，每次重启后其 IP 地址都将改变，因此使用 service 将一个或一组相同的 pod 进行关联。
- service 为 pod 提供统一的入口 IP 地址，该入口地址默认为 service 的虚拟 IP 地址（`ClusterIP`）。
- service 提供反向代理与负载均衡的功能，默认以 `Round Robin` 轮询的方式将流量转发至 pod。  

### 11.3 service 在 Kubernetes 中的实现方式

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

### 11.4 service 的类型

- 4大类型：
  - 1️⃣ `ClusterIP`
  - 2️⃣ `NodePort`
  - 3️⃣ `LoadBalancer`：这种类型构建在 NodePort 类型之上，其通过 cloud provider 提供的负载均衡器将服务暴露到集群外部，因此 LoadBalancer 一样具有 NodePort 和 ClusterIP。
  - 4️⃣ `ExternalIPs`

![K8s-service-type](images/K8s-service-type.jpg)

- 💥 使用 NodePort 类型 service 的资源定义文件更改后再创建 ClusterIP 类型 service 时，需删除其中的 `spec.externalTrafficPolicy` 字段，否则创建失败！
  
  ![change-service-type-error](images/change-service-type-error.jpg)

### 11.5 service 与 pod 的关联方式

service 通过 `selector` 与具有相同 `label` 的 pod 关联，将固定的 IP 地址与 pod 解耦，提高 pod 部署的灵活性，OCP 可根据 scheduler 调度器将 pod 部署至不同的 node 节点上，根据 ReplicationController (OCP3) 或 Deployment (OCP4) 部署相应副本数量的 pod，保证 pod 的服务高可用，此类 pod 应用一般为无状态类型服务。

![ocp-service-selector-match-pod-label](images/ocp-service-selector-match-pod-label.jpg)

> 💥 无论 OCP 集群使用 `ovs-subnet` 或 `ovs-multitenent` 类型的 SDN 插件，同一项目的 pod 间可直接通信，无需使用 service！

### 11.6 service 与服务发现

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

### 11.7 service 与实现跨节点通信的 CNI 插件的关系

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

## 12. Route 资源对象
  
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

## 13. PersistentVolume (pv) 资源对象

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

## 14. PersistentVolumeClaim (pvc) 资源对象

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

## 15. Secret 资源对象

### 15.1 基础概念

- 该资源对象保存 OCP 集群中的敏感数据，如密码、token 凭据等，将敏感数据与 pod 解耦。
- 数据使用 `base64` 编码（encode）存储在 secret 资源对象中。
- secret 资源对象可在命名空间中共享。
- 当来自 secret 的数据被注入到容器中时，可采用以下两种方式实现：
  - 1️⃣ 作为环境变量（env）注入到容器中
  - 2️⃣ 作为卷（volume）挂载：指定容器内挂载路径，容器映射目录中的各个文件名为 secret 的各个 key，文件的内容为对应 key 的值。
- secret 的类型：

  ![k8s-ocp-secret-type](images/k8s-ocp-secret-type.png)

### 15.2 创建与使用 secret 资源对象

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

### 15.3 提取与更新 secret

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

### 👨‍💻 **15.4 示例：OCP 4.6 中使用 secret 拉取外部私有容器镜像**

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

### 15.5 serviceaccount 与 secret 的关联

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

## 16. ConfigureMap (cm) 资源对象

### 16.1 基础概念

- 该资源对象类似于 secret 资源对象，但它们存储的是不敏感的数据。
- configmap 资源对象可用于存储细粒度（`fine-grained`）信息，如独立的属性，或粗粒度（`coarse-grained`）信息，如整个配置文件和 JSON 数据。
- 可使用 OpenShift CLI 或 Web 控制台创建 configmap 与 secret 资源，可在 pod 规范和 OpenShift 中自动引用它们。
- secret 资源对象与 configmap 资源对象的特点：
  - 均可以独立地被定义（definition）与被引用（referenced）
  - 出于安全原因，为这些资源挂载的卷由临时文件存储（tmpfs）支持，不存储在节点上。
  - 它们均作用于所在的命令空间（namespace）

### 16.2 创建与使用 configmap 资源

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
