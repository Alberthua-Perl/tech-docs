## Ansible 自动化进阶实践（2）- 管理清单

### 文档目录：

- 编写 YAML 清单文件

- 管理清单变量

### 编写 YAML 清单文件

- 清单插件：  
  
  - 从 `Ansible 2.4` 开始，Ansible 支持不同格式的清单文件，该功能由插件实现。
  
  - 插件是增强 Ansible 功能的代码片段。
  
  - 通过插件支持清单后，Ansible 可以仅通过提供新插件来支持生成清单数据的新格式和方法。
  
  - 传统的 `INI` 样式静态清单文件和动态清单脚本各自通过插件实现。
  
  - 默认情况下禁用大多数清单插件。
  
  - 可以通过 ansible.cfg 配置文件的 inventory 部分中的 `enable_plugins` 指令启用具体的插件。
    
    ```bash
    $ less /etc/ansible/ansible.cfg
      ...
      [inventory]
      # enable inventory plugins, default: 'host_list', 'script', 'auto', 'yaml', 'ini', 'toml'
      #enable_plugins = host_list, virtualbox, yaml, constructed
      ...
    
    $ sudo rpm -ql ansible | grep 'plugins/inventory' | egrep 'ini|yaml|script'
      /usr/lib/python3.6/site-packages/ansible/plugins/inventory/ini.py
      /usr/lib/python3.6/site-packages/ansible/plugins/inventory/script.py
      /usr/lib/python3.6/site-packages/ansible/plugins/inventory/yaml.py
    ```
  
  - 若不指定 enable_plugins 指令，则使用默认值。
  
  - 当 Ansible 解析清单来源时，将尝试 enable_plugins 指令中出现的顺序使用每个插件。
  
  > ✅ 动态清单脚本只需成为可执行文件，可使用任何编程语言实现。

- 👉 YAML 静态清单文件：
  
  - 可使用 `YAML` 清单插件以基于 YAML 的语法（而非基于 INI 的语法）编写静态清单文件。
  
  - 该清单插件默认启用。
  
  - 使用 YAML 清单插件的原因：
    
    - 易于用户阅读
    
    - 易于软件解析
    
    - 允许 YAML 用于 playbook、变量文件和清单
  
  - INI 与 YAML 静态清单文件对比：
    
    ```ini
    # ini 静态清单文件
    [lb_servers]
    servera.lab.example.com
    [web_servers]
    serverb.lab.example.com
    serverc.lab.example.com
    [backend_server_pool]
    server[b:f].lab.example.com
    ```
    
    ```yaml
    # yaml 静态清单文件
    lb_servers:
      hosts:
        servera.lab.example.com:
    web_servers:
      hosts:
        serverb.lab.example.com:
        serverc.lab.example.com:
    backend_server_pool:
      hosts:
        server[b:f].lab.example.com:
    ```
    
    - YAML 清单使用块（`block`）来组织相关的配置项。
    
    - 每个块以组的名称开头，后跟冒号（`:`）。
    
    - 组名称下方缩进的所有内容都从属于该组。
    
    - 👉 如果在组名称下缩进，则主机名块将以关键字 `hosts` 开始。
    
    - hosts 下缩进的所有服务器名称都从属于该组，这些服务器本⾝形成自己的组，因此它们必须以冒号（`:`）结尾。
    
    - 👉 可在组块中使用关键字 `children`，属于该组成员的组列表以此关键字开始，这些成员组可以有自己的 hosts 和 children 块。
    
    - 👉 `all` 组隐式存在于顶级，并包含其余的清单作为子级。
    
    - 可以在 YAML 清单文件中明确列出，但这不是必需的。
      
      ```yaml
      all:
        children:
          lb_servers:
            hosts:
              servera.lab.example.com:
          web_servers:
            hosts:
              serverb.lab.example.com:
      ```
  
  - 基于 INI 的静态清单文件中可能包含不属于任何组的主机。
    
    ```ini
    notinagroup.lab.example.com
    # ungrouped 未分组的主机
    [mailserver]
    mail.lab.example.com
    ```
  
  - Ansible 自动将不属于任何组的主机放置在特殊组 `ungrouped` 中。
    
    ```yaml
    all:
      children:
        ungrouped:
          notinagroup.lab.example.com:
        mailserver:
          mail.lab.example.com:
    ```

- 设置清单变量：
  
  - 可以和基于 INI 的清单文件一样，直接在基于 YAML 的清单文件中设置清单变量。
  
  > ✅ 最佳实践：
  > 
  > 1. 在许多场景中，推荐的最佳做法是避免将变量存储于静态清单文件中。
  > 
  > 2. 建议使用静态清单文件仅存储受管主机及它们属组的信息。
  > 
  > 3. 变量及其值存储在清单的 `host_vars` 或 `group_vars` 文件中。
  > 
  > 4. 但在某些情况下，`ansible_port` 或 `ansible_connection` 等变量与清单一同保存在同一文件中。
  > 
  > 5. 如果在过多的不同位置设置变量，将造成变量定义的混乱。
  
  - 组变量定义：
    
    ```
    # ini 静态清单文件组变量定义
    [monitoring]
    watcher.lab.example.com
    [monitoring:vars]
    smtp_relay: smtp.lab.example.com
    ```
    
    ```yaml
    # yaml 静态清单文件组变量定义
    monitoring:
      hosts:
        watcher.lab.example.com:
      vars:
      # vars 关键字定义
        smtp_relay: smtp.lab.example.com
    ```
  
  - 主机变量定义：
    
    ```ini
    # ini 静态清单文件主机变量定义
    [workstations]
    workstation.lab.example.com
    localhost ansible_connection=local
    host.lab.example.com
    ```
    
    ```yaml
    # yaml 静态清单文件主机变量定义
    workstations:
      hosts:
        workstation.lab.example.com:
        localhost:
          ansible_connection: local
        host.lab.example.com:
    ```

- 从 INI 转换到 YAML：
  
  - `ansible-inventory` 命令将基于 INI 的清单转换为 YAML 格式。
  
  - ansible-inventory 命令会解析和测试清单文件的格式，但不会尝试验证清单中的主机名是否确实存在。
  
  > 💥 注意：
  > 
  > 1. 清单文件中的名称不一定是有效的主机名，而由 playbook 应用特定的受管主机。
  > 
  > 2. 受管主机的名称可以有 ansible_host 主机变量，指向 Ansible 在连接受管主机时应使用的真实名称或 IP 地址，清单文件中的名称则可作为 playbook 的简化别名。
  
  - 基于 INI 的清单转换为 YAML 格式示例：
    
    - [原始 INI 静态清单文件](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter02/ini-yaml-inventory)：
      
      ```ini
      [lb_servers]
      servera.lab.example.com
      [web_servers]
      serverb.lab.example.com
      serverc.lab.example.com
      [web_servers:vars]
      alternate_server=serverd.lab.example.com
      # 组变量
      [backend_server_pool]
      server[b:f].lab.example.com
      ```
    
    - 🤘 INI 转换为 YAML 格式静态清单文件命令：
      
      ```bash
      $ ansible-inventory --yaml -i <inventory_name> \
        --list --output <new_inventory_name>.yaml
      # --list 选项：输出所有主机的信息
      # --graph 选项：输出所有主机的树形关系的信息 
      ```
    
    > 💥 `--list`、`--graph`、`--host=HOST` 选项必须使用其中之一，否则报错！
    
    - [转换后的 YAML 静态清单文件](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter02/ini-yaml-inventory-list.yaml)：使用 `--list` 选项
      
      ```yaml
      all:
        children:
          backend_server_pool:
            hosts:
              serverb.lab.example.com:
                alternate_server: serverd.lab.example.com
              serverc.lab.example.com:
                alternate_server: serverd.lab.example.com
              serverd.lab.example.com: {}
              servere.lab.example.com: {}
              serverf.lab.example.com: {}
          lb_servers:
            hosts:
              servera.lab.example.com: {}
          ungrouped: {}
          web_servers:
            hosts:
              serverb.lab.example.com: {}
              serverc.lab.example.com: {}
      ```
      
      若使用 `--graph` 选项，输出的静态清单文件，[如下所示](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter02/ini-yaml-inventory-graph.yaml)：
      
      ```yaml
      @all:
        |--@backend_server_pool:
        |  |--serverb.lab.example.com
        |  |--serverc.lab.example.com
        |  |--serverd.lab.example.com
        |  |--servere.lab.example.com
        |  |--serverf.lab.example.com
        |--@lb_servers:
        |  |--servera.lab.example.com
        |--@ungrouped:
        |--@web_servers:
        |  |--serverb.lab.example.com
        |  |--serverc.lab.example.com
      ```
  
  - 转换非常大的清单文件时，使用 ansible-inventory 命令可以节省大量时间。
  
  - ansible-inventory 命令的 YAML 输出中的部分组或主机行以 `{}` 结尾，这表示组没有任何成员或组变量，或者主机没有主机变量。
  
  - 如果手动编写 YAML 格式，则无需包含 `{}`。
  
  - ✅ 最佳实践：
    
    - 建议 INI 或 YAML 原始静态清单文件中不直接声明清单变量，而从 `host_vars` 和 `group_vars` 中的外部文件获取它们。
    
    - 若原始 INI 格式的清单文件与 host_vars 或 group_vars 变量定义目录位于同一目录中，使用 ansible-inventory 命令转换的 YAML 格式中将带有主机变量或组变量。

- YAML 故障排查提示：
  
  * 保护后面跟着空格的冒号：
    
    - 在未加引号的字符串中，冒号后跟空格将导致错误。
    
    - YAML 会将此解释为在字典中开始一个新元素。
    
    - 使用单引号或双引号圈引字符串即可。
      
      ```yaml
      title: Ansible: Best Practices
      # the second colon produces an error
      fine: Not:a:proble
      # No space after the colon means no special treatment
      simple: 'Quoting the value with the : character solves the problem'
      double: "Double quotes also work with the : but permit escaped characters \n"
      ```
  - 保护进行替换的变量：
    
    - Ansible 通过 `{{ variable }}` 进行变量替换，但 YAML 中以 `{` 开头的内容解释为字典的开头。
    
    - 因此，必须使用双引号括起变量占位符，即：`foo: "{{ variable }} 值的其余部分"`。
    
    - 通常，在使用任何保留字符 [] {} > | * & ! % # ` @ , 时，应在值的两旁使用双引号 ""。
    
    > ✅ Jinja2 模板中的变量可不使用双引号圈引！
  
  - 字符串、布尔值与浮点数的区别：
    
    布尔值与浮点数不加引号，字符串加引号。
    
    ```yaml
    active: yes              # Boolean value
    default_answer: "yes"    # string containing yes
    temperature: 36.5        # Floating point value
    version: "2.0"           # String containing a dot
    ```

### 管理清单变量

- 描述变量的基本原则：
  
  - 设置变量的位置，包括：
    
    - 在角色的 defaults 和 vars 目录中。
    
    - 在清单文件中，作为主机变量或组变量。
    
    - 👉 在 playbook 或清单的 host_vars 或 group_vars 子目录下的变量文件中。
    
    - 在 play、角色或任务中。
  
  - 在项目中定义与管理变量时，遵循的原则：
    
    - 保持简单
    
    - 不要重复
    
    - 在可读的小文件中组织变量
      
      - ✅ 管理一个包含许多主机组和变量的大型项目，可将变量定义拆分成多个文件。
      
      - 为了更加轻松地查找特定变量，可将相关的变量组合到一个文件中，并为该文件指定有意义的名称。
      
      - 可使用子目录，而不使用 host_vars 和 group_vars 中的文件。
      
      - 如，可以拥有一个用于组 webserver 的 `group_vars/webserver` 目录，并且该目录中可以包含名为 firewall.yml 的文件，其中仅包含与防火墙配置相关的变量，该目录还可以包含与服务器配置的其他部分相关的其他组变量文件。

- 变量合并与优先级简介：
  
  - 以多种方式定义相同的变量时，Ansible 将使用优先级规则为变量选取值。
  
  - ✨ 在处理完所有变量定义后，Ansible 将在每个任务开始时为每个主机生成一组合并变量。
  
  - ✨ 当 Ansible 合并变量时，若不同位置上同一变量有两个定义，将使用优先级最⾼的值。
  
  - 🚀 Ansible 变量优先级从高 ⬆ 到低 ⬇ 排列：
    
    - `ansible-playbook` 命令行中的 `-e` 选项定义的 `key=value` 变量
    
    - play 中的变量（优先级从高 ⬆ 到低 ⬇ 排列）：
      
      - `include_tasks` 模块下的 vars 指令定义的变量
        
        ```yaml
        tasks:
          - name: test vars directive under include_tasks module
            include_tasks: /path/to/<task_file>.yml
            vars:
              service: <service_name>
        ```
      
      - `include_role` 模块下的 vars 指令定义的变量
        
        ```yaml
        tasks:
          - name: vars directive under include_role module
            include_role:
              name: apache-web
            vars:
              service: httpd
        ```
  
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/manage-tasks/include_role-define-variable-error.jpg)
  
      - `register` 注册变量或 `set_fact` 模块设置的 `key=value` 变量
      
      - ✨ `include_vars` 模块动态加载的变量
        
        > ✅ 在许多场景中，若不使用该模块可使用 vars_files 指令替换。
      
      - 指定的 task 任务下的 vars 指令定义的变量
        
        ```yaml
        tasks:
          - name: install container-tools {{ package }}
            yum:
              name: "{{ package }}"
              state: present
            vars:
              package: skopeo
        ```
      
      - `block` 部分下的 vars 指令定义的变量
        
        ```yaml
        tasks:
          - block:
              - name: install {{ package }} rpm package
                yum:
                  name: "{{ package }}"
                  state: present
              - name: start {{ service }} service
                service:
                  name: "{{ service }}"
                  state: started
                  enabled: yes
            vars:
              package: nginx
              service: nginx
        ```
      
      - role 指令调用的角色下直接定义的 `key=value` 变量
        
        ```yaml
        roles:
          - role: lb-haproxy
            lbmethod: roundrobin
            # 以上的变量将覆盖角色 lb-haproxy/vars/ 子目录中存在的 lbmethod
            # 同名变量
        ```
      
      - role 指令调用的角色下 vars 指令定义的变量
        
        ```yaml
        roles:
          - role: lb-haproxy
            vars:
              lbmethod: roundrobin
            # 以上的变量将覆盖角色 lb-haproxy/vars/ 子目录中存在的 lbmethod
            # 同名变量
        ```
      
      - `<role_name>/vars/` 子目录中定义的变量
      
      - play 中 `vars_files` 指令使用外部文件中定义的变量
        
        > ✅ 推荐做法：
        > 
        > vars_files 指令非常有用，可按功能将不是特定于主机或组的大型变量列表组织到单独的文件中。
        > 
        > 还可将敏感的变量放入由 `Ansible Vault` 加密的文件中，与不敏感且不需要加密的变量隔开。
        
        ```yaml
        ---
        - name: test vars_files directive under play level
          hosts: localhost
          vars_files:
            - secfile1
        ```
      
      - play 中 vars 指令定义的变量
        
        ```yaml
        ---
        - name: test vars directive under play level
          hosts: localhost
          vars:
            username: tommy
        ```
    
    - 主机变量、组变量（优先级从高 ⬆ 到低 ⬇ 排列）：
      
      - 主机的事实变量（facts）与已缓存的事实变量
      
      - 👉 playbook 同级目录中的 `host_vars` 子目录中设置的主机变量
      
      - inventory 目录中 `host_vars` 子目录中设置的主机变量
      
      - 直接在 inventory 静态清单文件中或通过动态清单脚本设置的主机变量
      
      - 👉 playbook 同级目录中的 `group_vars` 子目录中为其他组设置的组变量
      
      - inventory 目录中 `group_vars` 子目录中为其他组设置的组变量
      
      - 👉 playbook 同级目录中的 `group_vars/all` 文件或子目录中为 all 设置的组变量
      
      - inventory 目录中 `group_vars/all` 文件或子目录中为 all 设置的组变量
      
      - 直接在 inventory 静态清单文件中或通过动态清单脚本设置的组变量
      
      - 优先级示例：
        
        ```bash
        .
        ├── ansible.cfg
        ├── group_vars/
        │   └── all
        ├── inventory/
        │   ├── group_vars/
        │   │   └── all
        │   ├── phoenix-dc
        │   └── singapore-dc
        └── playbook.yml
        # 注意：
        #   1. 与 playbook 位于同一目录中的 group_vars/all 文件定义的组变量
        #      优先级高于 inventory 目录中的 group_vars/all 文件定义的组变量。
        #   2. 因此，前者将覆盖后者组变量。
        ```
    
    - `<role_name>/defaults/` 子目录中定义的变量

- 从清单中分离变量：
  
  - 可在清单文件中定义主机变量与组变量，但这不是最佳实践。
  
  - 随着环境与种类不断扩展，清单文件也将逐渐扩大而变得难以维护。
  
  - 若从静态清单文件迁移到动态清单脚本，以独立的静态文件的方式管理主机变量与组变量，更加方便管理。
  
  - 组变量定义方式 1：
    
    ```bash
    [user@demo project]$ tree -F group_vars
    group_vars/
    ├── db_servers.yml
    ├── lb_servers.yml
    └── web_servers.yml
    # 推荐将变量定义从清单文件转移到单独的变量文件中，每个主机组分别对应一个。
    # 每个变量文件都以主机组命名，且包含该主机组的变量定义。
    ```
  
  - ✅ 推荐做法：组变量定义方式 2
    
    ```bash
    [user@demo project3]$ tree -F group_vars
    group_vars/
    ├── all/
    │   └── common.yml
    ├── db_servers/
    │   ├── mysql.yml
    │   └── firewall.yml
    ├── lb_servers/
    │   ├── firewall.yml
    │   ├── haproxy.yml
    │   └── ssl.yml
    └── web_servers/
        ├── firewall.yml
        ├── webapp.yml
        └── apache.yml 
    # 对于多样化的大型环境而言，推荐在 group_vars 目录下为每个主机组创建子目录。
    # Ansible 将分析这些子目录中的任何 YAML 文件，并根据父目录将变量与主机组关联。
    # 创建的子目录中的文件可根据不同功能来命名变量文件名。
    ```
  
  - 通过将清单与主机变量及组变量分离使得 Ansible 项目更加易于阅读与维护。

- 特殊清单变量（`inventory variable`）：
  
  - 可使用多个变量来更改 Ansible 连接到清单中列出的主机的方式。
  
  - `ansible_connection`：
    
    - 用于访问受管主机的连接插件。
    
    - 默认情况下，ssh 用于除 localhost 外的所有主机，后者使用 local。
  
  - ✨ `ansible_host`：
    
    - 实际 IP 地址或完全限定域名（FQDN），该变量定义真实的 IP 地址或主机名。
    
    - 在连接受管主机时使用，⽽不使用来自清单文件（`inventory_hostname`）中的名称。
    
    - 清单文件中的名称为 playbook 执行过程中显示的主机别名。
    
    - 默认情况下，此变量与清单主机名相同。
  
  - ✨ `ansible_port`：
    
    - Ansible 用于连接受管主机的端口。
    
    - 对于（默认）SSH 连接插件，默认为 `22` 端口。
  
  - `ansible_user`：
    
    - Ansible 以此用户身份连接受管主机。
    
    - 作为 Ansible 的默认行为，使用与控制节点上运行 Ansible Playbook 的用户相同的用户名来连接受管主机。
  
  - `ansible_become_user`：
    
    - Ansible 连接到受管主机后，使用 ansible_become_method（默认情况下为 sudo）切换到此用户。
    
    - 可能需要以某种方式提供身份验证凭据。
  
  - ✨ `ansible_python_interpreter`：
    
    - Ansible 应在受管主机上使用的 Python 可执行文件的路径。
    
    - 在 Ansible 2.8 及更⾼版本上，此选项默认为 `auto`，这时将根据运行的操作系统，在运行 playbook 的主机上自动选择 Python 解释器。
    
    - 与较旧版本的 Ansible 相比，不那么必要使用此设置。

- 使用变量识别当前主机：
  
  - play 运行时，可以使用多个变量和事实来识别执行某一任务的当前受管主机的名称。
  
  - ✨ `inventory_hostname`：当前正在处理的受管主机的名称，从清单中获取。
  
  - ✨ `ansible_host`：用于连接受管主机的实际 IP 地址或主机名。
  
  - `ansible_facts['hostname']`：事实变量，从受管主机收集的短格式主机名。
  
  - `ansible_facts['fqdn']`：事实变量，从受管主机收集的完全限定域名（FQDN）。
  
  - `ansible_play_hosts`：当前 play 期间尚未失败的所有主机的列表（因此将⽤于 play 中剩余的任务）。

- 用户可读的清单主机名：
  
  - 当 Ansible 在远程主机上执行任务时，输出中会包含清单主机名（inventory_hostname）。
  
  - 由于可使用上述特殊清单变量（ansible_host）来指定替代连接属性，因此可以为清单主机名分配任意名称。
  
  - 使用有意义的名称分配清单主机时，可更好地理解 playbook 输出并诊断 playbook 错误。
  
  - 若使用以下的清单文件，根据 playbook 运行时输出的主机名无法识别该主机的具体的作用。
    
    ```yaml
    web_servers:
      hosts:
        server100.example.com:
        server101.example.com:
        server102.example.com:
    lb_servers:
      hosts:
        server103.example.com:
    ```
    
    ```bash
    [user@demo project]$ ansible-playbook site.yml
    ...output omitted...
    PLAY RECAP *******************************************************************
    server100.example.com  : ok=4    changed=0    unreachable=0    failed=0  ...
    server101.example.com  : ok=4    changed=0    unreachable=0    failed=0  ...
    server102.example.com  : ok=4    changed=0    unreachable=0    failed=0  ...
    server103.example.com  : ok=4    changed=0    unreachable=0    failed=1  ... 
    ```
  
  - ✅ 推荐做法：定义 YAML 静态清单文件
    
    - 通过定义有意义的主机名，可轻松排除 playbook 运行中出现的问题。
    
    - 在很多情况下，可能想要在 playbook 中使用任意主机名，这些名称通过 ansible_host 映射到实际的 IP 地址或主机名。
      
      ```yaml
      web_servers:
        hosts:
          webserver_1:
          # 清单文件中的主机名（playbook 结果返回中输出）
          # 显示该主机名将更好地显示主机的具体用途
            ansible_host: server100.example.com
            # ansible_host 特殊清单变量定义的真实主机名
          webserver_2:
            ansible_host: server101.example.com
          webserver_3:
            ansible_host: server102.example.com
      lb_servers:
        hosts:
          loadbalancer:
            ansible_host: server103.example.com
      ```
      
      ```bash
      [user@demo project]$ ansible-playbook site.yml
      ...output omitted...
      PLAY RECAP *******************************************************************
      loadbalancer    : ok=4    changed=0    unreachable=0    failed=1  ...
      webserver_1     : ok=4    changed=0    unreachable=0    failed=0  ...
      webserver_2     : ok=4    changed=0    unreachable=0    failed=0  ...
      webserver_3     : ok=4    changed=0    unreachable=0    failed=0  ... 
      # YAML 静态清单文件中定义的主机名
      ```
    
    - 使用场景如下所示：
      
      - 可能希望 Ansible 使用与 DNS 中解析结果不同的特定 IP 地址来连接到该主机。如，可能有不公开的特定管理地址，或者主机在 DNS 中可能有多个地址，但其中一个与控制节点位于相同的网络中（一个主机名对应多个 IP 地址），因此，在 inventory 中指定自定义的主机名，而 ansible_host 定义真实的可被解析的 IP 地址。
      
      - 可能在调配具有任意名称的云系统，但希望在 playbook 中使用依据其所扮演角色的有意义名称来应用这些系统。
      
      - 可能会在 playbook 中以短名称引用主机，但需要在清单中通过长格式完全限定域名引用它，才能正确地连接。
