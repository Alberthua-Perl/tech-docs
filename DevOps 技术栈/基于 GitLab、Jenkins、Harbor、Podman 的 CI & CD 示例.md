## 基于 GitLab、Jenkins、Harbor、Podman 的 CI/CD 示例

### 文档说明：

- 该文档使用 GitLab、Jenkins、Harbor 与 Podman 容器运行时构建流水线管道以演示如何构建与发布容器化应用。

### 文档目录：

- 示例环境说明

- GitLab 代码仓库的用户说明

- Harbor 容器镜像仓库部署

- Jenkins CI 环境集成与应用发布

- 参考链接

### 示例环境说明：

- 各组件版本：
  
  - OS 版本：CentOS Linux release 7.4.1708 (Core)
  
  - Docker 版本：docker-ce-19.03.1-3.el7.x86_64
  
  - Docker-Compose 版本：docker-compose-1.24.1
  
  - Harbor 版本：harbor-offline-installer-v1.8.1
  
  - Java 版本：jdk-8u191-linux-x64
  
  - Tomcat 版本：apache-tomcat-9.0.30
  
  - Jenkins 版本：jenkins-2.204.2
  
  - Podman 版本：podman-1.4.4-4.el7.centos.x86_64

- 各节点硬件资源与角色：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/node-hardware-role.png)
  
  - 192.168.0.111 节点：仅作为 Git 推送代码至 GitLab 代码仓库用
  
  - 192.168.0.110 节点：GitLab 代码仓库、Harbor 容器镜像仓库
  
  - 192.168.0.108 节点：Jenkins 平台、Podman 容器引擎

- DevOps 生态图谱：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/devops-tools.jpg)

- 该示例架构：
  
  该架构中未使用 Quay、未集成 Kubernetes 或 OpenShift 集群部署应用容器。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-based-devops-cicd-demo.jpg)

### 🦊 GitLab 代码仓库的用户说明：

- 用户相关说明：
  
  - GitLab 中的开发者用户：`devuser`
  
  - 代码提交用户：cloud-ctl 节点的 `godev` 用户
  
  - 此处 godev 用户可使用其 SSH 公钥以 devuser 用户身份免密码登录 GitLab。
  
  - 需预先将 godev 用户的 SSH 公钥添加至 GitLab devuser 用户的 `SSH keys` 中。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/passwordless-login-gitlab.png)
  
  - godev 用户还将被作为 Jenkins 从 GitLab 中拉取代码的用户。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/devuser-gitlab-user.jpg)

- 该 [示例](https://github.com/Alberthua-Perl/jenkins-pipeline-demo/tree/master/tomcat-jenkins-demo) 中使用的代码结构如下所示：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/tomcat-jenkins-demo.png)

### 🐳 Harbor 容器镜像仓库部署：

- 该示例中使用 Harbor 容器镜像仓库作为私有 registry。

- 虽然可用 Red Hat Quay 公共容器镜像仓库，但在 Jenkins 拉取或推送容器镜像时由于超时而导致 `pipeline` 构建失败。

- Harbor 容器镜像仓库与 GitLab 代码仓库都部署于 192.168.0.110 节点。

- Harbor 使用 `docker-compose` 启动并管理，因此需安装 `Docker` 与 `Docker-Compose`。

- Docker 与 Docker-Compose 部署：
  
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

- 配置与启动 Harbor 容器镜像仓库：
  
  - 配置 Harbor 容器镜像仓库：
    
    ```bash
    $ sudo tar -zxvf harbor-offline-installer-v1.8.1.tgz -C /usr/local
    $ sudo vim /usr/local/harbor-v1.8.1/harbor.yml
    # 更改 Harbor 配置文件中默认参数
    ```
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-conf-yaml.png)
  
  - 安装 Harbor 各容器组件：
    
    ```bash
    $ sudo /usr/local/harbor-v1.8.1/install.sh
    # 启动 Harbor 容器镜像仓库服务，各组件以容器方式运行。
    ```
    
    <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-install-1.png" style="zoom:150%;" />
    
    <img src="https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-install-2.png" style="zoom:150%;" />
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-install-3.png)
  
  - 💥 注意：
    
    若 /usr/local/harbor-v1.8.1/harbor.yml 配置文件中指定非法的 http 端口，nginx 容器将启动失败报错！
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-nginx-error.jpg)

- 查看 Harbor 容器镜像仓库状态：
  
  ```bash
  $ sudo docker-compose -f /usr/local/harbor-v1.8.1/docker-compose.yml ps
  # 查看 docker-compose 启动的 Harbor 容器组件与状态
  ```
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-registry-status.jpg)

- Web 登录验证：http://harbor.domain12.example.com:8880
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/harbor-web-login.jpg)

- 停止 Harbor 容器镜像仓库：
  
  ```bash
  $ sudo docker-compose -f /usr/local/harbor-v1.8.1/docker-compose.yml stop
  # 停止 Harbor 容器镜像仓库，即停止 docker-compose 服务。
  ```

### 🚀 Jenkins CI 环境集成与应用发布：

- Jenkins 的多种部署方式：容器镜像、`rpm` 软件包、`war` 包

- Java Web 工程都是打成 `war` 包（一种打包格式）进行发布，打成 war 包的好处是不会缺少目录，并且只管理好一个发布文件就好，并且 Tomcat 服务器能够自动识别，将 war 包放在 Tomcat 的 `webapps` 下，启动服务即可运行该项目，该 war 包会自动解压出一个同名的目录。

- 在创建 Web 项目的时候要选择打包的方式，一般都是热部署（war explode）。

- war 包的结构：一个 Web 项目编译后的结果

- war 包的目录结构：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/java-war-package-structure.png)

- 该示例中使用 `Apache Tomcat` 运行 Jenkins war 包的方式进行部署。

- 该示例中仅使用 `Podman` 容器展示 `JSP Web` 静态页面，并未使用 Java 源代码构建与测试 Web 应用，因此可不安装 `Apacpe Maven`。

- ✨ 生产环境中多以源代码方式构建发布 Java 应用，需使用 Maven 构建打包为 war 包后再进行部署。

- 由于需要运行 Tomcat，因此需安装 Java 运行环境。虽然该示例中未安装 Maven，但该 Jenkins 版本所对应的 Maven 版本可在 Jenkins 安装成功后进行查看，保证两者的兼容性，即 `Manage Jenkins -> Global Tool Configuration -> Maven`。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-maven-compatibility.png)

- Java 环境部署：
  
  ```bash
  $ su - ciadmin
  # 该用户为 Jenkins 管理用户（具有 sudo 权限）
  $ sudo tar -zxvf jdk-8u191-linux-x64.tar.gz -C /usr/local
  $ sudo vim /etc/profile
  # 编辑系统全局环境配置文件，添加 Java 环境变量，即 JAVA_HOME、JRE_HOME。
  ```
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/java-env-var.png)
  
  ```bash
  $ source /etc/profile
  $ java -version
  ```
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/java-version.png)

- Tomcat 部署：
  
  - Apache Tomcat 软件压缩包解包：
    
    ```bash
    $ su - ciadmin
    $ sudo tar -zxvf apache-tomcat-9.0.30.tar.gz -C /usr/local
    $ ln -s /usr/local/apache-tomcat-9.0.30 tomcat-jenkins
    $ tomcat-jenkins/bin/catalina.sh version
    ```
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/tomcat-version.png)
  
  - 更改 Tomcat 配置文件并启动 Tomcat：
    
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

- Jenkins 部署：
  
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
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-deploy-1.png)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-deploy-2.png)
  
  由于 Tomcat 以 ciadmin 用户运行，因此 Jenkins 的工作目录位于 ciadmin 用户家目录中。Jenkins 的工作目录以隐藏文件的方式存储 `/home/ciadmin/.jenkins`。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-workdir.png)

- 初始化 Jenkins：
  
  - 登录 Jenkins Web：http://jenkins.domain12.example.com:8080/jenkins
  
  - 输入 Web 页面中指定的密码以完成首次登录解锁。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-initial.jpg)
  
  - 安装插件：
    
    - 默认情况下，Jenkins 安装推荐的插件。
    
    - 在离线环境下，可选择第二项不安装任何插件，根据实际需求再安装插件。
    
    - 该示例中使用推荐的插件安装。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-plugins-install-1.jpg)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-plugins-install-2.jpg)
    
    - 创建 Jenkins 管理员用户：admin
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-admin-user.jpg)
    
    - 配置 Jenkins Web 管理地址，并完成初始化。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-initial-complete-1.jpg)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-initial-complete-2.jpg)

- Jenkins Pipeline 构建与部署：
  
  - 确认已安装相关插件：Git、GitLab、Pipeline
  
  - 如下所示：Manage Jenkins -> Manage Plugins -> Installed
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/manage-plugins-1.png)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/manage-plugins-2.png)
  
  - 创建流水线（pipeline）类型的新项目：tomcat-java-demo
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-pipeline-project-1.png)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-pipeline-project-2.png)
  
  - 配置新建项目的构建 pipeline：
    
    - 点击项目中的 `Configure` 选项。
    
    - 将 GitLab 中 devuser 用户 tomcat-jenkins-demo 仓库中的 `Jenkinsfile` 拷贝至 `Script` 中。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/jenkins-pipeline-config.png)
    
    - ✨ Jenkins 用户使用 SSH 免密登录的认证方式拉取 GitLab 代码，并且可使用 `Pipeline Syntax` 链接生成 GitLab 中代码仓库的相应指令。
    
    - 该示例以 godev 用户 SSH 免密登录至 GitLab devuser 用户，以拉取 tomcat-jenkins-demo 仓库的代码。
      
      将 godev 用户的 **SSH 私钥** 添加至 **Jenkins 全局认证凭据** 中，若认证失败直接显示报错信息。godev 用户认证通过后，可使用 `Generate Pipeline Script` 按钮生成 pipeline 语句。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/generate-pipeline-1.png)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/generate-pipeline-2.png)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/generate-pipeline-3.png)
    
    - Jenkins 主机配置 insecure registry：
      
      由于 Harbor 容器镜像仓库为 `insecure registry`，因此需配置 Podman 对应的 registry。
      
      💥 该 insecure registry 必须使用 FQDN，否则 Podman 登录报错！
      
      💥 若 Podman 未指定 insecure registry，将身份验证失败无法登录 Harbor 容器镜像仓库。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/podman-insecure-registry.png)
    
    - 执行构建（build）：
      
      保存 pipeline 配置后，选中该项目即可执行构建，选择 `Build Now`。
      
      构建过程中若出现报错将中止构建，并可选择不同阶段（stage）查看构建日志。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/pipeline-build-1.png)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/pipeline-build-2.png)
    
    - ✅ 查看及验证构建与部署结果：
      
      该示例将 tomcat-java-demo 容器部署于 Jenkins 主机上，并使用 podman 运行。 
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/pipeline-build-result.png)
    
    - 查看 Podman 容器与 Web 应用状态：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/podman-container-status-1.png)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/podman-container-status-2.png)
    
    - pipeline script（Jenkinsfile）示例说明：
      
      必须登录 Harbor 容器镜像仓库才能拉取或推送镜像！
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/jenkins-cicd-demo/gitlab-pipeline.png)

### 参考链接：

- [war 包的目录结构](https://blog.csdn.net/jxq0816/article/details/46775769)

- [GitHub 与 GitLab 原理与使用配置](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/GitHub%20%E4%B8%8E%20GitLab%20%E5%8E%9F%E7%90%86%E4%B8%8E%E4%BD%BF%E7%94%A8%E9%85%8D%E7%BD%AE.md)
