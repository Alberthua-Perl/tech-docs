## ♾ 基于 GitLab + Jenkins + Harbor + Podman 的 CI/CD 部署示例

### 文档说明：

- OS 版本：CentOS Linux release 7.4.1708 (Core)

- 该示例将构建的 Tomcat 应用容器镜像部署于 Podman 主机上，未部署于 Kubernetes 或 OpenShift 集群。

- 最终交付的容器镜像可为不同编程语言及其依赖的环境（Java、Python、Golang、NodeJS、Ruby等）。

### 文档目录：

- 示例环境说明

- GitLab 代码仓库部署

- Harbor 容器镜像仓库部署

- Jenkins CI 环境集成

### 示例环境说明：

1. 各节点硬件资源与角色：

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/node-hardware-role.png)

   1）192.168.0.111 节点：仅作为 Git 推送代码至 GitLab 代码仓库用

   2）192.168.0.110 节点：GitLab 代码仓库、Harbor 容器镜像仓库

   3）192.168.0.108 节点：Jenkins 平台、Podman 容器运行时

2. DevOps 工具栈图谱：

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/devops-tools.jpg)

3. 该示例架构：

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-based-devops-cicd-demo.jpg)

   1）该架构中未使用 Red Hat Quay 且未集成 Kubernetes 或 OpenShift 集群部署应用容器。

   2）生产环境中可将上述流程与 Kubernetes 或 OpenShift 集群集成，或将上述组件以 Pod 的方式部署在集群中，并额外部署应用管理工具（Helm）及监控工具（Promethus）加固集群功能。

### GitLab 代码仓库部署：

1. 用户相关说明：

   1）GitLab 开发用户：devuser

   2）代码使用 cloud-ctl 节点的 godev 用户进行提交。

   3）godev 用户可使用其 SSH 公钥以 devuser 用户免密码登录 GitLab。

   4）godev 用户还将被作为 Jenkins 从 GitLab 上拉取代码的用户。

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/devuser-gitlab-user.jpg)

2. 该示例中使用的代码结构如下所示：

   <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/tomcat-jenkins-demo.png" style="zoom:150%;" />

   🔊 GitHub 仓库地址：

   https://github.com/Alberthua-Perl/jenkins-pipeline-demo/tree/master/tomcat-jenkins-demo

### Harbor 容器镜像仓库部署：

1. 该示例中使用 Harbor 容器镜像仓库作为私有 registry。

2. 虽然可用 Red Hat Quay 公共容器镜像仓库，但在 Jenkins 拉取或推送容器镜像时由于超时而导致 `pipeline` 构建失败。

3. Harbor 容器镜像仓库与 GitLab 代码仓库都部署于 192.168.0.110节点。

4. Harbor 使用 `docker-compose` 启动并管理，因此需先安装 `Docker` 与 `Docker-Compose`。

5. 各组件版本：

   1）Docker：docker-ce-19.03.1-3.el7.x86_64

   2）Docker-Compose：docker-compose-1.24.1

   3）Harbor：harbor-offline-installer-v1.8.1

6. Docker 与 Docker-Compose 部署：

   ```bash
   $ su - codeadmin
   # 切换至 codeadmin 用户
   # 该用户为 GitLab 与 Harbor 管理用户（具有sudo权限）
   
   $ sudo yum install -y docker-ce-19.03.1-3.el7.x86_64
   $ sudo systemctl enable docker.service
   $ sudo systemctl start docker.service
   # 安装启动 Docker 守护进程
        
   $ sudo cp docker-compose-1.24.1 /usr/local/bin/docker-compose
   # docker-compose 为二进制文件安装，直接运行该命令即可。
   $ sudo ln -s /usr/local/bin/docker-compose /usr/bin/docker-compose
   # 创建 docker-compose 软链接，否则启动 Harbor 失败。
   ```

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/install-docker-compose.png)

7. 配置与启动 Harbor 容器镜像仓库：

   1）配置 Harbor 容器镜像仓库：
   
   ```bash
   $ sudo tar -zxvf harbor-offline-installer-v1.8.1.tgz -C /usr/local
   $ sudo vim /usr/local/harbor-v1.8.1/harbor.yml
   # 更改 Harbor 配置文件中默认参数
   ```

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-conf-yaml.png)
   
   2）安装 Harbor 各容器组件：
   
   ```bash
   $ sudo /usr/local/harbor-v1.8.1/install.sh
   # 启动 Harbor 容器镜像仓库服务，各组件以容器方式运行。
   ```

   <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-install-1.png" style="zoom:150%;" />

   <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-install-2.png" style="zoom:150%;" />
   
   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-install-3.png)
   
   > 💥 注意：
   >
   > 若 /usr/local/harbor-v1.8.1/harbor.yml 配置文件中指定非法的 http 端口，nginx 容器将启动失败报错！
   >
   > ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-nginx-error.jpg)
   
   3）查看 Harbor 容器镜像仓库状态：
   
   ```bash
   $ sudo docker-compose -f /usr/local/harbor-v1.8.1/docker-compose.yml ps
   # 查看 docker-compose 启动的 Harbor 容器组件与状态
   ```
   
   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-registry-status.jpg)
   
   4）Web 登录验证：http://harbor.domain12.example.com:8880
   
   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-web-login.jpg)
   
   5）停止 Harbor 容器镜像仓库：
   
   ```bash
   $ sudo docker-compose -f /usr/local/harbor-v1.8.1/docker-compose.yml stop
   # 停止 Harbor 容器镜像仓库，即停止 docker-compose 服务。
   ```

### Jenkins CI 环境集成：

1. Jenkins 可通过多种方式部署：容器镜像、rpm 软件包、war 包

2. 该示例中使用 `Apache tomcat` 容器运行 Jenkins war 包的方式进行部署。

3. 该示例中仅使用 `Podman` 容器展示 `JSP Web` 页面，并未构建与测试 Java Web 应用，因此可不安装 `Apacpe Maven`。

4. 生产环境中多以部署 Java Web 应用，需使用 Maven 构建打包为 war 包后再进行部署。由于需要运行 Tomcat 容器，因此需安装 Java 运行环境。

5. 各组件版本：

   1）Java：jdk-8u191-linux-x64

   2）Tomcat：apache-tomcat-9.0.30

   3）Jenkins：jenkins-2.204.2
   
   4）Podman：podman-1.4.4-4.el7.centos.x86_64

   > ✅ 注意：
   >
   > 虽然该示例中未安装 Maven，但该 Jenkins 版本所对应的 Maven 版本可在 Jenkins 安装成功后进行查看，保证两者的`兼容性`，即 "Manage Jenkins -> Global Tool Configuration -> Maven"。
   >
   > ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-maven-compatibility.png)

6. Java 环境部署：

   ```bash
   $ su - ciadmin
   # 该用户为 Jenkins 管理用户（具有 sudo 权限）
   $ sudo tar -zxvf jdk-8u191-linux-x64.tar.gz -C /usr/local
   $ sudo vim /etc/profile
   # 编辑系统全局环境配置文件，添加 Java 环境变量，即 JAVA_HOME、JRE_HOME。
   ```

   <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/java-env-var.png" style="zoom:150%;" />

   ```bash
   $ source /etc/profile
   $ java -version
   ```

   <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/java-version.png" style="zoom:150%;" />

7. Tomcat 部署：

   1）Apache Tomcat 软件压缩包解包：

   ```bash
   $ su - ciadmin
   $ sudo tar -zxvf apache-tomcat-9.0.30.tar.gz -C /usr/local
   $ ln -s /usr/local/apache-tomcat-9.0.30 tomcat-jenkins
   $ tomcat-jenkins/bin/catalina.sh version
   ```

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/tomcat-version.png)

   2）更改 Tomcat 配置文件并启动 Tomcat：

   ```bash
   $ vim /usr/local/apache-tomcat-9.0.30/conf/server.xml
     ...
     <Connector port="8080" protocol="HTTP/1.1"
                connectionTimeout="20000"
                redirectPort="8443">
     ...           
   # 可自定义 Tomcat 监听的端口号，默认为 http/8080 端口与 https/8443 端口。
   
   $ tomcat-jenkins/bin/startup.sh && ps aux | grep java
   # 启动 Tomcat
   # 登陆 Web 验证：http://jenkins.domain12.example.com:8080
   # 注意：
   #   1. 以 ciadmin 用户身份启动 Tomcat
   #   2. $ tomcat-jenkins/bin/shutdown.sh：停止Tomcat
   #   3. 若缺少 Java 运行环境，Tomcat 无法正常启动。
   ```

8. Jenkins 部署：

   ```bash
   $ su - ciadmin     
   $ cp jenkins-2.204.2.war /usr/local/apache-tomcat-9.0.30/webapps
   # 拷贝 Jenkins war 包至 Tomcat webapps 目录中
   # 重新启动 Tomcat 时，Jenkins war 包将自动解包，并以独立的应用运行。
   # 注意：
   #   $ unzip jenkins-2.204.2.war -d <directory>：解包 war 包至指定目录中
   
   $ ls -lh /usr/local/apache-tomcat-9.0.30/webapps
   # webapps 中的默认目录对应 Tomcat Web 页面中的各链接
   # 可创建独立的目录用于存放不同的 Java Web 项目
   ```

   <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-deploy-1.png" style="zoom:150%;" />

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-deploy-2.png)

   > ✅ 注意：
   >
   > 1. 由于 Tomcat 以 ciadmin 用户运行，因此 Jenkins 的工作目录位于 ciadmin 用户家目录中。
   >
   > 2. Jenkins 的工作目录以隐藏文件的方式存储：/home/ciadmin/.jenkins
   >
   >    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-workdir.png)

9. 初始化 Jenkins：

   1）登录 Jenkins Web：http://jenkins.domain12.example.com:8080/jenkins

   2）输入 Web 页面中指定的密码以完成首次登录解锁。

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-initial.jpg)

   3）安装插件：

      a. 默认情况下，Jenkins 安装推荐的插件。

      b. 在离线环境下，可选择第二项不安装任何插件，根据实际需求再安装插件。

      c. 该示例中使用推荐的插件安装。

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-plugins-install-1.jpg)

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-plugins-install-2.jpg)

   4）创建 Jenkins 管理员用户：admin

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-admin-user.jpg)

   5）配置 Jenkins Web 管理地址，并完成初始化。

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-initial-complete-1.jpg)

   ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-initial-complete-2.jpg)

10. Jenkins Pipeline 构建与部署：

    1）确认已安装相关插件：Git、GitLab、Pipeline

    2）如下所示：Manage Jenkins -> Manage Plugins -> Installed

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/manage-plugins-1.png)

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/manage-plugins-2.png)

    3）创建流水线（pipeline）类型的新项目：tomcat-java-demo

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-pipeline-project-1.png)

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-pipeline-project-2.png)

    4）配置新建项目的构建 pipeline：

       a. 点击项目中的 `Configure` 选项。

       b. 将 GitLab 中 devuser 用户 tomcat-jenkins-demo 仓库中的 `Jenkinsfile` 拷贝至 `Script` 中。

    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-pipeline-config.png)

       c. Jenkins 拉取 GitLab 代码必须使用 SSH 免密码方式实现。
    
    ​     👉 使用 `Pipeline Syntax` 链接可生成 GitLab 中代码仓库的相应指令。
    
    ​     👉 该示例以 godev 用户 SSH 免密登录 GitLab 的 devuser用户，拉取 tomcat-jenkins-demo 仓库的代码。
    
    ​     👉 需预先将 godev 用户的 SSH 公钥添加至 GitLab devuser 用户的 `SSH keys` 中。
    
    <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/passwordless-login-gitlab.png"  />

    ​     👉 将 godev 用户的 `SSH 私钥`添加至 `Jenkins 全局认证凭据`中，若认证失败直接显示报错信息。
    
    ​     👉 godev 用户认证通过后，可使用 `Generate Pipeline Script` 按钮生成 pipeline 语句。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/generate-pipeline-1.png)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/generate-pipeline-2.png)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/generate-pipeline-3.png)

       d. Jenkins 主机配置 insecure registry：

    ​     👉 该示例中 Jenkins 主机用于部署 Podman 容器。
    
    ​     👉 由于 Harbor 容器镜像仓库为 `insecure registry`，因此需配置 Podman 对应的 registry。
    
    ​     👉 该 insecure registry 必须使用 FQDN，否则 Podman 登录报错！
    
    ​     👉 若 Podman 未指定 insecure registry，将身份验证失败无法登录 Harbor 容器镜像仓库。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/podman-insecure-registry.png)
    
       e. 执行构建（build）：
    
    ​     👉 保存 pipeline 配置后，选中该项目即可执行构建，选择 `Build Now`。
    
    ​     👉 构建过程中若出现报错将中止构建，并可选择不同阶段（stage）查看构建日志。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/pipeline-build-1.png)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/pipeline-build-2.png)
    
       f. 查看及验证构建与部署结果：该示例将 tomcat-java-demo 容器部署于 Jenkins 主机上，并使用 podman 运行。 
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/pipeline-build-result.png)
    
       g. 查看 Podman 容器与 Web 应用状态：
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/podman-container-status-1.png)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/podman-container-status-2.png)
    
       h. pipeline script（Jenkinsfile）示例说明：必须登录 Harbor 容器镜像仓库才能拉取或推送镜像！
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/gitlab-pipeline.png)

