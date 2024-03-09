## Git 基础进阶与场景实践

-----

### 文档说明

- 该文档中 Git 术语的工作区与工作树（working tree）同义
- 该文档中所涉及的 Git 命令与原理将持续更新

-----

### 文档目录

- 基础架构即代码
- Git 简介
- 设置 Git 代理
- 设置 Git 环境
- Git 工作流描述
- 初始化 Git 仓库
- 克隆远程代码仓库
- Git 工作区、暂存区操作
- Git 暂存操作
- Git 本地仓库、远程代码库操作
- Git 提交日志规范的说明
- 🛠 git push 推送的故障排查
- Git 日志检索
- Git 分支描述
- Git 分支操作
- 从旧提交创建分支
- 🔥 Git 合并分支
- 🔥 Git 变基
- Git 版本回退
- Git 还原提交
- Git 拣选
- 参考链接

-----

### 基础架构即代码

- 一个关键的 `DevOps` 概念是基础架构即代码（infrastructure as code）。

- 不必手动管理基础架构，而是通过运行自动化代码来定义和构建系统。

- Red Hat Ansible 自动化是一种可实施这种方法的关键工具。

- 若 Ansible 项目是用于定义基础架构的代码，则应使用 Git 等版本控制系统（version control system）来跟踪和控制代码的更改。

- 版本控制也能够为基础架构代码的不同阶段（如开发、QA 和生产）实施生命周期。

- 可以将更改提交到分支，并在非关键性开发和 QA 环境中测试这些更改。

- 确认更改后，可以将它们合并到主生产代码，并将更改应用到生产基础架构。

-----

### Git 简介

> 💥 注意：Git v1.x 与 Git v2.x 存在差异！

- Git 是一种分布式版本控制系统（distributed version control system, `DVCS`），能够让开发人员以协作方式管理项目中的文件更改。

- 版本控制系统提供的优势：
  
  - 能够查看和恢复旧版本文件。
  
  - 能够比较同一文件的两个版本。
  
  - 记录谁在何时做了哪些更改。
  
  - 多用户的机制，以协作方式修改文件、解决相互冲突的更改，并将更改进行合并。

- 使用 Git 的方式：
  
  - 每个开发⼈员都可以从远程代码仓库中克隆现有的共享项目。
  
  - 克隆项目会创建原始远程代码仓库的完整副本，作为本地仓库。
  
  - 👉 本地副本作为版本控制系统中文件的整个历史记录，而不仅仅是项目文件的最新快照。
  
  - 开发人员在工作区中编辑。
  
  - 暂存一组相关的更改并提交到本地仓库，但是，未对共享远程代码仓库做任何更改。
  
  - 当开发⼈员准备好共享其工作时，他们会将更改（`commit`）推送（`push`）到远程代码仓库。
  
  - 若本地仓库可以从网络访问，远程代码仓库的所有者可以将这些更改从开发⼈员的本地仓库中拉取（pull）到远程存储库。
  
  - 👉 当开发人员准备好使用远程代码仓库的最新更改更新本地仓库时，他们可以拉取（pull）更改（从远程代码仓库中获取），并合并到本地仓库中。

- Git 工作树的 3 种文件状态：
  
  - 已修改（`modified`）：
    
    工作区中文件的副本已被编辑，与仓库中的最新版本不同。
  
  - 已暂存（`staged`）：
    
    修改后的文件已添加到已更改文件的列表中，以作为一个集合一起提交，但还未提交。
  
  - 已提交（`commited`）：
    
    修改的文件已提交到本地仓库。
  
  > 💥 文件提交到本地仓库后，可以通过远程代码仓库来推送或拉取提交。

- Git 管理文件的 4 个区域：
  
  Local Working Tree -> Staging Area -> Local Repository -> Remote Repositiry (upstream) 
  
  ![](pictures/git-manage-files-foue-areas.png)

- Git 目录结构示例：
  
  ```bash
  $ mkdir -p gitlab-data/demoGit && cd gitlab-data/demoGit
  $ git init
  # 初始化本地 git 仓库
  $ tree -a .
  .
  └── .git    # 本地 git 仓库实体（暂存区、本地仓库）
      ├── branches
      ├── config    # git 配置文件：由 git config --global 命令更改
      ├── description    # 项目描述信息
      ├── HEAD    # 标记当前工作区的所在分支（分支指针文件）
      ├── hooks
      │   ├── applypatch-msg.sample
      │   ├── commit-msg.sample
      │   ├── post-update.sample
      │   ├── pre-applypatch.sample
      │   ├── pre-commit.sample
      │   ├── prepare-commit-msg.sample
      │   ├── pre-push.sample
      │   ├── pre-rebase.sample
      │   └── update.sample
      ├── info
      │   └── exclude
      ├── objects    # 用于本地仓库存放对象文件与文件快照，称为仓库（repository）。
      │   ├── info
      │   └── pack
      └── refs    # 存放分支、标签文件与远程代码仓库。
          ├── heads
          └── tags
  # index：索引文件，即暂存区（staging area），由 git add 命令修改。
  # logs：refs 的历史记录，如 HEAD、master 分支与 origin/master 分支等。
  
  10 directories, 13 files
  ```

-----

### 设置 Git 代理

- 由于国内特殊的网络环境，使用 Git 与 GitHub 交互时经常会出现无法访问或访问超时的现象。因此，可配置 Git 代理解决此问题。

- 设置 Git 代理需使用魔法上网，请确保自身的魔法可正常使用。

- 笔者的开发环境中宿主机为 Windows 10，使用 VMware 运行 RHEL8.5 虚拟机环境。Windows 中的魔法使用 Clash 实现，其转发的端口为 `7890`。开发环境位于虚拟机中，需对其中的 Git 设置代理，设置步骤如下所示：
  
  - Clash 打开 7890 端口
    
    ![](pictures/open-clash-port-7890.png)
  
  - Windows 防火墙放行 Clash 应用
    
    ![](pictures/firewall-allow-clash.png)
  
  - 虚拟机中由于使用 SSH 远程连接认证 GitHub 中的私有仓库，需在 `$HOME/.ssh/config` 配置文件中添加红框中的参数，而 Git 代理依赖 `nc` 程序，应预安装 `nmap-ncat` 软件包(此处未使用 HTTPS 方式进行认证)：
    
    ![](pictures/ssh-client-add-git-proxy.png)
    
    以上 `--proxy` 参数中指定的 IPv4 地址只需将其替换为自身环境中 Clash 所在节点的 IPv4 地址即可，若未使用默认的 7890 端口需自行定义。
  
  - SSH 客户端配置完成后，进一步运行 git config 命令，以满足 git pull 或 push 命令时的代理。
    
    ![](pictures/git-add-proxy.png)

-----

### 设置 Git 环境

- 由于 Git 用户经常与多个贡献者一起修改项目，Git 会在每一次提交时记录用户的名称和电子邮件地址。

- 这些值可以在项目级别上定义，但也可为用户设置全局默认值。

- `git config --global` 命令可管理用户参与的所有 Git 项目的默认设置，将设置保存到运行 git 命令用户的 `~/.gitconfig` 文件中。
  
  ```bash
  $ man git-config
  # 查看 git config 的配置详细信息
  
  ### Git 全局设置 ###
  $ git config --global user.name "<git_developer_name>"
  $ git config --global user.email "<git_developer_email>"
  # 配置 git 用户名与对应的邮箱，默认情况下，user、author 与 committer 相同。
  # 注意：
  #   1. 若未特别指定，与 author.name、author.email、committer.name 或 committer.email 一致。 
  #   2. 可手动配置上述参数以区分不同的名字与邮箱
  # 在每次推送代码至远程代码仓库后将在仓库中可见该用户信息
  
  $ git config --global credential.<remote_git_repository_url>.username <username>
  # 使用远程代码仓库中指定 git 用户的认证凭据
  $ git config --global credential.helper cache --timeout=<seconds>
  # 在指定的超时时间内使用指定 git 用户的认证缓存
  # 注意：该认证的 git 用户可推送本地仓库的代码至远程代码仓库，该远程库可不属于该 git 用户（如下示例）。
  $ git config --global credential.https://github.com.username alberthua-perl
  $ git config --global credential.helper cache [--timeout=7200]
  # 缓存用户所有访问代码仓库的认证凭据（token），首次认证后，再次 commit 无需输入凭据。
  # --timeout 选项指定 token 过期时间（秒）
  # 该 token 可为常用的 GitHub Personal access tokens，需提前于 GitHub 配置。
  $ git credential-cache exit
  # 清除用户认证凭据缓存
  # 注意：
  #   --global 选项作用于 Git 全局设置，也可单独在代码库的 .git/config 文件中指定
  #   token，以独立地在代码库使用 token 进行用户的认证。可参考下文 "Git 工作流 示例5"。
  
  $ git config --global color.ui true
  $ git config --global push.default simple
  # 定义 simple 的推送方式
  $ git config --global alias.st status
  # 示例：将 status 子命令别名为 st（其他子命令可使用类似方法）
  
  $ git config --global -l
  # 查看 git 全局配置信息
  $ cat ~/.gitconfig
  # 查看 git 开发者配置的全局信息（作用于所有参与的项目）
  
  # 注意：
  #   1. $ git config --global --edit：直接进入 Vim 编辑 ~/.gitconfig 文件
  #   2. Git 局部设置只需去除 --global 选项即可
  ```
  
  ![](pictures/git-config-user-info.png)
  
  👨‍🏫 示例：使用 git config 的 `credential` 配置参数，在推送本地仓库代码至远程代码仓库时，可能需要输入远程代码仓库中指定 git 用户的密码作为认证凭据，并且该远程仓库可能属于另一 git 用户所有，如下所示：
  
  👉 使用 `student` 用户推送本地仓库代码
  
  ![](pictures/git-config-credential.jpg)
  
  ![](pictures/git-config-credential-gitlab-1.jpg)
  
  👉 该 GitLab 仓库属于名为 `git` 的用户，仓库状态为 `public` 状态。
  
  ![](pictures/git-config-credential-gitlab-2.jpg)

- git 软件包附带的 `git-prompt.sh` 脚本可报告工作树状态。

- 使用该脚本修改 shell 提示符，将以下行添加到 `~/.bashrc` 文件中，如下所示：
  
  ```bash
  $ vim ~/.bashrc
    source /usr/share/git-core/contrib/completion/git-prompt.sh
    export GIT_PS1_SHOWDIRTYSTATE=true
    export GIT_PS1_SHOWUNTRACKEDFILES=true
    export PS1='[\u@\h \W$(declare -F __git_ps1 &>/dev/null && __git_ps1 " (%s)")]\$ '
  ```

- 若当前目录位于 Git 工作区中，则工作区的当前 Git 分支的名称将显示在圆括号中。

- 若未在工作区中提交的未跟踪、已修改或已暂存的文件，则提示符显示内容如下：
  
  - `(branch #)`：新创建并初始化的本地仓库
  
  - `(branch *)`：跟踪的文件已被修改
  
  - `(branch +)`：跟踪的文件已被修改且使用 git add 存放至暂存区
  
  - `(branch %)`：在工作区中存在未跟踪的文件
  
  > 👉 未跟踪文件（untracked file）：工作区中修改（modified）且未暂存的文件
  
  - 上述提示符标记可能相互结合，形如 `(branch *+)`。
  
  > 👉 除了使用 git-prompt.sh 脚本设置 PS1 环境变量外，也可使用如 `oh-my-bash` 等 shell 命令行配置工具更改 theme 以获得不同的 PS1 环境变量的样式。
  > 
  > ![](pictures/oh-my-bash-ps1.jpg)

-----

### Git 工作流描述

- Git 工作流涉及工作区、暂存区、本地仓库与远程代码仓库的相互协作。

- 开发人员工作过程中，会在当前项目的工作区中创建新文件、修改现有的文件，使工作区变为 `dirty` 状态。

- 🚀 Git 目录结构与 Git 命令对应关系：
  
  ![](pictures/git-workflow-1.png)
  
  ![](pictures/git-workflow-2.png)

-----  

### 初始化 Git 仓库

```bash
$ man 7 gittutorial
# 查看 git 命令的详细使用指导

$ git init
# 初始化新的私有仓库，该方式不创建远程代码仓库。
$ git init --bare --shared=true
# 初始化 Git 服务器并创建裸机仓库

# 注意：
#   1. 裸机仓库没有本地工作区。
#   2. Git 服务器通常包含裸机仓库，因为服务器中不需要工作区。
#   3. 当开发人员创建仓库的本地副本时，他们通常需要工作区来进行本地更改，此外
#      还必须设置服务器，以便用户使用 HTTPS 或 SSH 协议来克隆、拉取和推送到仓库。
```

![](pictures/git-init-clone.png)

-----

### 克隆远程代码仓库

```bash
$ git clone git@github.com:Alberthua-Perl/kani.git
# 克隆 GitHub 远程代码仓库至本地仓库
$ git clone git@gitlab.lab.example.com:devuser/do447-course-demo.git
# 克隆 GitLab 远程代码库至本地当前目录中同名的代码库目录

$ git clone <repository_url> <destination_dir>
# 克隆远程代码仓库至本地指定的目录中
```

-----  

### Git 工作区、暂存区操作

```bash
$ git status
# 查看当前分支的 Git 工作区、暂存区的状态

### 撤销工作区中未暂存文件的更改 ###
$ git checkout -- <file>
# 注意：该文件未暂存至暂存区只在工作区中发生改变
* $ git restore <file>
* # 该命令在高版本 Git 中可使用与上述命令效果相同，但该命令处于实验阶段！

### 忽略跟踪修改 ###
# 方式 1：
$ touch /path/to/project/.gitignore
# 本地工作目录中创建 .gitignore 文件，该文件中指定的文件不被跟踪至暂存区。
# 方式 2：
$ git update-index --assume-unchanged <file>
# 忽略对指定文件的跟踪修改，即使使用 git add -f 命令也无法跟踪与暂存。
$ git update-index --no-assume-unchanged <file>
# 取消忽略对指定文件的跟踪修改
# 注意：
#   使用以上方式的几种场景：
#   1. 避免冲突：当在一个多人协作的项目中工作时，可能会遇到文件被其他人修改的情况。
#      如果不想处理这些冲突，可以使用这个命令来避免它们。
#   2. 忽略特定文件的更改：有时候，可能想要提交其他文件，但不想提交某些特定的文件。
#   3. 临时忽略文件：在某些情况下，可能需要临时忽略文件的更改，比如在修复一个问题时，
#      不希望这些更改被提交。

$ git add <file>
# 暂存指定文件至暂存区中
$ git add --all <dir>
# 将指定目录中的文件添加入暂存区
$ git add --all
# 暂存所有文件至暂存区中

# 最佳实践：
#   1. 在下一次提交时，仅会将暂存到暂存区的文件保存到本地仓库中。
#   2. 若用户同时处理两个更改，则可以将文件组织到两个提交中，以更好地跟踪更改。
#   3. 先暂存并提交其中一组更改，然后暂存和提交其余的更改。

### 删除暂存区中的文件（不删除工作区中的同名文件） ###
$ git restore --staged <file>
# 撤销暂存区中的跟踪文件（不将文件添加至未跟踪列表中）
$ git reset HEAD <file>
# 该命令与上述命令具有相同的作用
# 注意：暂存区中的文件被删除，工作区中的文件状态变为 modified。

$ git rm -r --cached [<file>|<dir>]
# 撤销暂存区中的文件并将其添加至未跟踪列表中
# 执行以上命令后需注意以下状态变化：
#   1. 暂存区中的文件状态：deleted
#   2. 暂存区状态：Changes to be committed
#   3. 工作区中的文件状态：Untracked

$ git rm --force --cache <file>
# 若工作区与暂存区中的同名文件内容发生变化，将删除暂存区中的文件，不删除工作区中的
# 同名文件，内容为工作区中文件的更改。
$ git rm --force <file>
# 同时删除工作区与暂存区中的同名文件
```

👨‍🏫 示例：`git rm` 与 `git rm --cached` 的区别

![](pictures/git-rm-working-tree-and-staged.jpg)

![](pictures/git-rm-f-cached-staged.jpg)

👨‍🏫 示例：git add 针对于工作区已删除文件的操作

由于工作区中将已提交至本地仓库的文件删除，该文件进入未跟踪的状态（`untracked`），若要跟踪该已删除的文件可使用 `git add --all <pathspec>` 将其添加至暂存区中，待 commit 提交至本地仓库。  ![](pictures/git-add-removed-file-in-working-tree.jpg)

-----

### Git 暂存操作

由于当前活动分支上还存在未暂存的或未提交的文件，在切换至其他分支时将出现如下报错（切换至其他分支时文件的修改将被覆盖）：

![](pictures/git-stash-checkout-error.png)

因此，可通过当前活动分支中的暂存操作解决，即暂存当前活动分支中的修改，切换至其他分支完成提交后，再恢复暂存继续修改即可。

```bash
$ git stash
# 暂存当前活动分支中的修改
$ git stash list
# 查看当前活动分支中的暂存列表
$ git stash apply
# 恢复当前活动分支中的最近一次暂存
$ git stash pop
# 恢复当前活动分支中的暂存并删除暂存记录
$ git stash drop stash@{n}
# 移除当前活动分支中的指定暂存
```

-----

### Git 本地仓库、远程代码库操作

```bash
$ git commit -m "<commit_message>"
# 提交暂存区的文件至本地仓库中
$ git commit -a
# 一次性将修改的文件暂存、提交至本地仓库，但新创建的文件（未跟踪）不可使用。
# commit 提交请见下文 "Git 提交日志规范的说明"

$ git commit --amend -m "<commit_message>"
# 生成一个新的提交对象，替换掉上一次提交产生的提交对象。
# 这时如果暂存区有发生变化的文件，会一起提交到仓库。所以，--amend 选项不仅可以修改提交信息，
# 还可以整个把上一次提交替换掉。

$ git remove -v
# 查看追踪的远程代码库集合
$ git remote add <remote_name> <repository_url>
# 新增追踪的远程代码库
$ git remote add origin git@gitlab.lab.example.com:devuser/haproxy-lamp.git
# 添加远程代码仓库 URL 至 .git/config 文件中
# 注意：
#   1. 在追踪的远程代码库集合中，首个 remote 名称建议为 origin。
#   2. 追踪的远程代码库集合中可自定义 remote 名称且互不相同

$ git remote set-url origin git@github.com:Alberthua-Perl/tech-docs.git
# 更改 .git/config 文件中的原始远程代码仓库 URL
# 远程 origin 仓库通常是指最初克隆的 Git 仓库

$ git fetch
# 只从远程代码仓库中拉取最新的提交（即下载最新的提交数据）
$ git pull [<remote_name>] [<branch_name>]
# 从远程代码仓库中拉取最新的提交，并将更新合并到本地分支，以及合并到工作区中。
# 注意：
#   1. 应经常运行此命令，从而获得其他⼈对远程代码仓库中项目作出的最新更改。
#   2. git pull 实际上是两个命令合成了一个：git fetch 和 git merge。
#   3. 当从来源拉取修改时，首先是像 git fetch 那样取回所有数据，然后最新的修改会自动合并
#      到本地分支中。

$ git push [-u|--set-upstream] origin <branch_name>
# 推送本地仓库做出的更改至远程代码仓库的分支中
# --set-upstream 选项：关联本地分支与远程代码库分支（首次推送本地分支至远程必须使用）
$ git push -f origin <branch_name>
# 强制推送本地仓库至远程代码仓库中
# 使用场景示例：由于版本回退本地仓库中的提交比远程代码库中旧，此时推送将报错，使用如上命令
# 强制推送即可。
$ git push --delete origin <branch_name>
# 删除远程代码仓库中的分支
```

![](pictures/git-add-commit.png)

![](pictures/git-fetch.gif)

![](pictures/git-pull.gif)

![](pictures/git-pull-push.png)

-----

### Git 提交日志规范的说明

- 在一个团队协作的项目中，开发人员需要经常提交一些代码去修复 bug 或者实现新的 feature。而项目中的文件和实现什么功能、解决什么问题都会渐渐淡忘，最后需要浪费时间去阅读代码。但是好的日志规范 `commit messages` 编写有帮助到我们，它也反映了一个开发人员是否是良好的协作者。

- commit messages 要求：
  
  - 所有提交必须有注释，内容必须简洁明了地描述本次提交涵盖了哪些内容。严禁注释内容过于简单或不能明确表达提交内容！
  
  - 合理控制提交内容的颗粒度，一次提交含一个独立功能点。严禁一次提交涵盖多个功能项。
  
  - 正确为每个项目设置 Git 提交用到的 user.name 和 user.email 信息，不可随意设置以影响无法正确识别。

- 编写良好的 commit messages 可以达到 3 个重要的目的：
  
  - 加快 review 的流程
  
  - 帮助我们编写良好的版本发布日志
  
  - 让之后的维护者了解代码里出现特定变化和 feature 被添加的原因

- 目前，社区有多种 commit message 的写法规范。来自 `Angular` 规范是目前使用最广的写法，比较合理和系统化。

- commit messages 的基本语法：
  
  当前业界应用的比较广泛的是 [Angular Git Commit Guidelines](https://github.com/angular/angular.js/blob/master/DEVELOPERS.md#-git-commit-guidelines)
  
  ```html
  <type>(<scope>): <subject>
  <BLANK LINE>
  <body>
  <BLANK LINE>
  <footer>
  ```
  
  ```xml
  # 标题行：50 个字符以内，描述主要变更内容。
  #
  # 主体内容：更详细地说明文本，建议 72 个字符以内。需要描述的信息包括:
  #
  # * 为什么这个变更是必须的? 它可能是用来修复一个 bug，增加一个 feature，提升性能、可靠性、稳定性等等。
  # * 它如何解决这个问题? 具体描述解决问题的步骤
  # * 是否存在副作用、风险?
  #
  # 如果需要的化可以添加一个链接到 issue 地址或者其它文档
  ```
  
  1️⃣ type：本次 commit 的类型，如 bugfix、docs、style 等。
  
  - feat：添加新特性
  
  - fix：修复 bug
  
  - docs：仅仅修改了文档
  
  - style：仅仅修改了空格、格式缩进、逗号等等，不改变代码逻辑。
  
  - refactor：代码重构，没有加新功能或者修复 bug。
  
  - perf：增加代码进行性能测试
  
  - test：增加测试用例
  
  - chore：改变构建流程、或者增加依赖库、工具等
  
  2️⃣ scope：本次 commit 波及的范围
  
  3️⃣ subject：简明扼要地阐述本次 commit 的主旨，在原文中特意强调了几点：
  
  - 使用祈使句
  
  - 首字母不要大写
  
  - 结尾无需添加标点
  
  4️⃣ body：同样使用祈使句，在主体内容中需要详细描述本次 commit，比如此次变更的动机，如需换行，则使用 `|`。
  
  5️⃣ footer：描述下与之关联的 issue 或 break change

-----

### 🛠 git push 推送的故障排查

👨‍🏫 示例 1：

远程代码仓库的分支中已存在其他开发者提交的新代码，而在本地工作区与本地仓库中无这些更新，因此本地仓库推送更新的代码至远程代码仓库的分支中时会由于两者的状态不一致而产生冲突（`conflict`），此时可将远程的更新拉取（`pull`）至本地工作区与本地仓库，同步两者的状态后即可重新推送，如下所示：

![](pictures/git-push-error-resolv-1.jpg)

![](pictures/git-push-error-resolv-2.jpg)

👨‍🏫 示例 2：

本地工作区新建并切换至新分支，将代码更新提交至本地仓库中，但该工作区中的 `origin` 远程代码仓库中还未创建该分支，依然指向原先的远程分支，因此，若不使用 `-u` 或 `--set-upstream` 选项的话，将无法将本地仓库的新分支与远程分支关联，使用该选项后可推送成功。

💥 远程代码仓库中的分支无需手动创建，使用上述选项后将自动创建！

![](pictures/git-push-set-upstream-branch.jpg)

👨‍🏫 示例 3：

将本地未初始化的代码目录推送至远程代码仓库时，必须先在远程代码仓库上创建仓库，再对本地目录执行 `git init` 初始化，且需进行 `git commit` 提交后再推送至远程代码仓库，若不执行 commit 操作，报错如下：

```bash
$ git init
Initialized empty Git repository in /home/sysadmin/haproxy-lamp/.git/
$ git remote add origin git@gitlab.domain12.example.com:devuser/haproxy-lamp.git
# 添加远程代码仓库 URL
$ git push -u origin master
error: src refspec master does not match any.
error: failed to push some refs to 'git@gitlab.domain12.example.com:devuser/haproxy-lamp.git'
# 此时本地仓库为初始化状态，需将工作区的更新提交至本地仓库。
$ git add .
$ git commit -m "Initial commit"
[master (root-commit) 7c0d5e6] Initial commit
 17 files changed, 1376 insertions(+)
 create mode 100644 README.md
 create mode 100644 ansible.cfg
 create mode 100644 deploy.log
 create mode 100644 group_vars/haproxy
 create mode 100644 group_vars/mariadb
 create mode 100644 inventory
 create mode 100644 roles/apache-php/tasks/main.yaml
 create mode 100644 roles/apache-php/templates/index.php.j2
 create mode 100644 roles/base/files/CentOS-Base.repo
 create mode 100644 roles/base/files/epel.repo
 create mode 100644 roles/base/tasks/main.yml
 create mode 100644 roles/haproxy/handlers/main.yaml
 create mode 100644 roles/haproxy/tasks/main.yaml
 create mode 100644 roles/haproxy/templates/haproxy.cfg.j2
 create mode 100644 roles/mariadb/handlers/main.yaml
 create mode 100644 roles/mariadb/tasks/main.yaml
 create mode 100644 site.yaml
$ git push -u origin master
Enumerating objects: 34, done.
Counting objects: 100% (34/34), done.
Delta compression using up to 4 threads.
Compressing objects: 100% (25/25), done.
Writing objects: 100% (34/34), 14.84 KiB | 4.95 MiB/s, done.
Total 34 (delta 0), reused 0 (delta 0)
To gitlab.domain12.example.com:devuser/haproxy-lamp.git
 * [new branch]      master -> master
Branch 'master' set up to track remote branch 'master' from 'origin'.
```

👨‍🏫 示例 4：

在使用 `GitHub Personal Access Token` 推送代码至远程代码仓库时，显示如下 WARNING 信息，虽然不影响代码的推送，但是任需将其排除：

```bash
$ git push

(gnome-ssh-askpass:7218): Gtk-WARNING **: 04:16:33.711: cannot open display: 
error: unable to read askpass response from '/usr/libexec/openssh/gnome-ssh-askpass'
Username for 'https://github.com': 
```

可执行如下命令重置 `SSH_ASKPASS` 环境变量加以解决：

```bash
$ echo "unset SSH_ASKPASS" >> $HOME/.bashrc
$ source $HOME/.bashrc
```

👨‍🏫 示例 5：

若不使用 SSH 的方式认证登录 GitHub，以 `GitHub Personal Access Token` 认证登录的话，可单独在代码库 `.git/config` 文件的 `[remote "origin"]` 部分中更新如下配置：

```bash
[remote "origin"]
        url = https://<git_username>:<access_token>@github.com/<git_username>/<project_name>.git
        fetch = +refs/heads/*:refs/remotes/origin/*
```

-----

### Git 日志检索

- 版本控制系统的部分要点是跟踪提交日志记录。

- 每个提交都由提交哈希（commit hash）来标识。
  
  ```bash
  $ git log
  # 查看当前分支及其祖先分支的提交日志
  $ git log --since=[2.days|3.weeks|1.months|1.years]
  # 查看过去2天、3周、1月、1年的提交日志
  $ git log --all --grep="<keyword>"
  # 在所有提交日志中搜索具有指定关键字的日志记录
  $ git log --author="<author_name>"
  # 查询指定提交人的日志记录
  $ git log [--all] --oneline --abbrev --graph
  # 使用 7 位提交哈希值以图形化分支的方式显示全部分支提交日志记录
  # 查看当前活动分支的提交日志记录不使用 --all 选项
  
  $ git log -p -2
  # 查看最近两次提交的详细信息（包括以 diff 形式比较提交前后的变化等）
  $ git log --stat
  # 查看提交的简要修改状态
  $ git log <branch1>..<branch2>
  # 查看两个分支间的提交日志记录
  $ git log main..<brance1>
  # 查看 main 主分支与指定分支间的提交日志记录
  
  $ git log --pretty=oneline
    8936873bed36682b6885e61b8c76d761810a6179 (HEAD -> dev, origin/dev) Update README.md
    40cfa07c01887ceb1e86452e710e6d7621251f1e Update README.md
    38fe8f9b7212747a2603f79f48b14d705edc4aca (origin/master, master) Update README.md
    815f7de68ea026703384d7e40b7ce41d558c01a2 Update ansible.cfg
    cde250bc8f33e1e8477befd9d213107f6ad90bf3 Add http_restful_api_filter.yml
    2cddbdfa1c79a0b84fbfe5d433af80c57da8e592 Init commit
  # 查看 dev 分支及 master 原分支的单行提交日志记录
  $ git log --pretty=oneline -5
  # 查看当前分支及其原分支的最近 5 次单行提交日志记录
  $ git log --pretty=full
  # 查看提交日志（包括 Author 与 Commit）
  $ git log --pretty=short
  # 查看提交日志（包括 Author）
  
  $ git blame <file>
  # 查看指定文件修改的提交者、时间戳与每一行的上一次修改记录
  
  $ git show [<commit_hash>|<tag>]
  # 查看指定提交的更改内容（或指定 tag 的详细信息）
  # 注意：若直接使用 git show 即查看最新的提交信息
  
  $ git branch -v
    * dev  d8e93eb Update README.md in dev branch
      main 4161e95 Init corejava1
  # 查看所有分支的最后一次提交日志
  
  $ git diff
  # 比较当前工作目录与更改前的差异
  $ git diff --staged
  # 比较当前暂存区与上一次提交之间的差异
  
  $ git diff <commit_hash1> <commit_hash2>
  # 比较两个提交之间的更改差异
  $ git diff main..test
  # 比较 main 分支与 test 分支之间的差异
  ```
  
  ![](pictures/git-log-commit-info.png)

- 命令中不需要输入完整的哈希，只需其中的足够部分来唯一标识仓库中的特定提交。

- 这些哈希还可用于恢复到之前的提交，或者查看版本控制系统的历史记录。

-----

### Git 分支描述

- Git 提交（commit）包含 Git 为仓库创建和处理整个历史记录所需的全部信息，具体包括：
  
  - 提交的唯一 `ID`，采用 `40` 位 `十六进制` 字符串格式，是提交内容的 `SHA-1` 哈希。
  
  - 更改的仓库文件的列表，以及对每个文件的确切更改。
  
  - 更改可以是行的增减、重命名或删除。
  
  - ✨ 父级提交的 ID，即在应用当前提交更改之前定义仓库状态的提交的 ID。
  
  - 提交的作者和创建者（或提交者）。
  
  - 提交中还包含一个 `HEAD` 引用列表（a list of references）。

- 🚀 Git 中 `HEAD` 的说明：
  
  HEAD 是当前活动分支引用的指针，它总是指向某次提交，默认是上一次的提交。这表示 HEAD 将是下一次提交的父结点。通常，可以把 HEAD 看做上一次提交的快照。当然 HEAD 的指向是可以改变的，比如可指向提交（`commit`）、标签（`tag`）或分支（`branch`）等。
  
  ![](pictures/git-head-log.jpg)

- 若在工作区中进行更改，使用 git add 命令进行暂存，并使用 git commit 命令进行提交，则将创建一个新的提交，以最新的提交作为其父项，并且 `HEAD` 将转为指向新的提交。

- Git Flow 有主分支和辅助分支两类分支，通常主分支也被称为长期分支。
  
  - 主分支用于组织与软件开发、部署相关的活动
    
    - 主分支是所有开发活动的核心分支。
    
    - 所有的开发活动产生的输出物最终都会反映到主分支的代码中。
  
  - 辅助分支组织为了解决特定的问题而进行的各种活动

- Git 常用分支类型：
  
  - 主分支：
    
    - 1️⃣ **`master`** 分支：
      
      - 一个项目只能有一个 master 分支
      
      - master 分支存放的是随时可供在生产环境中部署的稳定版本代码
      
      - 仅在发布新的可供部署的代码时才更新 master 分支上的代码，任何时间都不能直接修改代码。
      
      - master 分支保存官方发布版本历史，release tag 标识不同的发布版本。
      
      - 每次更新 master，都需对 master 添加指定格式的 tag，用于发布或回滚。
      
      - master 分支是保护分支，不可直接 push 到远程仓库 master 分支。
      
      - master 分支代码只能被 release 分支或 hotfix 分支合并
    
    - 2️⃣ **`develop`** 分支：
      
      - 一个项目只能有一个 develop 分支
      
      - develop 分支为开发分支，一般包含正在开发的所有新特性。
      
      - 始终保持最新完成以及 bug 修复后的代码，一般开发新功能时，feature 分支都是基于 develop 分支下创建的。
      
      - 开发者在自己的分支上开发完成后，向 develop 分支合并。
      
      - develop 分支不能与 master 分支直接交互
      
      - develop 分支衍生出各个 feature 分支
      
      - develop 分支是保护分支，不可直接 push 到远程仓库 develop 分支。
      
      > **注意：** 一般来说，选择将 master 分支和 develop 分支作为长期分支，长期分支是不会被删除的，会和项目共存亡。即除非项目不再需要了，否则，这两个分支切出来以后就永远都不允许删除。当然也有一些特例，比如某些公司会将 develop 分支也进行删除，只保留 master 分支。
  
  - 辅助分支：
    
    用于组织解决特定问题的各种软件活动的分支。辅助分支主要用于组织软件新功能的并行开发、简化新功能开发代码的跟踪、辅助完成版本发布工作以及对生产代码的缺陷进行紧急修复工作、以及对版本代码的测试。这些分支与主分支不同，通常只会在有限的时间范围内存在。这个有限的时间范围比如说一个开发周期，规定在两个礼拜，那么到了第二个礼拜的最后一天开发周期完成，代码合并，该分支就应该被删除掉。
    
    - 1️⃣ **`feature`** 分支：
      
      - 命名规则：`feature/*` 或 `feature/JID-N/developerName`
      
      - develop 分支的功能分支
      
      - feature 分支使用 develop 分支作为它们的父类分支
      
      - 以功能为单位从 develop 分支拉一个 feature 分支
      
      - 每个 feature 分支颗粒要尽量小，以利于快速迭代和避免冲突。
      
      - 当其中一个 feature 分支完成后，它会合并回 develop 分支。
      
      - 当一个功能因为各种原因不开发了或者放弃了，这个分支直接废弃，不影响 develop 分支。
      
      - feature 分支代码可以保存在开发者自己的代码库中而不强制提交到主代码库里
      
      - feature 分支只与 develop 分支交互，不能与 master 分支直接交互。
      
      > **说明：**
      > 
      > - 如有几个同事同时开发，需要分割成几个小功能，每个人都需要从 develop 中拉出一个 feature 分支，但是每个 feature 颗粒要尽量小，因为它需要我们能尽早 merge 回 develop 分支，否则冲突解决起来就没完没了。
      > 
      > - 也正是因为可能多个同事协同开发同一个生命周期的项目的不同功能，因此在这里 feature 分支命名中加入了第二种命名规则，JID-N 表示的是哪个任务 ID 号，developerName 表示开发者名称，用以方便区分是哪个任务下哪个开发者的分支。
    
    - 2️⃣ **`release`** 分支：
      
      - 命名规则：`release/*`，"*" 以本次发布的版本号为标识。
      
      - release 分支为预分布分支，主要用来为发布新版的测试、修复做准备。
      
      - 当需要为发布新版做准备时，从 develop 分支衍生出一个 release 分支。
      
      - release 分支可以从 develop 分支上指定 commit 派生出（从旧提交创建）
      
      - release 分支测试通过后，合并到 master 分支并且给 master 标记一个版本号。
      
      - release 分支一旦建立就将独立，不可再从其他分支 pull 代码。
      
      - 必须合并回 develop 分支和 master 分支
      
      > **说明：**
      > 
      > - release 分支是为发布新的产品版本而设计的。在这个分支上的代码允许做小的缺陷修正、准备发布版本所需的各项说明信息（版本号、发布时间、编译时间等）。通过在 release 分支上进行这些工作可以让 develop 分支空闲出来以接受新的 feature 分支上的代码提交，进入新的软件开发迭代周期。
      > 
      > - 当 develop 分支上的代码已经包含了所有即将发布的版本中所计划包含的软件功能，并且已通过所有测试时，我们就可以考虑准备创建 release 分支了。而所有在当前即将发布的版本之外的业务需求一定要确保不能混到 release 分支之内（避免由此引入一些不可控的系统缺陷）。
      > 
      > - 成功地派生了 release 分支，并被赋予版本号之后，develop 分支就可以为 "下一个版本" 服务了。所谓的 "下一个版本" 是在当前即将发布的版本之后发布的版本。版本号的命名可以依据项目定义的版本号命名规则进行。
    
    - 3️⃣ **`hotfix`** 分支：
      
      - 命名规则：`hotfix/*`
      
      - 线上出现紧急问题时，如快速给已发布产品修复 bug 或微调功能，以 master 分支为基线，创建 hotfix 分支。
      
      - 只能从 master 分支指定 tag 版本衍生出来
      
      - 一旦完成修复 bug，必须合并回 master 分支和 develop 分支。
      
      - master 被合并后，应该被标记一个新的版本号。
      
      - hotfix 分支一旦建立就将独立，不可再从其他分支 pull 代码。
      
      > **说明：**
      > 
      > - 除了是计划外创建的以外，hotfix 分支与 release 分支十分相似：都可以产生一个新的可供在生产环境部署的软件版本。
      > 
      > - 当生产环境中的软件遇到了异常情况或者发现了严重到必须立即修复的软件缺陷的时候，就需要从 master 分支上指定的 tag 版本派生 hotfix 分支来组织代码的紧急修复工作。
      > 
      > - 这样做的显而易见的好处是不会打断正在进行的 develop 分支的开发工作，能够让团队中负责新功能开发的人与负责代码紧急修复的人并行的开展工作。
    
    - 4️⃣ **`bugfix`** 分支：
      
      - 修补分支：软件发布以后，难免会出现 bug。这时就需要创建一个分支，进行 bug 修补。
      
      - bugfix 分支是从 master 或 release 分支上面分出来的。修补结束以后，再合并进 master 和 develop 以及 release 对应版本分支。它的命名，可以采用 `fixbug-*`（日期）的形式。

- 上述各分支关系示意：
  
  ![](pictures/git-branch-workflow.png)
  
  主分支 master 和 develop 是保护分支，只能进行合并请求（merge request），均不可直接提交代码。

> **注意：** 合并请求（merge request）是 GitLab 中的合并方式，而拉取请求（pull request）是 GitHub 中的合并方式，两者本质都是合并，只是称呼不同（见文末参考链接）。

- 分支对应的环境：
  
  | 分支类型                 | 所在环境          |
  | -------------------- | ------------- |
  | master/hotfix/bugfix | 生产环境（release） |
  | develop              | 开发环境（alpha）   |
  | release              | 预发布（RC）环境     |

- GitHub Flow 工作流示意：
  
  ![](pictures/git-branch-workflow-from-github.png)

-----

### Git 分支操作

- Git 中的不同分支允许不同的工作流在统一 Git 仓库上并行演变。

- Git 创建分支实际上是创建了一个可以移动的新的指针，该指针将指向当前分支的新的提交。

- Git 分支相关操作：
  
  ```bash
  $ git branch <branch_name>
  # 从当前的 HEAD 引用提交创建一个新的分支，但不切换至该分支（HEAD 引用不指向该分支）。
  $ git branch -M <spec_branch_name>
  # 使用指定名称重命名当前分支名称
  # 注意：需在当前分支中先提交才能更改其名称！
  
  $ git branch [--list]
  # 查看本地仓库的分支
  $ git branch --list --all
  # 查看本地仓库与远程代码仓库中的分支
  $ git branch --remote
  # 查看远程代码仓库的分支
  $ git branch -v
  # 查看所有分支的最后一次提交
  $ git branch -d <branch_name>
  # 删除指定的本地分支（如果分支已合并）
  # 注意：删除分支，必须将其合并入其原始分支，否则无法删除！
  $ git branch -D <branch_name>
  # 强制删除本地分支
  
  $ git checkout <branch_name>
  # 切换至指定分支，将 HEAD 引用指向该分支。
  $ git checkout -b <branch_name>
  # 创建并切换分支，该命令即 git branch 与 git checkout 的集合，HEAD 引用指向该分支。 
  # 注意：
  #   切换分支的时候，如果当前分支有改动没有提交，则不能切换分支，需要先把改动的内容提交
  #   或者放入暂存区。
  $ git checkout -
  # 切换至之前的分支
  
  $ git checkout --orphan <orphan_branch_name>
  # 创建并切换至孤儿分支（即无父分支与任何提交记录的分支）
  # 注意：
  #   1. 孤儿分支未做任何提交时无法被识别
  #   2. 孤儿分支中创建提交后无法被常规删除，需合并分支后删除，或使用 git branch -D <orphan_branch_name> 
  #      强制删除。
  
  ### 删除并更新远程代码仓库分支 ###
  $ git branch -r -d origin/<branch_name>
  # 删除本地的跟踪远程代码仓库分支
  $ git push origin --delete <branch_name>
  # 删除远程代码库分支
  $ git push origin --set-upstream :<branch_name>
  # 更新远程代码库分支
  ```
  
  👨‍🏫 示例：从 `master` 主分支当前的 `38fe8f9` commit 上创建 `feature` 分支
  
  ![](pictures/git-different-branch-checkout.jpg)
  
  ![](pictures/git-branch-checkout.png)
  
  👨‍🏫 示例：删除并更新远程代码仓库分支
  
  ![](pictures/delete-remote-repository-branch.png)

> 💥 注意：
> 
> 1. 分支切换会改变工作区中的文件。
> 
> 2. 在切换分支时，一定要注意工作区中的文件会被改变。
> 
> 3. 若切换到一个较旧的分支，工作区会恢复到该分支最后一次提交时的样子。
> 
> 4. 若 Git 不能干净利落地完成这个任务，它将禁止切换分支。

-----

### 从旧提交创建分支

- 除了从当前分支的最新提交上创建新分支外，也可从其他分支的旧 `commit` 中创建新分支。
  
  ```bash
  $ git checkout <previous_commit_hash>
  # 指定某分支之前的提交，用于在该提交上创建分支，即 HEAD 引用指向该提交。
  $ git checkout -b <branch_name>
  # 创建并切换至新分支中
  ```
  
  ![](pictures/git-create-new-branch-from-previous-commit.png)

-----

### 🔥 Git 合并分支

- 分支上的工作完成后，可以将该分支与其他分支合并（通常为 master 分支或 main 分支）。

- 这允许并行操作新功能和漏洞修复，而主分支则没有未完成或未经测试的工作。

- 分支合并命令如下：
  
  ```bash
  $ git checkout main
  $ git merge feature1 [--no-ff|--ff] -m '<merge_commit>'  # main 分支
  # 将 feature1 分支合并入 main 分支
  # 注意：默认 Fast-forward 方式（--ff），也可指定 No-fast-forward 方式（--no-ff）。
  
  $ git branch --merged
  # 查看与当前活动分支合并过的分支
  $ git branch --no-merged
  # 查看与当前活动分支未合并过的分支
  ```

- Git 可执行两种类型的合并：`fast-forward (--ff)` 和 `no-fast-forward (--no-ff)`
  
  1️⃣ Fast-forward (--ff) 的说明：
  
  在当前分支相比于要合并的分支没有额外的提交时，可以执行 fast-forward 合并。即 Git 合并两个分支时，如果顺着一个分支走下去可以到达另一个分支的话，那么 Git 在合并两者时，只会简单地把指针右移，叫做 "快进"（fast-forward）。不过这种情况如果删除分支，则会丢失 merge 分支信息。
  
  Git 很懒，首先会尝试执行最简单的选项 fast-forward！这类合并不会创建新的提交，而是会将正在合并的分支上的提交直接合并到当前分支。
  ![](pictures/git-fast-forward-detail_sm.png)
  
  ![](pictures/git-merge-ff.gif)
  
  ![](pictures/git-merge-demo.png)
  
  2️⃣ No-fast-forward (--no-ff) 的说明：
  
  如果当前分支相比于想要合并的分支没有任何提交，那当然很好，但很遗憾现实情况很少如此！如果在当前分支上提交想要合并的分支不具备的改变，那么 git 将会执行 no-fast-forward 合并。关闭 fast-forward 模式，使用 no-fast-forward 合并，在提交的时候，Git 会在当前活动分支上创建一个 merge 的提交信息，然后将合并分支，此时 HEAD 引用指向当前活动分支。
  
  ![](pictures/cannot-git-fast-forward-detail_sm.png)
  
  ![](pictures/git-merge-no-ff.gif)
  
  ![](pictures/git-no-fast-forward-detail_sm.png)
  
  ![](pictures/git-merge-no-ff-notification.png)
  
  ![](pictures/git-merge-no-ff-demo.png)
  
  ![](pictures/git-merge-no-ff-log.png)
  
  从以上提交历史与提交时间戳可知，ea4b 提交不在要合并的 dev 分支中，并且该提交的时间在 dev 分支最后一次提交（49be）之后，因此将使用 No-fast-forward 模式合并。合并过程中 Git 要求说明合并的原因，并单独创建一个提交，这种方式由于新建提交可说明合并进入的分支，而 Fast-forward 方式无法确定哪个分支合并进入。

> ✨ 最佳实践：推荐使用 No-fast-forward 模式，由于在合并过程中新建提交信息，可有效的区分合并的分支，而 Fast-forward 模式无法实现，以此来区分 master 分支或 main 分支的提交历史！

- 💥 合并冲突（merge conflict）：
  
  尽管 Git 能够很好地决定如何合并分支以及如何向文件添加修改，但它并不总是能完全自己做决定。**当想要合并的两个分支的同一文件中的同一行代码上有不同的修改，或者一个分支删除了一个文件而另一个分支修改了这个文件时，Git 就不知道如何取舍了。这种情况称为合并冲突，需要通过编辑受影响的文件来手动解决。**
  
  在这样的情况下，Git 会询问想要保留哪种选择？假设在这两个分支中，都编辑了 README.md 的第一行。
  
  ![](pictures/git-merge-conflict.gif)

-----

### 🔥 Git 变基

- 可通过执行 git merge 将一个分支的修改应用到另一个分支。另一种可将一个分支的修改融入到另一个分支的方式是执行变基（rebase）。

- 通过简单的提交节点图解感受一下 rebase 在干什么：
  
  ![](pictures/git-rebase-intro-1.png)
  
  - 构造两个分支 master 和 feature，其中 feature 是在提交点 B 处从 master 上拉出的分支。
  
  - master 上有一个新提交 M，feature 上有两个新提交 C 和 D。
  
  - 此时切换到 feature 分支上，执行 rebase 命令，相当于是想要把 master 分支合并到feature 分支。这一步的场景就可以类比为在自己的分支 feature 上开发了一段时间，准备从主干 master 上拉一下最新改动，模拟了 `git pull --rebase` 的情形。
  
  ![](pictures/git-rebase-intro-2.png)
  
  - feature：待变基分支、当前分支
  
  - master：基分支、目标分支

- 📚 官方文档中的说明：当执行 rebase 操作时，git 会从两个分支的共同祖先开始提取待变基分支上的修改，然后将待变基分支指向基分支的最新提交，最后将刚才提取的修改应用到基分支的最新提交的后面。

- 以上提交节点图说明：
  
  - 当在 feature 分支上执行 git rebase master 时，git 会从 master 和 featuer 的共同祖先 B 开始提取 feature 分支上的修改，也就是 C 和 D 两个提交，先提取到。然后将 feature 分支指向 master 分支的最新提交上，也就是 M。最后把提取的 C 和 D 接到 M 后面，注意这里的接法，官方没说清楚，实际是会依次拿 M 和 C、D内 容分别比较，处理冲突后生成新的 C' 和 D'。一定注意，这里新 C'、D' 和之前的 C、D 已经不一样了，是处理冲突后的新内容，feature 指针自然最后也是指向 D'。
  
  - rebase 变基，可以直接理解为改变基底。feature 分支是基于 master 分支的 B 拉出来的分支，feature 的基底是 B。而 master 在 B 之后有新的提交，就相当于此时要用 master 上新的提交来作为 feature 分支的新基底。实际操作为把 B 之后 feature 的提交先暂存下来，然后删掉原来这些提交，再找到 master 的最新提交位置，把存下来的提交再接上去（接上去是逐个和新基底处理冲突的过程），如此 feature 分支的基底就相当于变成了 M 而不是原来的 B 了。
  
  - 💥 注意：如果 master 上在 B 以后没有新提交，那么就还是用原来的 B 作为基，rebase 操作相当于无效，此时和 git merge 就基本没区别了，差异只在于 git merge 会多一条记录 merge 操作的提交记录。

- 🔥 使用 git rebase 需要注意两点：
  
  - 找到待变基分支与基分支的共同祖先（共有的提交）。只有找到共同祖先才知道与待变基分支连接的基分支的最新提交。
  
  - 确认两个分支中是否存在引起合并冲突的文件修改。若不存在合并冲突，那么可直接完成 rebase；若存在合并冲突，需手动解决合并冲突，才能完成 rebase（该场景见下文示例）。

- 上述示例可抽象为如下实际工作场景：
  
  远程代码库上有一个 master 分支目前开发到 B 了。张三从 B 拉了代码到本地的 feature 分支进行开发，目前提交了两次，开发到 D了；李四也从 B 拉到本地的 master 分支，他提交到了 M，然后合到远程代码库的 master 上了。此时张三想从远程库 master 拉下最新代码，于是他在feature 分支上执行了 `git pull origin master:feature --rebase`（注意要加 `--rebase` 参数），即把远程代码库 master 分支给 rebase 下来，由于李四更早开发完，此时远程 master 上是李四的最新内容，rebase 后再看张三的历史提交记录，就相当于是张三是基于李四的最新提交 M 进行的开发了。（但实际上张三更早拉代码下来，李四拉的晚但提交早。） 

- 👨‍🏫 示例：
  
  本地 main 分支与 feature 分支间的 rebase，并解决 rebase 过程中的合并冲突。
  
  ![](pictures/git-rebase-demo-1.png)
  
  ![](pictures/git-rebase-demo-2.png)如上图所示，提交 8e7324f 是 main 分支与 feature 分支的共同祖先。因此，在 feature 分支中执行 rebase 操作后，新提交将以 main 分支中的最新提交 5f97dfe 为基础连接上去。
  
  ```bash
  $ git rebase main feature
  $ git rebase <基分支> <待变基分支>
  # 可使用图中命令实现，也可使用此命令实现变基。
  ```
  
  ![](pictures/git-rebase-demo-3.png)
  
  如上图所示，由于 main 分支中的提交 5f97dfe 与 feature 分支中的提交 b379af8 均对源码文件的同一部分进行更改，因此在 rebase 过程中出现合并冲突。此时需手动解决冲突，决定保留哪个提交中的修改内容。
  
  ![](pictures/git-rebase-demo-4-5.png)如上图所示，手动解决合并冲突后即可完成 rebase。以下为 rebase 后的 feature 分支提交日志记录，可见 rebase 后的新提交连接在提交 5f97dfe 之后。
  
  ![](pictures/git-rebase-demo-6.png)

- ✨ 推荐使用场景：
  
  - **拉公共分支最新代码：rebase**
    
    ```bash
    $ git pull origin master:<branch_name> --rebase
    ```
  
  - **往公共分支上合代码：merge**
    
    ```bash
    $ git push origin --set-upstream :<branch_name>
    ```
  
  - 一般在没有特殊要求的前提下，一律使用 merge 的方式！

-----

### Git 版本回退

- Git 的版本回退（重置）实际上是将当前分支的 HEAD 引用指向旧提交。

- 版本回退（重置）相关命令：
  
  ```bash
  $ git reflog
  # 查看 HEAD 引用的历史列表，已经执行过的所有动作的日志。包括合并、重置、还原等，
  # 基本上包含对分支所做的任何修改。
  # 注意：
  #   git log 命令只能查看当前提交及之前的提交历史信息，而无法查看提交回退的历史信息。
  #   因此，可使用 git reflog 命令查看当前分支及其原分支的所有提交。
  
  $ git reset [--soft|--hard] [<commit_hash>|HEAD@{n}]
  # 回退版本至当前分支的指定提交，在工作区与暂存区中保留该提交之后的所有修改过的文件。
  # 可直接使用提交的哈希值，也可使用 reflog 查询到的提交对应的 HEAD 索引号（n）。
  
  $ git reset --hard HEAD    # 硬重置回退到当前最新提交
  $ git reset --hard HEAD^   # 硬重置回退到上次提交
  $ git reset --hard HEAD~n  # 硬重置回退到上n次提交
  ```
  
  ![](pictures/git-reflog.gif)

- 版本回退（重置）可分为：软重置、硬重置
  
  💥 注意：以下软重置与硬重置均已回退至旧提交为例

- 1️⃣ 软重置：
  
  软重置会将 HEAD 移至指定的提交（或与 HEAD 相比的提交的索引），而不会移除该提交之后加入的修改！
  
  如下图，若不想保留添加了一个 style.css 文件的提交 9e78i，而且也不想保留添加了一个 index.js 文件的提交 035cc。但是，确实又想要保留新添加的 style.css 和 index.js 文件。这是软重置的一个完美用例。
  
  ![](pictures/git-reset-soft.gif)输入 git status 后，仍然可以访问在之前的提交上做过的所有修改。它们依然在暂存区中，可以被修改与提交。
  
  ![](pictures/git-reset-soft.png)
  
  如上图所示，使用软重置版本回退至指定的提交，该提交之后的所有提交在 git log 中消失，但是该提交后的修改过的文件全部保留在工作区与暂存区中，依然可以再次修改与提交。

- 2️⃣ 硬重置：
  
  直接重置到指定的提交状态，该提交之后的所有提交在 git log 中消失，并且之后提交中修改的文件在工作区与暂存区中也一起被删除。
  
  ![](pictures/git-reset-hard-5.gif)
  
  ![](pictures/git-reset-hard-1.png)
  
  ![](pictures/git-reset-hard-2.png)
  
  如上图所示，根据 git log 与 git reflog 确认需回退的提交，可直接使用提交的哈希值，也可使用 HEAD 索引（此处使用 HEAD 索引）。
  
  ![](pictures/git-reset-hard-3.png)
  
  如上图所示，硬重置后在工作区与暂存区中不存在修改过的文件。
  
  ![](pictures/git-reset-hard-4.png)
  
  硬重置后的 HEAD 引用指向指定的提交，在 git log 中无法查看该提交之后的更改。但是可使用 git reflog 查看所有的 Git 相关操作。

- 🔥 问题：软重置与硬重置后真的无法再回退至当前提交之后的提交（即未来版本）吗？
  
  由于 git log 无法显示分支回退的提交历史，因此也就无法查看回退版本前的提交历史。使用 git reflog 查看完整的 Git 操作历史记录，确定需要回退的当前提交之后的提交哈希或 HEAD 索引，再使用软重置或硬重置回退即可。
  
  ![](pictures/git-log-reflog-1.png)
  
  如上图所示，当前 test 分支的最新提交为 ea4b，需回退到 test 分支中该提交之后的 HEAD 索引 49 的提交。如果只是从 git log（上图左侧）确认的话就无法实现，而通过 git reflog（上图右侧）的话即可实现。
  
  ![](pictures/git-log-reflog-2.png)
  
  如上图所示，回退后已修改的文件在工作区与暂存区中都存在。
  
  ![](pictures/git-log-reflog-3.png)

- ✨ 推荐使用场景：
  
  **如果想恢复到之前某个提交的版本，且那个版本之后提交的版本都不要了，就可以用这种方法。**

-----

### Git 还原提交

- 除了使用版本回退（重置）以撤销指定提交及之后的提交外，如果要单独撤销指定的提交，可使用还原（revert），还原过程中将创建新的提交来说明此次还原的相关信息，同时也不会修改分支的历史。
  
  ```bash
  $ git revert HEAD
  # 在当前提交后面，新增一次提交，抵消掉上一次提交导致的所有变化。
  # 它不会改变过去的历史，所以是首选方式，没有任何丢失代码的风险。
  $ git revert <commit_hash>
  # 指定提交并还原
  # 注意：
  #   --no-edit 选项：执行时不打开默认编辑器，直接使用 Git 自动生成的提交信息。
  #   --no-commit 选项：只抵消暂存区和工作区的文件变化，不产生新的提交。
  ```
  
  ![](pictures/git-revert.gif)

- 👨‍🏫 示例：git revert 还原提交 903d6
  
  ![](pictures/git-revert-1.png)
  
  ![](pictures/git-revert-3.png)
  
  如下图所示：使用 git revert 还原提交时创建新的提交说明
  
  ![](pictures/git-revert-2.png)
  
  如下图所示：执行 git log -p 903d6 查看该提交的原始更改已经被还原
  
  ![](pictures/git-revert-4.png)

- ✨ 推荐使用场景：
  
  **如果想撤销之前的某一版本，但是又想保留该目标版本后面的版本，记录下这整个版本变动流程，就可以用这种方法。**

- revert 还原可指定提交以恢复为该提交之前的状态，但无法直接更细粒度地恢复提交中文件在提交之前的状态。因此，可使用以下方式指定提交与文件还原：
  
  ```bash
  $ git checkout <commit> <file>
  # 指定提交与文件，将其还原为该提交中的文件状态。
  # 还原的文件恢复至暂存区与工作区，可用于还原误操作的文件。
  ```
  
  ![](pictures/git-checkout-commit-demo.png)

-----

### Git 拣选

- 当一个特定分支包含活动分支需要的某个提交时，可对那个提交执行拣选（cherry-pick）。对一个提交执行拣选时，会在活动分支上创建一个新的提交，其中包含由拣选出来的提交所引入的修改。
  
  ```bash
  $ git cherry-pick <commit>
  # 指定提交将其拣选至当前活动分支中
  ```

- 如下图所示，假设 dev 分支上的提交 76d12 为 index.js 文件添加了一项修改，而希望将其整合到 master 分支中，且不想要整个 dev 分支，而只需要这个提交。
  
  ![](pictures/git-cherry-pick.gif)

- 如下图所示，将 test 分支中的提交 f5ac0 拣选至 main 分支中，main 分支中新建了 .gitignore 文件修改的提交。
  
  ![](pictures/git-cherry-pick.png)

-----

### 参考链接

- [Git-Book](https://git-scm.com/book/en/v2)
- [Git 核心知识总结](https://mp.weixin.qq.com/s/dlLm2BGn_PhgyvgL8dMmVA)
- [Git Fast-forward Merge](https://fmbase.tw/blog/2015/04/05/git-fast-forward-merge/#before-git-merge)
- [git merge 和 git merge --no-ff 的区别 | TY·Loafer](https://tyloafer.github.io/posts/132/)
- [git 使用 access token](https://www.cnblogs.com/ajanuw/p/16721335.html)
- [吵疯了，Pull Request 到底是个啥？](https://mp.weixin.qq.com/s?__biz=MzIwODI1OTk1Nw==&mid=2650323040&idx=1&sn=12b5f1342661c7964f8908eb1e14f590&chksm=8f09c67ab87e4f6ce6aa04b9a12ab95700089455b682eea6007e90172ec2d92f705277da34f6&mpshare=1&scene=24&srcid=01317kborZGowCYXvkkCmJNx&sharer_sharetime=1612094795958&sharer_shareid=6f52fcf5de1f55a8e68dc61bb30e3bc1#rd)
- [About pull requests - GitHub Docs](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/about-pull-requests)
- [🌳🚀 CS Visualized: Useful Git Commands](https://dev.to/lydiahallie/cs-visualized-useful-git-commands-37p1)
- [Git 分支使用规范](https://mp.weixin.qq.com/s/3i5XHoACpqpyGRVkPMSbzw)
