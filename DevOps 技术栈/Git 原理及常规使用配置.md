## Git 原理及常规使用配置

### 文档说明：

- 该文档中 Git 术语的工作区与工作树（working tree）同义。
- ✨ 该文档中所涉及的 Git 命令与原理将持续更新。

### 文档目录：

- 基础架构即代码

- Git 简介

- 初始化 Git 配置

- Git 工作流

- Git 分支与引用

- 创建与删除 Git 分支

- 合并 Git 分支

- 从旧 commit 创建分支

- 从本地仓库的版本回退

### 基础架构即代码（infrastructure as code）：

- 一个关键的 `DevOps` 概念是基础架构即代码。

- 不必手动管理基础架构，而是通过运行自动化代码来定义和构建系统。

- Red Hat Ansible 自动化是一种可实施这种方法的关键工具。

- 若 Ansible 项目是用于定义基础架构的代码，则应使用 Git 等版本控制系统（version control system）来跟踪和控制代码的更改。

- 版本控制也能够为基础架构代码的不同阶段（如开发、QA 和生产）实施生命周期。

- 可以将更改提交到分支，并在非关键性开发和 QA 环境中测试这些更改。

- 确认更改后，可以将它们合并到主生产代码，并将更改应用到生产基础架构。

### Git 简介：

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
  
  - ✅ 本地副本作为版本控制系统中文件的整个历史记录，而不仅仅是项目文件的最新快照。
  
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
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-manage-files-foue-areas.png)

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

### 初始化 Git 配置：

- 由于 Git 用户经常与多个贡献者一起修改项目，Git 会在每一次提交时记录用户的名称和电子邮件地址。

- 这些值可以在项目级别上定义，但也可为用户设置全局默认值。

- `git config --global` 命令可管理用户参与的所有 Git 项目的默认设置，将设置保存到运行 git 命令用户的 `~/.gitconfig` 文件中。
  
  ```bash
  $ man git-config
  # 查看 git config 的配置详细信息
  $ git config --global user.name "<git_developer_name>"
  $ git config --global user.email "<git_developer_email>"
  # 配置 git 开发者用户名与对应的邮箱
  # 在每次推送代码至远程代码仓库后将在仓库中可见该用户信息
  
  $ git config --global color.ui true
  $ git config --global push.default simple
  # 定义 simple 的推送方式
  $ git config --global -l
  # 查看 git 全局配置信息
  $ cat ~/.gitconfig
  # 查看 git 开发者配置的全局信息（作用于所有参与的项目）
  
  # 注意：
  #   $ git config --global --edit：直接进入 Vim 编辑 ~/.gitconfig 文件 
  ```
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-config-user-info.png)

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
  
  > 🤘 除了使用 git-prompt.sh 脚本设置 PS1 环境变量外，也可使用如 `oh-my-bash` 等 shell 命令行配置工具更改 theme 以获得不同的 PS1 环境变量的样式。
  > 
  > ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/oh-my-bash-ps1.jpg)

### Git 工作流：

- Git 工作流涉及工作区、暂存区、本地仓库与远程代码仓库的相互协作。

- 使用 `git clone` 命令克隆远程代码仓库至本地仓库，如下所示：
  
  ```bash
  $ git clone git@gitlab.lab.example.com:devuser/do447-course-demo.git
  ```

- 开发人员工作过程中，会在当前项目的工作区中创建新文件、修改现有的文件，使工作区变为 `dirty` 状态。

- 🚀 Git 目录结构与 Git 命令对应关系：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-workflow-1.png)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-workflow-2.png)

- 初始化 Git 仓库：
  
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
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-init-clone.png)

- 🤘 查看当前分支的 Git 工作区、暂存区的状态：
  
  ```bash
  $ git status
  ```

- 克隆远程代码仓库：
  
  ```bash
  $ git clone git@github.com:Alberthua-Perl/kani.git
  # 克隆 GitHub 远程代码仓库至本地仓库
  $ git clone git@gitlab.lab.example.com:devuser/do447-course-demo.git
  # 克隆 GitLab 远程代码仓库至本地仓库
  ```

- 工作区、暂存区相关操作：
  
  ```bash
  $ git checkout -- <file>
  # 撤销工作区中未暂存文件的更改
  
  $ git add <file>
  # 暂存指定文件至暂存区中
  $ git add --all <dir>
  # 将指定目录中的文件添加入暂存区
  $ git add --all
  # 暂存所有文件至暂存区中
  
  # 注意：
  #   1. 在下一次提交时，仅会将暂存到暂存区的文件保存到本地仓库中。
  #   2. 若用户同时处理两个更改，则可以将文件组织到两个提交中，以更好地跟踪更改。
  #   3. 先暂存并提交其中一组更改，然后暂存和提交其余的更改。
  
  $ git rm --force <file>
  # 同时删除工作区与暂存区中的同名文件
  $ git rm --cache <file>
  # 删除暂存区中的文件，不删除工作区中的同名文件。
  $ git reset HEAD <file>
  # 该命令与上述命令具有相同的作用
  $ git rm --force --cache <file>
  # 若工作区与暂存区中的同名文件内容发生变化，将删除暂存区中的文件，不删除工作区中的
  # 同名文件，内容为工作区中文件的更改。
  ```
  
  👉 示例：`git rm` 与 `git rm --cached` 的区别
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-rm-working-tree-and-staged.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-rm-f-cached-staged.jpg)
  
  👉 git add 针对于工作区已删除文件的操作：
  
  由于工作区中将已提交至本地仓库的文件删除，该文件进入未跟踪的状态（`untracked`），若要跟踪该已删除的文件可使用 `git add --all <pathspec>` 将其添加至暂存区中，待 commit 提交至本地仓库。
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-add-removed-file-in-working-tree.jpg)

- 本地仓库相关操作：
  
  ```bash
  $ git commit -m "<commit_context>"
  # 提交暂存区的文件至本地仓库中
  $ git commit -a
  # 一次性将修改的文件暂存、提交至本地仓库，但新创建的文件（未跟踪）不可使用。
  # commit 提交注意事项：
  #   1. 第一行为提交原因的简短摘要，通常少于 50 个字符。
  #   2. 接下来为一个空行。
  #   3. 然后消息的其余部分必须解释提交的所有详细信息和原因。
  
  $ git remote add origin git@gitlab.lab.example.com:devuser/haproxy-lamp.git
  # 添加远程代码仓库 URL 至 .git/config 文件中
  $ git remote set-url origin git@github.com:Alberthua-Perl/tech-docs.git
  # 更改 .git/config 文件中的原始远程代码仓库 URL
  # 远程 origin 仓库通常是指最初克隆的 Git 仓库。
  
  $ git pull
  # 从远程代码仓库中获取最新的提交，并将更新添加到本地仓库，以及合并到工作区中。
  # 注意：应经常运行此命令，从而获得其他⼈对远程代码仓库中项目作出的最新更改。
  $ git push [-u|--set-upstream] origin <branch_name>
  # 推送本地仓库做出的更改至远程代码仓库的分支中
  # --set-upstream 选项：关联本地分支与远程代码库分支
  ```
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-add-commit.png)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-pull-push.png)

- ✨ git push 推送代码的故障排查：
  
  👉 示例 1：
  
  远程代码仓库的分支中已存在其他开发者提交的新代码，而在本地工作区与本地仓库中无这些更新，因此本地仓库推送更新的代码至远程代码仓库的分支中时会由于两者的状态不一致而产生冲突（`conflict`），此时可将远程的更新拉取（`pull`）至本地工作区与本地仓库，同步两者的状态后即可重新推送，如下所示：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-push-error-resolv-1.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-push-error-resolv-2.jpg)
  
  👉 示例 2：
  
  本地工作区新建并切换至新分支，将代码更新提交至本地仓库中，但该工作区中的 `origin` 远程代码仓库中还未创建该分支，依然指向原先的远程分支，因此，若不使用 `-u` 或 `--set-upstream` 选项的话，将无法将本地仓库的新分支与远程分支关联，使用该选项后可推送成功。
  
  💥 远程代码仓库中的分支无需手动创建，使用上述选项后将自动创建！
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-push-set-upstream-branch.jpg)
  
  👉 示例 3：
  
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

- 检查 Git 日志：
  
  - 版本控制系统的部分要点是跟踪提交历史记录。
  
  - 每个提交都由提交哈希（commit hash）来标识。
    
    ```bash
    $ git log
    # 查看当前分支及其原分支的提交日志详细列表
    $ git log --pretty=oneline
      8936873bed36682b6885e61b8c76d761810a6179 (HEAD -> dev, origin/dev) Update README.md
      40cfa07c01887ceb1e86452e710e6d7621251f1e Update README.md
      38fe8f9b7212747a2603f79f48b14d705edc4aca (origin/master, master) Update README.md
      815f7de68ea026703384d7e40b7ce41d558c01a2 Update ansible.cfg
      cde250bc8f33e1e8477befd9d213107f6ad90bf3 Add http_restful_api_filter.yml
      2cddbdfa1c79a0b84fbfe5d433af80c57da8e592 Init commit
    # 查看 dev 分支及 master 原分支的部分提交日志列表
    $ git log --pretty=oneline -5
    # 查看当前分支及其原分支的最近 5 次提交日志简要列表
    $ git show [<commit_hash>|<tag>]
    # 查看指定提交的更改内容（或指定 tag 的详细信息）
    ```
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-log-commit-info.png)
  
  - 命令中不需要输入完整的哈希，只需其中的足够部分来唯一标识仓库中的特定提交。
  
  - 这些哈希还可用于恢复到之前的提交，或者查看版本控制系统的历史记录。

### Git 分支与引用（branch and reference）：

- Git 提交（commit）包含 Git 为仓库创建和处理整个历史记录所需的全部信息，具体包括：
  
  - ✨ 提交的唯一 `ID`，采用 `40` 位 `十六进制` 字符串格式，是提交内容的 `SHA-1` 哈希。
  
  - 更改的仓库文件的列表，以及对每个文件的确切更改。
  
  - 更改可以是行的增减、重命名或删除。
  
  - ✨ 父级提交的 ID，即在应用当前提交更改之前定义仓库状态的提交的 ID。
  
  - 提交的作者和创建者（或提交者）。
  
  - 提交中还包含一个 `HEAD` 引用列表（a list of references）。

- 🚀 Git 中 `HEAD` 引用的说明：
  
  - `HEAD` 引用是一个命名指针，指向当前所在本地分支的提交。
  
  - 引用可指向提交（`commit`）、标签（`tag`）或分支（`branch`）
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-head-log.jpg)

- 若在工作区中进行更改，使用 git add 命令进行暂存，并使用 git commit 命令进行提交，则将创建一个新的提交，以最新的提交作为其父项，并且 `HEAD` 将转为指向新的提交。

- Git 常用分支类型：
  
  - `master`：
    
    主分支，有且只有一个。
  
  - `release`：
    
    线上分支，一般为线上版本，线上版本发布后，会将 release 分支合并到 master。
  
  - `develop`：
    
    开发分支，通常给测试部署环境或者打包的分支，每个人在自己的分支上开发完成后，向develop 分支合并。
  
  - `feature`：
    
    通常是一个功能分支或者个人分支，每个公司的用法不一样，feature 分支一般会有多个，通常合并完成后会删除。
  
  - 各分支间的关系如下所示：
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-branch-workflow.png)

- Git 分支工作流示意：
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-branch-workflow-from-github.png)

### 创建与删除 Git 分支：

- Git 中的不同分支允许不同的工作流在统一 Git 仓库上并行演变。

- Git 创建分支实际上是创建了一个可以移动的新的指针，该指针将指向当前分支的新的提交。

- Git 分支相关操作：
  
  ```bash
  $ git branch <branch_name>
  # 从当前的 HEAD 引用提交创建一个新的分支，但不切换至该分支（HEAD 引用不指向该分支）。
  
  $ git branch [--list]
  # 查看本地仓库的分支
  $ git branch --list --all
  # 查看本地仓库与远程代码仓库中的分支
  $ git branch --remote
  # 查看远程代码仓库的分支
  $ git branch --delete <branch_name>
  # 删除指定的本地分支
  # 注意：删除分支，必须将其合并入其原始分支，否则无法删除！
  
  $ git checkout <branch_name>
  # 切换至指定分支，将 HEAD 引用指向该分支。
  $ git checkout -b <branch_name>
  # 创建并切换分支，该命令即 git branch 与 git checkout 的集合，HEAD 引用指向该分支。 
  # 注意：
  #   切换分支的时候，如果当前分支有改动没有提交，则不能切换分支，需要先把改动的内容提交
  #   或者放入暂存区。
  
  ### 删除并更新远程代码仓库分支 ###
  $ git branch -r -d <branch_name>
  # 删除远程代码仓库分支
  $ git push --set-upstream origin :<branch_name>
  # 更新远程代码仓库分支
  ```
  
  👉 示例：从 `master` 主分支当前的 `38fe8f9` commit 上创建 `feature` 分支
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-different-branch-checkout.jpg)
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-branch-checkout.png)
  
  👉 示例：删除并更新远程代码仓库分支
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/delete-remote-repository-branch.png)

> 💥 注意：
> 
> 1. 分支切换会改变工作区中的文件。
> 
> 2. 在切换分支时，一定要注意工作区中的文件会被改变。
> 
> 3. 若切换到一个较旧的分支，工作区会恢复到该分支最后一次提交时的样子。
> 
> 4. 若 Git 不能干净利落地完成这个任务，它将禁止切换分支。

### 🚀 合并 Git 分支：

- 分支上的工作完成后，可以将该分支与原始分支合并。

- 这允许并行操作新功能和漏洞修复，而主分支则没有未完成或未经测试的工作。

- 💥 有时，无法自动合并对多个分支的更改，由于每个分支都对相同文件的相同部分进行更改，这种情况称为合并冲突（`merge conflict`），需要通过编辑受影响的文件来手动解决。

- ❓ 合并策略和冲突解决方案将在另外的文档中加以解决说明！
  
  ```bash
  $ git merge <branch_name>
  # 将指定的分支合并入原始创建的分支
  # 注意：分支合并入原始分支后，其更改将全部还原。
  ```
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-merge-demo.png)

### 从旧 commit 创建分支：

- 除了从当前分支的最新 commit 上创建新分支外，也可从其他分支的旧 `commit` 中创建新分支。
  
  ```bash
  $ git checkout <previous_commit_hash>
  # 指定某分支之前的提交，用于在该提交上创建分支，即 HEAD 引用指向该提交。
  $ git checkout -b <branch_name>
  # 创建并切换至新分支中
  ```
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/git-basic-command/git-create-new-branch-from-previous-commit.png)

### 从本地仓库的版本回退：

- Git 的版本回退实际上是将当前分支的 HEAD 引用指向旧 commit。
  
  ```bash
  $ git reflog
  # 查看 HEAD 引用的历史列表，即所有的 commit 历史列表信息。
  $ git reset --hard <commit>
  # 回退版本至当前分支的指定 commit，同时回退工作区与暂存区。
  ```

- 🚀 示例：从 `cf0c68c` commit 回退至 `3f9bd69` commit 
  
  ```bash
  ┌─[devops][workstation][±][dev ✓][~/scm-demo]
  └─➞ cat README.md 
  ## Init git repo for dev branch
  
  - Add one line to test
  
  - Add one line to test git log 
  # 查看当前分支工作区中的 README.md 文件内容
  # 此时 HEAD 引用指向 dev 分支的 cf0c68c commit
  
  ┌─[devops][workstation][±][dev ✓][~/scm-demo]
  └─➞ git reflog
  cf0c68c (HEAD -> dev) HEAD@{0}: reset: moving to cf0c68c    # 当前的 HEAD 引用指针
  cf0c68c (HEAD -> dev) HEAD@{1}: reset: moving to cf0c68c
  3f9bd69 HEAD@{2}: reset: moving to 3f9bd69f31e817297858ee25efeb08dadf50331a
  cf0c68c (HEAD -> dev) HEAD@{3}: reset: moving to cf0c68c
  3f9bd69 HEAD@{4}: reset: moving to 3f9bd69f31e817297858ee25efeb08dadf50331a
  cf0c68c (HEAD -> dev) HEAD@{5}: commit: dev branch - add one line to test git log
  3f9bd69 HEAD@{6}: commit: dev branch - change title
  44f1181 (master) HEAD@{7}: checkout: moving from master to dev
  44f1181 (master) HEAD@{8}: commit: master branch - add one line to test
  1ae7ef0 HEAD@{9}: commit (initial): Init master branch
  # git log 命令只能查看当前 commit 及之前的提交历史信息，而无法查看 commit 回退的历史信息。
  # 因此，此处使用 git reflog 命令查看当前分支及其原分支的所有 commit。
  
  ┌─[devops][workstation][±][dev ✓][~/scm-demo]
  └─➞ git reset --hard 3f9bd69
  HEAD is now at 3f9bd69 dev branch - change title
  # 重置当前分支的 HEAD 为指定 commit，同时重置工作区与暂存区，与指定 commit 一致。
  # 注意：此时该 HEAD 引用分支之后的提交将无法使用 git log 命令查看，需使用 git reflog 命令查看。
  ┌─[devops][workstation][±][dev ✓][~/scm-demo]
  └─➞ cat README.md 
  ## Init git repo for dev branch
  
  - Add one line to test
  # 当前分支工作区中的 README.md 文件内容已回退至 3f9bd69 commit
  
  ┌─[devops][workstation][±][dev ✓][~/scm-demo]
  └─➞ git log --pretty=oneline
  3f9bd69f31e817297858ee25efeb08dadf50331a (HEAD -> dev) dev branch - change title
  44f11819d9cb2b7c4f3ba893798309fb60a34ee4 (master) master branch - add one line to test
  1ae7ef00d8657bc6802b657a4dcd88ceb6e256f6 Init master branch 
  # 此时的 HEAD 引用也指向 3f9bd69 commit
  ```
