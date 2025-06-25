# 🧪 基于 Ansible Navigator 部署管理分布式 Jenkins CI/CD 平台 —— 构建发布容器化 Node.js、Python、Java 应用

## 文档说明

- ✍ 本文档使用 Ansible Navigator 自动化部署 Jenkins 分布式构建平台（Master+Agent），并发布 Java、Node.js、Flask 与 Golang 应用。
- 📚 文档项目源代码：[jenkins-ci-plt | GitHub](https://github.com/Alberthua-Perl/ansible-demo/tree/master/jenkins-ci-plt)

## 文档目录

- [0. Lab 环境架构与实现](#0-lab-环境架构与实现)
- [1. Lab 相关用户名与登录密码](#1-lab-相关用户名与登录密码)
- [2. Foundation 中设置节点资源](#2-foundation-中设置节点资源)
  - [2.1 调整虚拟机计算资源](#21-调整虚拟机计算资源)
  - [2.2 配置 classroom 可实现外网连接](#22-配置-classroom-可实现外网连接)
  - [2.3 创建与附加 raw 磁盘镜像](#23-创建与附加-raw-磁盘镜像)
- [3. 注意 Ansible 清单文件中的 ansible 变量 ansible_host](#3-注意-ansible-清单文件中的-ansible-变量-ansible_host)
- [4. 生成 devops 用户 SSH 公私钥并实现各节点免密登录](#4-生成-devops-用户-ssh-公私钥并实现各节点免密登录)
- [5. 部署与设置 GitLab-CE 容器](#5-部署与设置-gitlab-ce-容器)
  - [5.1 安装 Ansible 内容集合（collection）](#51-安装-ansible-内容集合collection)
  - [5.2 部署 GitLab-CE 容器](#52-部署-gitlab-ce-容器)
  - [5.3 重置 GitLab-CE 的 root 密码](#53-重置-gitlab-ce-的-root-密码)
  - [5.4 创建与批准 GitLab-CE 的 devuser0 开发者用户](#54-创建与批准-gitlab-ce-的-devuser0-开发者用户)
  - [5.5 Node.js 应用导入](#55-nodejs-应用导入)
    - [5.5.1 创建新项目 etherpad-lite-postgres](#551-创建新项目-etherpad-lite-postgres)
    - [5.5.2 导入 etherpad-lite-postgres 外部代码库](#552-导入-etherpad-lite-postgres-外部代码库)
  - [5.6 Python 应用导入](#56-python-应用导入)
    - [5.6.1 创建新项目 cnn_mnist_train](#561-创建新项目-cnn_mnist_train)
    - [5.6.2 导入 cnn_mnist_train 外部代码库](#562-导入-cnn_mnist_train-外部代码库)
  - [5.7 Java 应用导入](#57-java-应用导入)
    - [5.7.1 创建新项目 spring-boot-helloworld](#571-创建新项目-spring-boot-helloworld)
    - [5.7.2 导入 spring-boot-helloworld 外部代码库](#572-导入-spring-boot-helloworld-外部代码库)
- [6. 部署与设置 Nexus3 容器](#6-部署与设置-nexus3-容器)
  - [6.1 部署 Nexus3 容器](#61-部署-nexus3-容器)
  - [6.2 创建 Nexus3 的 devuser0 用户](#62-创建-nexus3-的-devuser0-用户)
  - [6.3 创建 Nexus3 的容器镜像仓库（hosted 类型）](#63-创建-nexus3-的容器镜像仓库hosted-类型)
  - [6.4 创建 Nexus3 的 npm 构件仓库（proxy 类型）](#64-创建-nexus3-的-npm-构件仓库proxy-类型)
  - [6.5 创建 Nexus3 的 maven 构件仓库](#65-创建-nexus3-的-maven-构件仓库)
- [7. 部署应用运行及构建环境](#7-部署应用运行及构建环境)
  - [7.1 Node.js 运行环境](#71-nodejs-运行环境)
  - [7.2 Maven 构建环境](#72-maven-构建环境)
  - [7.3 使用 spring-boot 应用测试 maven (group) 类型构件库](#73-使用-spring-boot-应用测试-maven-group-类型构件库)
- [8. 部署与设置 PostgreSQL 数据库](#8-部署与设置-postgresql-数据库)
  - [8.1 安装部署 PostgreSQL 数据库](#81-安装部署-postgresql-数据库)
  - [8.2 数据库服务器中创建 etherpad-lite 应用相关用户与数据库](#82-数据库服务器中创建-etherpad-lite-应用相关用户与数据库)
- [9. 部署 Jenkins Master 服务](#9-部署-jenkins-master-服务)
- [10. 运行自由风格的作业（Free Style Project）](#10-运行自由风格的作业free-style-project)
  - [10.1 Node.js 应用 —— 构建测试 etherpad-lite 应用及容器镜像](#101-nodejs-应用--构建测试-etherpad-lite-应用及容器镜像)
    - [10.1.1 创建基于 SSH 私钥的凭据连接 GitLab-CE](#1011-创建基于-ssh-私钥的凭据连接-gitlab-ce)
    - [10.1.2 安装 Jenkins 的 Blue Ocean 插件](#1012-安装-jenkins-的-blue-ocean-插件)
    - [10.1.3 jenkins 用户的 SSH 连接代码库的主机密钥校验与配置](#1013-jenkins-用户的-ssh-连接代码库的主机密钥校验与配置)
    - [10.1.4 设置 jenkins 用户的 subuid/subgid 以满足 podman 的 rootless 构建环境](#1014-设置-jenkins-用户的-subuidsubgid-以满足-podman-的-rootless-构建环境)
    - [10.1.5 构建与推送 node-pnpm 容器镜像](#1015-构建与推送-node-pnpm-容器镜像)
    - [10.1.6 创建与运行作业](#1016-创建与运行作业)
  - [10.2 Python 应用 —— 训练 CNN 模型、构建 app-tf-flask 应用及推理容器镜像](#102-python-应用--训练-cnn-模型构建-app-tf-flask-应用及推理容器镜像)
    - [10.2.1 推送 tf-flask 构建用容器镜像](#1021-推送-tf-flask-构建用容器镜像)
    - [10.2.2 安装 AnsiColor 插件](#1022-安装-ansicolor-插件)
    - [10.2.3 创建与运行作业](#1023-创建与运行作业)
  - [10.3 Java 应用 —— 构建测试 spring-boot 应用及容器镜像](#103-java-应用--构建测试-spring-boot-应用及容器镜像)
    - [10.3.1 推送 openjdk-17 构建用容器镜像](#1031-推送-openjdk-17-构建用容器镜像)
    - [10.3.2 创建与运行作业](#1032-创建与运行作业)
- [11. 设置 Jenkins 分布式构建环境](#11-设置-jenkins-分布式构建环境)
  - [11.1 JNLP 连接方式](#111-jnlp-连接方式)
    - [11.1.1 调整 Master 支持 JNLP 的 Agent](#1111-调整-master-支持-jnlp-的-agent)
    - [11.1.2 添加支持 JNLP 的 Agent](#1112-添加支持-jnlp-的-agent)
    - [11.1.3 使用流水线风格作业测试 Agent 功能](#1113-使用流水线风格作业测试-agent-功能)
  - [11.2 SSH 连接方式](#112-ssh-连接方式)
    - [11.2.1 调整 Master 支持 SSH 连接 Agent](#1121-调整-master-支持-ssh-连接-agent)
- [附录A. PostgreSQL 常用命令](#附录a-postgresql-常用命令)
  - [A.1 登录数据库](#a1-登录数据库)
  - [A.2 更新数据库管理员 postgres 密码](#a2-更新数据库管理员-postgres-密码)
  - [A.3 用户（角色）与数据库操作](#a3-用户角色与数据库操作)
  - [A.4 psql 常用的元命令](#a4-psql-常用的元命令)
- [参考链接](#参考链接)

## 0. Lab 环境架构与实现

![ansible-navigator-jenkins-nodejs-python-java.png](images/ansible-navigator-jenkins-nodejs-python-java.png)

## 1. Lab 相关用户名与登录密码

| 节点角色 | 管理员用户 | 管理员密码 | 普通用户 | 普通用户密码 |
| ----- | ----- | ----- | ----- | ----- |
| GitLab CE | root | 1qazZSE$ | devuser0 | 1qazZSE$ |
| Nexus3 | admin | admin123 | devuser0 | 1qazZSE$ |
| Jenkins Master | admin | 1qazZSE$ | devuser0 | 1qazZSE$ |
| RedHat AAP2.2 | admin | redhat | NA | NA |

## 2. Foundation 中设置节点资源

### 2.1 调整虚拟机计算资源

- 本实验使用 RedHat RH294v9.0 实验环境，各节点的资源调整如下：

  > 说明：如果读者具有相应的计算与存储资源也可根据以下资源需求构建自定义实验环境。

  | 主机名 | 主机别名 | IPv4 | vCPU | 内存 | 节点角色 |
  | ----- | ----- | ----- | ----- | ----- | ----- |
  | foundation0.ilt.example.com | NA | 172.25.254.250 | 8 | 48 | 虚拟机宿主机 & 应用部署 |
  | workstation.lab.example.com | gitlab-ce.lab.example.com | 172.25.250.9 | 8 | 8 | GitLab CE 容器 |
  | servera.lab.example.com | jenkins-master.lab.example.com | 172.25.250.10 | 4 | 4 | Jenkins Master |
  | serverb.lab.example.com | jenkins-agent0.lab.example.com | 172.25.250.11 | 4 | 4 | Jenkins Agent（JNLP 连接方式）|
  | serverc.lab.example.com | jenkins-agent1.lab.example.com | 172.25.250.12 | 4 | 4 | Jenkins Agent（SSH 连接方式）|
  | serverd.lab.example.com | nexus3.lab.example.com | 172.25.250.13 | 6 | 6 | Nexus3 容器 & PostgreSQL 数据库 |

  > 💥 注意：servera，serverb，serverc，serverd 节点的 qcow2 磁盘镜像由于存储容量的限制在 Jenkins CI 流程中无法满足需求，因此，在本实验环境中进行了重新构建。workstation 节点需根据 2.3 添加新磁盘以满足存储需求。

- 调整每个节点的 vCPU 与内存直接通过在 foundation0 节点上运行以下命令完成：

  ```bash
  [kiosk@foundation0 ~]$ su -  #密码：Asimov
  [root@foundation0 ~]# virsh edit <domain_name>
  ```

### 2.2 配置 classroom 可实现外网连接

```bash
[kiosk@foundation0 ~]$ wget http://content.example.com/jenkins-ci-plt/set_classroom_extnet
[kiosk@foundation0 ~]$ ./set_classroom_extnet
```

### 2.3 创建与附加 raw 磁盘镜像

workstation 节点用作 GitLab-CE 运行节点需要更多的计算资源与较大的存储空间用于存储配置、数据与日志，但由于 workstation 节点的存储空间有限，因此，需要创建额外的 raw 格式磁盘镜像，并将其附加（attach）至此节点上，可参考以下步骤：

```bash
## 注意：
##   1. 为 foundation0 分配一个新磁盘 90G
##   2. 将此磁盘创建为逻辑卷并挂载 /mnt/vmdisk，此目录用于存储 workstation 的额外存储。
[root@foundation0 ~]# cd /mnt/vmdisk  #切换至已挂载逻辑卷的目录中
[root@foundation0 vmdisk]# qemu-img create -f raw /mnt/vmdisk/workstation-vdb-10G.raw 10G
[root@foundation0 vmdisk]# chown qemu:qemu /mnt/vmdisk/workstation-vdb-10G.raw  #更改为 qemu 用户与用户组
[root@foundation0 vmdisk]# virsh attach-disk workstation --source /mnt/vmdisk/workstation-vdb-10G.raw --target vdb --persistent --cache none  #--persistent 选项：持久化添加 vdb 磁盘（否则关机后将自动删除此磁盘）

## 登录 devops@workstation 执行
[devops@workstation ~]$ sudo pvcreate /dev/vdb
[devops@workstation ~]$ sudo vgcreate vgscmdata /dev/vdb
[devops@workstation ~]$ sudo lvcreate -l+100%FREE vgscmdata -n lvscmdata
[devops@workstation ~]$ sudo mkfs.xfs /dev/mapper/vgscmdata-lvscmdata
[devops@workstation ~]$ sudo mkdir /home/devops/middleware
[devops@workstation ~]$ sudo vim /etc/fstab
...
/dev/mapper/vgscmdata-lvscmdata  /home/devops/middleware  xfs  defaults  0 0
...
[devops@workstation ~]$ sudo systemctl daemon-reload
[devops@workstation ~]$ sudo mount -a
[devops@workstation ~]$ sudo mkdir -p /home/devops/middleware/gitlab-ce/{data,config,logs}
[devops@workstation ~]$ sudo chown -R devops:devops /home/devops/middleware
[devops@workstation ~]$ sudo chmod -R 0777 /home/devops/middleware
[devops@workstation ~]$ tree /home/devops/middleware
/home/devops/middleware/
└── gitlab-ce
    ├── config
    ├── data
    └── logs
# 注意：以上 gitlab-ce 容器相关的映射目录创建后，在 ansible playbook 中无需再次创建！    
```

## 3. 注意 Ansible 清单文件中的 ansible 变量 ansible_host

- 在 `ini` 或 `yaml` 格式的主机清单文件中使用 `ansible_host` 变量：

  ```ini
  ...
  [master]
  jenkins-master.lab.example.com  ansible_host=servera.lab.example.com
  #inventory_hostname             #ansible_host
  ...
  ```

- Ansible 使用 `ansible_host` 变量指定的真实主机进行 SSH 连接, 而 `inventory_hostname` 变量仅仅在 Ansible 运行返回中显示。因此，需要将以上两个变量指定的>主机名称均写入 `/etc/hosts` 文件中完成主机名解析（笔者环境中使用），当然也可加入上游 DNS 服务器的解析文件中实现。

## 4. 生成 devops 用户 SSH 公私钥并实现各节点免密登录

```bash
[devops@workstation ~]$ ssh-keygen 
Generating public/private rsa key pair.
Enter file in which to save the key (/home/devops/.ssh/id_rsa): /home/devops/.ssh/devops-jenkins    
Created directory '/home/devops/.ssh'.
Enter passphrase (empty for no passphrase): 
Enter same passphrase again: 
Your identification has been saved in /home/devops/.ssh/devops-jenkins
Your public key has been saved in /home/devops/.ssh/devops-jenkins.pub
The key fingerprint is:
SHA256:SInrTKN4O8GL6m2fft4nWo8Bp3teHO2ArBalKC2wx3A devops@foundation0.ilt.example.com
The key's randomart image is:
+---[RSA 3072]----+
|                 |
|     . .         |
| o E. o  .       |
|  * .o..+ . .    |
| o =+o.+S+ o .   |
| .+=o.  * . +    |
|..ooo  + o o .   |
|..+o  o.+o=.     |
|+.ooo+o+++o.     |
+----[SHA256]-----+
# 生成 devops 用户 SSH 公私钥

[devops@workstation ~]$ ls -lh ~/.ssh
total 8.0K
-rw-------. 1 devops devops 2.6K May 27 11:19 devops-jenkins
-rw-r--r--. 1 devops devops  588 May 27 11:19 devops-jenkins.pub

[devops@workstation ~]$ for i in workstation.lab.example.com gitlab-ce.lab.example.com \
  servera.lab.example.com jenkins-master.lab.example.com \
  serverb.lab.example.com jenkins-agent0.lab.example.com \
  serverc.lab.example.com jenkins-agent1.lab.example.com \
  serverd.lab.example.com nexus3.lab.example.com \
  utility.lab.example.com; do ssh-copy-id -i ~/.ssh/devops-jenkins devops@$i; done
```

## 5. 部署与设置 GitLab-CE 容器

### 5.1 安装 Ansible 内容集合（collection）

```bash
[kiosk@foundation0 ~]$ ssh devops@workstation
[devops@workstation ~]$ eval $(ssh-agent)
[devops@workstation ~]$ ssh-add ~/.ssh/devops-jenkins
[devops@workstation ~]$ cd ~/jenkins-ci-plt
[devops@workstation jenkins-ci-plt]$ ansible-galaxy collection install -r collections/requirements.yml -p collections/
[devops@workstation jenkins-ci-plt]$ ansible-galaxy collection list
```

### 5.2 部署 GitLab-CE 容器

```bash
[devops@workstation jenkins-ci-plt]$ sh scm-artifact/deploy-prep.sh
# 部署容器前准备
## 可选命令：$ ansible-playbook --list-tags <playbook_name>.yml  #列举 playbook 中的 tag
[devops@workstation jenkins-ci-plt]$ podman login utility.lab.example.com  #login: admin/redhat
[devops@workstation jenkins-ci-plt]$ podman pull utility.lab.example.com/ansible-automation-platform-22/ee-supported-rhel8:latest
[devops@workstation jenkins-ci-plt]$ ansible-navigator run scm-artifact/prep-scm-nexus3.yml --tag gitlab-ce
# 部署 gitlab-ce 容器（容器启动需 7~8 分钟），容器状态将从 starting -> unhealthy -> healthy。
```

### [5.3 重置 GitLab-CE 的 root 密码](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/GitHub%20%E4%B8%8E%20GitLab%20%E5%9F%BA%E7%A1%80%E4%B8%8E%E5%B8%B8%E8%A7%84%E4%BD%BF%E7%94%A8/GitHub%20%E4%B8%8E%20GitLab%20%E5%9F%BA%E7%A1%80%E4%B8%8E%E5%B8%B8%E8%A7%84%E4%BD%BF%E7%94%A8.md#%E4%BD%BF%E7%94%A8%E5%AE%B9%E5%99%A8%E9%95%9C%E5%83%8F%E9%83%A8%E7%BD%B2)

### [5.4 创建与批准 GitLab-CE 的 devuser0 开发者用户](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/GitHub%20%E4%B8%8E%20GitLab%20%E5%9F%BA%E7%A1%80%E4%B8%8E%E5%B8%B8%E8%A7%84%E4%BD%BF%E7%94%A8/GitHub%20%E4%B8%8E%20GitLab%20%E5%9F%BA%E7%A1%80%E4%B8%8E%E5%B8%B8%E8%A7%84%E4%BD%BF%E7%94%A8.md#%E4%BD%BF%E7%94%A8%E5%AE%B9%E5%99%A8%E9%95%9C%E5%83%8F%E9%83%A8%E7%BD%B2)

- Web 注册界面创建 devuser0 开发者用户
- 设置此用户的 SSH 连接公钥（此处使用 `devops@workstation` 中名为 devops-jenkins.pub 的 SSH 公钥）

### 5.5 Node.js 应用导入

#### 5.5.1 创建新项目 etherpad-lite-postgres

以 devuser0 用户登录代码库创建新项目：

<center><img src="images/gitlab-create-new-project-1.png" style="width:80%"></center>

<center><img src="images/gitlab-create-new-project-2.png" style="width:80%"></center>

<center><img src="images/gitlab-create-new-project-3.png" style="width:80%"></center>

💥 上图中，如果源代码目录中已有 README 文件，可不选择创建此文件。

#### 5.5.2 导入 etherpad-lite-postgres 外部代码库

下载 etherpad-lite-postgres.tar 源代码文件并推送至 GitLab-CE 中。此应用的源代码为 [ether/etherpad-lite | GitHub](https://github.com/ether/etherpad-lite) 项目的 *master* 分支的克隆。源代码目录中新增 `.npmrc` 与 `settings.json` 文件，前者用于 Nexus3 的 npm (proxy) 与 docker (hosted) 仓库的认证连接，后者用于应用运行后与已部署的 PostgreSQL 服务器的连接认证。其中 npm (proxy) 仓库可从外部仓库中缓存 npm 包（如果不存在相应包的话），PostgreSQL 数据库用于应用的连接与数据存储。如下所示：

```bash
### file: .npmrc
### 注意：Nexus3 的资源在 `6. 部署与设置 Nexus3 容器` 中设置完成
registry=http://nexus3.lab.example.com:8881/repository/npm-proxy/
always-auth=true
//nexus3.lab.example.com:8881/repository/npm-proxy/:_auth="ZGV2dXNlcjA6MXFhelpTRSQ="
//nexus3.lab.example.com:8881/repository/npm-hosted/:_auth="ZGV2dXNlcjA6MXFhelpTRSQ="

### settings.json
### 注意：PostgreSQL 服务器资源在 `8. 部署与设置 PostgreSQL 数据库` 中设置完成（通过 ansible playbook 实现）
{
  "dbType": "postgres",  #连接数据库类型
  "dbSettings": {
    "user": "etherpad_user",  #数据库用户
    "host": "serverd.lab.example.com",  #数据库节点
    "port": 5432,  #数据库监听端口
    "password": "redhat",  #数据库用户密码
    "database": "etherpad_db"  #数据库名称（存储 etherpad-lite 应用数据）
  },
  "ip": "0.0.0.0",  #应用监听地址
  "port": 9001,  #应用监听端口
  "title": "Etherpad"
}
```

```bash
[devops@workstation ~]$ wget http://content.example.com/jenkins-ci-plt/code-examples/etherpad-lite-postgres.tar
[devops@workstation ~]$ tar -xf etherpad-lite-postgres.tar
# 下载应用源代码

[devops@workstation ~]$ cd etherpad-lite-postgres
[devops@workstation etherpad-lite-postgres]$ cat > ~/.ssh/config <<EOF
Host gitlab-ce.lab.example.com
  Port 8882
  IdentityFile ~/.ssh/devops-jenkins
EOF
[devops@workstation etherpad-lite-postgres]$ chmod 0644 ~/.ssh/config
[devops@workstation etherpad-lite-postgres]$ ssh -T git@gitlab-ce.lab.example.com
Welcome to GitLab, @devuser0!
# 使用 5.4 中已设置的 devuser0 SSH 公钥登录代码库

[devops@workstation etherpad-lite-postgres]$ git config --global user.name devuser0
[devops@workstation etherpad-lite-postgres]$ git config --global user.email devuser0@lab.example.com
# 设置开发者用户信息

[devops@workstation etherpad-lite-postgres]$ vim .gitignore  #注释 settings.json，否则无法推送此文件配置。
...
#/settings.json
...
[devops@workstation etherpad-lite-postgres]$ git remote set-url origin git@gitlab-ce.lab.example.com:devuser0/etherpad-lite-postgres.git
# 💥 此应用为 GitHub 中应用的克隆，具有原始 github 源地址，因此使用上述命令更新远程源代码仓库地址。
[devops@workstation etherpad-lite-postgres]$ cat .git/config
[core]
        repositoryformatversion = 0
        filemode = true
        bare = false
        logallrefupdates = true
[remote "origin"]
        url = git@gitlab-ce.lab.example.com:devuser0/etherpad-lite-postgres.git  #更新成功
        fetch = +refs/heads/*:refs/remotes/origin/*
[branch "develop"]
        remote = origin
        merge = refs/heads/develop
[branch "master"]
        remote = origin
        merge = refs/heads/master

[devops@workstation etherpad-lite-postgres]$ git push --set-upstream origin --all  #推送分支代码至仓库中
Enumerating objects: 57577, done.
Counting objects: 100% (57577/57577), done.
Delta compression using up to 8 threads
Compressing objects: 100% (14480/14480), done.
Writing objects: 100% (57577/57577), 26.20 MiB | 33.21 MiB/s, done.
Total 57577 (delta 42590), reused 57561 (delta 42583), pack-reused 0
remote: Resolving deltas: 100% (42590/42590), done.
remote:
remote: To create a merge request for develop, visit:
remote:   http://workstation.lab.example.com/devuser0/etherpad-lite-postgres/-/merge_requests/new?merge_request%5Bsource_branch%5D=develop
remote:
remote:
remote: To create a merge request for master, visit:
remote:   http://workstation.lab.example.com/devuser0/etherpad-lite-postgres/-/merge_requests/new?merge_request%5Bsource_branch%5D=master
remote:
To gitlab-ce.lab.example.com:devuser0/etherpad-lite-postgres.git
 * [new branch]          develop -> develop
 * [new branch]          master -> master
Branch 'develop' set up to track remote branch 'develop' from 'origin'.
Branch 'master' set up to track remote branch 'master' from 'origin'.

[devops@workstation etherpad-lite-postgres]$ git push origin --tags  #推送本地 tag 至远程代码库
Enumerating objects: 60, done.
Counting objects: 100% (60/60), done.
Delta compression using up to 8 threads
Compressing objects: 100% (42/42), done.
Writing objects: 100% (60/60), 19.15 KiB | 19.15 MiB/s, done.
Total 60 (delta 18), reused 60 (delta 18), pack-reused 0
To gitlab-ce.lab.example.com:devuser0/etherpad-lite-postgres.git
 * [new tag]             1.0 -> 1.0
 * [new tag]             1.1.1 -> 1.1.1
 * [new tag]             1.1.2 -> 1.1.2
 * [new tag]             1.1.3 -> 1.1.3
 * [new tag]             1.1.4 -> 1.1.4
 * [new tag]             1.1.5 -> 1.1.5
 * [new tag]             1.2.0 -> 1.2.0
 * [new tag]             1.2.1 -> 1.2.1
 * [new tag]             1.2.10 -> 1.2.10
 * [new tag]             1.2.11 -> 1.2.11
 * [new tag]             1.2.12 -> 1.2.12
...
[devops@workstation etherpad-lite-postgres]$ git add .
[devops@workstation etherpad-lite-postgres]$ git commit -m "Upload .npmrc and settings.json"
[devops@workstation etherpad-lite-postgres]$ git push origin master
```

导入完成后的仓库后续将用于 etherpad-lite 应用的构建与测试，如下所示：

<center><img src="images/gitlab-create-new-project-4.png" style="width:80%"></center>

### 5.6 Python 应用导入

#### 5.6.1 创建新项目 cnn_mnist_train

以 devuser0 用户登录代码库创建新项目：

<center><img src="images/gitlab-create-cnn-demo-1.png" style="width:80%"></center>

<center><img src="images/gitlab-create-cnn-demo-2.png" style="width:80%"></center>

<center><img src="images/gitlab-create-cnn-demo-3.png" style="width:80%"></center>

💥 上图中，如果源代码目录中已有 README 文件，可不选择创建此文件。

#### 5.6.2 导入 cnn_mnist_train 外部代码库

本示例使用基于 MNIST 数据集进行 CNN 模型的训练，并将训练好的模型部署于 Flask 中，用户可访问 Flask 应用页面完成手写数字识别。可参考 [基于 TensorFlow 实现 CNN 手写数字识别](https://github.com/Alberthua-Perl/python-project-demo/blob/develop/%E6%9C%BA%E5%99%A8%E5%AD%A6%E4%B9%A0%20%26%20%E6%B7%B1%E5%BA%A6%E5%AD%A6%E4%B9%A0%E7%B3%BB%E5%88%97/%E3%80%90Lab%E3%80%91%E5%9F%BA%E4%BA%8E%20TensorFlow%20%E5%AE%9E%E7%8E%B0%20CNN%20%E6%89%8B%E5%86%99%E6%95%B0%E5%AD%97%E8%AF%86%E5%88%AB/%E5%9F%BA%E4%BA%8E%20TensorFlow%20%E5%AE%9E%E7%8E%B0%20CNN%20%E6%89%8B%E5%86%99%E6%95%B0%E5%AD%97%E8%AF%86%E5%88%AB.ipynb) 了解此项目。

```bash
[devops@workstation ~]$ wget http://content.example.com/jenkins-ci-plt/code-examples/cnn_mnist_train.tar
[devops@workstation ~]$ tar -xf cnn_mnist_train.tar
[devops@workstation ~]$ cd cnn_mnist_train
[devops@workstation cnn_mnist_train]$ git config --global init.defaultBranch main  #设置默认的初始化分支名称
[devops@workstation cnn_mnist_train]$ git init .  #此应用首次上传至源代码仓库中，原目录中不具有 git 仓库的信息，需执行初始化。
Initialized empty Git repository in /home/devops/cnn_mnist_train/.git/
[devops@workstation cnn_mnist_train]$ git remote add origin git@gitlab-ce.lab.example.com:devuser0/cnn_mnist_train.git
#添加远程代码库地址
[devops@workstation cnn_mnist_train]$ git pull origin main  #拉取远程代码库中内容合并至本地 main 分支
From gitlab-ce.lab.example.com:devuser0/cnn_mnist_train
 * branch            main       -> FETCH_HEAD
# 注意：
#   1. 如果仅仅执行 git pull 命令拉取远程代码库，那么 git 无法合并分支代码，因此需指定合并的本地代码分支（main 分支）。
#   2. 如下所示，仅仅执行 git pull 的返回，提示需执行合并的分支：
#        There is no tracking information for the current branch.
#        Please specify which branch you want to merge with.
#        See git-pull(1) for details.
#
#            git pull <remote> <branch>
#
#        If you wish to set tracking information for this branch you can do so with:
#
#            git branch --set-upstream-to=origin/<branch> main

[devops@workstation cnn_mnist_train]$ ls -lh
total 16K
-rw-r--r--. 1 devops devops 1.8K Jun  8 03:21 app.py
-rw-r--r--. 1 devops devops 6.1K Jun  9 01:38 README.md  #远程代码库中的文件被拉取至本地
drwxr-xr-x. 2 devops devops   24 Jun  8 09:10 templates
-rw-r--r--. 1 devops devops 1.7K Jun  8 03:21 train_mnist_model_tf.py
[devops@workstation cnn_mnist_train]$ git add .
[devops@workstation cnn_mnist_train]$ git commit -m "Update cnn project"
[devops@workstation cnn_mnist_train]$ git push origin main  #推送本地源代码
Enumerating objects: 7, done.
Counting objects: 100% (7/7), done.
Delta compression using up to 8 threads
Compressing objects: 100% (5/5), done.
Writing objects: 100% (6/6), 3.62 KiB | 3.62 MiB/s, done.
Total 6 (delta 0), reused 0 (delta 0), pack-reused 0
To gitlab-ce.lab.example.com:devuser0/cnn_mnist_train.git
   9f5c478..fb1d868  main -> main
```

导入完成后的仓库后续将用于 cnn_mnist_train 应用的构建与测试，如下所示：

<center><img src="images/gitlab-create-cnn-demo-4.png" style="width:80%"></center>

### 5.7 Java 应用导入

#### 5.7.1 创建新项目 spring-boot-helloworld

以 devuser0 用户登录代码库创建新项目：

<center><img src="images/gitlab-create-java-demo-1.png" style="width:80%"></center>

<center><img src="images/gitlab-create-java-demo-2.png" style="width:80%"></center>

<center><img src="images/gitlab-create-java-demo-3.png" style="width:80%"></center>

💥 上图中，如果源代码目录中已有 README 文件，可不选择创建此文件。

#### 5.7.2 导入 spring-boot-helloworld 外部代码库

本示例基于 Spring Boot 框架构建的 helloworld 应用演示 Java 语言应用在 Jenkins 中的持续构建过程。此项目源代码可参考 [iKubernetes/spring-boot-helloWorld | GitHub](https://github.com/iKubernetes/spring-boot-helloWorld#)。如下所示，将源代码导入远程代码库：

> 说明：Spring 应用生成可参考 [spring initializr | spring.io](https://start.spring.io/)

```bash
[devops@workstation ~]$ wget http://content.example.com/jenkins-ci-plt/code-examples/spring-boot-helloworld.tar
[devops@workstation ~]$ tar -xf spring-boot-helloworld.tar
[devops@workstation ~]$ cd spring-boot-helloworld
[devops@workstation spring-boot-helloworld]$ git remote set-url origin git@gitlab-ce.lab.example.com:devuser0/spring-boot-helloworld.git
# 💥 此应用为 GitHub 中应用的克隆，具有原始 github 源地址，因此使用上述命令更新远程源代码仓库地址。
[devops@workstation spring-boot-helloworld]$ git push --set-upstream origin --all  #推送分支代码至仓库中
Enumerating objects: 178, done.
Counting objects: 100% (178/178), done.
Delta compression using up to 8 threads
Compressing objects: 100% (92/92), done.
Writing objects: 100% (178/178), 22.95 KiB | 22.95 MiB/s, done.
Total 178 (delta 55), reused 178 (delta 55), pack-reused 0
remote: Resolving deltas: 100% (55/55), done.
To gitlab-ce.lab.example.com:devuser0/spring-boot-helloworld.git
 * [new branch]      main -> main
Branch 'main' set up to track remote branch 'main' from 'origin'.
[devops@workstation spring-boot-helloworld]$ git push origin --tags  #推送本地 tag 至远程代码库
Enumerating objects: 1, done.
Counting objects: 100% (1/1), done.
Writing objects: 100% (1/1), 155 bytes | 155.00 KiB/s, done.
Total 1 (delta 0), reused 1 (delta 0), pack-reused 0
To gitlab-ce.lab.example.com:devuser0/spring-boot-helloworld.git
 * [new tag]         v0.9.6 -> v0.9.6

[devops@workstation spring-boot-helloworld]$ mkdir ./backup && cp pom.xml ./backup
[devops@workstation spring-boot-helloworld]$ vim pom.xml
# 💥 添加 Nexus3 Maven 私服仓库配置，以满足后续 spring-boot 应用从 maven-proxy 仓库中拉取 jar 包的需求。
### 注意：
###   设置 Maven 连接 Nexus3 中的私服仓库，必须配置以下两个文件：
###     1. Maven 的 settings.xml 配置文件，编辑 servers 部分与 mirrors 部分（见 `7.2 Maven 构建环境` 中 playbook 的设置）
###     2. 父项目中的 pom.xml（见以下文件） 
...
        <!-- Modify to use local nexus3 group repo -->                                                                                                  
        <repositories>                       
            <repository>                     
                <id>maven-group</id>
                <!-- 指定 Nexus3 中的 maven-group 仓库 ID -->
                <name>Maven2 Group Local Repository</name>
                <!-- 自定义以上仓库的名称 -->                                                                                              
                <url>http://nexus3.lab.example.com:8881/repository/maven-group/</url>
                <!-- 指定 maven-group 仓库的 URL 地址，可在 Browse 仓库列表页 Copy 此地址。 -->                                                                   
                <releases>                   
                    <enabled>true</enabled>  
                </releases>                  
                <snapshots>                  
                    <!-- <enabled>false</enabled> -->                                                                                                   
                    <enabled>true</enabled>  
                </snapshots>                 
            </repository>                    
        </repositories>

        <pluginRepositories>                 
            <pluginRepository>
            <!-- 插件仓库的信息与上述配置相同 -->               
                <id>maven-group</id>         
                <name>Maven2 Group Local Repository</name>                                                                                              
                <url>http://nexus3.lab.example.com:8881/repository/maven-group/</url>                                                                   
                <releases>                   
                    <enabled>true</enabled>  
                </releases>                  
                <snapshots>                  
                    <enabled>false</enabled> 
                </snapshots>                 
            </pluginRepository>              
        </pluginRepositories>

        <!-- 推送构件至私服中的配置段 -->
        <distributionManagement>
            <repository>
                <id>maven-group</id>
                <name>Maven2 Group Local Repository</name>
                <url>http://nexus3.lab.example.com:8881/repository/maven-group/</url>
            </repository>
            <snapshotRepository>
                <id>maven-group</id>
                <name>Maven2 Group Local Repository</name>
                <url>http://nexus3.lab.example.com:8881/repository/maven-group/</url>
            </snapshotRepository>
        </distributionManagement>                
        <!-- edited by hualongfeiyyy@163.com -->

[devops@workstation spring-boot-helloworld]$ git .
[devops@workstation spring-boot-helloworld]$ git commit -m "Update nexus3 group repo info"
[main 86758fa] Update nexus3 group repo info
 2 files changed, 90 insertions(+), 5 deletions(-)
 create mode 100644 backup/pom.xml
[devops@workstation spring-boot-helloworld]$ git push origin main  #推送修改后的 pom.xml 
Enumerating objects: 6, done.
Counting objects: 100% (6/6), done.
Delta compression using up to 8 threads
Compressing objects: 100% (3/3), done.
Writing objects: 100% (4/4), 615 bytes | 615.00 KiB/s, done.
Total 4 (delta 2), reused 0 (delta 0), pack-reused 0
To gitlab-ce.lab.example.com:devuser0/spring-boot-helloworld.git
   fea6fe4..86758fa  main -> main
```

🔗 以上 pom.xml 文件可直接参考此文件的拷贝 [jenkins-ci-plt/files/pom.xml.bak | GitHub](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/files/pom.xml.bak)

导入完成后的仓库后续将用于 spring-boot-helloworld 应用的构建与测试，如下所示：

<center><img src="images/gitlab-create-java-project.png" style="width:80%"></center>

## 6. 部署与设置 Nexus3 容器

### 6.1 部署 Nexus3 容器

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run scm-artifact/prep-scm-nexus3.yml --tag nexus3
# 部署 nexus3 容器（容器启动需 2~3 分钟）
```

### [6.2 创建 Nexus3 的 devuser0 用户](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE.md#2-%E5%88%9B%E5%BB%BA-nexus3-%E7%94%A8%E6%88%B7)

### [6.3 创建 Nexus3 的容器镜像仓库（hosted 类型）](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE.md#41-%E5%88%9B%E5%BB%BA-docker-hosted-%E7%B1%BB%E5%9E%8B%E7%9A%84%E5%AE%B9%E5%99%A8%E9%95%9C%E5%83%8F%E4%BB%93%E5%BA%93)

### [6.4 创建 Nexus3 的 npm 构件仓库（proxy 类型）](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE.md#5-npm-%E6%9E%84%E4%BB%B6%E5%BA%93)

### 6.5 创建 Nexus3 的 maven 构件仓库

- [创建 maven（proxy）类型仓库](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE.md#63-%E5%88%9B%E5%BB%BA-maven-proxy-%E7%B1%BB%E5%9E%8B%E6%9E%84%E4%BB%B6%E5%BA%93)
- [创建 maven（group）类型仓库](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE/Nexus3%20%E7%9A%84%E9%83%A8%E7%BD%B2%E4%B8%8E%E5%B8%B8%E8%A7%84%E8%AE%BE%E7%BD%AE.md#64-%E5%88%9B%E5%BB%BA-maven-group-%E7%B1%BB%E5%9E%8B%E6%9E%84%E4%BB%B6%E5%BA%93)

## 7. 部署应用运行及构建环境

### 7.1 Node.js 运行环境

Jenkins Master与 Agent使用 Node.js 管理工具构建与管理模块与应用，因此，各节点需安装 node 运行环境、npm 与 pnpm 工具，可参考以下步骤：

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run build-env/prep-nodejs-env.yml
```

### 7.2 Maven 构建环境

Jenkins Master与 Agent使用 Maven 构建与管理 Java 项目，因此，各节点需安装 maven。此处不使用 Jenkins Dashboard 中 "全局工具" 提供的 maven 安装方式，而是直接使用以下 playbook 安装与设置 maven，以及同步 maven 的 settings.xml 配置文件，以满足 maven-proxy 私服的认证连接，此私服可缓存来自外部仓库的各个 jar 包，方便后续应用构建使用。

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run build-env/prep-maven-env.yml
```

### 7.3 使用 spring-boot 应用测试 maven (group) 类型构件库

可选择一个 Jenkins Agent（serverb 节点为例）克隆应用代码并使用 maven 测试，如下所示：

```bash
[devops@workstation ~]$ scp -r spring-boot-helloworld devops@serverb:~  #同步源代码至 serverb 节点上
[devops@serverb ~]$ cd spring-boot-helloworld
[devops@serverb spring-boot-helloworld]$ export PATH=$PATH:/usr/local/apache-maven-3.9.9/bin/  #设置 mvn 命令环境变量
[devops@serverb spring-boot-helloworld]$ mvn --version
Apache Maven 3.9.9 (8e8579a9e76f7d015ee5ec7bfcdc97d260186937)
Maven home: /usr/local/apache-maven-3.9.9
Java version: 17.0.3, vendor: Red Hat, Inc., runtime: /usr/lib/jvm/java-17-openjdk-17.0.3.0.7-1.el9_0.x86_64
Default locale: en_US, platform encoding: UTF-8
OS name: "linux", version: "5.14.0-70.13.1.el9_0.x86_64", arch: "amd64", family: "unix"

[devops@serverb spring-boot-helloworld]$ mvn dependency:tree  #显示项目的依赖树
[INFO] Scanning for projects...
Downloading from maven-group: http://nexus3.lab.example.com:8881/repository/maven-group/org/springframework/boot/spring-boot-starter-parent/2.1.3.RELEASE/spring-boot-starter-parent-2.1.3.RELEASE.pom
Downloaded from maven-group: http://nexus3.lab.example.com:8881/repository/maven-group/org/springframework/boot/spring-boot-starter-parent/2.1.3.RELEASE/spring-boot-starter-parent-2.1.3.RELEASE.pom (12 kB at 12 kB/s)
Downloading from maven-group: http://nexus3.lab.example.com:8881/repository/maven-group/org/springframework/boot/spring-boot-dependencies/2.1.3.RELEASE/spring-boot-dependencies-2.1.3.RELEASE.pom
...
[WARNING] Could not validate integrity of download from http://nexus3.lab.example.com:8881/repository/maven-group/com/fasterxml/oss-parent/34/oss-parent-34.pom
...
[INFO] com.neo:spring-boot-helloworld:jar:0.9.6-SNAPSHOT
[INFO] +- org.springframework.boot:spring-boot-starter-web:jar:2.1.3.RELEASE:compile
[INFO] |  +- org.springframework.boot:spring-boot-starter:jar:2.1.3.RELEASE:compile
[INFO] |  |  +- org.springframework.boot:spring-boot-starter-logging:jar:2.1.3.RELEASE:compile
[INFO] |  |  |  +- ch.qos.logback:logback-classic:jar:1.2.3:compile
[INFO] |  |  |  |  \- ch.qos.logback:logback-core:jar:1.2.3:compile
[INFO] |  |  |  +- org.apache.logging.log4j:log4j-to-slf4j:jar:2.11.2:compile
[INFO] |  |  |  |  \- org.apache.logging.log4j:log4j-api:jar:2.11.2:compile
[INFO] |  |  |  \- org.slf4j:jul-to-slf4j:jar:1.7.25:compile
[INFO] |  |  +- javax.annotation:javax.annotation-api:jar:1.3.2:compile
[INFO] |  |  \- org.yaml:snakeyaml:jar:1.23:runtime
[INFO] |  +- org.springframework.boot:spring-boot-starter-json:jar:2.1.3.RELEASE:compile
[INFO] |  |  +- com.fasterxml.jackson.core:jackson-databind:jar:2.9.8:compile
[INFO] |  |  |  +- com.fasterxml.jackson.core:jackson-annotations:jar:2.9.0:compile
[INFO] |  |  |  \- com.fasterxml.jackson.core:jackson-core:jar:2.9.8:compile
[INFO] |  |  +- com.fasterxml.jackson.datatype:jackson-datatype-jdk8:jar:2.9.8:compile
[INFO] |  |  +- com.fasterxml.jackson.datatype:jackson-datatype-jsr310:jar:2.9.8:compile
[INFO] |  |  \- com.fasterxml.jackson.module:jackson-module-parameter-names:jar:2.9.8:compile
[INFO] |  +- org.springframework.boot:spring-boot-starter-tomcat:jar:2.1.3.RELEASE:compile
[INFO] |  |  +- org.apache.tomcat.embed:tomcat-embed-core:jar:9.0.16:compile
[INFO] |  |  +- org.apache.tomcat.embed:tomcat-embed-el:jar:9.0.16:compile
[INFO] |  |  \- org.apache.tomcat.embed:tomcat-embed-websocket:jar:9.0.16:compile
[INFO] |  +- org.hibernate.validator:hibernate-validator:jar:6.0.14.Final:compile
[INFO] |  |  +- javax.validation:validation-api:jar:2.0.1.Final:compile
[INFO] |  |  +- org.jboss.logging:jboss-logging:jar:3.3.2.Final:compile
[INFO] |  |  \- com.fasterxml:classmate:jar:1.4.0:compile
[INFO] |  +- org.springframework:spring-web:jar:5.1.5.RELEASE:compile
[INFO] |  |  \- org.springframework:spring-beans:jar:5.1.5.RELEASE:compile
[INFO] |  \- org.springframework:spring-webmvc:jar:5.1.5.RELEASE:compile
[INFO] |     +- org.springframework:spring-aop:jar:5.1.5.RELEASE:compile
[INFO] |     +- org.springframework:spring-context:jar:5.1.5.RELEASE:compile
[INFO] |     \- org.springframework:spring-expression:jar:5.1.5.RELEASE:compile
[INFO] +- org.springframework.boot:spring-boot-starter-test:jar:2.1.3.RELEASE:test
[INFO] |  +- org.springframework.boot:spring-boot-test:jar:2.1.3.RELEASE:test
[INFO] |  +- org.springframework.boot:spring-boot-test-autoconfigure:jar:2.1.3.RELEASE:test
[INFO] |  +- com.jayway.jsonpath:json-path:jar:2.4.0:test
[INFO] |  |  +- net.minidev:json-smart:jar:2.3:test
[INFO] |  |  |  \- net.minidev:accessors-smart:jar:1.2:test
[INFO] |  |  |     \- org.ow2.asm:asm:jar:5.0.4:test
[INFO] |  |  \- org.slf4j:slf4j-api:jar:1.7.25:compile
[INFO] |  +- junit:junit:jar:4.12:test
[INFO] |  +- org.assertj:assertj-core:jar:3.11.1:test
[INFO] |  +- org.mockito:mockito-core:jar:2.23.4:test
[INFO] |  |  +- net.bytebuddy:byte-buddy:jar:1.9.10:test
[INFO] |  |  +- net.bytebuddy:byte-buddy-agent:jar:1.9.10:test
[INFO] |  |  \- org.objenesis:objenesis:jar:2.6:test
[INFO] |  +- org.hamcrest:hamcrest-core:jar:1.3:test
[INFO] |  +- org.hamcrest:hamcrest-library:jar:1.3:test
[INFO] |  +- org.skyscreamer:jsonassert:jar:1.5.0:test
[INFO] |  |  \- com.vaadin.external.google:android-json:jar:0.0.20131108.vaadin1:test
[INFO] |  +- org.springframework:spring-core:jar:5.1.5.RELEASE:compile
[INFO] |  |  \- org.springframework:spring-jcl:jar:5.1.5.RELEASE:compile
[INFO] |  +- org.springframework:spring-test:jar:5.1.5.RELEASE:test
[INFO] |  \- org.xmlunit:xmlunit-core:jar:2.6.2:test
[INFO] |     \- javax.xml.bind:jaxb-api:jar:2.3.1:test
[INFO] |        \- javax.activation:javax.activation-api:jar:1.2.0:test
[INFO] \- org.springframework.boot:spring-boot-devtools:jar:2.1.3.RELEASE:compile (optional) 
[INFO]    +- org.springframework.boot:spring-boot:jar:2.1.3.RELEASE:compile
[INFO]    \- org.springframework.boot:spring-boot-autoconfigure:jar:2.1.3.RELEASE:compile
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  01:05 min
[INFO] Finished at: 2025-06-14T10:00:23-04:00
[INFO] ------------------------------------------------------------------------
# 从以上输出可见，mvn 已连接 Nexus3 中的 maven 私服，此私服仓库可缓存 Jar 包。

[devops@serverb spring-boot-helloworld]$ mvn clean install -DskipTest  #跳过单元测试，安装项目到本地 maven 仓库。
[INFO] Scanning for projects...
[INFO] 
[INFO] -------------------< com.neo:spring-boot-helloworld >-------------------
[INFO] Building spring-boot-helloworld 0.9.6-SNAPSHOT
[INFO]   from pom.xml
[INFO] --------------------------------[ jar ]---------------------------------
[INFO] 
[INFO] --- clean:3.1.0:clean (default-clean) @ spring-boot-helloworld ---
Downloading from maven-group: http://nexus3.lab.example.com:8881/repository/maven-group/org/apache/maven/shared/maven-shared-utils/3.2.1/maven-shared-utils-3.2.1.pom
Downloaded from maven-group: http://nexus3.lab.example.com:8881/repository/maven-group/org/apache/maven/shared/maven-shared-utils/3.2.1/maven-shared-utils-3.2.1.pom (5.6 kB at 76 kB/s)
Downloading from maven-group: http://nexus3.lab.example.com:8881/repository/maven-group/org/codehaus/plexus/plexus-utils/2.0.4/plexus-utils-2.0.4.jar
...
  .   ____          _            __ _ _
 /\\ / ___'_ __ _ _(_)_ __  __ _ \ \ \ \
( ( )\___ | '_ | '_| | '_ \/ _` | \ \ \ \
 \\/  ___)| |_)| | | | | || (_| |  ) ) ) )
  '  |____| .__|_| |_|_| |_\__, | / / / /
 =========|_|==============|___/=/_/_/_/
 :: Spring Boot ::        (v2.1.3.RELEASE)

2025-06-14 10:02:22.860  INFO 9672 --- [           main] com.neo.ApplicationTests                 : Starting ApplicationTests on serverc.lab.example.com with PID 9672 (started by devops in /home/devops/spring-boot-helloworld)
2025-06-14 10:02:22.884  INFO 9672 --- [           main] com.neo.ApplicationTests                 : No active profile set, falling back to default profiles: default
2025-06-14 10:02:24.792  INFO 9672 --- [           main] o.s.s.concurrent.ThreadPoolTaskExecutor  : Initializing ExecutorService 'applicationTaskExecutor'
2025-06-14 10:02:25.011  INFO 9672 --- [           main] com.neo.ApplicationTests                 : Started ApplicationTests in 2.508 seconds (JVM running for 3.613)
hello word
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0, Time elapsed: 3.452 s - in com.neo.ApplicationTests
[INFO] Running com.neo.controller.HelloTests
...
[INFO] Installing /home/devops/spring-boot-helloworld/target/spring-boot-helloworld-0.9.6-SNAPSHOT.jar to /home/devops/.m2/repository/com/neo/spring-boot-helloworld/0.9.6-SNAPSHOT/spring-boot-helloworld-0.9.6-SNAPSHOT.jar
[INFO] Installing /home/devops/spring-boot-helloworld/pom.xml to /home/devops/.m2/repository/com/neo/spring-boot-helloworld/0.9.6-SNAPSHOT/spring-boot-helloworld-0.9.6-SNAPSHOT.pom
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  14.444 s
[INFO] Finished at: 2025-06-14T10:02:29-04:00
[INFO] ------------------------------------------------------------------------

[devops@serverb spring-boot-helloworld]$ mvn test  #运行项目的单元测试
[INFO] Scanning for projects...
[INFO] 
[INFO] -------------------< com.neo:spring-boot-helloworld >-------------------
[INFO] Building spring-boot-helloworld 0.9.6-SNAPSHOT
[INFO]   from pom.xml
[INFO] --------------------------------[ jar ]---------------------------------
[INFO] 
[INFO] --- resources:3.1.0:resources (default-resources) @ spring-boot-helloworld ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] Copying 1 resource
[INFO] Copying 0 resource
[INFO] 
[INFO] --- compiler:3.8.0:compile (default-compile) @ spring-boot-helloworld ---
[INFO] Nothing to compile - all classes are up to date
[INFO] 
[INFO] --- resources:3.1.0:testResources (default-testResources) @ spring-boot-helloworld ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /home/devops/spring-boot-helloworld/src/test/resources
[INFO] 
[INFO] --- compiler:3.8.0:testCompile (default-testCompile) @ spring-boot-helloworld ---
[INFO] Nothing to compile - all classes are up to date
[INFO] 
[INFO] --- surefire:2.22.1:test (default-test) @ spring-boot-helloworld ---
...
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0, Time elapsed: 0.018 s - in com.neo.controller.HelloWorldControlerTests
2025-06-14 10:03:04.205  INFO 9779 --- [       Thread-2] o.s.s.concurrent.ThreadPoolTaskExecutor  : Shutting down ExecutorService 'applicationTaskExecutor'
[INFO] 
[INFO] Results:
[INFO] 
[INFO] Tests run: 3, Failures: 0, Errors: 0, Skipped: 0
[INFO] 
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  4.037 s
[INFO] Finished at: 2025-06-14T10:03:04-04:00
[INFO] ------------------------------------------------------------------------

[devops@serverb spring-boot-helloworld]$ mvn package  #打包项目
...
  .   ____          _            __ _ _
 /\\ / ___'_ __ _ _(_)_ __  __ _ \ \ \ \
( ( )\___ | '_ | '_| | '_ \/ _` | \ \ \ \
 \\/  ___)| |_)| | | | | || (_| |  ) ) ) )
  '  |____| .__|_| |_|_| |_\__, | / / / /
 =========|_|==============|___/=/_/_/_/
 :: Spring Boot ::        (v2.1.3.RELEASE)

2025-06-14 12:08:04.154  INFO 10642 --- [           main] com.neo.ApplicationTests                 : Starting ApplicationTests on serverc.lab.example.com with PID 10642 (started by devops in /home/devops/spring-boot-helloworld)
2025-06-14 12:08:04.156  INFO 10642 --- [           main] com.neo.ApplicationTests                 : No active profile set, falling back to default profiles: default
2025-06-14 12:08:05.238  INFO 10642 --- [           main] o.s.s.concurrent.ThreadPoolTaskExecutor  : Initializing ExecutorService 'applicationTaskExecutor'
2025-06-14 12:08:05.432  INFO 10642 --- [           main] com.neo.ApplicationTests                 : Started ApplicationTests in 1.484 seconds (JVM running for 2.035)
hello word
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0, Time elapsed: 2.057 s - in com.neo.ApplicationTests
[INFO] Running com.neo.controller.HelloTests
...
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0, Time elapsed: 0.026 s - in com.neo.controller.HelloWorldControlerTests
2025-06-14 12:08:05.809  INFO 10642 --- [       Thread-2] o.s.s.concurrent.ThreadPoolTaskExecutor  : Shutting down ExecutorService 'applicationTaskExecutor'
[INFO] 
[INFO] Results:
[INFO] 
[INFO] Tests run: 3, Failures: 0, Errors: 0, Skipped: 0
[INFO] 
[INFO] 
[INFO] --- jar:3.1.1:jar (default-jar) @ spring-boot-helloworld ---
[INFO] Building jar: /home/devops/spring-boot-helloworld/target/spring-boot-helloworld-0.9.6-SNAPSHOT.jar
[INFO] 
[INFO] --- spring-boot:2.1.3.RELEASE:repackage (repackage) @ spring-boot-helloworld ---
[INFO] Replacing main artifact with repackaged archive
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  4.509 s
[INFO] Finished at: 2025-06-14T12:08:06-04:00
[INFO] ------------------------------------------------------------------------

[devops@serverb spring-boot-helloworld]$ java -jar target/spring-boot-helloworld-0.9.6-SNAPSHOT.jar 

  .   ____          _            __ _ _
 /\\ / ___'_ __ _ _(_)_ __  __ _ \ \ \ \
( ( )\___ | '_ | '_| | '_ \/ _` | \ \ \ \
 \\/  ___)| |_)| | | | | || (_| |  ) ) ) )
  '  |____| .__|_| |_|_| |_\__, | / / / /
 =========|_|==============|___/=/_/_/_/
 :: Spring Boot ::        (v2.1.3.RELEASE)

2025-06-14 12:13:30.719  INFO 10693 --- [           main] com.neo.Application                      : Starting Application v0.9.6-SNAPSHOT on serverc.lab.example.com with PID 10693 (/home/devops/spring-boot-helloworld/target/spring-boot-helloworld-0.9.6-SNAPSHOT.jar started by devops in /home/devops/spring-boot-helloworld)
2025-06-14 12:13:30.721  INFO 10693 --- [           main] com.neo.Application                      : No active profile set, falling back to default profiles: default
2025-06-14 12:13:31.662  INFO 10693 --- [           main] o.s.b.w.embedded.tomcat.TomcatWebServer  : Tomcat initialized with port(s): 8080 (http)
2025-06-14 12:13:31.702  INFO 10693 --- [           main] o.apache.catalina.core.StandardService   : Starting service [Tomcat]
2025-06-14 12:13:31.703  INFO 10693 --- [           main] org.apache.catalina.core.StandardEngine  : Starting Servlet engine: [Apache Tomcat/9.0.16]
2025-06-14 12:13:31.713  INFO 10693 --- [           main] o.a.catalina.core.AprLifecycleListener   : The APR based Apache Tomcat Native library which allows optimal performance in production environments was not found on the java.library.path: [/usr/java/packages/lib:/usr/lib64:/lib64:/lib:/usr/lib]
2025-06-14 12:13:31.779  INFO 10693 --- [           main] o.a.c.c.C.[Tomcat].[localhost].[/]       : Initializing Spring embedded WebApplicationContext
2025-06-14 12:13:31.780  INFO 10693 --- [           main] o.s.web.context.ContextLoader            : Root WebApplicationContext: initialization completed in 1011 ms
2025-06-14 12:13:32.005  INFO 10693 --- [           main] o.s.s.concurrent.ThreadPoolTaskExecutor  : Initializing ExecutorService 'applicationTaskExecutor'
2025-06-14 12:13:32.183  INFO 10693 --- [           main] o.s.b.w.embedded.tomcat.TomcatWebServer  : Tomcat started on port(s): 8080 (http) with context path ''
2025-06-14 12:13:32.187  INFO 10693 --- [           main] com.neo.Application                      : Started Application in 1.798 seconds (JVM running for 2.109)


### 打开其他节点访问测试
[devops@workstation ~]$ curl http://serverc.lab.example.com:8080
Hello Spring Boot 2.0!
```

## 8. 部署与设置 PostgreSQL 数据库

此 postgresql 数据库服务器用于 Node.js 应用 etherpad-lite 在运行后的数据连接。etherpad-lite 应用在容器镜像构建过程中通过本地的 Nexus3 的 npm-proxy 仓库安装依赖模块，构建与指定执行应用方式。当应用容器镜像构建完成，可将其推送至本地 Nexus3 的 docker-hosted 镜像仓库中，待到部署此应用时拉取运行即可。连接 postgresql 服务器的应用，如果通过应用 Web 界面编辑文本，那么这些文本将存储于数据库中。

### 8.1 安装部署 PostgreSQL 数据库

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run build-env/setup-postgres-db.yml --skip-tags allow_local
[devops@workstation jenkins-ci-plt]$ ssh root@serverd
[root@serverd ~]# su - postgres
[postgres@serverd ~]$ psql -c "ALTER USER postgres WITH PASSWORD '1qazZSE$';"
ALTER ROLE
# 默认免密码登录本地数据库服务器，因此，设置数据库服务器 postgres 管理员密码。
[postgres@serverd ~]$ exit
logout

[devops@workstation jenkins-ci-plt]$ ansible-navigator run build-env/setup-postgres-db.yml --tag allow_local
# 数据库服务器 postgres 管理员密码设置完成后，配置以密码认证方式本地连接数据库。
```

### 8.2 数据库服务器中创建 etherpad-lite 应用相关用户与数据库

构建的 etherpad-lite 应用可通过源代码中的 settings.json 文件定义的数据库服务器对接。因此，此处创建相关用户与数据库，如下所示：

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run build-env/setup-postgres-db.yml --tag create_user_db
```

如果用户创建或数据库创建失败或报错，可执行以下命令回退：

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run build-env/setup-postgres-db.yml --tag revoke_user_db
```

## 9. 部署 Jenkins Master 服务

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run jenkins/jkn-cluster.yml
# 部署 Jenkins Master服务
```

Jenkins Master 服务部署完成后需登录 Web UI 继续设置，可参考 [Jenkins 安装与配置](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/Jenkins%20%E6%A6%82%E8%BF%B0%E4%B8%8E%E9%83%A8%E7%BD%B2/Jenkins%20%E6%A6%82%E8%BF%B0%E4%B8%8E%E9%83%A8%E7%BD%B2.md#3-jenkins-%E5%AE%89%E8%A3%85%E4%B8%8E%E9%85%8D%E7%BD%AE)中的部分内容。

## 10. 运行自由风格的作业（Free Style Project）

### 10.1 Node.js 应用 —— 构建测试 etherpad-lite 应用及容器镜像

#### 10.1.1 创建基于 SSH 私钥的凭据连接 GitLab-CE

此处创建的凭据后续用于连接 GitLab-CE 代码库

<center><img src="images/jenkins-create-credentials-1.png" style="width:80%"></center>

<center><img src="images/jenkins-create-credentials-2.png" style="width:80%"></center>

<center><img src="images/jenkins-create-credentials-5.png" style="width:80%"></center>

<center><img src="images/jenkins-create-credentials-6.png" style="width:80%"></center>

<center><img src="images/jenkins-create-credentials-7.png" style="width:80%"></center>

#### 10.1.2 安装 Jenkins 的 Blue Ocean 插件

点击 `Dashboard > Manage Jenkins > Plugins > Available plugins`，搜索 `Blue Ocean` 插件，点击 Install 即可逐步安装。此插件可作为新一代的 Jenkins 作业构建面板。

<center><img src="images/blue-ocean-plugin-install-1.png" style="width:80%"></center>

<center><img src="images/blue-ocean-plugin-install-2.jpg" style="width:80%"></center>

#### 10.1.3 jenkins 用户的 SSH 连接代码库的主机密钥校验与配置

配置自由风格作业的过程中，如需连接远程代码库，那么要指定连接远程代码库的凭据，此处使用基于 SSH 私钥的凭据。本次采用容器化部署的 gitlab-ce 远程代码库，其对外暴露的 SSH 监听端口不再是默认的 22/tcp 端口，而是映射至宿主机的 8882/tcp 端口，因此，Jenkins Master使用 SSH 连接时需执行以下步骤：

```bash
### 步骤1：切换 jenkins 用户
[devops@servera ~]$ sudo su -
[root@servera ~]# grep jenkins /etc/passwd
jenkins:x:977:977:Jenkins Automation Server:/var/lib/jenkins:/bin/false
# jenkins 用户不具有登录 shell，因此切换至此用户需指定登录的 shell 类型。
[root@servera ~]# su - -s /bin/bash jenkins  #切换至 jenkins 用户
[jenkins@servera ~]$ pwd  #jenkins 用户家目录，$JENKINS_HOME 环境变量引用。
/var/lib/jenkins

### 步骤2：指定远程代码库监听的 SSH 端口
[jenkins@servera ~]$ cat > ~/.ssh/config <<EOF
> Host workstation.lab.example.com gitlab-ce.lab.example.com
>   Port 8882
> EOF
# 直接在 jenkins 用户家目录中创建 SSH 配置文件，指定远程代码库监听的 SSH 端口（gitlab-ce 容器映射至宿主机的端口）。
[jenkins@servera ~]$ ls -lh ~/.ssh/config  #确认 SSH 配置文件的权限
-rw-r--r--. 1 jenkins jenkins 71 Jun  6 11:33 /var/lib/jenkins/.ssh/config

### 步骤3：接受远程代码库的 SSH 主机公钥
[jenkins@servera ~]$ ssh devops@workstation.lab.example.com
The authenticity of host '[workstation.lab.example.com]:8882 ([172.25.250.9]:8882)' can't be established.
ED25519 key fingerprint is SHA256:IO2evgSVI11S4LZh75hWb7F/bS9kY1zHW1dkFasDiQM.
This key is not known by any other names
Are you sure you want to continue connecting (yes/no/[fingerprint])? yes
Warning: Permanently added '[workstation.lab.example.com]:8882' (ED25519) to the list of known hosts.
devops@workstation.lab.example.com: Permission denied (publickey).

[jenkins@servera ~]$ ssh devops@gitlab-ce.lab.example.com
The authenticity of host '[gitlab-ce.lab.example.com]:8882 ([172.25.250.9]:8882)' can't be established.
ED25519 key fingerprint is SHA256:IO2evgSVI11S4LZh75hWb7F/bS9kY1zHW1dkFasDiQM.
This host key is known by the following other names/addresses:
    ~/.ssh/known_hosts:1: [workstation.lab.example.com]:8882
Are you sure you want to continue connecting (yes/no/[fingerprint])? yes
Warning: Permanently added '[gitlab-ce.lab.example.com]:8882' (ED25519) to the list of known hosts.
devops@gitlab-ce.lab.example.com: Permission denied (publickey).

[jenkins@servera ~]$ cat ~/.ssh/known_hosts  
[workstation.lab.example.com]:8882 ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIHnWFj9xJ4k3/OLAX5pDCNiISbbuAduzECuQHOo1GCOJ
[gitlab-ce.lab.example.com]:8882 ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIHnWFj9xJ4k3/OLAX5pDCNiISbbuAduzECuQHOo1GCOJ
# 满足 SSH 主机公钥加密算法 ed25519，远程代码库监听的 SSH 端口（gitlab-ce 容器映射至宿主机的端口）。
```

以上 3 步保证在创建自由风格作业的过程中，连接远程代码库时，避免出现以下报错：

- Jenkins 连接远程代码库时需验证此节点的主机公钥，因此需验证此公钥以免连接失败（步骤3）。当然，也可在 Jenkins 的 Dashboard 中设置，如下图所示，但关闭主机 SSH 公钥验证将降低 Jenkins 安全性（此方法不推荐）。

  <center><img src="images/host-key-checking.png" style="width:80%"></center>

- 连接的远程代码库对外映射的 SSH 端口为 8882/tcp，如果直接在源代码 url 地址中指定端口的话，Jenkins 解析地址失败，因此可直接在 jenkins 用户的 SSH 连接配置中直接指定映射至宿主机的监听端口解决（步骤2）。

#### 10.1.4 设置 jenkins 用户的 subuid/subgid 以满足 podman 的 rootless 构建环境

自由风格的作业中以 jenkins 用户运行 podman 构建应用容器镜像，而 jenkins 用户作为非特权用户需以 rootless 模式执行构建，如果此用户未在 `/etc/subuid` 与 `/etc/subgid` 中设置子用户与子用户组映射的话，在 Jenkins 作业执行过程中 Blue Ocean 界面报错如下：

<center><img src="images/jenkins-create-freestyle-job-nodejs-6.png" style="width:80%"></center>

可执行以下步骤解决此问题：

```bash
[devops@servera ~]$ sudo su -
[root@servera ~]# loginctl enable-linger 977  #jenkins 的 UID
[root@servera ~]# vim /etc/subuid
devops:100000:65536
jenkins:165536:65536
[root@servera ~]# vim /etc/subgid
devops:100000:65536
jenkins:165536:65536
# 分别添加 jenkins 用户的子用户与用户组映射
```

#### 10.1.5 构建与推送 node-pnpm 容器镜像

etherpad-lite-postgres 应用容器镜像基于 node 运行环境与 pnpm 构建，因此需预先构建此类基础镜像，再推送至 Nexus3 容器镜像仓库中。构建此镜像的 Containerfile 可使用 [node-pnpm | GitHub](https://github.com/Alberthua-Perl/dockerfile-s2i-demo/tree/master/node-pnpm)，容器镜像存储于 [node-pnpm | DockerHub](https://hub.docker.com/repository/docker/alberthua/node-pnpm/general) 中。

```bash
### 注意：由于其他节点的存储空间有限，因此在 foundation0 节点中构建容器镜像。
[kiosk@foundation0 ~]$ wget http://content.example.com/jenkins-ci-plt/container-images/rockylinux-9.3.tar
# docker.io 中的容器镜像，由于拉取超时失败，因此已提前准备。
[kiosk@foundation0 ~]$ podman load -i rockylinux-9.3.tar
[kiosk@foundation0 ~]$ wget http://content.example.com/jenkins-ci-plt/code-examples/node-pnpm.tar
[kiosk@foundation0 ~]$ tar -xf node-pnpm.tar
[kiosk@foundation0 ~]$ cd node-pnpm
[kiosk@foundation0 node-pnpm]$ podman build -t node-pnpm:10.11.0 --format=docker .
# 上传至 Nexus3 中的容器镜像不支持 oci 格式，在构建时指定兼容 docker 镜像格式（v2s2），否则后续推送镜像直接失败！
[kiosk@foundation0 node-pnpm]$ podman tag localhost/node-pnpm:10.11.0 nexus3.lab.example.com:8882/node-pnpm:10.11.0
[kiosk@foundation0 node-pnpm]$ podman login --tls-verify=false --username devuser0 --password 1qazZSE$ nexus3.lab.example.com:8882
[kiosk@foundation0 node-pnpm]$ podman push --tls-verify=false nexus3.lab.example.com:8882/node-pnpm:10.11.0
```

#### 10.1.6 创建与运行作业

1️⃣ 点击 Dashboard > Create a job 创建作业：

<center><img src="images/jenkins-create-freestyle-job-nodejs-1.png" style="width:80%"></center>

2️⃣ 指定创建的作业名称，以及作业的类型，此处选择自由风格的作业（项目）类型：

<center><img src="images/jenkins-create-freestyle-job-nodejs-2.png" style="width:80%"></center>

3️⃣ 点击 Source Code Management，选择 Git 源代码仓库选项，填入 Repository URL，即 `git@workstation.lab.example.com:devuser0/etherpad-lite-postgres.git`（5.6 中已导入），并选择相应的连接凭据 devuser0（10.1.1 中已创建）。

> 注意：💥 如果未在 10.1.3 中完成设置，那么在选择完 devuser0 凭据后，可能出现连接的远程代码库 SSH 主机公钥验证失败或者指定的源代码 namespace 不存在，亦或访问权限不足等报错。

<center><img src="images/jenkins-create-freestyle-job-nodejs-3.jpg" style="width:80%"></center>

4️⃣ 构建需指定源代码分支，此处选择 *master* 分支：

<center><img src="images/jenkins-create-freestyle-job-nodejs-4.jpg" style="width:80%"></center>

5️⃣ Build Steps 中选择 Execute shell，编写作业的执行逻辑，最后点击 Save 保存此作业。

<center><img src="images/jenkins-create-freestyle-job-nodejs-5.jpg" style="width:80%"></center>

此脚本的执行思路：先构建、测试源码，成功通过后再使用 Containerfile 构建此应用的容器镜像，并推送至 Nexus3 中待后续部署。

```bash
#!/bin/bash

# 注意：
#   Jenkins 的作业存储于 /var/lib/jenkins/workspace/<job_name>/ 目录中，因此，拉取的源代码目录中的内容直接保存于此目录中。
#   在当前作业目录中直接保存了所有源代码文件。
echo -e "\n---> Create app build env..."
mkdir build/  #创建新目录
shopt -s extglob  #设置 shell 通配符扩展，如果不设置，那么下一步命令无法执行。
mv !(build) build/ && mv .[a-zA-Z]* build/
#将源代码文件及隐藏文件全部移入 build/ 目录中，方便之后在当前目录中创建 Containerfile 用于构建应用镜像。

echo -e "\n---> Show local dir structure..."
tree -L 2 .  #查看当前目录结构

echo -e "\n---> Test etherpad-lite-postgres app..."
cd build/
pnpm install --no-frozen-lockfile --force  #强制安装应用依赖模块，不使用 --force 选项将停滞在是否安装模块选项中。
pnpm run build:etherpad  #构建应用
[[ $? -eq 0 ]] || exit 10  #构建成功继续执行，否则错误退出。

echo -e "\n---> Generate app Containerfile..."
cd ../  #切换至源码目录外
cat > Containerfile <<EOF  #生成 Containerfile
FROM nexus3.lab.example.com:8882/node-pnpm:10.11.0  #此基础镜像需提前上传至 Nexus3 中
MAINTAINER hualongfeiyyy@163.com

RUN mkdir /app
ADD ./build /app  #将源码文件全部拷贝至容器镜像中
WORKDIR /app
RUN pnpm install --no-frozen-lockfile --force && \
    pnpm run build:etherpad

EXPOSE 9001  #暴露 9001 端口
ENTRYPOINT ["pnpm", "run", "prod"]  #运行应用
EOF

echo -e "\n---> Login and pull base image..."
podman login --tls-verify=false --username devuser0 --password 1qazZSE$ nexus3.lab.example.com:8882
podman pull --tls-verify=false nexus3.lab.example.com:8882/node-pnpm:10.11.0  
#提前拉取镜像，如果构建时自动拉取镜像，会由于连接 Nexus3 镜像仓库的证书认证失败而导致拉取失败！
podman build -t etherpad-lite-postgres:v1.0 --format=docker .  #指定构建镜像格式执行构建
if [[ $? -eq 0 ]]; then
  podman tag localhost/etherpad-lite-postgres:v1.0 nexus3.lab.example.com:8882/etherpad-lite-postgres:v1.0
  echo -e "\n---> Push etherpad-lite-postgres app image..."
  podman push --tls-verify=false nexus3.lab.example.com:8882/etherpad-lite-postgres:v1.0
  if [[ $? -eq 0 ]]; then
    podman rmi localhost/etherpad-lite-postgres:v1.0 nexus3.lab.example.com:8882/etherpad-lite-postgres:v1.0
  fi  
else
  echo -e "\n---> [ERROR] Build failure..."
  exit 10
fi
#如果构建成功，那么推送镜像，反之退出作业流程。
```

以上构建脚本可参考 [jenkins-ci-plt/jenkins/free-style-demo/etherpad-lite-postgres-job.sh](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/jenkins/free-style-demo/etherpad-lite-postgres-job.sh)

6️⃣ 点击 Build Now 开始构建，也可点击 Open Blue Ocean 打开浏览构建过程：

<center><img src="images/jenkins-create-freestyle-job-nodejs-7.png" style="width:80%"></center>

7️⃣ 应用构建、测试与容器镜像构建、推送完成：

<center><img src="images/jenkins-create-freestyle-job-nodejs-9.png" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-nodejs-8.png" style="width:80%"></center>

### 10.2 Python 应用 —— 训练 CNN 模型、构建 app-tf-flask 应用及推理容器镜像

> ✍ 容器镜像说明：tf-flask 为构建用镜像（包含 TensorFlow 等深度学习框架）、app-tf-flask 为推理容器镜像（包含 TensorFlow、Flask 等框架）

#### 10.2.1 推送 tf-flask 构建用容器镜像

`5.6 Flask 应用导入` 中已介绍此项目的相关信息，此处使用自由风格作业方式完成持续构建。在后续的 app-tf-flask 容器镜像构建过程中依赖 nexus3.lab.example.com:8882/tf-flask:2.18.0 基础镜像，构建此镜像的 Containerfile 可使用 [tf-flask | GitHub](https://github.com/Alberthua-Perl/dockerfile-s2i-demo/tree/master/tf-flask)，容器镜像存储于 [tf-flask | DockerHub](https://hub.docker.com/repository/docker/alberthua/tf-flask/general)，并将其推送至 Nexus3 仓库中。如下所示：

```bash
[kiosk@foundation0 ~]$ wget http://content.example.com/jenkins-ci-plt/container-images/tf-flask-2.18.0.tar
[kiosk@foundation0 ~]$ podman load -i tf-flask-2.18.0.tar
[kiosk@foundation0 ~]$ podman images  #此容器镜像已标记为 Nexus3 仓库
[kiosk@foundation0 ~]$ podman push --tls-verify=false nexus3.lab.example.com:8882/tf-flask:2.18.0  #推送构建用镜像
```

#### 10.2.2 安装 AnsiColor 插件

在后续的 CNN 模型训练过程中，Blue Ocean 界面无法以 ANSI 形式进度条的方式显示，因此先预安装 `AnsiColor` 插件支持此功能。安装方法可参考 `10.1.2` 的方式。

#### 10.2.3 创建与运行作业

可参考以下步骤完成 CNN 模型训练与推理容器镜像（Flask 应用）的构建及推送：

<center><img src="images/jenkins-create-freestyle-job-cnn-1.jpg" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-cnn-2.jpg" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-cnn-3.jpg" style="width:80%"></center>

此脚本的执行思路：在 Jenkins 节点上使用 MNIST 数据集训练 CNN 模型，并将模型的训练结果保存于本地，拉取构建用基础镜像，创建 Containerfile，构建推理容器镜像（Flask 应用中部署模型），将构建的镜像推送至 Nexus3 中。

```bash
#!/bin/bash

echo -e "\n---> Create build env..."
mkdir build/
shopt -s extglob
mv !(build) build/

echo -e "\n---> Train MNIST and generate module..."
cd build/
python ./train_mnist_model_tf.py  #训练与保存 CNN 模型
tree .

echo -e "\n---> Generate Containerfile..."
cd ../
cat > Containerfile <<EOF
FROM nexus3.lab.example.com:8882/tf-flask:2.18.0

ADD build/ /app
WORKDIR /app
EXPOSE 5000

ENTRYPOINT ["python", "app.py"]
EOF

echo -e "\n---> Login and pull tf-flask image..."
podman login --tls-verify=false --username devuser0 --password 1qazZSE$ nexus3.lab.example.com:8882
podman pull --tls-verify=false nexus3.lab.example.com:8882/tf-flask:2.18.0

echo -e "\n---> Build app-tf-flask app image..."
podman build -t app-tf-flask:v1.0 --format=docker .  #构建推理容器镜像
if [[ $? -eq 0 ]]; then
  podman tag localhost/app-tf-flask:v1.0 nexus3.lab.example.com:8882/app-tf-flask:v1.0
  echo -e "\n---> Push app-tf-flask app image..."
  podman push --tls-verify=false nexus3.lab.example.com:8882/app-tf-flask:v1.0
  if [[ $? -eq 0 ]]; then
    echo -e "\n--> Remove local builded image..."
    podman rmi localhost/app-tf-flask:v1.0 nexus3.lab.example.com:8882/app-tf-flask:v1.0  #移除本地构建的镜像，释放存储空间。
  fi
else
  echo -e "\n---> [ERROR] Build failure..."
  exit 10
fi
```

以上脚本可参考 [jenkins-ci-plt/jenkins/free-style-demo/cnn-mnist-train-job.sh](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/jenkins/free-style-demo/cnn-mnist-train-job.sh)。

<center><img src="images/jenkins-create-freestyle-job-cnn-4.jpg" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-cnn-5.jpg" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-cnn-6.jpg" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-cnn-7.jpg" style="width:80%"></center>

如上图所示，Nexus3 中已存储推送的 app-tf-flask 容器镜像。

### 10.3 Java 应用 —— 构建测试 spring-boot 应用及容器镜像

#### 10.3.1 推送 openjdk-17 构建用容器镜像

openjdk-17 容器镜像作为 spring-boot 应用的构建用镜像，需上传至 Nexus3 仓库中以便后续自由风格的作业或流水线风格的作业中应用的持续构建。此容器镜像来源于 [openjdk | DockerHub](https://hub.docker.com/_/openjdk)。如下所示：

```bash
[kiosk@foundation0 ~]$ wget http://content.example.com/jenkins-ci-plt/container-images/openjdk-17-jdk-alpine.tar
[kiosk@foundation0 ~]$ podman load -i openjdk-17-jdk-alpine.tar
[kiosk@foundation0 ~]$ podman images  #此容器镜像已标记为 Nexus3 仓库
[kiosk@foundation0 ~]$ podman push --tls-verify=false nexus3.lab.example.com:8882/openjdk:17-jdk-alpine
#推送构建用镜像
```

#### 10.3.2 创建与运行作业

此 spring-boot 应用的自由风格的作业创建过程可参考如下：

<center><img src="images/jenkins-create-freestyle-job-spring-boot-1.png" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-spring-boot-2.png" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-spring-boot-3.png" style="width:80%"></center>

如上图所示，可勾选 `Build Environment > Color ANSI Console Output`，选择 `xterm` 或其他选项，使持续构建的过程输出支持颜色显示。

<center><img src="images/jenkins-create-freestyle-job-spring-boot-4.png" style="width:80%"></center>

此脚本的执行思路：mvn 工具为当前 spring boot 项目安装所需依赖 jar 包，测试应用，将生成的应用 jar 包通过 Containerfile 封装构建为应用容器镜像，再将此镜像推送至 Nexus3 仓库中用于之后的容器化部署。

```bash
#!/bin/bash

echo -e "\n---> Create build env..."
mkdir build/
shopt -s extglob
mv !(build) build/ && mv .[a-zA-Z]* build/

echo -e "\n---> Install mvn modules and test..."
cd build/
export PATH=$PATH:/usr/local/apache-maven-3.9.9/bin
# 💥 根据 `7.2 Maven 构建环境` 的说明，由于使用自定义的安装方式，MAVEN_HOME 与默认路径不同，因此，直接指定 PATH 环境变量。
mvn clean install -DskipTest

echo -e "\n---> Test spring app..."
mvn test

echo -e "\n---> Generate Containerfile..."
cd ../
cat > Containerfile <<EOF
FROM nexus3.lab.example.com:8882/openjdk:17-jdk-alpine
MAINTAINER hualongfeiyyy@163.com

RUN mkdir /app
ADD ./build/target/spring-boot-helloworld-0.9.6-SNAPSHOT.jar /app

WORKDIR /app

EXPOSE 8080

ENTRYPOINT ["sh", "-c", "/opt/openjdk-17/bin/java -jar spring-boot-helloworld-0.9.6-SNAPSHOT.jar --server.port=80]
EOF

echo -e "\n---> Login and pull base image..."
podman login --tls-verify=false --username devuser0 --password 1qazZSE$ nexus3.lab.example.com:8882
podman pull --tls-verify=false nexus3.lab.example.com:8882/openjdk:17-jdk-alpine

echo -e "\n---> Build spring app image..."
podman build -t spring-boot-app:v1.0 --format=docker .
if [[ $? -eq 0 ]]; then
  podman tag localhost/spring-boot-app:v1.0 nexus3.lab.example.com:8882/spring-boot-app:v1.0
  echo -e "\n---> Push spring app image..."
  podman push --tls-verify=false nexus3.lab.example.com:8882/spring-boot-app:v1.0
  if [[ $? -eq 0 ]]; then
    echo -e "\n--> Remove local builded image..."
    podman rmi localhost/spring-boot-app:v1.0 nexus3.lab.example.com:8882/spring-boot-app:v1.0
  fi
else
  echo -e "\n---> [ERROR] Build failure..."
fi
```

以上脚本可参考 [jenkins-ci-plt/jenkins/free-style-demo/spring-boot-app.sh | GitHub](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/jenkins/free-style-demo/spring-boot-app.sh)

<center><img src="images/jenkins-create-freestyle-job-spring-boot-5.png" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-spring-boot-6.png" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-spring-boot-7.png" style="width:80%"></center>

<center><img src="images/jenkins-create-freestyle-job-spring-boot-8.png" style="width:80%"></center>

如上图所示，spring boot 应用构建测试完成，并将构建的应用镜像已推送至 Nexus3 中。

## 11. 设置 Jenkins 分布式构建环境

关于 Jenkins 分布式构建环境的说明，可参考此[文档](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/Jenkins%20%E5%88%86%E5%B8%83%E5%BC%8F%E6%9E%84%E5%BB%BA%E7%8E%AF%E5%A2%83/Jenkins%20%E5%88%86%E5%B8%83%E5%BC%8F%E6%9E%84%E5%BB%BA%E7%8E%AF%E5%A2%83.md)。

### 11.1 JNLP 连接方式

#### 11.1.1 配置 Master 支持 JNLP Agent

1️⃣ 登录 Jenkins UI，点击 Dashboard > Manage Jenkins > Security，修改 TCP port for inbound agents 参数，此参数用于 Master与 Agent之间的 JNLP 连接，默认为 Disable（不启用）。此处设置参数 Fixed 为 `50000` 端口监听 Agent：

> 注意：以上端口只要 Master上未被占用可自定义端口号。

<center><img src="images/jenkins-agent-config-1.jpg" style="width:80%"></center>

2️⃣ 设置完 Master的监听端口后，点击 Dashboard > Manage Jenkins > Nodes，进行 Agent的添加与管理：

<center><img src="images/jenkins-agent-config-2.jpg" style="width:80%"></center>

3️⃣ 点击 New Node 添加新节点：

<center><img src="images/jenkins-agent-config-3.jpg" style="width:80%"></center>

4️⃣ 指定新节点的名称与类型，点击 Create 创建节点：

> 💥 注意：此处的创建仅仅在 Master 上的节点列表中创建，仍需单独配置 Agent才能连接 Master。

<center><img src="images/jenkins-agent-config-4.jpg" style="width:80%"></center>

5️⃣ 如下图的重要参数说明：

- Number of executors 参数：
  - 在每个 Node 中定义执行器（Executor）的数量，即并发构建任务的最大数量。
  - 通常应该与 Agent 的 CPU 核心数相同。
  - 一个执行器可以被理解为一个单独的进程（事实上是线程）。
  - 在一个节点上可以运行多个执行器。如果执行器均在执行相应的作业，那么此节点上无法运行额外的作业，需等待作业完成后才能执行。
- Remote root directory 参数：
  - 指定远程 Agent 上的 Jenkins 根目录
  - 授权 Master 运行构建任务时使用的工作空间
  - Agent 不会存储关键数据，作业配置、构建日志和制品等都会存储在 Master 上。
  - 一般应该使用绝对路径

> 注意：此目录在 Agent上需提前创建，否则将 Agent连接 Master时将报错失败！如下所示：
>
> <center><img src="images/jenkins-agent-config-9.jpg" style="width:80%"></center>

<center><img src="images/jenkins-agent-config-5.jpg" style="width:80%"></center>

6️⃣ 如下图的重要参数说明：

- Labels 参数：
  - 指定 Agent 的标签
  - 用于对节点进行逻辑分组
  - 多个标签之间使用空格分隔
- Usage 参数：
  - 控制 Jenkins 如何在此 Agent 上安排构建任务
  - 两种方式：
    - Use this node as much as possible：尽可能地使用此节点
    - Only build jobs with label expressions matching this node：仅仅使用标签表达式匹配的节点构建作业
- Launch method 参数：
  - Master 识别此 Agent 主机并与其建立双向连接的方式
  - 两种方式：
    - Launch agent by connecting it to the controller：通过 Java Web 启动代理（JNLP）。此方法可跨平台，但是必须提前在固定 Agent上安装配置 JRE 环境，最常用的一种方式。
    - Launch agent via SSH：Master通过 SSH 连接到固定 Agent。此方式比较简单，但是不能跨平台。
- Availability 参数：
  - Master 与 Agent 间的连接保持机制
  - 三种方式：
    - Keep this agent online as much as possible：尽量保持代理在线
    - Bring this agent online according to a schedule：根据调度使代理上线
    - Bring this agent online when in demand, and take offline when idle：有需要时保持代理在线，空闲时使代理下线。
- Node properties 参数：
  - Environment variables：配置 Agent 使用的环境变量
  - Tools Location：配置 Agent 的构建工具与安装位置

<center><img src="images/jenkins-agent-config-6.jpg" style="width:80%"></center>

#### 11.1.2 添加支持 JNLP Agent

1️⃣ 在 Dashboard > Manage Jenkins > Nodes 中可见，新添加的 Agent处于离线状态，原因在于此节点上尚未连接至 Master。

<center><img src="images/jenkins-agent-config-7.jpg" style="width:80%"></center>

2️⃣ 点击此节点，使用给定的命令在对应的 Agent上运行。此处使用固定 Agent 类型，因此使用第一种方式添加。

<center><img src="images/jenkins-agent-config-8.jpg" style="width:80%"></center>

3️⃣ Agent上执行以下命令，可参考：

```bash
### 此节点使用 JNLP 连接 Master ###
[root@serverb ~]# curl -sO http://jenkins-master.lab.example.com:8080/jnlpJars/agent.jar
[root@serverb ~]# mkdir /opt/jenkins-agent0/  #创建 Jenkins 根目录
[root@serverb ~]# nohup java -jar agent.jar -url http://jenkins-master.lab.example.com:8080/ -secret f240790575bfd564d2ebda8142f8c074d6f1a4bac66b426bec9849340fd764ff -name "jenkins-agent0" -webSocket -workDir "/opt/jenkins-agent0" &
# 使用 nohup 将进程以后台运行（JDK 环境在 `9. 部署 Jenkins Master 服务` 中已部署完成）
```

如果不使用 nohup 命令运行而是前台直接运行，那么将返回以下输出：

```plaintext
Jun 24, 2025 6:32:45 AM org.jenkinsci.remoting.engine.WorkDirManager initializeWorkDir
INFO: Using /opt/jenkins-agent0/remoting as a remoting work directory
Jun 24, 2025 6:32:45 AM org.jenkinsci.remoting.engine.WorkDirManager setupLogging
INFO: Both error and output logs will be printed to /opt/jenkins-agent0/remoting
Jun 24, 2025 6:32:45 AM hudson.remoting.Launcher createEngine
INFO: Setting up agent: jenkins-agent0
Jun 24, 2025 6:32:45 AM hudson.remoting.Engine startEngine
INFO: Using Remoting version: 3261.v9c670a_4748a_9
Jun 24, 2025 6:32:45 AM org.jenkinsci.remoting.engine.WorkDirManager initializeWorkDir
INFO: Using /opt/jenkins-agent0/remoting as a remoting work directory
Jun 24, 2025 6:32:45 AM hudson.remoting.Launcher$CuiListener status
INFO: WebSocket connection open
Jun 24, 2025 6:32:45 AM hudson.remoting.Launcher$CuiListener status
INFO: Connected
...
```

4️⃣ Master上验证 Agent是否加入成功：

<center><img src="images/jenkins-agent-config-10.png" style="width:80%"></center>

如上图所示，Agent已成功与 Master连接，可用于后续的作业执行。

#### 11.1.3 调用 JNLP Agent 进行构建 —— 使用流水线风格作业

根据前文介绍的创建流水线风格作业的方法，此处创建名为 pipeline-test-labeld-agent 的作业测试 Agent。

<center><img src="images/jenkins-agent-pipeline-test-1.png" style="width:80%"></center>

```groovy
pipeline {
  agent {
    label 'node && maven && flask'  //使用对应 label 标签的 Agent执行作业
  }
 
  stages {
    stage('Print Agent Hostname') {  //返回执行作业的 Agent主机名，验证连接是否成功。
      steps {
        script {
          def hostname = sh(script: 'hostname', returnStdout: true).trim()
          echo "Running on agent with hostname: ${hostname}"
        }
      }
    }

    stage('Non-Paralles Stage') {
      steps {
        echo "This stage will be executed first."
      }
    }
 
    stage('Parallel Stage') {
      failFast true
      parallel {  //3个并行化阶段
        stage('Parallel One') {
          steps {
            echo "Parallel One"
          }
        }
        
        stage('Parallel Two') {
          steps {
            echo "Parallel Two"
          }
        }
        
        stage('Parallel Three') {  //stage 中嵌套 stages
          stages {
            stage('Nested 1') {
              steps {
                echo "In stage Nested 1 within Branch C"
              }
            }
            stage('Nested 2') {
              steps {
                echo "In stage Nested 2 within Branch C"
              }
            }
          }
        }
      }
    }
  }
}
```

以上 Jenkinsfile 可参考 [jenkins-ci-plt/jenkins/pipeline-test-labeld-agent/Jenkinsfile](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/jenkins/pipeline-test-labeld-agent/Jenkinsfile)。

执行此作业，可在 Blue Ocean 中返回如下测试结果，即为测试成功。

<center><img src="images/jenkins-agent-pipeline-test-2.png" style="width:80%"></center>

### 11.2 SSH 连接方式

#### 11.2.1 调整 Master 支持 SSH 连接 Agent

## 附录A. PostgreSQL 常用命令

### A.1 登录数据库

```bash
# 方法1：命令行指定登录
$ psql -U <username> -d <db_name> -h <pg_server> -p <port>
# 指定数据库登录用户名、密码、连接的服务器地址与监听端口（默认监听 5432 端口）
$ psql -U etherpad_user -d etherpad_db -h 172.25.250.13
Password for user etherpad_user:  #输入此用户密码即可登录 
psql (13.7)
Type "help" for help.

etherpad_db=> 

# 方法2：设置环境变量登录
$ export PGDATABASE=<db_name>
$ export PGUSER=<db_username>
$ export PGHOST=<db_host>
$ export PGPORT=<db_port>
$ psql
psql (13.7)
Type "help" for help.

postgres=#
```

### A.2 更新数据库管理员 postgres 密码

```bash
(root)$ su - postgres
# PostgreSQL 服务器节点切换至 postgres 用户，此用户作为 PostgreSQL 服务器的管理员用户。
(postgres)$ psql -c "ALTER USER postgres WITH PASSWORD '<password>';"
# 更新 postgres 管理员用户密码
```

### A.3 用户（角色）与数据库操作

```bash
(postgres)$ psql -U postgres
Password for user postgres: 
psql (13.7)
Type "help" for help.

postgres=# CREATE USER <username> WITH PASSWORD '<password>';  #创建指定用户
postgres=# CREATE DATABASE <db_name> OWNER <username>;  #创建所属指定用户的数据库
postgres=# GRANT ALL PRIVILEGES ON DATABASE <db_name> TO <username>;  #授权数据库

postgres=# DROP DATABASE <db_name>  #删除数据库
postgres=# DROP USER <username>  #输出用户

postgres=# \du  #查看所以用户
                                     List of roles
   Role name   |                         Attributes                         | Member of 
---------------+------------------------------------------------------------+-----------
 etherpad_user |                                                            | {}
 postgres      | Superuser, Create role, Create DB, Replication, Bypass RLS | {}

postgres=# \l  #查看所以数据库
                                          List of databases
    Name     |     Owner     | Encoding |   Collate   |    Ctype    |        Access privileges        
-------------+---------------+----------+-------------+-------------+---------------------------------
 etherpad_db | etherpad_user | UTF8     | en_US.UTF-8 | en_US.UTF-8 | =Tc/etherpad_user              +
             |               |          |             |             | etherpad_user=CTc/etherpad_user
 postgres    | postgres      | UTF8     | en_US.UTF-8 | en_US.UTF-8 | 
 template0   | postgres      | UTF8     | en_US.UTF-8 | en_US.UTF-8 | =c/postgres                    +
             |               |          |             |             | postgres=CTc/postgres
 template1   | postgres      | UTF8     | en_US.UTF-8 | en_US.UTF-8 | =c/postgres                    +
             |               |          |             |             | postgres=CTc/postgres
(4 rows)
```

### A.4 psql 常用的元命令

| 元命令 | 作用 | 元命令 | 作用 |
| ----- | ----- | ----- | ----- |
| \l | 列出数据库中所有数据库的名称、所有者、字符集编码以及使用权限等 | \d | 列出当前 search_path 中模式下所有的表、视图和序列 |
| \db | 列出所有可用的表空间 | \dn | 列出所有的模式（名称空间）|
| \du | 列出所有的数据库角色 | \dt | 列出数据库中的表 |
| \di | 列出所有的索引 | \dv | 列出所有的视图 |
| \ds | 列出所有的序列 | \dp | 列出权限信息 |
| \d *table* | 列出表的详细信息 | \d *index* | 列出索引的详细信息 |
| \df | 列出所有的函数 | \sf | 列出函数的定义 |
| \timing | 显示每条 SQL 语句的执行时间（以毫秒为单位）| \echo *string* | 把字符串写到标准输出 |
| \i *file*.sql | 从文件中读取内容，并将其当作输入，执行查询 | \\! *os_command* | 执行操作系统命令 |
| \? | 查看 psql 的帮助命令 | \h | 查看 SQL 语法帮助 |
| \conninfo | 查询当前连接的数据库的信息 | \c *dbname* *dbuser* | 更换连接的数据库和用户（角色）|
| \o *file* | 把所有的查询结果发送到文件中 | \q | 退出 psql |

## 参考链接

- 📚 [Apache Maven 3.x | Maven](https://maven.apache.org/ref/3.9.9/)
- [containers.podman.podman_container module – Manage podman containers | Ansible Docs](https://docs.ansible.com/ansible/latest/collections/containers/podman/podman_container_module.html)
- [Community.Postgresql | Ansible Docs](https://docs.ansible.com/ansible/latest/collections/community/postgresql/index.html)
- [ansible.builtin.pip module – Manages Python library dependencies | Ansible Docs](https://docs.ansible.com/ansible/latest/collections/ansible/builtin/pip_module.html)
- [community.postgresql | Galaxy Docs](https://galaxy.ansible.com/ui/repo/published/community/postgresql/?version=4.0.0)
- [Automate container and pod deployments with Podman and Ansible | RedHat Blog](https://www.redhat.com/en/blog/ansible-podman-container-deployment)
- [How to use 'mv' command to move files except those in a specific directory? | stack overflow](https://stackoverflow.com/questions/4612157/how-to-use-mv-command-to-move-files-except-those-in-a-specific-directory)
- [Cannot run docker commands through Jenkin's Blue Ocean: ERRO[0000] No subuid ranges found for user “jenkins” in /etc/subuid | stack overflow](https://stackoverflow.com/questions/58855758/cannot-run-docker-commands-through-jenkins-blue-ocean-erro0000-no-subuid-ran)
- [pip cannot uninstall <package>: "It is a distutils installed project" | stack overflow](https://stackoverflow.com/questions/53807511/pip-cannot-uninstall-package-it-is-a-distutils-installed-project)
- [tensorflow/tensorflow | DockerHub](https://hub.docker.com/r/tensorflow/tensorflow/tags/)
- [PostgreSQL 入门指南：安装、配置与基本命令](https://developer.aliyun.com/article/1655700)
- [python 报错 Missing dependencies for SOCKS support 解决方法](https://blog.csdn.net/whatday/article/details/109287343)
- [Jenkins 配置分布式构建环境 —— 添加固定 Agent 并使用 JNLP 启动 Agent 详解](https://www.cnblogs.com/zhangmingcheng/p/18356890)
