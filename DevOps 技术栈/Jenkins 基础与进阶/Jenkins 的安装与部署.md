# Jenkins 的安装与部署

## 文档说明

- OS 版本：CentOS Linux release 7.4.1708 (Core)
- Jenkins 版本：jenkins-2.204.2-1.1.noarch

## 文档目录

- Jenkins 基本概念与架构
- Jenkins 安装与配置
- [参考链接](#参考链接)

## Jenkins 基本概念与架构

- Jenkins 2 的特性：
  - 用于泛指支持流水线即代码及其他类似 `Jenkinsfile` 等新特性的新版 Jenkins，`DSL` 是 Jenkins 2 的核心组件，作为构建模块让其他核心的用户导向特性成为可能。
  - 支持将原本写在 Jenkins 中的代码提取到一个 Jenkinsfile 文件中，从而以一种更加结构化的方法来创建工作流 —— 声明式流水线，它比传统的脚本式流水线有着更加清晰、可预期的结构，以及更强大的 DSL 元素和结构体；
  - 具有称为 `Blue Ocean` 的全新用户界面，为流水线的每个阶段添加了图形化展示，能够显示成功/失败和进展等标识，并对每个任务都提供了点选式日志查看功能。
  - 组织：用于支持源码控制平台（SCM）上将多个代码库聚合成 “组织” 的功能，目前已经支持 Github 和 Bitbucket 平台上的 “组织” 功能，未来将会逐步支持其他的平台。
  - 👍 多分支流水线：在有着 Jenkinsfile 的项目中，若创建了一个新的分支，Jenkins 将自动基于这个新分支创建一个新项目，此类型的项目可应用于任何Git或SVN代码库。
- Jenkins 最大的优势在于具有众多的 **插件**（plugin），实际工作的是插件。
- Jenkins 使用插件的源位于国外服务器，下载更新速度很慢，可更换为国内清华的插件源。
- Jenkins 更换插件源：
  - `Manage Jenkins` > `Manage Plugins` > `Advanced`
  - [清华的插件源](https://mirrors.tuna.tsinghua.edu.cn/jenkins/updates/update-center.json)：

    ![jenkins-plugins-repo.jpg](images/jenkins-plugins-repo.jpg)

## Jenkins 的安装方式

- Jenkins 具有两种不同的软件发行周期：
  - 长期稳定版（`LTS`）：每12周更新一次，每4周发布 bug 修复与安全补丁加固。
  - 每周更新版：每周更新一次以获得软件的最新特性
- 使用长期稳定版的 RPM 方式安装：
  
  ![rpm-install-jenkins.jpg](images/rpm-install-jenkins.jpg)
  
  该安装方式可参考 [该文档链接](https://github.com/Alberthua-Perl/tech-docs/blob/master/%E5%9F%BA%E4%BA%8E%20Jenkins%20%E7%9A%84%20DevOps%20CICD%20%E9%83%A8%E7%BD%B2%E7%A4%BA%E4%BE%8B.md)。

## 参考链接

- [Download and deploy | Jenkins Docs](https://www.jenkins.io/download/#download-and-deploy)
