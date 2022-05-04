## 第三章 管理任务执行

### 章节：

- 控制特权升级

- 控制任务执行

- 运行选定的任务

- 优化执行速度

### 第一节 控制特权升级

- 特权升级策略：  
  
  - Ansible Playbook 可以在许多不同的级别上实现特权升级。
  
  - 对于角色、play、块和任务，可使用以下特权升级指令：
    
    become、become_user、become_method、become_flags

- 通过配置特权升级：
  
  - 若将 Ansible 配置文件的 `privilege_escalation` 部分中的 become 布尔值设为 yes （或true），则 playbook 中的所有 play 都将默认使用特权升级。
  
  - 在受管主机上运行时，这些 play 将使用当前的 become_method 来切换到当前的 become_user。
  
  - 在使用命令行选项启动 playbook 时，可覆盖配置文件并指定特权升级设置。
  
  - 配置指令和命令行选项：
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\CICD\Ansible高级自动化最佳实践\pictures\Chapter03\privileged-escalation-directives-options.jpg)
  
  > 若 Ansible 配置文件指定 `become: false`，但命令行中包含 `-b` 选项，则 Ansible 将忽略配置文件，并且默认使用特权升级。

- Play 中的特权升级：
  
  - 编写 playbook 时，可能需要对某些 play 而不是所有 play 升级特权。
  
  - 可以明确指定各个 play 是否应使用特权升级。
  
  - 👉 若 play 不指定是否使用特权升级，则使用配置文件或命令行中的默认设置。
  
  - 👉 若 play 中明确指定 become 升级特权，将忽略配置文件或命令行中的 become 特权升级。
  
  - 根据所涉及的 play 或主机，可能需要在配置设置或清单变量中指定升级方法或特权用户。
    
    ```yaml
    ---
    - name: Become the user "manager"
      hosts: webservers
      become: true
      tasks:
        - name: Show the user used by this play
          debug:
            var: ansible_user_id
    - name: Do not use privilege escalation
      hosts: webservers
      become: false
      tasks:
        - name: Show the user used by this play
          debug:
            var: ansible_user_id
    - name: Use privilege escalation based on defaults
      hosts: webservers
      tasks:
        - name: Show the user used by this play
          debug:
            var: ansible_user_id
    ```

- 任务（task）中的特权升级：
  
  可为 play 中的一个任务打开（或关闭）特权升级，可将 become 指令添加到任务中。
  
  ```yaml
  ---
  - name: Play with two tasks, one uses privilege escalation
    hosts: all
    become: false
    tasks:
      - name: This task needs privileges
        yum:
          name: perl
          state: installed
        become: true
      - name: This task does not need privileges
        shell: perl -v
        register: perlcheck
        failed_when: perlcheck.rc != 0
        # 若 Perl 安装成功可查看版本信息，而安装失败，该命令无法正确执行，failed_when 语句
        # 将标记该任务为 failed 状态。
  ```

- 块（block）中的特权升级：
  
  - 若 play 中有一部分任务需要（或不需要）特权升级，可在 `block` 上设置 become。
  
  - 👉 block 中的所有任务都共享相同的特权升级，而且此设置将覆盖 play 级别上的设置。
  
  - 与 `become_user` 搭配，使用特权升级来以应用所使用的某个普通用户，而不是以 root 执行一部分任务。
    
    ```yaml
    ---
    - name: Deploy web services
      hosts: webservers
      become: false
      tasks:
        - block:
          - name: Ensure httpd is installed
            yum:
              name: httpd
              state: installed
          - name: Start and enable webserver
            service:
              name: httpd
              state: started
              enabled: yes
          become: true
        - name: Test website from itself, do not become
          uri:
            url: http://{{ ansible_host }}
            return_content: yes
          register: webpage
          failed_when: webpage.status != 200
          # 若返回码不为 200 时，将该任务设置为 failed。
    ```

- 角色（role）中的特权升级：
  
  - 方法 1：
    
    - 针对角色本⾝，在其内部或者针对其任务设置特权升级变量。
    
    - 角色文档可能会指定是否必须设置其他变量（如 become_method），才能使用该角色。
  
  - 方法 2：
    
    - 在 Ansible 配置或 playbook 中自行指定此信息。
    
    - 另外，可以对调用该角色的 play 设置特权升级设置。
      
      ```yaml
      - name: Example play with one role
        hosts: localhost
        roles:
          - role: role-name
            become: true
      ```

- 💥 使用连接变量（connection variables）进行特权升级：
  
  - 连接变量可作为清单变量应用到组或各个主机上。
  
  - playbook 和配置指令与连接变量名称的比较：
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\CICD\Ansible高级自动化最佳实践\pictures\Chapter03\connection-variables.jpg)
  
  - 👉 连接变量的特权升级优先级⾼于配置和 playbook 指令，将覆盖配置文件、play、task、block 和 role 中的 become 设置。
  
  > ✅ 注意：使用配置或 playbook 指令通常对使用特权升级提供更多灵活性和控制力！
  
  - 总结：最小范围中指定的特权升级具有最高优先级
  
  - 示例：
    
    - 主机组的 YAML 清单中使用特权升级变量：
      
      ```yaml
      webservers:
        hosts:
          servera.lab.example.com:
          serverb.lab.example.com:
        vars:
          ansible_become: true
      ```
    
    - 主机级别上使用连接变量配置特权升级：
      
      ```yaml
      webservers:
        hosts:
          servera.lab.example.com:
            ansible_become: true
          serverb.lab.example.com:
      ```
    
    - 在 playbook 中设置连接变量（对 play 本身进行设置）：
      
      该方法将覆盖清单变量（通过正常的变量优先顺序），以及任何 become 指令的设置。
      
      ```yaml
      ---
      - name: Example play using connection variables
        hosts: webservers
        vars:
          ansible_become: true
        tasks:
          - name: Play will use privilege escalation even if inventory says no
            yum:
              name: perl
              state: installed
      ```

- 选择特权升级的方法：
  
  - 选择控制特权升级时考虑的因素：
    
    - 使 playbook 保持简单
    
    - 要尽可能运行具有最低特权的任务，以避免由于 playbook 错误而意外损害受管主机。
  
  - 👉 在运行 playbook 时始终开启特权升级，虽然简单且在许多情况下，执行的任务必须以 root 用户运行，但不需要以 root 用户运行的任务仍使用升级的特权运行，可能增加风险。
    
    - 因此，良好的做法是有选择地控制需要特权升级的 play 或任务。
    
    - 如，若 `apache` 用户可以启动 httpd 服务器，则无需以 root 用户运行该任务。
  
  - 应以尽可能简单的方式配置特权升级，并且明确是否用于某个任务。
    
    - 如，可对 play 使用 become: true 来开启特权升级，然后对任务使用 become: false 有选择地禁用不需要特权升级的任务。
    
    - 或者，如果与工作流（workflow）兼容，可以将需要特权升级的任务分组到一个 play 中，并将无此需求的任务分组到另一个 play 中。
  
  - 若 playbook 需要特权升级，但出于某种原因无法编辑，则可能需要在配置文件中设置。
  
  - 👉 最大的挑战在于，使用 playbook 的不同主机需要不同的特权升级方法才能正常工作。
  
  - 👉 这种情况下，可为这些主机或其所在的组设置 `ansible_become_method` 等清单变量，同时在 playbook 中通过 become 来指定是否使用特权升级。

### 第二节 控制任务执行

> 该小节涉及的 Ansible Playbook 的 demo 片段可参考以下链接：
> 
> [include_import_role.yml](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/include_import_role.yml)、[listen_handlers.yml](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/listen_handlers.yml)

- 控制执行顺序：
  
  - 若运行具有角色的 play 时，始终通过 roles 调用角色，再执行 tasks 下的任务。
  
  - 最好将 tasks 部分放在 roles 部分后编写，使得 playbook 的顺序与执行顺序相符。
    
    ```yaml
    ---
    - name: Ensure Apache is deployed
      hosts: www1.example.com
      gather_facts: no
      tasks:
        - name: Open the firewall
          firewalld:
            service: http
            permanent: yes
            state: enabled
      roles:
        - role: apache
      # 无论 tasks 定义在 roles 的前后，roles 部分始终最先运行。
    ```
  
  - 顶级指令的（name、hosts、tasks 和 roles 等）的顺序实际上是任意的，但 Ansible 在解析并运行 play 时按照标准的顺序来处理它们。
  
  - ✅ 最佳实践：从 play 名称开始以统一的顺序编写 play

- （静态）导入或（动态）包含作为任务的角色：
  
  - Ansible 的最新版本允许将包含（include）或导入（import）作为任务的角色，而不使用
    
    play 的 roles 部分。
  
  - 优点：可以轻松地运行一组任务，导入或包含角色，然后运行更多任务。
  
  - 潜在的缺点：若不仔细检查 playbook 所用的角色，可能会不太清楚。
  
  - `import_role` 模块：静态导入角色
  
  - `include_role` 模块：动态包含角色
    
    ```yaml
    ---
    - name: Executing a role as a task
      hosts: remote.example.com
      tasks:
        - name: A normal task
          debug:
            msg: 'first task'
        - name: A task to include role2 here
          include_role:
            name: role2
          # 可以不再使用 roles 指令调用
        - name: Another normal task
          debug:
            msg: 'second task'
    ```
  
  - 以上模块的执行方式：
    
    - 使用 import_role 模块时，ansible-playbook 命令首先解析角色并插入到 play 中，然后开始执行，Ansible 会立即检测和报告语法错误，不会开始执行 playbook。
    
    - 使用 include_role 模块时，Ansible 会在 play 执行期间到达 include_role 任务时解析角色并插入到 play 中，若 Ansible 检测到角色中存在语法错误，则中止执行 playbook。
  
  - 使用 include_role 或 import_role 任务时，若 when 指令中的条件为 false，则 Ansible 不解析角色（直接 `skipping`）。
    
    ```yaml
    ---
    - name: use motd role playbook
      hosts: serverb.lab.example.com
      # gather_facts: yes
    
      tasks:
        - name: verify managed host cpu architecture
          debug:
            msg: Arch of managed host is {{ ansible_facts['architecture'] }}
    
        - name: test include_role module with when statement
          include_role:
          # import_role:
            name: motd
          when: ansible_facts['architecture'] == "amd"
    ```
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\CICD\Ansible高级自动化最佳实践\pictures\Chapter03\import_roles-include_roles.jpg)
  
  > 与 import_tasks 及 include_tasks 指令比较。

- 定义 Pre 与 Post 任务：
  
  - `pre_tasks`：在 roles 部分前运行的 tasks 部分
  
  - `post_tasks`：在 tasks 部分以及 tasks 所通知的任何处理程序后运行的 tasks 部分
    
    > 👉 一个 play 中通常不会同时包含所有这些部分。
    
    ```yaml
    ---
    - name: Deploying New Application Version
      hosts: webservers
      pre_tasks:
        # Stop monitoring the web server to avoid sending false alarms
        # while the service is updating.
        - name: Disabling Nagios for this host
          nagios:
            action: disable_alerts
            host: "{{ inventory_hostname }}"
            services: http
          delegate_to: nagios-srv
      roles:
        - role: deploy-content
      tasks:
        - name: Restarting memcached
          service:
            name: memcached
            status: restarted
          notify: Notifying the support team
        # Confirm that the application is fully operational
        # after the update.
        - name: Validating the new deployment
          uri:
            url: "http://{{ inventory_hostname }}/healthz"
            return_content: yes
          register: result
          failed_when: "'OK' not in result.content"
          # 若满足该判断条件，将任务标记为 failed 状态，中止 playbook 的运行。
      post_tasks:
        - name: Enabling Nagios for this host
          nagios:
            action: enable_alerts
            host: "{{ inventory_hostname }}"
            services: http
          delegate_to: nagios-srv
      handlers:
        # Send a message to the support team through Slack
        # every time the memcached service is restarted.
        - name: Notifying the support team
          slack:
            token: G922VJP25/D923DW937/3Ffe373sfhRE6y52Fg3rvf5GlK
            msg: 'Memcached on {{ inventory_hostname }} restarted'
          delegate_to: localhost
          become: false
    ```

- 查看执行顺序：
  
  - Ansible 按照以下顺序运行 play 的不同部分：
    
    - pre_tasks
    
    - pre_tasks 部分中通知的处理程序（若定义的话）
    
    - roles
    
    - tasks
    
    - roles 和 tasks 部分中通知的处理程序（若定义的话）
    
    - post_tasks
    
    - post_tasks 部分中通知的处理程序（若定义的话）
  
  - 这些部分在 play 中的顺序不会修改以上列出的执行顺序。
  
  - 👉 为了便于阅读，按照执行的顺序组织 play：
    
    `pre_tasks`、`roles`、`tasks`、`post_tasks`、`handlers`（通常位于末尾）
  
  - Ansible 会运行期间执行和清空通知的处理程序，即 pre_tasks 部分之后、role 和 tasks 部分之后，以及 post_tasks 部分之后。
  
  - 如果在多个部分中获得通知，处理程序可以在 play 执行期间的不同时间运行多次。
  
  - `meta` 模块及 `flush_handlers` 参数：
    
    立即运行由 play 中特定任务通知的任何处理程序，可定义任务执行期间执行所有通知的处理程序的具体时间点。
  
  - 示例：
    
    - 若 Restart api server 处理程序得到通知，那么 play 会在部署新配置文件之后、使用应用 API 之前执行该处理程序。
    
    - 💥 若不调用 meta 模块，play 仅在运行所有任务后调用该处理程序。
    
    - 在使用 API 执行最终任务前没有运行该处理程序，那么该任务可能会失败，因为配置文件已经更新，但应用没有重新读取新的配置。
      
      ```yaml
      ---
      - name: Updating the application configuration and cache
        hosts: app_servers
        tasks:
          - name: Deploying the configuration file
            template:
              src: api-server.cfg.j2
              dest: /etc/api-server.cfg
            notify: Restart api server
          - name: Running all notified handlers
            meta: flush_handlers
            # 先刷新执行 handlers 处理程序，再执行下方的 tasks 任务！
          - name: Asking the API server to rebuild its internal cache
            uri:
              url: "https://{{ inventory_hostname }}/rest/api/2/cache/"
              method: POST
              force_basic_auth: yes
              user: admin
              password: redhat
              body_format: json
              body:
                type: data
                delay: 0
              status_code: 201
            # 使用 uri 模块模拟 curl 命令发送命令行数据
        handlers:
          - name: Restart api server
            service:
              name: api-server
              state: restarted
              enabled: yes
      ```
    
    - play 中的处理程序具有全局作用域，play 可以通知角色中定义的处理程序。
    
    - 👉 一个角色可以通知由另一个角色或 play 定义的处理程序。
    
    - Ansible 始终按照 play 的 handlers 部分中列出的顺序，而不是按照通知的顺序运行已通知的处理程序。

- 侦听处理程序：
  
  - 除任务通知外，处理程序也可以 "订阅（`subcribe`）" 特定的通知，在触发该通知时运行。
  
  - 👉 这允许一个 notify 通知触发多个 handler 处理程序。
  
  - 默认情况下，处理程序会在通知字符串与处理程序名称匹配时执行。
  
  - `listen` 指令：
    
    - 每个处理程序必须具有唯一名称
    
    - 使多个不同名称的处理程序（handler）都订阅相同的通知名称（notify），订阅通知触发运行。
  
  - 示例：
    
    ```yaml
    ---
    - name: Testing the listen directive
      hosts: localhost
      gather_facts: no
      become: false
    
      tasks:
        - debug:
            msg: Trigerring the handlers
          notify: My handlers
          changed_when: true
    
      handlers:
        # Listening to the "My handlers" event
        - name: Listening to a notification
          debug:
            msg: First handler was notified
          listen: My handlers
    
        # As an example, this handler is also triggered because
        # its name matches the notification, but no two handlers
        # can have the same name.
        - name: My handlers
          debug:
            msg: Second handler was notified
    
        - name: Subcribe a notification
          debug:
            msg: Third handler was notified
          listen: My handlers
    ```
    
    由于设置了 `changed_when: true` 语句，任务会在其变化时（即每当它运行时）通知 My handlers，My handlers 任务处理程序或者在 `listen` 指令中列出 My handlers 的处理程序会得到通知。
  
  - 👉 在角色配合使用时，`listen` 指令特别有用。
  
  - 👉 其他角色或 play 或可使用此通知来运行在角色外定义的其他处理程序。
  
  - 示例：
    
    - 当服务需要重启时，角色可以通知其中一个处理程序，在 playbook 中，可以定义一个处理程序，以侦听该通知并在 Ansible 重新启动服务时执行其他任务，如发送消息到监控工具或重新启动依赖服务。
    
    - 可以创建一个角色来检查给定 SSL 证书的有效性并通知处理程序，以便在过期后续订证书，在 playbook 中，可以调用此角色来检查 Apache HTTP 服务器证书的有效性，并创建一个处理程序，以便在角色续订证书时侦听该通知，以重新启动 httpd 服务。

- 处理程序通知方法：
  
  - 👉 任务（task）可以至少通过两种方式通知多个处理程序（handler）：
    
    - 通过 name 逐个通知一系列处理程序
    
    - 通知多个处理程序要配置为 listen 的一个通知名称
  
  - 方法 1：
    
    - 必须查找和编辑每个受影响的任务，以便将处理程序添加到列表中。
    
    - 确保这些处理程序按照正确的顺序列在 handlers 部分中。
  
  - ✅ 方法 2：
    
    - 当任务需要通知大量处理程序时，这种方法非常有用。
    
    - 任务仅发送一个通知，而不按名称通知每个处理程序（否则任务的 notify 语句中将指定大量的 handler 名称列表项）。
    
    - 通过这种方式，可以添加或删除处理程序，而无需更新任务。

- 控制主机执行顺序：
  
  - Ansible 将根据 play 的 hosts 指令确定要为 play 管理的主机。
  
  - 默认情况下，Ansible `2.4` 及以上版本按照清单中列出的顺序为主机运行 play。
  
  - `order` 指令：
    
    - 功能：更改 inventory 中执行 play 的主机顺序
    
    - 对应的值：
      
      - `inventory`：清单顺序（默认值）
      
      - `reverse_inventory`：清单的反序
      
      - `sorted`：数字排在字母之前，按字母顺序排列主机。
      
      - `reverse_sorted`：按字母倒序排列主机
      
      - `shuffle`：每次运行 play 时随机设定顺序
  
  - 示例：
    
    ```yaml
    ---
    - name: Testing host order
      hosts: web_servers
      order: sorted
      tasks:
        - name: Creating a test file in /tmp
          copy:
            content: 'This is a sample file'
            dest: /tmp/test.out
    ```
    
    如上所示，playbook 在运行任务前，按字母顺序对 web_servers 组中的主机进行排序。
  
  > 💥 注意：
  > 
  > 1. 由于 Ansible 通常会在多个主机上并行运行每一个任务，ansible-playbook 命令的输出可能不会反映预期的顺序（输出中显示任务完成顺序，而不是执行顺序）。
  > 
  > 2. 若清单配置为按字母顺序列出主机，而 play 按清单顺序在受管主机上运行（默认行为）。

### 第三节 运行选定的任务

> 该小节涉及的 Ansible Playbook 的 demo 片段可参考以下链接：
> 
> [block_tags.yml](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/block_tags.yml)、[use_tags.yml](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/use_tags.yml)

- 标记 Ansible 资源：
  
  - 在处理大型或复杂的 playbook 时，可能希望仅运行一部分 play 或任务。
  
  - 标记（tag）是 Ansible 资源上的文本标签，如 play 或任务。
  
  - 若要标记资源，对该资源使用 `tags` 关键字，再加上要应用的标记列表。
  
  - 标记可应用到想要跳过或运行的特定资源。
  
  - ansible-playbook 运行 playbook 时，使用 `--tags` 选项来筛选 playbook 并且仅运行带有特定标记的 play 或任务。

- 标记可用于下列资源：
  
  - 标记每个任务（task）：
    
    > 👉 最常见的方式之一
    
    ```yaml
    ---
    - name: Install application
      hosts: dbservers
      vars:
        packages:
          - postfix
          - mariadb-server
      tasks:
        - name: Ensure that packages are installed
          yum:
            name: "{{ packages }}"
            state: installed
          tags:
            - install
          # 为 task 标记
    ```
  
  - 标记整个 play：
    
    > 👉 在 play 级别使用 tags 指令
    
    ```yaml
    ---
    - name: Setup web services
      hosts: webservers
      tags:
        - setup
      # 为整个 play 标记
      tasks:
      - name: Install http daemon
        yum:
          name: httpd
          state: present
    ```
  
  - 当 playbook 中包含（include）任务文件时，该任务可以添加标记，允许管理员为 `include_tasks` 加载的任务设置全局标记：
    
    > 👉 为 `roles` 或 `include_tasks` 语句添加标记时，该标记将全局标记角色或包含任务中的所有任务。
    
    ```yaml
    tasks:
       - name: Include common web services tasks
         include_tasks: common.yml
         tags:
           - webproxy
           - webserver
    ```
  
  - 在 roles 部分中标记角色：
    
    - 角色中的所有任务都与此标记关联
    
    - 示例：角色 databases 具有两个标记，即 production 和 staging。
      
      ```yaml
      roles:
        - { role: databases, tags: ['production', 'staging'] }
      ```
      
      ```yaml
      roles:
        - role: databases
          tags:
            - production
            - staging
      ```
  
  - 标记 tasks 的 `block`：
    
    - 块中的所有任务都与此标记关联
    
    - 示例：将所有 httpd 相关任务组合到 webserver 标记下
      
      ```yaml
      ---
      - name: Setup httpd service
        hosts: all
        order: reverse_sorted
      
        tasks:
          - name: Notify start of process
            debug:
              msg: Beginning httpd setup
      
          - block:
              - name: Ensure httpd packages are installed
                yum:
                  name:
                    - httpd
                    - php
                    - git
                    - php-mysqlnd
                  state: present
      
              - name: Ensure SELinux allows httpd connection to a remote database
                seboolean:
                  name: httpd_can_network_connect_db
                  state: true
                  persistent: true
      
              - name: Ensure httpd service is started and enabled
                service:
                  name: httpd
                  state: started
                  enabled: true
            tags:
              - webserver
            # tags 关键字层级与 block 层级相同
      ```

- 管理标记的资源：
  
  - 使用 ansible-playbook 命令的 `--tags` 选项运行带有特定标记的任务，或者使用
    
    `--skip-tags` 选项跳过带有特定标记的任务。
  
  - 以逗号分隔列表的形式使用 --tags 指定多个标记。
  
  - 示例：
    
    ```yaml
    ---
    - name: Example play using tagging
      hosts:
        - servera.lab.example.com
        - serverb.lab.example.com
        # 列表形式的主机组
      tasks:
        - name: httpd is installed
          yum:
            name: httpd
            state: latest
          tags: 
            - webserver
    
        - name: postfix is installed
          yum:
            name: postfix
            state: latest
    
    
    ```

    ```
    
    ```
    
    ```
    
    ```
    
    ```
    
    ```
    
    ```
    
    ```bash
    $ ansible-playbook --list-tags use_tags.yml 
    
    playbook: use_tags.yml
    
      play #1 (servera.lab.example.com,serverb.lab.example.com): Example play using tagging TAGS: []
          TASK TAGS: [webserver]
    # 查看 playbook 中已定义的 tag 标签
    ```

- 特殊标记：
  
  - `always` 标记：
    
    - 即使使用 --tags 选项，带有 always 标记的资源始终都会运行。
    
    - 可使用 `--skip-tags always` 选项显式跳过。
  
  - `never` 标记：
    
    - never 特殊标记的任务不会运行
    
    - 可使用 `--tags never` 或与其他任务关联的任何其他标记之一
  
  - `tagged` 标记：
    
    - 将运行任何带有显式标记的资源
  
  - `untagged` 标记：
    
    - 将运行任何不带有显式标记的资源，并且排除所有带标记的资源。
  
  - `all` 标记：
    
    - 将运行 play 中的所有任务，不论它们是否带有标记（Ansible 的默认行为）。

- ansible-playbook 相关命令：
  
  ```bash
  $ ansible-playbook --step <playbook>.yml
  # 进入交互式执行过程，逐步执行 ansible 任务，可用于新 playbook 的部署。    
  
  $ ansible-playbook <playbook>.yml --tags '<tag_name>'
  # 只执行 playbook 中标记为 tag_name 的任务，跳过其他任务。
  
  $ ansible-playbook <playbook>.yml --skip-tags '<tag_name>'
  # 执行除标记为 tag_name 外的其他任务（跳过 tag_name 任务），包括未标记 tag 的任务
  # 与标记为其他 tag 的任务。
  
  $ ansible-playbook <playbook>.yml --tags tagged
  # 执行全部标记 tag 的任务
  
  $ ansible-playbook <playbook>.yml --tags untagged
  # 执行全部未标记 tag 的任务  
  
  $ ansible-playbook --ask-vault-pass <playbook>.yml
  # 交互式运行使用 ansible 加密的 playbook，提示输入密码。
  ```

### 第四节 优化执行速度

- ✅ 优化 Ansible playbook 执行速度可参考的方式：
  
  - 优化基础架构
  
  - 禁用事实收集
  
  - 增加并行度（parallelism）
  
  - 使用软件包管理器模块避免循环
  
  - 高效复制文件到受管主机
  
  - 使用 Jinja2 模板（template）
  
  - 优化 SSH 连接
  
  - 启用流水线（pipelining）

- 优化基础架构：
  
  - 红帽 Ansible 引擎的每一个发行版都会添加功能增强和改进。
  
  - 运行最新版本的 Ansible 可帮助提高 playbook 的运行速度，特别是 Ansible 附带的模块会不时得到优化。
  
  - 👉 可进行的架构优化：
    
    - 从网络⻆度使控制节点 "靠近" 受管节点
    
    - Ansible 在很大程度上依赖于网络通信和数据传输
    
    - 若控制节点与受管主机之间存在高延迟连接或低带宽，会使 playbook 执行时间降级。

- 禁用事实收集：
  
  - 每个 play 都有一个最先运行的隐藏任务，它使用 `setup` 模块从各个主机收集事实。
  
  - 事实提供了有关 play 可通过 `ansible_facts` 变量使用的节点信息。
  
  - 收集每个远程主机的事实需要花费时间。
  
  - `gather_facts` 指令设为 false（或 no）来跳过 play 中的事实收集。
  
  > ✅ 可使用 `Ansible timer` 回调插件以获取 playbook 运行的时间。
  
  - 可作为当前处理的主机的事实变量（任意一种）：
    
    - ansible_facts['hostname']
    
    - ansible_hostname
    
    - ansible_facts['nodename']
    
    - ansible_nodename
  
  - 可将以上事实变量替换为 `inventory_hostname`、`inventory_hostname_short` 魔法变量。
  
  - 可以选择通过在 play 中将 `setup` 模块作为任务在选定主机上运行来手动收集事实，这些事实可供 playbook 中的后续 play 使用。

- 增加并行度（parallelism）：
  
  - 当 Ansible 运行 play 时，将在当前批处理中的每一主机上运行第一项 task，然后在当前批处理中的每个主机上运行第二个 task，依此类推，直到该 play 完成。
  
  - `forks` 参数：
    
    - 功能：控制 Ansible 可以有多少个连接同时处于活动状态。
    
    - 默认值：`5`
    
    - 工作方式：若在当前任务中要处理 100 个主机，Ansible 也以 5 个为一组与它们通信，一旦完成与所有 100 个主机的通信，Ansible 将移动到下一 task。
    
    > 💥 注意：只有当 100 台主机完成上一个 task 后，才能执行下一个 task。
    
    - 调整参数值：
      
      - 若增大 forks 值，Ansible 一次可在更多主机上同时运行每一项 task，而 playbook 运行需要更少的时间。
      
      - 增大 forks 值的同时需注意控制节点是否能承受较大的负载，forks 值指定 Ansible 要启动的工作程序进程数量，因此太高的数字可能会损害控制节点和网络。
      
      - 若将 forks 设为 100，则 Ansible 可以尝试同时打开与 100 个主机的连接。
      
      - 这将给控制节点带来更多负载，仍需要足够的时间来与每个主机进行通信。
    
    - 参数修改方法：
      
      - 修改 Ansible 配置文件中的 forks
      
      - ansible-playbook 命令行的 `-f` 选项
    
    - 需尝试保守的值，如 `20` 或 `50`，然后在每次监控系统时逐步增加数值。
    
    > ✅ `cgroup_perf_recap` 回调插件在 playbook 运行期间监控控制节点的 CPU 和内存使用。

- 使用软件包管理器模块避免循环：
  
  - 某些模块接受要处理的项的列表，不需要使用循环。
  
  - 示例：yum 模块在单个事务中安装多个软件包（最高效的方式）
    
    ```yaml
    ---
    - name: Install the packages on the web servers
      hosts: web_servers
      become: True
      gather_facts: False
      tasks:
        - name: Ensure the packages are installed
          yum:    # yum 模块支持列表的形式安装多个软件包
            name:
              - httpd
              - mod_ssl
              - httpd-tools
              - mariadb-server
              - mariadb
              - php
              - php-mysqlnd
            state: present    # 不使用 loop 循环以增加执行效率
    ```
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\CICD\Ansible高级自动化最佳实践\pictures\Chapter03\yum-module-no-loop.jpg)
    
    👉 以上示例相当于在 shell 命令行中只执行一次 yum 命令与依赖包解析，而使用 loop 循环，相当于执行多次 yum 命令与依赖包解析，运行速度慢且效率低。
  
  - 并非所有 Ansible 模块都接受 name 参数的列表。
  
  - service 模块仅接受其 name 参数的单个值，需循环来对多个列表项进行迭代。
    
    ```yaml
    - name: Starting the services on the web servers
      hosts: web_servers
      become: True
      gather_facts: False
      tasks:
        - name: Ensure the services are started
          service:
            name: "{{ item }}"
            state: started
            enabled: yes
          loop:
            - httpd
            - mariadb
    ```

- 高效复制文件到受管主机：
  
  - copy 模块可将目录递归复制到受管主机。
  
  - 当目录很大并含有许多文件时，可能需要花费很长时间才能完成复制。
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\CICD\Ansible高级自动化最佳实践\pictures\Chapter03\copy-module-demo.jpg)
  
  - 若多次运行 playbook，由于该模块仅复制不同的文件，后续的复制将花费较少时间。
  
  - `synchronize` 模块：
    
    - 高效地将大量文件复制到受管主机
    
    - 大多数情况下，此模块在后台使用 `rsync`，速度比 copy 模块更快。
    
    - 因此，受管主机必须安装 rsync 命令，否则报错！
      
      ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\CICD\Ansible高级自动化最佳实践\pictures\Chapter03\synchronize-module-demo.jpg)

- 使用 Jinja2 模板（template）：
  
  - `lineinfile` 模块：在文件中插入或删除行，如配置文件中的配置指令。
  
  - 示例：替换几行代码更新 Apache HTTP 服务器配置文件
    
    ```yaml
    ---
    - name: Configure the Apache HTTP Server
      hosts: web_servers
      become: True
      gather_facts: False
      tasks:
        - name: Ensure proper configuration of the Apache HTTP Server
          lineinfile:
            dest: /etc/httpd/conf/httpd.conf
            regexp: "{{ item.regexp }}"
            line: "{{ item.line }}"
            state: present
          loop:
           - regexp: '^Listen 80$'
             line: 'Listen 8181'
           - regexp: '^ServerAdmin root@localhost'
             line: 'ServerAdmin support@example.com'
           - regexp: '^DocumentRoot "/var/www/html"'
             line: 'DocumentRoot "/var/www/web"'
           - regexp: '^<Directory "/var/www/html">'
             line: '<Directory "/var/www/web">'
    ```
  
  - lineinfile 模块与 loop 循环搭配使用时，运行效率低且易出错。
  
  - ✅ 建议 template 或 copy 模块直接替换 Jinja2 模板或直接复制已有的配置文件。

- 优化 SSH 连接：
  
  - 建立 SSH 连接可能是速度较慢的过程。
  
  - 当 play 具有很多任务并以大量的节点为目标时，Ansible 在建立这些连接上所花费的总时间将显著增加 playbook 的全局执行时间。
  
  - Ansible 依赖于 SSH 提供的两个功能来缓解该问题：
    
    - `ControlMaster`：
      
      - 允许多个同时与远程主机连接的 SSH 会话（session）使用单一网络连接。
      
      - 第一个 SSH 会话建立连接，与同一主机连接的其他会话则重复利用该连接，从而绕过较慢的初始过程，即在同一主机上执行不同的 task 任务可使用同一网络连接。
      
      - SSH 在最后一个会话关闭后，立即销毁共享的连接。
    
    - `ControlPersist`：
      
      - 使连接在后台保持打开，而不是在上一次会话后销毁连接。
      
      - 此指令允许稍后的 SSH 会话重用该连接。
      
      - ControlPersist 指示 SSH 应使空闲连接保持打开的时间长度，每个新会话将重置此空闲计时器（idle timer）。
      
      > 空闲计时器：记录空闲连接保持打开的时间长度
  
  - Ansible 通过 ansible.cfg 的 `[ssh_connection]` 部分下的 `ssh_args` 指令启用 `ControlMaster` 和 `ControlPersist` 功能。
    
    ```bash
    [ssh_connection]
    ssh_args = -C -o ControlMaster=auto -o ControlPersist=60s
    ```
    
    - 👉 默认 Ansible 在最后一次会话完成后使 SSH 连接保持打开 60 秒。
    
    - 若存在大量主机，可增加 ControlPersist 值，使完成任务所需的时间大于 60 秒。
    
    - 使用默认值 60 秒时，SSH 在下一任务启动时已关闭了与该主机的连接，并且必须重新建立连接。
    
    - 👉 若 forks 参数或 ControlPersist 设置较大，控制节点可能会使用更多的并发连接，此时，需确保控制节点配置有足够的文件句柄（`file handlers`），以支持更多活动的网络连接。

- 启用流水线（pipelining）：
  
  - Ansible 在远程节点上执行多个 SSH 操作运行任务，将模块及其所有数据复制到远程节点并执行该模块。
  
  - 若要提高 playbook 的性能，可激活流水线（`pipelining`）功能。
  
  - 借助流水线，Ansible 将建立较少的 SSH 连接。
  
  - 激活流水线：
    
    - 设置 Ansible 配置文件的 `[ssh_connection]` 部分中的 `pipelining = True`。
    
    - 所有远程节点上禁用 `requiretty sudo` 选项（默认不使用流水线）。
    
    - 在 RHEL 8 上，该 sudo 选项默认为禁用，但其他系统中可能为启用状态。
    
    > $ man 5 sudoers：查看 sudo 选项的详细信息
    
    - 若要禁用此选项，可使用 `visudo` 命令编辑受管主机上的 `/etc/sudoers` 文件并禁用`requiretty` 标志。
      
      ```bash
      $ cat /etc/sudoers
        ...
        Defaults  !requiretty
        ...
      ```

- ✨ 使用回调插件分析 playbook 执行性能：
  
  - 回调插件（`callback plug-ins`）通过调整对各种事件的响应来扩展 Ansible。
  
  - 其中一些插件会修改命令行工具（如 ansible-playbook）的输出，以提供额外的信息。
  
  - `timer` 回调插件：可在 ansible-playbook 命令的输出中显示 playbook 执行时间
  
  > 💥 注意：
  > 
  > 1. Red Hat Ansible Tower 在日志中记录与作业 jobs（playbook 运行）相关的信息，此信息提取自 ansible-playbook 的输出。
  > 
  > 2. 由于某些回调插件修改了此输出，因此应谨慎使用，或者避免将它们与 Ansible Tower 搭配使用。
  > 
  > 3. 需避免回调插件与 Ansible Tower 的混用！
  
  - 启用回调插件的方式：`callback_whitelist` 指令在 ansible.cfg 文件中显示指定
    
    ```ini
    [defaults]
    ...
    callback_whitelist = cgroup_perf_recap,timer,profile_tasks,profile_roles
    ...
    
    [callback_cgroup_perf_recap]
    # control_group = ansible_profile
    ```
    
    ```bash
    $ ansible-doc -t callback -l
    # 查看可用的回调插件列表
     
    $ ansible-doc -t callback <plug_in_name>
    # 查看指定回调插件的详细信息
    ```

- ✨ 分析控制节点 CPU 与内存：
  
  - `cgroup_perf_recap` 回调插件：
    
    - 功能：分析 playbook 运行期间的控制节点性能
    
    - 依赖：依赖于 cgroup 功能来监控和分析 ansible-playbook 命令
    
    - 在 playbook 执行结束时，将显示全局摘要和每个任务的摘要，摘要包括 CPU 和内存消耗，以及在 playbook 和任务执行期间启动的进程最大数量。
    
    - 在调优 ansible.cfg 文件中的 `forks` 参数时，也可利用 cgroup_perf_recap 来监控系统活动，确保选择的值不会使系统饱和。
  
  - Linux 中可以使用 cgroup 来限制和监控一组进程可以消耗的资源，如内存或 CPU。
  
  - 若要设置这些限值，可创建一个新组，设置限值，然后将进程添加到该组中。
  
  - 若要使用 cgroup_perf_recap 回调插件，必须⾸先创建一个运行 ansible-playbook 命令的专用控制组。
  
  - 该插件使用控制组的监控功能收集 ansible-playbook 进程及其子项的性能指标。
  
  - 🚀 示例：
    
    创建 cgroup 实现 playbook 性能指标收集
    
    ```bash
    $ sudo yum install -y libcgroup-tools
    $ sudo cgcreate -a devops:devops -t devops:devops \
      -g cpuacct,memory,pids:ansible_profile
    # 创建名为 ansible_profile 的控制组，该控制组在系统重启后将失效。
    # cgcreate 命令由 libcgroup-tools 软件包提供
    $ vim ansible.cfg
    # 指定 cgroup_perf_recap 回调插件使用的控制组名称
    ```
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\CICD\Ansible高级自动化最佳实践\pictures\Chapter03\cgroup_perf_recap-callback-demo-1.jpg)
    
    ```bash
    $ cgexec -g cpuacct,memory,pids:ansible_profile \
      ansible-playbook <playbook_name>.yml
    # 使用指定的控制组监控与收集 playbook 执行时进程的性能指标
    ```
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\CICD\Ansible高级自动化最佳实践\pictures\Chapter03\cgroup_perf_recap-callback-demo-2.jpg)
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\CICD\Ansible高级自动化最佳实践\pictures\Chapter03\cgroup_perf_recap-callback-demo-3.jpg)

- 任务和角色计时：
  
  - 以下回调插件可用于确定速度较慢的任务和角色：
    
    - `timer` 回调插件：
      
      显示 playbook 执行的持续时间
    
    - `profile_tasks` 回调插件：
      
      添加每个任务的开始时间，并在 playbook 执行结束时显示每个任务所用的时间，按降序排列。
    
    - `profile_roles` 回调插件：
      
      在结束时显示每个角色所用的时间，按降序排列。
  
  - 👉 callback_whitelist = timer,profile_tasks,profile_roles 回调插件可根据需求加以使用，而不必同时激活使用。
