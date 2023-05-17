## 使用自动化控制器运行 Playbook

### 文档目录：

- 自动化控制器架构

- 在自动化控制器中运行 Playbook

### 自动化控制器架构：

- 描述自动化控制器架构：
  
  - Ansible 自动化平台 2 包含自动化控制器（`automation controller`）的组件，其在 Ansible 自动化平台的早期版本中被称为 `Ansible Tower`。
  
  - 自动化控制器提供可用于运行 Ansible 自动化代码的集中式中心。
  
  - 自动化控制器提供一个框架，用于在企业规模上高效运行和管理 Ansible。
  
  - 此外，通过引入各种功能来维持组织的安全性，如用于管理 playbook 的集中式 Web 界⾯、基于角色的访问控制（`RBAC`）以及集中式日志记录与审计。
  
  - 其 `REST API` 确保自动化控制器与企业的现有工作流（workflow）与工具集（tool set）轻松集成。
  
  - Ansible 自动化平台 2 中引入的自动化执行环境使得自动化控制器能够将其控制平面与其执行环境分离。
  
  - Ansible Tower 3.8 及更早版本中，执行环境与运行 Ansible Tower 的系统紧密耦合，这使得管理运行 Ansible Playbook 所需的各种模块的依赖项变得困难。
  
  - 🤔 对执行环境的更改必须直接在 Ansible Tower 系统上完成，若两个 playbook 需要不同的环境，则必须手动设置和更新 Python 虚拟环境（`venvs`）来管理它们，一些企业最终拥有成百上千个 venv。
  
  - 🤘 自动化控制器不使用系统可执行文件和 Python 安装或虚拟环境，而是使用自动化执行环境，通过该方法显著改善架构。
  
  - 由于自动化执行环境是容器，可从中心化的容器镜像仓库拉取它们，将它们⾃动安装到自动化控制器上并通过 Web UI 进行管理。
  
  - 👉 开发人员可以创建精确的自动化执行环境，再使用 `ansible-navigator` 确保它们适用于其自动化代码，这有助于确保自动化代码在开发人员的系统和自动化控制器中一致地运行。

- Ansible Tower 3.8 架构与 Ansible 自动化平台 2 架构对比：
  
  - Ansible Tower 3.8 架构：
    
    - 集中式单体应用
    
    - 控制节点包含控制平面与执行平面
    
    - 刚性架构导致可扩展性较差
  
  - Ansible 自动化平台 2 架构：
    
    - 分散式模块化应⽤程序
    
    - 控制平面与执行平面分离
    
    - 容器化的虚拟环境
    
    - 随时使用容器编排器按需扩展
    
    👉 该设计还使自动化控制器的未来版本能够在自动化控制器系统上仅运行控制平⾯（具有 Web UI 和 API），并在更靠近受管主机的其他计算机上运行其自动化执行环境，从而提⾼效率与扩展性。

- 自动化控制器功能：
  
  - 可视化控制面板
  
  - 基于角色的访问控制（RBAC）
  
  - 图形化清单管理
  
  - 任务管理器与作业调度
  
  - 实时与历史作业状态报告
  
  - 用户触发的自动化
  
  - 凭据管理
  
  - 集中式日志记录与审计
  
  - 集成式通知
  
  - 多 Playbook 工作流
  
  - 可浏览的 RESTful API

### 在自动化控制器中运行 Playbook：

- 自动化控制器提供一个集中位置来运行其 Ansible 工作负载。

- 必须存在以下资源，以供开发人员在自动化控制器中创建作业模板测试 playbook：
  
  - 1️⃣ 用于连接受管主机的计算机凭据
  
  - 2️⃣ 用于下载和同步远程源代码的控制凭据
  
  > 💥 注意：
  > 
  > 控制平面执行环境目前只接受由认可的证书颁发机构进行签名的 SSL 证书，而不接受自签名的 SSL 证书。若远程代码仓库的证书不是由认可的证书颁发机构签名，需通过 SSH 而不是 HTTPS 同步存储库。
  
  - 3️⃣ 包含内容位置的项目，如 ansible.cfg 配置文件、inventory 与 playbook 等。
  
  - 4️⃣ 包含主机或组的清单
    
    - 自动化控制器可以包含静态和动态清单。
    
    - 包含动态清单显示最近一次尝试同步清单的状态，静态清单的 Status 列显示 Disabled。
    
    - 👉 自动化控制器支持**手动**添加主机与组，也支持使用**项目清单文件填充**主机与组。
  
  - 5️⃣ 作业模板资源：
    
    - 以上资源全部创建完成后即可用于创建作业模板资源
    
    - 作业模板资源与命令行对应的设置与选项：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/automation-controller-playbook/job-template-cmd-options.png)
    
    - 自动化控制器中除了 playbook 运行的输出外，`Jobs` 页面还显示清单同步作业和源代码控制更新作业。每个作业都包含一个数字，用于显示自动化控制器运行作业的顺序。
    
    - ✨ 自动化控制器中可指定模板的 `Execution Environment`，当作业执行时，由自动化控制器所在节点上的 `awx` 用户使用该执行环境运行 podman 容器，推送 playbook 中的任务至受管主机。因此，自动化控制器（automation controller）中的执行环境需提前在自动化中心（automation hub）中配置。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/automation-controller-playbook/automation-controller-ee-setting.png)
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/automation-controller-playbook/automation-hub-container-images.png)
      
      ✨ 自动化中心中可存储名称格式为 `registry_uri/repository[:tag]` 或 `registry_uri/user_or_org/repository[:tag]` 的容器镜像。

- 凭据的类型包括：
  
  - `Ansible Galaxy/Automation Hub API Token`：
    
    创建此类型的资源，以便自动化控制器可以从 Ansible Galaxy、自动化中心和私有自动化中心下载内容集合和角色。创建凭据后，必须为组织启用凭据，在此过程中，可通过对凭据进行排序来指定优先级。
  
  - `Container Registry`（容器镜像仓库）：
    
    先进行身份验证，再从容器镜像仓库（如 registry.redhat.io 或私有自动化中心）拉取容器镜像，需创建此类凭据。
  
  - `GitHub Personal Access Token`（GitHub 个人访问令牌）：
    
    GitHub 不再允许使用基于密码的身份验证来通过 HTTPS 协议同步项目。要继续使用
    HTTPS（而非 SSH），需创建并使用 GitHub 个人访问令牌，然后创建使用该个人访问令牌的自动化控制器凭据。
  
  - `Machine`（计算机）：
    
    自动化控制器可使用此凭据类型来访问和更改受管主机。此凭据指定用户名和密码，或用户
    的 SSH 私钥。若凭据应允许特权升级，需指定特权升级用户名（通常为 root），并在必要时指定特权升级密码，也可设置为在使用凭据时提示输入密码。
  
  - `Source Control`（源码控制）：
    
    自动化控制器可以使用此凭据类型从远程代码库同步项目资源。指定用户名和密码，或用户的 SSH 私钥。
  
  - `Vault`：
    
    使用 Vault 凭据解密已使用 Ansible Vault 加密的文件。

### 参考链接：

- [RedHat Docs - 安装并升级 Private Automation Hub](https://access.redhat.com/documentation/zh-cn/red_hat_ansible_automation_platform/1.2/html-single/installing_and_upgrading_private_automation_hub/index)

- [To Install & configure the Ansible Ansible Automation Hub](https://www.jazakallah.info/post/to-install-configure-the-ansible-ansible-automation-hub)

- [Ansible Docs - Automation Controller User Guide v4.3](https://docs.ansible.com/automation-controller/latest/html/userguide/index.html)
