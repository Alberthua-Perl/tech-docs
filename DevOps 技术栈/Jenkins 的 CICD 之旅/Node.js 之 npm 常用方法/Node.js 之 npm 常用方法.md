# Jenkins 的 CI/CD 之旅 —— Node.js 之 npm 常用方法

## 文档目录

- [安装 Node.js 环境](#安装-nodejs-环境)
- [安装 pnpm 包管理器](#安装-pnpm-包管理器)
- [使用 npm 管理包](#使用-npm-管理包)

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

## 使用 npm 管理包

注意：应用目录中直接运行 npm 检索当前目录中存在的模块

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

$ npm -v
# 查看 npm 的版本
```
