# Jenkins 的 CI/CD 之旅（）—— Node.js 之 npm 常用方法

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

## npm 包管理

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

## 🧪 编译并发布 etherpad-lite 应用

```bash
[devops@serverb etherpad-lite]$ pnpm install
Scope: all 6 workspace projects
Lockfile is up to date, resolution step is skipped
Packages: +1
+
Progress: resolved 1, reused 0, downloaded 1, added 1, done
Done in 5.7s
# 在当前的项目目录的 node_modules/ 中安装包

[devops@serverb etherpad-lite]$ pnpm run build:etherpad

> etherpad@2.3.0 build:etherpad /home/devops/etherpad-lite
> pnpm --filter admin run build-copy && pnpm --filter ui run build-copy


> admin@2.3.0 build-copy /home/devops/etherpad-lite/admin
> tsc && vite build --outDir ../src/templates/admin --emptyOutDir

vite v6.3.5 building for production...
✓ 1742 modules transformed.
../src/templates/admin/index.html                   0.49 kB │ gzip:   0.31 kB
../src/templates/admin/assets/index-BSQTLHmC.css   10.89 kB │ gzip:   3.19 kB
../src/templates/admin/assets/index-CmBU3i6n.js   479.31 kB │ gzip: 154.07 kB
[vite-plugin-static-copy] Copied 1 items.
✓ built in 9.51s

> ui@0.0.0 build-copy /home/devops/etherpad-lite/ui
> tsc && vite build --outDir ../src/static/oidc --emptyOutDir

vite v6.3.5 building for production...
✓ 6 modules transformed.
../src/static/oidc/consent.html               1.01 kB │ gzip: 0.49 kB
../src/static/oidc/login.html                 2.60 kB │ gzip: 1.03 kB
../src/static/oidc/assets/style-CLgJS82q.css  1.58 kB │ gzip: 0.75 kB
../src/static/oidc/assets/main-BcSxKxdW.js    0.15 kB │ gzip: 0.15 kB
../src/static/oidc/assets/style-CNP0ENT6.js   0.71 kB │ gzip: 0.40 kB
../src/static/oidc/assets/nested-DhUhjXU9.js  1.06 kB │ gzip: 0.53 kB
✓ built in 185ms
# 编译应用

[devops@serverb etherpad-lite]$ pnpm run prod

> etherpad@2.3.0 prod /home/devops/etherpad-lite
> pnpm --filter ep_etherpad-lite run prod


> ep_etherpad-lite@2.3.0 prod /home/devops/etherpad-lite/src
> cross-env NODE_ENV=production node --require tsx/cjs node/server.ts

[2025-05-19T22:35:22.481] [INFO] settings - All relative paths will be interpreted relative to the identified Etherpad base dir: /home/devops/etherpad-lite
[2025-05-19T22:35:22.485] [WARN] settings - No settings file found in /home/devops/etherpad-lite/settings.json. Continuing using defaults!
[2025-05-19T22:35:22.486] [INFO] settings - No credentials file found in /home/devops/etherpad-lite/credentials.json. Ignoring.
[2025-05-19T22:35:22.487] [WARN] settings - loglevel: INFO
[2025-05-19T22:35:22.492] [WARN] settings - logLayoutType: colored
[2025-05-19T22:35:22.493] [WARN] settings - No "skinName" parameter found. Please check out settings.json.template and update your settings.json. Falling back to the default "colibris".
[2025-05-19T22:35:22.494] [INFO] settings - Using skin "colibris" in dir: /home/devops/etherpad-lite/src/static/skins/colibris
[2025-05-19T22:35:22.494] [WARN] settings - File location: /home/devops/etherpad-lite/var/rusty.db
[2025-05-19T22:35:22.495] [INFO] settings - Random string used for versioning assets: 7718e753
[2025-05-19T22:35:23.851] [INFO] server - Starting Etherpad...
[2025-05-19T22:35:23.911] [INFO] plugins - check installed plugins for migration
[2025-05-19T22:35:23.921] [INFO] plugins - pnpm --version: 8.15.9
[2025-05-19T22:35:23.924] [INFO] plugins - Loading plugin ep_etherpad-lite...
[2025-05-19T22:35:23.925] [INFO] plugins - Loaded 1 plugins
[2025-05-19T22:35:25.491] [INFO] server - Installed plugins:
[2025-05-19T22:35:25.493] [INFO] settings - Report bugs at https://github.com/ether/etherpad-lite/issues
[2025-05-19T22:35:25.494] [INFO] settings - Your Etherpad version is 2.3.0 (3a9e126)
[2025-05-19T22:35:29.306] [INFO] http - HTTP server listening for connections
[2025-05-19T22:35:29.307] [INFO] settings - You can access your Etherpad instance at http://0.0.0.0:9001/
[2025-05-19T22:35:29.307] [WARN] settings - Admin username and password not set in settings.json. To access admin please uncomment and edit "users" in settings.json
[2025-05-19T22:35:29.308] [INFO] server - Etherpad is running
...
# 运行测试应用
```
