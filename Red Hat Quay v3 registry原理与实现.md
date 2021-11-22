## Red Hat Quay v3 registry 原理与实现

##### 文档目录：

- 常用私有容器镜像仓库

- Red Hat Quay v3 私有容器镜像仓库搭建

- 参考链接



##### 常用私有容器镜像仓库：

1. Harbor v1/v2：由 VMware 主导开发，并从 CNCF 云原生计算基金会孵化成功。
2. **`Red Hat Quay v3`**：由 Red Hat 开源的私有容器镜像仓库，类似于 CoreOS 的 Quay。
3. registry v2：Docker 公司发布的 v2 版本容器镜像仓库镜像，可直接运行提供服务。
4. docker-distribution：由 docker-distribution RPM 软件包提供，以 systemd 方式运行。



##### Red Hat Quay v3 私有容器镜像仓库搭建：basic 方式（非HA）

1. Red Hat Quay v3 私有容器镜像仓库组件：

   1）`Database`：MySQL 数据库，主要存储镜像的元数据信息，而非镜像存储。

   2）`Redis`：键值型存储，存储实时构建日志与 Quay 的向导。

   3）`Quay`：容器镜像仓库，运行 quay 容器服务，该服务由多个组件组成。

   4）**`Clair`**：扫描容器镜像的安全隐患（vulnerabilities）与修复问题（fixes）

2. 部署的容器镜像版本：

   1）MySQL：[registry.access.redhat.com/rhscl/mysql-57-rhel7:latest](http://registry.access.redhat.com/rhscl/mysql-57-rhel7:latest)

   2）Redis：[registry.assess.redhat.com/rhscl/redis-32-rhel7:latest](http://registry.assess.redhat.com/rhscl/redis-32-rhel7:latest)

   3）Quay：quay.io/redhat/quay:v3.3.0

   > **注意**：拉取该容器镜像前必须先使用相应账号登录 Quay，如下脚本所示。

3. 部署脚本如下所示：

   https://github.com/Alberthua-Perl/summary-scripts/blob/master/shell-examples/deploy-quay-registry.sh

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/deploy-quay-registry.png)

4. 配置并生成 Quay 配置文件：

   1）登录 Quay 并完成认证：

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/first-login-config-quay.png)

   2）生成 Quay 配置文件：

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/config-quay-1.png)

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/config-quay-2.png)

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/config-quay-3.png)

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/config-quay-4.png)

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/config-quay-5.png)

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/config-quay-6.png)

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/config-quay-7.png)

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/config-quay-8.png)

5. 登录与验证 Quay 私有容器镜像仓库：

   1）用户名：`admin`

   2）密码：`1qazZSE$`

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/deploy-quay-registry/normal-login-quay.png)

6. Podman 客户端登录 Quay：

   ```bash
   $ sudo mkdir /etc/docker/certs.d/<quay_registry_fqdn>/
   # 创建 Podman 客户端 Quay CA 证书存储目录
   $ sudo scp root@<quay_registry_fqdn>:/mnt/quay/config/ssl.cert \
     /etc/docker/certs.d/<quay_registry_fqdn>/ssl.crt
   # 同步 Quay CA 证书至 Podman 客户端
   
   # su - contsvc
   $ vim ~/.config/containers/registries.conf
     unqualified-search-registries = ['<quay_registry_fqdn>']
     # 该地址形如 registry.lab.example.com
     [[registry]]
     location = "<quay_registry_fqdn>"
     insecure = true
     blocked = false
   # 配置普通用户的 Quay 私有容器镜像仓库地址
   
   $ podman login <quay_registry_fqdn> \
     --username admin --password 1qazZSE$ \
     --log-level=debug
   # 成功登录 Quay 私有容器镜像仓库，并开启 debug 模式。
   ```



##### 参考链接：

- https://www.cnblogs.com/ericnie/p/12233269.html
- [https://docs.projectquay.io/deploy_quay.html](https://docs.projectquay.io/deploy_quay.html)
- https://github.com/docker/docker.github.io/blob/master/registry/deploying.md#get-a-certificate
- https://github.com/docker/docker.github.io/blob/master/registry/insecure.md

