# 🦎 SLE15SP6 中实践 ansible-navigator 与 EE 容器镜像

## 文档说明 & 实验环境

- 该文档在 SLE15SP6 中实践 ansible-navigator 与 EE 容器镜像，验证 ansible-navigator 在 SUSE 中运行 playbook 的可行性。
- 操作系统：SUSE Linux Enterprise Server 15 SP6
- 资源分配：
  - Ansible 控制节点：
    - 主机名：`mysuse-amd`
    - 硬件资源：8C16G
    - Python 版本：`3.6/3.11`
  - Ansible 受管节点：
    - 主机名：
      - `jenkins-agent0`
      - `jenkins-agent1`
    - 硬件资源：2C4G
    - Python 版本：`3.6/3.11`
- ansible-navigator 版本：`25.1.0`
- EE 容器镜像版本：`ghcr.io/ansible-community/community-ee-base:2.18.3-1`

## 文档目录

- 安装 Python 3.11 环境
- 切换 Python 多版本环境
- 部署用户的 Python venv 虚拟环境
- 安装 ansible-navigator
- 选择与拉取 EE 容器镜像
- 验证测试 ansible-navigator 与 EE 容器镜像
- 安装 community-general collection
- 测试已安装的 collection
- 参考链接

## 安装 Python 3.11 环境

笔者实验环境中由于 Ansible 控制节点使用 Python 3.6 的 pip3 工具安装 ansible-navigator 报错显示，ansible-navigator 需使用 Python 3.7 及以上版本支持，因此额外安装 Python 3.11。可参考此 [repo 文件](https://github.com/Alberthua-Perl/sc-col/blob/master/dnf-repo/OpenSUSE-Leap_15.6-python311.repo) 完成安装。

为了确保后续使用 EE 容器镜像中的 Python 版本（该镜像中的 Python 版本为 3.13.2）与 Ansible 受管节点的 Python 版本有更好的兼容性，此处在所有受管节点中也安装 Python 3.11。

```bash
## 以 Ansible 控制节点为例（受管节点使用相同的方法）
mysuse-amd:~ # vim /etc/zypp/repos.d/OpenSUSE-Leap_15.6-python311.repo 
[OpenSUSE-Leap_15.6-python311]
name=OpenSUSE Leap 15.6 of Python311
enabled=1
autorefresh=1
baseurl=https://download.opensuse.org/repositories/devel:/languages:/python/15.6/
mysuse-amd:/etc/zypp/repos.d # zypper refresh
mysuse-amd:/etc/zypp/repos.d # zypper install -y python311
```

## 切换 Python 多版本环境

当前 Ansible 控制节点与受管节点都具有 Python 3.6 与 3.11 环境。为了更加方便地管理本地 Python 多版本环境，可使用 `update-alternatives` 软件包提供的 `alternatives` 命令集中管理 Python。如下所示：

```bash
mysuse-amd:~ # alternatives --install /usr/bin/python python /usr/bin/python3.6 100
# --install 选项的命令行格式：<link> <name> <path> <priority>
# <link> 指创建的 python 软链接
# <name> 指 alternatives 配置的名称
# <path> 指程序的绝对路径
# <priority> 指 alternatives 选择作为默认版本程序的优先级，该值越高，越倾向于使用此版本。
mysuse-amd:~ # alternatives --install /usr/bin/python python /usr/bin/python3.11 200
mysuse-amd:~ # alternatives --config python
There are 2 choices for the alternative python (providing /usr/bin/python).

  Selection    Path                 Priority   Status
------------------------------------------------------------
* 0            /usr/bin/python3.11   200       auto mode
  1            /usr/bin/python3.11   200       manual mode
  2            /usr/bin/python3.6    100       manual mode
# 选择 Python 3.11 版本用于 Ansible 控制节点的 ansible-navigator 安装
```

## 部署用户的 Python venv 虚拟环境

Python 的 `venv` 是一个用于创建和管理虚拟环境的模块。虚拟环境是一个独立的 Python 环境，允许在同一台机器上为不同的项目使用不同版本的 Python 包，而不会产生冲突。以下是对 venv 的介绍说明：

- 什么是虚拟环境？
  - 虚拟环境是一个隔离的 Python 环境，包含独立的 Python 解释器、库和脚本。
  - 每个虚拟环境都有自己的 site-packages 目录，用于安装第三方包。
  - 通过虚拟环境，可以为不同的项目使用不同版本的包，避免全局环境的污染。
- 为什么使用虚拟环境？
  - 隔离依赖：不同项目可能依赖不同版本的包，虚拟环境可以避免版本冲突。
  - 避免污染全局环境：安装的包仅对当前虚拟环境有效，不会影响系统的全局 Python 环境。

此次实验中以普通用户身份安装 ansible-navigator，因此，在普通用户家目录中创建 `venv` 的虚拟运行环境。这样可将 ansible-navigator 限制安装在普通用户环境中而不对全局 Python 环境产生影响。如下所示：

```bash
alberthua@mysuse-amd:~/backup> python --version
Python 3.11.11
alberthua@mysuse-amd:~/backup> python -m venv venv-navigator
# 使用 venv 模块创建名为 venv-navigator 的虚拟运行环境
alberthua@mysuse-amd:~/backup> ls -lhd venv-navigator
drwxr-xr-x 5 alberthua users 74 Mar 18 22:05 venv-navigator
# 该虚拟运行环境为单独的目录（也可为家目录中的隐藏目录）
alberthua@mysuse-amd:~/backup> source venv-navigator/bin/activate
# 激活 venv 虚拟运行环境
(venv-navigator) alberthua@mysuse-amd:~/backup> deactivate
# 退出 venv 虚拟运行环境
alberthua@mysuse-amd:~/backup> 

### 可选操作 ###
alberthua@mysuse-amd:~/backup> rm -rf ./venv-navigator
# 删除指定的 venv 虚拟运行环境
```

## 安装 ansible-navigator

如下所示，在 venv 虚拟运行环境中安装 ansible-navigator：

```bash
alberthua@mysuse-amd:~/backup> source venv-navigator/bin/activate
(venv-navigator) alberthua@mysuse-amd:~/backup> echo $PATH  #查看当前 venv 虚拟环境的 PATH 环境变量
/home/alberthua/backup/venv-navigator/bin:/home/alberthua/bin:/usr/local/bin:/usr/bin:/bin:/home/alberthua/.local/bin:/home/alberthua/.local/bin:/home/alberthua/backup/venv-navigator/bin
(venv-navigator) alberthua@mysuse-amd:~/backup> pip --version
pip 25.0.1 from /home/alberthua/backup/venv-navigator/lib64/python3.11/site-packages/pip (python 3.11)
(venv-navigator) alberthua@mysuse-amd:~/backup> pip config set global.index-url https://mirrors.aliyun.com/pypi/simple/
(venv-navigator) alberthua@mysuse-amd:~/backup> pip config set install.trusted-host mirrors.aliyun.com
(venv-navigator) alberthua@mysuse-amd:~/backup> pip -v config list
(venv-navigator) alberthua@mysuse-amd:~/backup> pip install ansible-navigator
Looking in indexes: https://mirrors.aliyun.com/pypi/simple/
...
(venv-navigator) alberthua@mysuse-amd:~/backup> ansible-navigator --version
ansible-navigator 25.1.0

(venv-navigator) alberthua@mysuse-amd:~/backup> vim ~/.profile
...
export PATH=$PATH:~/.local/bin:~/backup/venv-navigator/bin  #添加 PATH 环境变量，以便退出 venv 虚拟运行环境后依然可使用。
(venv-navigator) alberthua@mysuse-amd:~/backup> source ~/.profile
(venv-navigator) alberthua@mysuse-amd:~/backup> deactivate  #安装完成后即可退出 venv 虚拟运行环境
```

## 选择与拉取 EE 镜像

ansible-navigator 在运行环境（execution envinonment, EE）中推送 playbook 至受管节点，根据受管节点上的 python 环境执行相关任务。可参考 [此图](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-rhaap20-basic/ansible-automation-platform-2.0-arch.png) 了解更加详细的细节。

根据 [Running Ansible with the community EE image](https://docs.ansible.com/ansible/latest/getting_started_ee/run_community_ee_image.html) 指出，可选择名为 `ghcr.io/ansible-community/community-ee-base` 的容器镜像，并且保持该镜像内的 Python 版本与受管节点的 Python 版本一致（均为 Python 3.11），因此选择 `2.16.0-1` 版本的镜像 。

当前环境中使用普通用户运行 ansible-navigator，同样使用此用户拉取与运行 EE 容器镜像，此时 Podman 以 `rootless` 模式执行。

以上镜像来自于 google 容器镜像仓库，国内无法拉取或拉取超时导致失败退出。笔者已将此镜像同步至个人 Quay 镜像仓库中，可使用以下命令拉取：

```bash
alberthua@mysuse-amd:~/backup> podman pull quay.io/alberthua/community-ee-base:2.16.0-1
alberthua@mysuse-amd:~/backup> podman tag quay.io/alberthua/community-ee-base:2.16.0-1 ghcr.io/ansible-community/community-ee-base:2.16.0-1
alberthua@mysuse-amd:~/backup> podman rmi quay.io/alberthua/community-ee-base:2.16.0-1
```

## 验证测试 ansible-navigator 与 EE 容器镜像

至此，ansible-navigator 与 EE 容器镜像已准备就绪，接下来需测试其可用性。如下所示：

```bash
# 创建项目目录
alberthua@mysuse-amd:~/backup> mkdir navigator-devops
alberthua@mysuse-amd:~/backup> cd navigator-devops/

# 编辑 ansible-navigator 配置文件
alberthua@mysuse-amd:~/backup/navigator-devops> vim ansible-navigator.yml 
---
ansible-navigator:
  ansible:
    config:
      path: ./ansible.cfg  #指定 ansible.cfg 配置文件路径
  execution-environment:
    container-engine: podman  #当前环境的容器引擎
    enabled: true
    environment-variables:
      set:
        ANSIBLE_CONFIG: ansible.cfg
    image: 'ghcr.io/ansible-community/community-ee-base:2.16.0-1'  #指定 EE 容器镜像
    pull:
      policy: missing  #EE 容器镜像本地不存在时将自动拉取
  logging:
    level: critical  #ansible-navigator 运行的日志等级
  mode: stdout  #默认情况下，ansible-navigator 以交互模式运行；stdout 模式为命令行标准输出
  playbook-artifact:
    enable: false  #不生成 artifact 文件

# 编辑 ansible 配置文件
alberthua@mysuse-amd:~/backup/navigator-devops> vim ansible.cfg 
[defaults]
inventory = ./suse-inventory
remote_user = devops
ask_pass = False
host_key_checking = False
collections_path = /home/alberthua/backup/navigator-devops/collections  #关于 collections_path 指令的说明
#[DEPRECATION WARNING]: [defaults]collections_paths option, does not fit var naming standard, 
#use the singular form collections_path instead. 
#This feature will be removed from ansible-core in version 2.19. 
#Deprecation warnings can be disabled by setting deprecation_warnings=False in ansible.cfg.
# interpreter_python = /usr/bin/python3.6
# interpreter_python = /usr/bin/python3.11
# deprecation_warnings = False

[privilege_escalation]
become = true
become_user = root
become_method = sudo
become_ask_pass = False

# 编辑 inventory 主机清单文件
alberthua@mysuse-amd:~/backup/navigator-devops> vim suse-inventory 
[jenkins_master]
mysuse-amd

[jenkins_nodes]
jenkins-agent0
jenkins-agent1

# Ad-hoc 单行命令测试
alberthua@mysuse-amd:~/backup/navigator-devops> ansible all -m shell -a 'id; python --version'

# 编辑测试 playbook 并运行
alberthua@mysuse-amd:~/backup/navigator-devops> mkdir playbooks
alberthua@mysuse-amd:~/backup/navigator-devops> vim playbooks/00-test_debug.yml 
---
- name: Initially test ansible-navigator and ee container image
  hosts: all

  tasks:
    - name: Echo test plain text info
      ansible.builtin.debug:
        msg: Deploy ansible-navigator, ee container image and run test playbook successfully!

alberthua@mysuse-amd:~/backup/navigator-devops> ansible-navigator run playbooks/00-test_debug.yml
...
TASK [Echo test plain text info] *************************************************************************************************************************************************************
ok: [mysuse-amd] => {
    "msg": "Deploy ansible-navigator, ee container image and run test playbook successfully!"
}
ok: [jenkins-agent0] => {
    "msg": "Deploy ansible-navigator, ee container image and run test playbook successfully!"
}
ok: [jenkins-agent1] => {
    "msg": "Deploy ansible-navigator, ee container image and run test playbook successfully!"
}

PLAY RECAP ***********************************************************************************************************************************************************************************
jenkins-agent0             : ok=2    changed=0    unreachable=0    failed=0    skipped=0    rescued=0    ignored=0   
jenkins-agent1             : ok=2    changed=0    unreachable=0    failed=0    skipped=0    rescued=0    ignored=0   
mysuse-amd                 : ok=2    changed=0    unreachable=0    failed=0    skipped=0    rescued=0    ignored=0
```

## 安装 community-general collection

在原始 EE 容器镜像中只有少量内置的管理 SLE15SP6 系统的模块，因此需安装额外的 collection 用于支持后续的系统管理。此处，可从 [Ansible Galaxy Collection - community.general](https://galaxy.ansible.com/ui/repo/published/community/general/docs/?version=10.4.0) 中下载 tarball 安装或直接安装该 collection。如下所示：

```bash
alberthua@mysuse-amd:~/backup/navigator-devops> mkdir collections
alberthua@mysuse-amd:~/backup/navigator-devops> vim collections/requirements.yml
---
collections:
  - name: http://192.168.110.220/packages/community-general-10.3.1.tar.gz
  # 离线安装：此处已将 collection 的 tarball 下载并存储在指定的 Apache2 HTTPD 服务器中

  #- name: community.general
  #  version: 10.3.1
  # 在线安装：直接指定 collection 名称与版本可直接从 Ansible Galaxy 中下载安装

alberthua@mysuse-amd:~/backup/navigator-devops> ansible-galaxy collection install -r collections/requirements.yml -p ./collections
alberthua@mysuse-amd:~/backup/navigator-devops> ansible-galaxy collection list

# /home/alberthua/backup/navigator-devops/collections/ansible_collections
Collection        Version
----------------- -------
community.general 10.3.1
```

## 测试已安装的 collection

SUSE 中使用 `zypper` 命令安装软件包，而使用 Ansible 安装软件包时对应的模块名为 `community.general.zypper`，可用此模块测试 collection 是否安装成功。如下所示：

```bash
alberthua@mysuse-amd:~/backup/navigator-devops> cat playbooks/01-install_apache2.yml
---
- name: Deploy Apache2 HTTPD Server
  hosts: jenkins_nodes

  tasks:
    - name: Install Apache2 HTTPD package
      community.general.zypper:
        name: apache2
        state: present

    - name: Start and enable Apache2 HTTPD Server
      ansible.builtin.service:
        name: apache2
        state: started
        enabled: yes

alberthua@mysuse-amd:~/backup/navigator-devops> ansible-navigator run playbooks/01-install_apache2.yml
# 运行成功说明 collection 安装正确可用
```

## 参考链接

- 📦 [GitHub: community-ee-base](https://github.com/ansible-community/images/pkgs/container/community-ee-base/versions?filters%5Bversion_type%5D=tagged)
- 📦 [GitHub: community-ansible-dev-tools](https://github.com/ansible/ansible-dev-tools/pkgs/container/community-ansible-dev-tools)
- 📚 [Ansible Core Documentation: Interpreter Discovery](https://docs.ansible.com/ansible-core/2.16/reference_appendices/interpreter_discovery.html)
- 📚 [Ansible Core Documentation: Running Ansible with the community EE image](https://docs.ansible.com/ansible/latest/getting_started_ee/run_community_ee_image.html)
- [Ansible Community Documentation: Ansible Configuration Settings](https://docs.ansible.com/ansible/latest/reference_appendices/config.html)
- [Ansible Galaxy Collection: community.general](https://galaxy.ansible.com/ui/repo/published/community/general/docs/?version=10.4.0)
- [Ansible Development Tools Documentation: community-ansible-dev-tools](https://ansible.readthedocs.io/projects/dev-tools/container/)
- [Ansible 开发工具文档: community-ansible-dev-tools](https://readthedocs.ansible.org.cn/projects/dev-tools/container/)
- [Linux：使用 Alternatives 管理多版本程序](https://cloud.tencent.com/developer/article/2414109)
