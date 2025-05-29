# 🧪 基于 Ansible Navigator 部署分布式 Jenkins CI/CD 平台 —— 构建发布 Java、Node.js、Flask 与 Golang 应用

## 文档说明

- 本文档给出使用 Ansible Navigator 自动化部署 Jenkins 分布式构建平台（Master+Agent），并发布 Java、Node.js、Flask 与 Golang 应用。
- 本实验使用 RedHat RH294v9.0 实验环境，各节点的资源调整如下：

  | 主机名 | 主机别名 | IPv4 | vCPU | 内存 | 节点角色 |
  | ----- | ----- | ----- | ----- | ----- | ----- |
  | workstation.lab.example.com | gitlab-ce.lab.example.com | 172.25.250.9 |   |   | GitLab CE |
  | servera.lab.example.com | jenkins-master.lab.example.com | 172.25.250.10 |   |   | Jenkins Master |
  | serverb.lab.example.com | jenkins-agent0.lab.example.com | 172.25.250.11 |   |   | Jenkins Agent |
  | serverc.lab.example.com | jenkins-agent1.lab.example.com | 172.25.250.12 |   |   | Jenkins Agent |
  | serverd.lab.example.com | nexus3.lab.example.com | 172.25.250.13 |   |   | Nexus3 & PostgreSQL Server |
  | utility.lab.example.com | NA | 172.25.250.220 |   |   | Application Test & RedHat AAP2.2 |

## 文档目录

- [Lab 相关用户名与登录密码](#lab-相关用户名与登录密码)
- [Foundation 中设置节点资源](#foundation-中设置节点资源)
  - [设置虚拟机最大内存使用量与当前内存使用量](#设置虚拟机最大内存使用量与当前内存使用量)
  - [配置 classroom 可实现外网连接](#配置-classroom-可实现外网连接)
- [注意 Ansible 清单文件中的 ansible 变量 ansible_host](#注意-ansible-清单文件中的-ansible-变量-ansible_host)
- [生成 devops 用户 SSH 公私钥并实现各节点免密登录](#生成-devops-用户-ssh-公私钥并实现各节点免密登录)
- [部署与设置 GitLab-CE 容器](#部署与设置-gitlab-ce-容器)
  - [安装 Ansible 内容集合（collection）](#安装-ansible-内容集合collection)
  - [部署 GitLab-CE 容器](#部署-gitlab-ce-容器)
  - [重置 GitLab-CE 的 root 密码](#重置-gitlab-ce-的-root-密码)
  - [创建与批准 GitLab-CE 的 devuser0 开发者用户](#创建与批准-gitlab-ce-的-devuser0-开发者用户)
  - [GitLab-CE 中导入外部代码库（etherpad-lite）](#gitlab-ce-中导入外部代码库etherpad-lite)
- [部署与设置 Nexus3 容器](#部署与设置-nexus3-容器)
  - [部署 Nexus3 容器](#部署-nexus3-容器)
  - [创建 Nexus3 的 devuser0 用户](#创建-nexus3-的-devuser0-用户)
  - [创建 Nexus3 的容器镜像仓库](#创建-nexus3-的容器镜像仓库)
  - [创建 Nexus3 的 npm 构建仓库（proxy 类型）](#创建-nexus3-的-npm-构建仓库proxy-类型)
- [【可选步骤】部署 npm 运行及构建环境](#可选步骤部署-npm-运行及构建环境)
- [部署 Jenkins Master 服务](#部署-jenkins-master-服务)
- [PostgreSQL 常用命令](#postgresql-常用命令)
  - [登录数据库](#登录数据库)
  - [更新数据库管理员 postgres 密码](#更新数据库管理员-postgres-密码)
  - [用户（角色）与数据库操作](#用户角色与数据库操作)
  - [psql 常用的元命令](#psql-常用的元命令)
- [参考链接](#参考链接)

## Lab 相关用户名与登录密码

| 节点角色 | 管理员用户 | 管理员密码 | 普通用户 | 普通用户密码 |
| ----- | ----- | ----- | ----- | ----- |
| GitLab CE | root | 1qazZSE$ | devuser0 | 1qazZSE$ |
| Nexus3 | admin | admin123 | devuser0 | 1qazZSE$ |
| Jenkins Master | admin | 1qazZSE$ | devuser0 | 1qazZSE$ |
| RedHat AAP2.2 | admin | redhat | NA | NA |

## Foundation 中设置节点资源

### 设置虚拟机最大内存使用量与当前内存使用量

**关机状态**下设置最大内存使用量：

```bash
[kiosk@foundation0 ~]$ wget http://content.example.com/jenkins-ci-plt/set_vm_maxmem
[kiosk@foundation0 ~]$ ./set_vm_maxmem
```

**开机状态**下设置当前内存使用量：

```bash
[kiosk@foundation0 ~]$ rht-vmctl start all  #wait several minutes
[kiosk@foundation0 ~]$ wget http://content.example.com/jenkins-ci-plt/set_vm_current_mem
[kiosk@foundation0 ~]$ ./set_vm_current_mem
```

### 配置 classroom 可实现外网连接

```bash
[kiosk@foundation0 ~]$ wget http://content.example.com/jenkins-ci-plt/set_classroom_extnet
[kiosk@foundation0 ~]$ ./set_classroom_extnet
```

## 注意 Ansible 清单文件中的 ansible 变量 ansible_host

- 在 `ini` 或 `yaml` 格式的主机清单文件中使用 `ansible_host` 变量：

  ```ini
  ...
  [master]
  jenkins-master.lab.example.com  ansible_host=servera.lab.example.com
  #inventory_hostname             #ansible_host
  ...
  ```

- Ansible 使用 `ansible_host` 变量指定的真实主机进行 SSH 连接, 而 `inventory_hostname` 变量仅仅在 Ansible 运行返回中显示。因此，需要将以上两个变量指定的>主机名称均写入 `/etc/hosts` 文件中完成主机名解析（笔者环境中使用），当然也可加入上游 DNS 服务器的解析文件中实现。

## 生成 devops 用户 SSH 公私钥并实现各节点免密登录

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

## 部署与设置 GitLab-CE 容器

### 安装 Ansible 内容集合（collection）

```bash
[kiosk@foundation0 ~]$ ssh devops@workstation
[devops@workstation ~]$ eval $(ssh-agent)
[devops@workstation ~]$ ssh-add ~/.ssh/devops-jenkins
[devops@workstation ~]$ cd ~/jenkins-ci-plt
[devops@workstation jenkins-ci-plt]$ ansible-galaxy collection install -r collections/requirements.yml -p collections/
[devops@workstation jenkins-ci-plt]$ ansible-galaxy collection list
```

### 部署 GitLab-CE 容器

```bash
[devops@workstation jenkins-ci-plt]$ sh scm-artifact/deploy-prep.sh
# 部署容器前准备
## 可选命令：$ ansible-playbook --list-tags <playbook_name>.yml  #列举 playbook 中的 tag
[devops@workstation jenkins-ci-plt]$ podman login utility.lab.example.com  #login: admin/redhat
[devops@workstation jenkins-ci-plt]$ podman pull utility.lab.example.com/ansible-automation-platform-22/ee-supported-rhel8:latest
[devops@workstation jenkins-ci-plt]$ ansible-navigator run scm-artifact/prep-scm-nexus3.yml --tag gitlab-ce
# 部署 gitlab-ce 容器（容器启动需 7~8 分钟），容器状态将从 starting -> unhealthy -> healthy。
```

### 重置 GitLab-CE 的 root 密码

### 创建与批准 GitLab-CE 的 devuser0 开发者用户

- Web 注册界面创建 devuser0 开发者用户
- 设置该用户的 SSH 连接公钥（此处使用 `devops@workstation` 中名为 devops-jenkins.pub 的 SSH 公钥）

### GitLab-CE 中导入外部代码库（etherpad-lite）

## 部署与设置 Nexus3 容器

### 部署 Nexus3 容器

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run scm-artifact/prep-scm-nexus3.yml --tag nexus3
# 部署 nexus3 容器（容器启动需 2~3 分钟）
```

### 创建 Nexus3 的 devuser0 用户

### 创建 Nexus3 的容器镜像仓库

### 创建 Nexus3 的 npm 构建仓库（proxy 类型）

## 【可选步骤】部署 npm 运行及构建环境

此步为可选步骤，该实验中不执行。

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run build-env/prep-nodejs-env.yml
```

## 部署与设置 PostgreSQL 数据库

该 postgresql 数据库服务器用于 Node.js 应用 etherpad-lite 在运行后的数据连接。etherpad-lite 应用在容器镜像构建过程中通过本地的 Nexus3 的 npm-proxy 仓库安装依赖模块，构建与指定执行应用方式。当应用容器镜像构建完成，可将其推送至本地 Nexus3 的 docker-hosted 镜像仓库中，待到部署此应用时拉取运行即可。连接 postgresql 服务器的应用，如果通过应用 Web 界面编辑文本，那么这些文本将存储于数据库中。

### 安装部署 PostgreSQL 数据库

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run build-env/setup-postgres.yml
[devops@workstation jenkins-ci-plt]$ ssh root@serverd
[root@serverd ~]# su - postgres
[postgres@serverd ~]$ psql -c "ALTER USER postgres WITH PASSWORD '1qazZSE$';"
ALTER ROLE
# 默认免密码登录本地数据库服务器，因此，设置数据库服务器 postgres 管理员密码。
[postgres@serverd ~]$ exit
logout

[devops@workstation jenkins-ci-plt]$ ansible-navigator run build-env/setup-postgres.yml --tag allow_local
# 数据库服务器 postgres 管理员密码设置完成后，配置以密码认证方式本地连接数据库。
```

## 部署 Jenkins Master 服务

```bash
[devops@workstation jenkins-ci-plt]$ ansible-navigator run jenkins/jkn-cluster.yml --tag master_config
# 部署 Jenkins Master 节点服务
```

## PostgreSQL 常用命令

### 登录数据库

```bash
# 方法1：命令行指定登录
$ psql -U <username> -d <db_name> -h <pg_server> -p <port>
# 指定数据库登录用户名、密码、连接的服务器地址与监听端口（默认监听 5432 端口）
$ psql -U etherpad_user -d etherpad_db -h 172.25.250.13
Password for user etherpad_user:  #输入该用户密码即可登录 
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

### 更新数据库管理员 postgres 密码

```bash
(root)$ su - postgres
# PostgreSQL 服务器节点切换至 postgres 用户，该用户作为 PostgreSQL 服务器的管理员用户。
(postgres)$ psql -c "ALTER USER postgres WITH PASSWORD '<password>';"
# 更新 postgres 管理员用户密码
```

### 用户（角色）与数据库操作

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

### psql 常用的元命令

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

- [containers.podman.podman_container module – Manage podman containers | Ansible Docs](https://docs.ansible.com/ansible/latest/collections/containers/podman/podman_container_module.html)
- [Community.Postgresql  | Ansible Docs](https://docs.ansible.com/ansible/latest/collections/community/postgresql/index.html)
- [community.postgresql | Galaxy Docs](https://galaxy.ansible.com/ui/repo/published/community/postgresql/?version=4.0.0)
