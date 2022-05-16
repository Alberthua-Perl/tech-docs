## Ansible 自动化进阶实践 - 利用推荐做法进行 Playbook 开发

### 文档目录：

- 实施推荐做法

### 实施推荐做法

- 保持简单：
  
  - Ansible 的一大优势是简洁性。
  
  - 如果使 playbook 保持简单，就能更加轻松地使用、修改和理解它们。
  
  - 保持 playbooks 的可读性：
    
    - YAML 不属于编程语言，而是标记型语言。
    
    - 它非常适合表述一系列任务或项目，或者一组键值对。
    
    - 若难以在 Ansible Playbook 中表述一些复杂的控制结构或条件，可考虑通过另一种方式解决问题。
    
    - 可通过模板和 Jinja2 过滤器处理变量中的数据，这可能更适用于解决问题。
    
    - 使用原生的 `key: value` YAML 语法，而不使用如下的 "折叠"（`key=value`）语法：
      
      ```yaml
        - name: Postfix is installed and updated
          yum: name=postfix state=latest
          notify: restart_postfix
        - name: Postfix is running
          service: name=postfix state=started
      ```
  
  - 使用现有模块：
    
    - 编写新 playbook 时，从基本 playbook 开始，并尽可能使用静态 inventory。
    
    - 在构建设计时，使用 debug 模块。
    
    - 酌情使用 Ansible 中包含的特殊用途模块，而不是 command、shell、raw 或其他类似的模块。
    
    - 许多模块具有一个默认的 state 或其他变量控制其用途。
    
    - 但应该明确指定所需的 state，这样，可轻松地阅读 playbook，防止模块的默认行为在Ansible 的后续版本中发生更改。
    
    - 👉 遵循标准样式：
      
      - 编写 Ansible 项目时，应考虑一个需要遵循的标准 "样式"：
        
        - 缩进多少空格、如何使用垂直空白
        
        - 如何命名任务、play、角色和变量
        
        - 应对什么进行注释、如何注释
      
      - 遵循同一的标准有助于提⾼可维护性和可读性。

- 井然有序：
  
  - 👉 遵循变量命名约定：
    
    - 变量命名非常重要，因为 Ansible 具有相对扁平的命名空间。
    
    - 使用描述性变量。
    
    - 在角色中，使用角色名称为角色变量加上前缀是好做法。
      
      如角色命名为 myapp，那么变量名称开头为 `myapp_`。
    
    - 变量名称应阐明内容。
      
      如 apache_max_keepalive 等名称可以解释相关的一个或多个值的含义。
    
    - 为角色和组变量加上变量所属角色或组的名称作为其前缀。
      
      如 apache_port_number 比 port_number 更能防止错误。
  
  - 标准化项目结构：
    
    - 构造 Ansible 项目的文件时，使用统一的模式。
    
    - 👉 示例 1：[HAProxy LAMP 集群项目结构示例](https://github.com/Alberthua-Perl/ansible-demo/tree/master/haproxy-lamp)
      
      ```bash
      ┌─[devops][workstation][±][master ✓][~/gitlab/haproxy-lamp]
      └─➞ tree .
      .
      ├── ansible.cfg
      ├── deploy.log
      ├── group_vars
      │   ├── haproxy
      │   └── mariadb
      ├── inventory
      ├── README.md
      ├── roles
      │   ├── apache-php
      │   │   ├── tasks
      │   │   │   └── main.yaml
      │   │   └── templates
      │   │       └── index.php.j2
      │   ├── base
      │   │   ├── files
      │   │   │   ├── CentOS-Base.repo
      │   │   │   └── epel.repo
      │   │   └── tasks
      │   │       └── main.yml
      │   ├── haproxy
      │   │   ├── handlers
      │   │   │   └── main.yaml
      │   │   ├── tasks
      │   │   │   └── main.yaml
      │   │   └── templates
      │   │       └── haproxy.cfg.j2
      │   └── mariadb
      │       ├── handlers
      │       │   └── main.yaml
      │       └── tasks
      │           └── main.yaml
      └── site.yaml
      
      15 directories, 17 files
      ```
    
    - 👉 示例 2：
      
      ```bash
      .
      ├── dbservers.yml
      ├── inventories/
      │   ├── prod/
      │   │   ├── group_vars/
      │   │   ├── host_vars/
      │   │   └── inventory/
      │   └── stage/
      │       ├── group_vars/
      │       ├── host_vars/
      │       └── inventory/
      ├── roles/
      │   └── std_server/
      ├── site.yml
      ├── storage.yml
      └── webservers.yml
      ```
      
      - site.yml：主 playbook，其中包含或导入执行特定任务的 playbook （dbservers.yml、storage.yml、webservers.yml）。
      
      - 角色：位于 roles 目录的子目录中，如 std_server。
      
      - inventories/prod 与inventories/stage 目录中有两个静态清单，分别具有单独的清单变量文件，可以通过更改使用的清单来选择不同的主机组。
      
      - playbook 结构的一大优势在于，可将较大的 playbook 分成较小的文件，使其更易阅读。
      
      - 这些较小的子 playbook 可能包含独立运行的、适合特定用途的 play。
        
        如：部署 Ceph 集群的 Ansible Playbook、部署 OpenShift 集群的 Ansible Playbook 等。
  
  - 使用动态清单（dynamic inventory）：
    
    - 尽可能使用动态清单。
    
    - 动态清单支持从一个真实的中央来源集中管理主机和组，并确保清单自动更新。
    
    - [python 动态清单示例](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter01/simple-dynamic-inventory.py)：
      
      ```python
      #!/usr/bin/env python
      
      '''
       Demo example for Ansible dynamic inventory.
       Modified by hualf on 2020-02-04.
      '''
      
      # coding=utf-8
      import sys
      import json
      
      host1_ip = ['192.168.0.110']
      host2_ip = ['192.168.0.112', '192.168.0.113']
      host3_ip = ['192.168.0.114']
      group1 = 'haproxy'
      group2 = 'web'
      group3 = 'mariadb'
      hostdata = {group1:{"hosts":host1_ip}, group2:{"hosts":host2_ip}, group3:{"hosts":host3_ip}}
      # print json.dumps(hostdata,indent=4)
      
      if len(sys.argv) == 2 and sys.argv[1] == '--list':
          print(json.dumps(hostdata,indent=4))
      elif len(sys.argv) == 3 and sys.argv[1] == '--host':
          print(json.dumps({}))
      else:
          print("Requires an argument, please use --list or --host <host>")
      ```
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-playbook-develop-recommend/python-simple-inventory.png)
    
    - 动态清单在与云提供商、容器和虚拟机管理系统结合使用时尤其强大。
    
    - 这些系统可能已具有 Ansible 可使用的某种形式的清单信息。
    
    - 若无法使用动态清单，其他工具可以动态构建组或其他信息。
    
    - 可使用 `group_by` 模块根据 facts 事实动态生成组成员资格，该组成员资格对playbook 的其余部分有效。
      
      ```yaml
      - name: Generate dynamic groups
        hosts: all
        tasks:
          - name: Generate dynamic groups based on architecture
            group_by:
              key: arch_"{{ ansible_facts['architecture'] }}"
      - name: Configure x86_64 systems
        hosts: arch_x86_64
        tasks:
          - name: First task for x86_64 configuration
          ...output omitted...
      ```
  
  - 充分利用组：
    
    - 主机可以是多个组的成员。
    
    - 根据不同的特征将主机划分为不同的类别：
      
      - 地理位置：
        
        将不同地区、国家、洲或数据中心的主机区别开来。
      
      - 环境：
        
        将专用于软件生命周期不同阶段的主机区别开来，如开发、暂存、测试或生产等。
      
      - 站点或服务：
        
        将提供或链接到功能子集的主机分组，如特定 Web 站点、应用或功能子集。
      
      > 💥 注意：
      > 
      > 1. 主机将从所属的组中继承变量。
      > 
      > 2. 如果两个组对同一个变量具有不同的设置，并且主机是两者的成员，则使用的值是最后一次加载的值。
  
  - 角色重用：
    
    - 角色可将 playbook 保持简单，能够通过重复利用项目间的通用代码来减少工作量。
    
    - 如果已编写自定义的角色，其重点放在与 playbook 类似的特定用途或功能上。
    
    - 通过变量使角色成为可配置的通用角色，以便用于不同的 playbook 时无需再编辑。
    
    - 使用 `ansible-galaxy` 命令初始化角色的目录层次结构，并提供初始模板文件。
    
    - 也可在 `Ansible Galaxy` 网站上共享角色。
    
    - RHEL 中的 `rhel-system-roles` 软件包提供的角色受官方支持（一些角色可能处于技术预览阶段）。
    
    - 检查提供的角色，以确定它们是否有用。
    
    - 请谨慎并仔细选地选取社区提供的 Ansible Galaxy 网站上的角色。
    
    - 角色保存在项目的 `roles` 子目录中。
    
    - 可使用 ansible-galaxy 命令自动从单独的 Git 存储库中获取角色。
  
  - 集中运行 Playbooks：
    
    - 若要控制对系统的访问并审核 Ansible 活动，使用一个专用控制节点，让所有 Ansible Playbook 都从中运行。
    
    - 系统管理员仍应在系统上拥有自己的账户，以及用于连接受管主机的凭据，并在需要时
      
      升级特权。
    
    - 当系统管理员离职时，可以从受管主机的 `authorized_keys` 文件中删除其 SSH 公钥，并且撤销其 `sudo` 命令特权，而不影响其他管理员。
    
    - 可使用红帽 `Ansible Tower` 作为中央主机。
    
    - 红帽 Ansible Tower 包含在新的红帽 Ansible 自动化订阅中，提供的功能包括：
      
      - 更轻松地控制对凭据的访问
      
      - 控制 playbook 执行
      
      - 简化不熟悉 Linux 命令行的用户的自动化
      
      - 审核和追踪 playbook 运行

- 经常测试：
  
  ✅ 测试任务的结果：若需确认任务是否已成功，验证任务的结果，而不要信任模块的返回代码。
  
  ```yaml
    - name: Start web server
      service:
        name: httpd
        status: started
    - name: Check web site from web server
      uri:
        url: http://{{ ansible_fqdn }}
        return_content: yes
      register: example_webpage
      failed_when: example_webpage.status != 200
  ```

- 使用 Block/Rescue 来恢复或回滚（rollback）：
  
  `block` 指令（directive）用于对任务进行分组，与 `rescue` 指令结合使用时，帮助从错误或故障中恢复。
  
  ```yaml
    - block:
        - name: Check web site from web server
          uri:
            url: http://{{ ansible_fqdn }}
            return_content: yes
          register: example_webpage
          failed_when: example_webpage.status != 200
      rescue:
        - name: Restart web server
          service:
            name: httpd
            status: restarted
  ```

- 使用最新的 Ansible 版本开发 Playbook：
  
  - 即使不在生产中使用最新版本的 Ansible，也应该定期针对 Ansible 的最新版本测试playbook。
  
  - 该测试将避免在 Ansible 模块和功能不断演变时出现问题。
  
  - 若 playbook 在运行时显示警告或启用消息（`deprecation warning`），应注意它们并做出相应的调整。
  
  - 🤘 通常，如果 Ansible 中的某一功能已启用或有变化，则该项目会在删除或更改功能之前提早四个小版本（minor releases）提供启用通知。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-playbook-develop-recommend/ansible-playbook-deprecation-warning.png)
  
  > yum 模块从 2.11 版本开始，将移除使用循环的语法，可使用列表形式安装多个软件包。

- 使用测试工具：
  
  ```bash
  $ ansible-playbook --syntax-check <playbook_name>.yml
  # 检查 playbook 的语法，而无需运行。
  $ ansible-playbook --check <playbook_name>.yml
  # 针对 "检查模式" 中的实际受管主机运行 playbook，以查看 playbook 执行的更改。
  # 此项检查不能保证完全准确，因为 playbook 可能需要实际运行一些任务，playbook 中的后续任务
  # 才能正常运行。
  ```
  
  - 🤘 `ansible-lint` 工具：
    
    - 对 playbook 进行分析并查找可能存在的问题。
    
    - 报告的所有问题为可能存在的错误。
  
  - `yamllint` 工具：
    
    该工具不关心 Ansible，只是用于识别并分析 YAML 文件语法相关的问题。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-playbook-develop-recommend/yamllint-demo.png)
    
  > 👉 yamllint 工具位于 [epel8 软件源](https://github.com/Alberthua-Perl/scripts-confs/blob/master/yum-repo/epel8.repo) 中！  
    
