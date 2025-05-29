# Jenkins 的 CI/CD 之旅 —— Node.js 之 npm 常用方法

## 文档目录

- [安装 Node.js 环境](#安装-nodejs-环境)
- [安装 pnpm 包管理器](#安装-pnpm-包管理器)
- [使用 npm 管理模块](#使用-npm-管理模块)
- [常用 Node.js 国内加速源](#常用-nodejs-国内加速源)
- [配置上游 registry](#配置上游-registry)
- [npm 缓存操作](#npm-缓存操作)
- [参考链接](#参考链接)

## 安装 Node.js 环境

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

$ node -v
v18.20.8
# 查看 node 版本

$ npm -v
10.8.2
# 查看 npm 版本
```

## 安装 pnpm 包管理器

```bash
$ npm view pnpm versions
# 查看 pnpm 可用的版本

$ sudo npm install -g pnpm@10.10.0

added 1 package in 27s

1 package is looking for funding
  run `npm fund` for details
# 全局安装 pnpm 包管理器
```

## 使用 npm 管理模块

> 💥 注意：应用目录中直接运行 npm 检索当前目录中存在的模块

查看 npm 帮助文档：

```bash
$ npm help npm
$ npm help install
$ npm help npmrc
```

查看 npm 当前目录或全局配置：

```bash
$ npm config ls -l
# 查看更加详细的 npm 模块管理配置

$ sudo npm list -g
/usr/lib
├── corepack@0.32.0
├── npm@10.8.2
└── pnpm@10.10.0
# 查看系统全局已安装的包

$ sudo npm list -g --depth=0
/usr/lib
├── corepack@0.32.0
├── npm@10.8.2
└── pnpm@10.10.0
# 查看系统全局已安装的包，并且只显示第一层目录，不显示子目录与子文件。
```

查看模块可用的版本：

```bash
$ npm view <module_name> versions
$ npm --registry=https://registry.npmmirror.com view react versions
# 指定查看上游 registry 中可用的 react 模块版本
```

安装模块：

```bash
$ npm --loglevel info --registry=https://registry.npmmirror.com install react
# 指定上游 registry 与日志等级安装 react 模块
```

## 常用 Node.js 国内加速源

- 淘宝 npm 镜像站：`https://registry.npmmirror.com`
- 华为 npm 镜像站：`https://mirrors.huaweicloud.com/repository/npm/`
- 腾讯 npm 镜像站：`http://mirrors.cloud.tencent.com/npm/`

### 配置上游 registry

永久配置（命令行）：

```bash
$ npm config set registry <registry_url>
# 设置上游 npm 仓库
$ npm config set registry https://registry.npmmirror.com
# 设置淘宝 npm 仓库
$ npm config get registry
# 获取设置的 npm 仓库
```

永久配置（~/.npmrc 或 ./.npmrc）：

```bash
### 编辑用户家目录中的 ~/.npmrc 文件
$ cat > ~/.npmrc <<EOF
registry=https://registry.npmmirror.com
EOF

### 编辑应用目录中的 .npmrc 文件
$ cat > .npmrc <<EOF
registry=https://registry.npmmirror.com
EOF
```

临时使用（命令行）：

```bash
$ npm --registry=https://registry.npmmirror.com install express
# 命令行临时使用安装模块
```

## npm 缓存操作

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

## 参考链接

- [Node.js 官网](https://nodejs.org/zh-cn)
