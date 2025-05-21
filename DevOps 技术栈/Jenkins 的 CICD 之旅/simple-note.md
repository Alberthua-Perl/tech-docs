# Jenkins 的 CI/CD 之旅简易大纲

## Jenkins Part1

- Web Server：
  - http 协议接入客户端请求
  - 调用 API
  - 使用 Web UI
- 高度插件化：
  - 插件 Hub
  - 动态管理：Jenkins 运行过程中管理插件
  - 流水线的定义高度依赖于插件
- 流水线定义：支持多种风格
  - Freestyle Job
  - Pipeline Job
  - Maven Job
  - Multibranch Pipeline Job
  - ......
- 分布式构建：
  - master/agent
  - master：定义管理流水线
  - agent：执行流水线

        

    1.x, 2.x

    安装部署Jenkins：
        war文件，由Servlet Container负责运行
            tomcat
            jetty 
        docker-compose 
            docker container agent 
        kubernetes编排
            Pod agent 

    Jenkins： 
        代码仓库：GitLab 
        代码质量评估：SonarQube Server 
        制品库：Harbor
        部署环境：Kubernetes
            Deployment： 滚动发布
        ArgoCD：GitOps
            Rollouts：GitOPs
            Prometheus：渐进式交付


1. 功能：CI/CD Server
2. 运行环境：Java, JDK, Servlet Container
3. 稳定的 Java 版本：JDK 11, JDK 17
4. 每个 Jenkins 的代理节点（agent）上具有执行器（executor），执行器的数量代表了作业（job）并发执行的能力。官方建议，单个代理节点

    Freestyle作业步骤:
        不同的步骤类型, 他们加载所需工具的逻辑有所主别
            执行shell:
                依赖的工具, 源自于Jenkins所在的操作系统环境
                这些工具, 可能依赖于管理手动部署

            其它类型的步骤:
                调用Jenkins定义的全局构建工具:
                    支持自动安装
                    安装位置:$JENKINS_HOME/tools/

    思考:
        1、如何为golang、python、npm提供全局构建环境?
        2、如何Jenkins请求下线tomcat上运行中的web app?


        
---
<mirror>
    <id>nexus-aliyun</id>
    <mirrorOf>central</mirrorOf>
    <name>Nexus aliyun</name>
    <url>http://maven.aliyun.com/nexus/content/groups/public</url>
</mirror>



作业: 
    1、freestyle完成helloworldJSP应用的构建和部署;
        (1) shell构建
        (2) 调用顶层maven目标
        (3) 使用maven job 
    2、部署到单个tomcat环境;部署多个tomcat环境;
    3、扩展:使用ansible插件, 调用ansible完成应用部署;

Jenkins:
    CICD Server 
        帮助用户定制流水线
    工具：依赖系统环境、全局工具配置
    高度插件化：
        社区推荐的插件

    定制流水线：
        多种Job类型：
            Freestyle：自由风格作业
                配置接口：UI

                定制构建步骤：
                    执行shell命令
                        mvn clean package test verify
                    调用顶层Maven目标
            Pipeline：流水线风格作业
                配置接口：
                    脚本式流水线
                    声明式流水线
            Maven Job： 

    Freestyle:
        流水线： 
            Spring-boot-helloworld, maven工程, ...

            克隆 --> 构建 --> 测试 --> [Nexus（推送）]--> 部署 

            构建：mvn -B -DskipTests clean package 
            测试：mvn test

        Workspace：
            $JENKINS_HOME/workspaces/JOB_NAME/

    环境变量：
        http://jenkins.magedu.com:8080/env-vars.html/

        
    创建一个SerivceAccount
        授权该账户
            hello admin 

    部署到Kubernetes上：
        kubeApiServer 
            kubeapi.magedu.com --> kubeapi.magedu.com:6443 --> https://kubeapi.magedu.com:6443

            校验Server Cert，跳过校验步骤 

        kubectl命令
            部署客户端程序

                --insecure-skip-tls-verify=true 

        认证到kubeApiServer
            ServiceAccount 
                名称空间:hello 
                SA： hello-admin 

                rolebinding:
                    ClusterRole: Cluster-Admin

        hub.magedu.com/ikubernetes/spring-boot-helloworld:latest


    
    #!/bin/bash
    sed -i "s@__IMAGE__@${registry}/ikubernetes/${JOB_NAME}:${imageTag}@" deploy/all-in-one.yaml
    kubectl --server ${kubeApiServer} --insecure-skip-tls-verify=${skipTLSCertVerify} --token=${serviceAccountToken} apply -f deploy/all-in-one.yaml

    作业：  
        1、完成spring-boot-helloworld的流水线：构建、测试、打包、制作镜像、推送镜像及问过到Kubernetes集群；
        2、基于Git Parameter为每次构建的镜像生成一个Tag，让发生变更的分支成流水线处理分支，或者让发生变更的Rivision，成为构建的目标Rivision；
        3、配置GitLab可通过GitLab Trigger或GWT触发freestyle作业
        4、配置通知机制：
            Email/Extended Email 
            DingTalk
            Qy Wechat


    分支： 
        master、develop 
            master：发布历史 
                hotfix 
            develop：变更历史 
                变更：由feature分支合并生成
                feature
                    feature/x, feature/y, feature/z
                release
             

    glpat-_vpQT5Gb7pzkx3Sxhtwu


    curl -X POST -H "Content-Type: application/json" -d '{ "ref": "refs/heads/develop" }' -vs http://jenkins.magedu.com:8080/generic-webhook-trigger/invoke?token=YWTVmiT1eU2cDhzTiVIf2jvaVNzBMP2dJ2FEUykyvw8     