# Jenkins 的 CI/CD 之旅 —— Node.js 之 npm 常用方法

## 文档目录

- [Jenkins 的 CI/CD 之旅 —— Node.js 之 npm 常用方法](#jenkins-的-cicd-之旅--nodejs-之-npm-常用方法)
  - [文档目录](#文档目录)
  - [1. 安装 Node.js 环境](#1-安装-nodejs-环境)
    - [1.1 方式1：RPM 软件包方式安装（全局安装）](#11-方式1rpm-软件包方式安装全局安装)
    - [1.2 方式2：NVM 管理多版本 Node.js 环境（用户环境安装）](#12-方式2nvm-管理多版本-nodejs-环境用户环境安装)
  - [2. 常用 Node.js 国内加速源](#2-常用-nodejs-国内加速源)
    - [2.1 永久配置上游 registry](#21-永久配置上游-registry)
    - [2.2 临时配置上游 registry](#22-临时配置上游-registry)
  - [3. 安装 pnpm 包管理器](#3-安装-pnpm-包管理器)
  - [4. npm 命令管理模块](#4-npm-命令管理模块)
    - [4.1 查看 npm 帮助文档](#41-查看-npm-帮助文档)
    - [4.2 安装 npm 模块](#42-安装-npm-模块)
    - [4.3 查看模块可用的版本](#43-查看模块可用的版本)
    - [4.4 安装模块](#44-安装模块)
  - [5. npm 缓存操作](#5-npm-缓存操作)
  - [6. 参考链接](#6-参考链接)

## 1. 安装 Node.js 环境

对 "全局" 概念的说明：

- 如果采用 RPM 方式安装，那么全局指的是系统路径 /usr/local/lib/node_modules/ 目录。
- 如果采用 NVM 多版本 Node.js 环境管理，那么全局指的是当前用户家目录中的 $HOME/.nvm 目录。

### 1.1 方式1：RPM 软件包方式安装（全局安装）

```bash
$ curl -sL https://rpm.nodesource.com/setup_18.x | sudo -E bash -
2025-05-19 11:50:54 - Cleaning up old repositories...
2025-05-19 11:50:54 - Old repositories removed
2025-05-19 11:50:54 - Supported architecture: x86_64
2025-05-19 11:50:54 - Added N|Solid repository for LTS version: 18.x
2025-05-19 11:50:54 - dnf available, updating...
Node.js Packages for Linux RPM based distros - x86_64                                                                294 kB/s | 1.0 MB     00:03
Last metadata expiration check: 0:00:03 ago on Mon 19 May 2025 11:50:56 AM EDT.
Metadata cache created.
N|Solid Packages for Linux RPM based distros - x86_64                                                                230 kB/s | 665 kB     00:02
Last metadata expiration check: 0:00:02 ago on Mon 19 May 2025 11:51:00 AM EDT.
Metadata cache created.
2025-05-19 11:51:02 - Repository is configured and updated.
2025-05-19 11:51:02 - You can use N|solid Runtime as a node.js alternative
2025-05-19 11:51:02 - To install N|solid Runtime, run: dnf install nsolid -y
2025-05-19 11:51:02 - Run 'dnf install nodejs -y' to complete the installation.
# 获取 Node.js 18 软件安装源

$ sudo dnf install nodejs
# 安装 Node.js 18.x 环境

$ sudo node -v
v18.20.8
# 查看 node 版本

$ sudo npm -v
10.8.2
# 查看 npm 版本
```

### 1.2 方式2：NVM 管理多版本 Node.js 环境（用户环境安装）

NVM（Node Version Manager）是一款用于在同一台主机上安装、管理和切换多个 Node.js 版本的命令行工具。它允许开发者轻松地在不同项目所需的 Node.js 版本（如 v14、v16、v18 等）之间切换，解决版本不兼容导致的开发环境问题，具有安装、卸载、列出和设置默认版本等核心功能。

```bash
### 以普通用户 devops 为例 ###
$ su - devops
$ curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.4/install.sh | bash
=> Downloading nvm from git to '/home/alberthua/.nvm'
=> Cloning into '/home/alberthua/.nvm'...
remote: Enumerating objects: 424, done.
remote: Counting objects: 100% (424/424), done.
remote: Compressing objects: 100% (351/351), done.
remote: Total 424 (delta 59), reused 188 (delta 45), pack-reused 0 (from 0)
Receiving objects: 100% (424/424), 414.13 KiB | 29.00 KiB/s, done.
Resolving deltas: 100% (59/59), done.
* (HEAD detached at FETCH_HEAD)
  master
=> Compressing and cleaning up git repository

=> nvm source string already in /home/alberthua/.bashrc
=> bash_completion source string already in /home/alberthua/.bashrc
=> Close and reopen your terminal to start using nvm or run the following to use it now:

export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm
[ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"  # This loads nvm bash_completion
# 在用户家目录中安装 NVM，路径位于 $HOME/.nvm。

$ nvm install --lts
Installing latest LTS version.
Downloading and installing node v24.15.0...
Downloading https://nodejs.org/dist/v24.15.0/node-v24.15.0-linux-x64.tar.xz...
############################################################################### 100.0%
Computing checksum with sha256sum
Checksums matched!
Creating default alias: default -> lts/* (-> v24.15.0 *)
# 安装 node 长期稳定版

$ nvm --version
0.40.4

$ node --version
v24.15.0

$ npm --version
11.12.1
```

## 2. 常用 Node.js 国内加速源

| 站点名称 | URL 地址 |
| ----- | ----- |
| 淘宝 npm 镜像站 | `https://registry.npmmirror.com` |
| 华为 npm 镜像站 | `https://mirrors.huaweicloud.com/repository/npm/` |
| 腾讯 npm 镜像站 | `http://mirrors.cloud.tencent.com/npm/` |

### 2.1 永久配置上游 registry

1️⃣ 方式1：命令行配置方式：

```bash
$ npm config set registry <registry_url>
# 设置上游 npm 仓库

### 示例 ###
$ npm config set registry https://registry.npmmirror.com
# 设置淘宝 npm 仓库
$ npm config get registry
# 获取设置的 npm 仓库
```

2️⃣ 方式2：配置文件方式：~/.npmrc 或 ./.npmrc

```bash
### 编辑用户家目录中的 ~/.npmrc 文件
$ cat > ~/.npmrc <<EOF
registry=https://registry.npmmirror.com
EOF

### 编辑当前项目目录中的 .npmrc 文件
$ cat > ./.npmrc <<EOF
registry=https://registry.npmmirror.com
EOF
```

### 2.2 临时配置上游 registry

```bash
$ npm install express --registry=https://registry.npmmirror.com
# 命令行临时使用上游 registry 安装模块
```

## 3. 安装 pnpm 包管理器

Node.js 除了使用 npm 包管理器外，还可使用 pnpm 包管理器。

```bash
$ npm view pnpm versions
# 查看 pnpm 可用的版本

$ npm install -g pnpm@10.10.0

added 1 package in 27s

1 package is looking for funding
  run `npm fund` for details
# 全局安装 pnpm 包管理器
```

## 4. npm 命令管理模块

### 4.1 查看 npm 帮助文档

```bash
$ npm help npm
$ npm help install
$ npm help npmrc
```

### 4.2 安装 npm 模块

```bash
$ npm config ls -l
# 查看更加详细的 npm 模块管理配置

### RPM 方式安装的 Node.js 环境 ###
$ npm root list
$PWD/node_modules
# 查看模块默认安装路径
# node_modules 目录默认未创建，此目录默认在当前工作目录中。

$ npm root list -g
/usr/local/lib/node_modules
# 查看系统全局 npm 模块安装路径

$ npm list -g
/usr/lib
├── corepack@0.32.0
├── npm@10.8.2
└── pnpm@10.10.0
# 查看系统全局已安装的模块

$ npm list -g --depth=0
/usr/lib
├── corepack@0.32.0
├── npm@10.8.2
└── pnpm@10.10.0
# 查看系统全局已安装的模块，并且只显示第一层目录，不显示子目录与子文件。

### NVM 方式安装的 Node.js 环境 ###
$ npm root list
$PWD/node_modules
# 查看模块默认安装路径
# 与方式1相同，默认安装路径位于当前工作目录。

$ npm root list -g
$HOME/.nvm/versions/node/${VERSION}/lib/node_modules
# node_modules 目录暂未创建，在下载模块时自动创建。
```

### 4.3 查看模块可用的版本

```bash
$ npm view <module_name> versions
# 查看本地已安装的模块版本

$ npm view <module_name> versions --registry=<registry_url>
# 查看指定上游 registry 中可用的模块版本
$ npm view react versions --registry=https://registry.npmmirror.com
# 查看指定上游 registry 中可用的 react 模块版本
```

### 4.4 安装模块

```bash
$ npm install react --loglevel info --registry=https://registry.npmmirror.com
# 指定上游 registry 与日志等级安装 react 模块
```

## 5. npm 缓存操作

```bash
$ npm cache ls
$ npm cache verify
# 查看与确认 npm 缓存
$ npm cache clean --force
# 强制清除 npm 缓存

### 安装模块失败时操作
$ npm cache clean --force
$ rm -rf node_modules/ package-lock.json
```

## 6. 参考链接

- [Node.js 官网](https://nodejs.org/zh-cn)
