## DO700 定制课程课堂笔记

### 第一天:

- 第一章：容器技术简介

  - 第一节：描述容器技术

    - 容器关键技术点：命名空间(namespace)、进程控制组(cgroup)、联合文件系统(unionfs)

    - Google 在容器技术推进中首先将 CGroup 称为进程容器，但在后来的开发演进中发现这种称谓不贴切而弃用。

    - OpenShift 1.x/2.x 等都依托于 Gear 框架开发。

    - Moby 项目来源于 Docker 公司的社区开源版本。

    - 传统虚拟机的虚拟化是 OS 的虚拟化，而容器的虚拟化技术是进程的虚拟化，容器之间共享宿主机的 kernel。

  - 第二节：Docker 架构

    - Docker <= 1.13.x: 使用完整的 Docker 引擎(dockerd-current)，该引擎实现容器镜像、容器生命周期的全部管理。

    - 课程环境中的用户名与密码：
				
      - foundation 节点：`root: Asimov`, `kiosk: redhat`

      - 其余所有节点：`student: student`, `root: redhat`
				
    - Docker 17.x~20.x: 使用 dockerd 守护进程与 containerd 守护进程协同运行的方式管理容器镜像、容器生命周期以及容器网络与存储。

  - 第三节：Kubernetes 与 OpenShift 的架构基础

    - OCP 3.x/4.x 的 master 节点 API server 的客户端：
		   
      - kubelet 守护进程的和解循环与资源的 watch 反馈

      - 注意：OCP 中 kubelet 守护进程已经集成至 `openshift-node` 守护进程中。

      - 运维、测试或开发人员对集群资源的管理，可通过 `oc` 或 `kubectl` 标准命令亦或是其他自研的命令。

      - 第三方组件实现的 API server 调用。

    - OCP 3.x 的 master 节点 authentication：OCP 的认证方式包括但不限于 http-basic, OAuth, OpenStack KeyStone, OpenLDAP, IdM, AD 等等。

    - OCP 3.x 的数据库：基于键值型存储的 etcd 数据库，存储集群的状态以及资源的信息等。

    - OCP 3.x 的调度器：default-scheduler 或自研的调度器，用于调度集群最小的调度单位 Pod 至不同的 worker node 之上。

    - OCP 3.x 的控制管理器：controller-manager，如控制无状态应用的副本冗余数。

    - 开发者使用 OpenShift 的话主要用于应用的构建发布，可采用集群外部的 CI 平台，如 Jenkins、GitLab、Drone 等来完成集成，当然也可以直接使用集群原生的 S2I 构建方式。

- 第二章：创建容器化服务

  - 第一节：编排数据库容器

    - 获取容器镜像的方式：

      - docker.io: Docker 公司对外的公共镜像仓库，以及私有的镜像仓库。

      - registry.access.redhat.com: RedHat 对外的部分公共镜像仓库，以及私有镜像仓库。

      - quay.io: RedHat 对外的公共镜像仓库，但是 RedHat 对仓库中的容器镜像不做任何的质量保障！

      - `k8s.gcr.io`: Google 的镜像仓库，当时需要科学上网并使用代理加速拉取下载。

      - registry.aliyuncs.com: 阿里云对外的公共镜像仓库

    - 在 RedHat 容器镜像仓库中的镜像只有达到 `Health Index` 的 B 级及以上才能用于生产环境。

    - UBI: universal build image, 通用构建镜像源自于 RHEL 7/8/9，其使用方法与 RHEL 基本类似。

    - 常用的容器镜像仓库组件：

      - Harbor v2.x: VMware 主导开发并开源的 registry，支持单节点与高可用集群。

      - Quay v3.x: RedHat 主导开发并开源的 registry，支持单节点与高可用集群。

      - registry v2.x: Docker 主导开发的容器镜像镜像仓库组件，以容器的方式启动运行，使用与管理便捷。

      - docker-distribution: 社区发布的容器镜像仓库管理软件包，以 `rpm` 方式发布，安装后由 systemd 管理，课程环境中使用该组件。

    - 容器镜像名称的命名方式：

      - `<registry_uri>/<user>/<image_name>:[tag]`：
      
        该命名方式常见于 docker-distribution 中，甚至可省略中间的 `<user>` 部分。

      - `<reigstry_uri>/<orgnization>/<repositiry>:[tag]`：
      
        该命令方式常见于 Harbor 或 Quay 中，中间的 `<orgnization>` 部分不可省略。

    - 创建容器的两种方式：

      - 创建静态容器并运行该容器

        ```bash
        $ sudo docker container create --name=mysqldb -e MYSQL_ROOT_PASSWORD=redhat registry.lab.example.com/rhscl/mysql-56-rhel7:latest
        $ sudo docker start mysqldb
        ```

      - 直接从容器镜像加载创建容器

        ```bash
        $ sudo docker run -d --name=mysqldb -e MYSQL_ROOT_PASSWORD=redhat registry.lab.example.com/rhscl/mysql-56-rhel7:latest
        $ sudo docker ps
        ```

- 第三章：管理容器

  - 第一节：管理容器生命周期

    - `docker search` 命令：只能用于在线镜像仓库中的镜像搜索，而不能够用于离线镜像仓库的镜像搜素，这取决于该命令使用的 RESTful API 的版本。


