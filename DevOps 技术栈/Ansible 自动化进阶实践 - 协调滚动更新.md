## Ansible 自动化进阶实践（5）- 协调滚动更新

### 文档目录：

- 委派任务和事实

- 管理滚动更新

### 委派任务和事实

- 委派任务（task）：
  
  - 🤘 委派就是在 play 中，将任务推送到指定的不同的主机或主机组上运行，而不是在当前受管主机上运行。
  
  - ✨ `delegate_to` 指令：
    
    - 将任务操作委托给主机
    
    - 该指令将 Ansible 指向将执行任务的主机，以替代对应的目标。
  
  - 委派任务的常见节点：
    
    - Ansible 控制节点 `localhost`
    
    - 若需要与某服务的 API 进行通信，而该服务出于某种原因无法从受管主机访问，但可从控制节点访问，则可能会这样做。
  
  - [示例 1](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter05/delegate_tasks_facts.yaml)：
    
    在 play 中的每个主机上运行 uname -a 命令，然后代表 play 中的每个主机在 localhost 上运行 uname -a 命令。
    
    ```yaml
    ---
    - name: Test delegation tasks
      hosts: servera.lab.example.com
    
      tasks:
        - name: Get system information
          command: uname -a
          register: server
    
        - name: Display servera system information
          debug:
            msg: "{{ server }}"
    
        - name: Get system information
          command: uname -a
          delegate_to: localhost
          register: local
    
        - name: Display localhost system information
          debug:
            msg: "{{ local }}"
    ```
    
    该 playbook 执行结果如下所示：
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/delegate-rolling-update/delegate_to-demo.png)
  
  - ✅ 示例 2：
    
    ```yaml
      - name: Remove the server from HAProxy
        haproxy:
          state: disabled
          host: "{{ ansible_facts['fqdn'] }}"
          socket: /var/lib/haproxy/stats
        delegate_to: "{{ item }}"
        loop: "{{ groups['lbservers'] }}" 
        # ansible 将该任务委派至 lbservers 主机组中主机上以禁用 haproxy 服务，
        # 而不是在指定的受管主机上执行！
      - name: Make sure Apache HTTPD is stopped
        service:
          name: httpd
          state: stopped
    ```
  
  - 🤘 委派任务的应用场景：
    
    - 在告警系统中启用基于主机的告警
    
    - 向负载均衡器中添加或移除一台主机
    
    - 在 DNS 上添加或修改针对某个主机的解析
    
    - 在存储节点上创建一个存储以用于主机挂载
    
    - 使用一个外部程序来检测主机上的服务是否正常

- 委派事实（fact）：
  
  - 如前所示，使用了受管主机的事实 `ansible_facts['fqdn']`，而非 localhost 的事实。
  
  - 💥 委派任务时，使用正在为其运行任务的受管主机（当前的 `inventory_hostname`）的主机变量和事实，如任务正在为 `servera` 运行，但已被委派到 localhost，则使用 `servera` 的变量和事实。
  
  - `delegate_facts` 指令
    
    - 设置值：true
    
    - 功能：将委派任务所收集的事实分配到该任务所委派到的主机上
  
  - [示例](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter05/delegate_tasks_facts.yaml)：
    
    delegate_facts 指令可将事实收集到委派主机的 `hostvars['servera.lab.example.com']` 命名空间中，而不是当前受管主机的默认 `hostvars['localhost']` 命名空间中。
    
    ```yaml
    - name: Test delegation facts
      hosts: localhost
      gather_facts: no
    
      tasks:
        - name: Set a fact in delegated task on servera
          set_fact:
            myfact: Where am I set?
          delegate_to: servera.lab.example.com
          delegate_facts: true
          # 在 localhost 上设置事实变量，并将该事实委派至 servera 节点上
    
        - name: Display the facts from servera.lab.example.com
          debug:
            msg: "{{ hostvars['servera.lab.example.com']['myfact'] }}"
          # use magic variable to verify delegation fact
    ```
  
  > 💥 `delegate_to` 与 `delegate_facts` 指令必须同时使用！

### 管理滚动更新

- 概述：  
  
  - Ansible 可启用滚动更新（`rolling updates`），一种在服务器批次错开部署的策略。
  
  - 借助该策略，可在不停机的情况下部署基础架构更新。
  
  - 发生不可预见的问题时，Ansible 可暂停部署，并且任何错误都可仅限于特定批中的服务器。
  
  - 通过实施测试和监控，可将 playbook 任务配置为：
    
    - 对受影响的批处理中的主机进行回滚配置（`rollback configuration`）。
    
    - 隔离受影响的主机，以启用对失败部署的分析。
    
    - 向利益相关者发送部署通知。

- 控制批处理大小（batch size）：  
  
  - 默认 Ansible 会在开始执行下一个任务前，对 play 中的所有主机运行一个任务。
  
  - 💥 若某一任务失败，则所有主机将只有一部分通过该任务。
  
  - 这可能意味着任何主机都无法正常工作，可能导致中断。
  
  - 理想情况下，可在启动下一批主机之前全程通过 play 处理一些主机。
  
  - 💥 若有太多主机失败，则可中止整个 play。
  
  - 批处理大小设置为固定值：
    
    - `serial` 关键字：
      
      - 在 play 级别实现批处理主机
      
      - 指定每个批处理中应当有多少个主机
      
      - 在开始下一批处理之前，Ansible 将全程通过 play 处理每一批主机。
      
      - 若当前批处理中的所有主机都失败，则整个 play 将中止，且不会启动下一个批处理。
      
      > ✅ forks 参数与 serial 关键字同时只有一者生效！
      
      - 示例：
        
        ```yaml
        ---
        - name: Update Webservers
          hosts: web_servers
          serial: 2
        ```
        
        - serial 关键字将在两个主机的批处理中处理 web_servers 组中的主机。
        
        - 若 play 正常执行且没有错误，则使用新的批处理再次重复该 play。
        
        - 此过程将继续，直到已处理 play 中的所有主机。
    
    - 若 play 中的主机总数不能被批处理大小整除，则最后一个批处理包含的主机可能比 serial 关键字的指定值更少。
    
    - 如上所示，若 web 服务器的总数量为奇数值，则最后一个批处理包含一个主机。
    
    - 👉 若定义的 `forks` 参数为 100，`serial` 值为 2，则具有 200 个主机的主机组将需要 100 个批处理才能完成，即 `2 x (2 serial x 50)` forks。
  
  - 批处理大小设置为百分比：
    
    - 可将 serial 关键字设置为百分比：
      
      ```yaml
      ---
      - name: Update Webservers
        hosts: web_servers
        serial: 25%
      ```
    
    - 若 serial 的值为 25%，那么无论 web_servers 组中是包含 20 个主机还是 200 个主机，都需要 4 个批处理来完成所有主机的 play。
    
    - 💥 Ansible 将该百分比应用到主机组中的主机总数，Ansible 将根据主机组的总大小而非未处理主机组的大小计算批处理大小。
    
    - 👉 若生成的值不是整数数量的主机，则值将被 `向下舍入` 到最接近的整数。
    
    - 剩余的主机在最终的较小批处理中运行。
    
    - 批处理大小不能为 `0` 个主机。
    
    - 若向下舍入后的值为 `0`，Ansible 会将批处理大小更改为 `1` 个主机。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/delegate-rolling-update/serial-percent-demo.png)
  
  - 🚀 设置要更改的批处理大小：
    
    - 可在 play 运行时更改批处理大小。
    
    - 可在一个主机的批处理上测试 play，若该主机失败，则整个 play 都会中止。
    
    - ✅ 但若该 play 在一个主机上成功，可以将批处理大小增加到主机的 `10%`，然后增加到受管主机的 `50%`，再增加到受管主机的其余部分。
    
    - 👉 可通过将 serial 关键字设置为值列表来逐步更改批处理大小。
    
    - 👉 此列表可以包含 `整数 + 百分比` 的任意组合，并按顺序重置每个批处理的大小。
    
    - 💥 若对应于 serial 关键字的最后一个条目的最后一个批处理后仍有未处理的主机，则最后一个批处理大小将重复，直到所有主机都得到处理。
    
    - 示例：
      
      针对包含 100 个主机的 web_servers 主机组执行
      
      ```yaml
      ---
      - name: Update Webservers
        hosts: web_servers
        serial:
          - 1
          - 10%
          - 25%
      ```
      
      - 第一个批处理：1 个主机
      
      - 第二个批处理：10 个主机（100 的 10%）
      
      - 第三个批处理：25 个主机（100 的 25%）
      
      > 👉 留下 64 个未经处理的主机（已处理 1 + 10 + 25 个）
      
      - Ansible 继续以 25 个主机（100 的 25%）的批处理大小执行 play，直到剩余的未处理主机数少于 25 个。
      
      - 剩余的 `14` 个主机在最终批处理中处理
      
      > 🤘 整个批处理的过程：`1 + 10 + 25 + 25 + 25 + 14 = 100`

- 中止 play：
  
  - 默认情况下，Ansible 尝试获取尽可能多的主机来完成 play。
  
  - 🚀 Ansible 运行 playbook 的要点：
    
    - 若某一任务对于某一主机失败，则它将从 play 中丢弃，但 Ansible 将继续为其他主机运行 play 中剩余的任务。
    
    - 💥 仅当所有主机都失败时，play 才会停止。
    
    - 但若使用 serial 关键字将主机组织到批处理中，那么若当前批处理中的所有主机都失败，则 Ansible 将停止所有剩余主机的 play，而不仅仅是当前批处理中剩余的主机。
    
    - 若由于批处理中的所有主机失败而停止了该 play 的执行，则下一个批处理将不会启动。
  
  - ✨ `ansible_play_batch` 变量：
    
    - 每个批处理保留活动主机列表。
    
    - 任何有任务失败的主机都将从 ansible_play_batch 列表中删除。
    
    - Ansible 会在每项任务后更新此列表。
  
  - 示例：
    
    针对包含 100 个主机的 web_servers 主机组执行的 2 种方法
    
    👉 方法 1：
    
    ```yaml
    ---
    - name: Update Webservers
      hosts: web_servers
      tasks:
        - name: Step One
          shell: /usr/bin/some_command
        - name: Step Two
          shell: /usr/bin/some_other_command
    ```
    
    - 若 Web 服务器中有 99 个的第一个任务失败，但一个主机成功，则 Ansible 将继续执行第二个任务。
    
    - 当 Ansible 执行第二个任务时，Ansible 仅为之前成功的一个主机执行该任务。
    
    👉 方法 2：
    
    ```yaml
    ---
    - name: Update Webservers
      hosts: web_servers
      serial: 2
      tasks:
        - name: Step One
          shell: /usr/bin/some_command
        - name: Step Two
          shell: /usr/bin/some_other_command
    ```
    
    - 若使用 serial 关键字，则只有当前批处理中剩余了没有失败的主机，playbook 执行才会继续。
    
    - 若第一个批处理中包含两个主机，一个成功，一个失败，则批处理将完成，并且 Ansible将移动到第二个包含两个主机的批处理。
    
    - 若第二个批处理中的两个主机在 play 中的任务上失败，则 Ansible 将中止整个 play，不再启动批处理。
    
    - 在这种情况下，playbook 执行后：
      
      - 一个主机成功完成 play
      
      - 三个主机可能处于错误状态
      
      - 其余的主机保持不变

- 指定容错（failure tolerance）：
  
  - 默认情况下，Ansible 仅在批处理中的所有主机失败时停止 play 执行。
  
  - 若清单中有超过一定百分比的主机失败，也可能希望 play 中止，即使整个批处理没全部失败。
  
  - ✨ `max_fail_percentage` 关键字：
    
    - 默认值 `100%`
    
    - 当批处理中失败的主机数量超过此百分比时，Ansible 将暂停 playbook 执行。
    
    - 可设置为 `0`，若有任何任务失败，可直接中止 play。
  
  - 示例：
    
    针对包含 100 个主机的 web_servers 主机组执行
    
    ```yaml
    ---
    - name: Update Webservers
      hosts: web_servers
      max_fail_percentage: 30%
      # 该参数在每个批处理中都将进行判断
      serial:
        - 2
        - 10%
        - 100%
      tasks:
        - name: Step One
          shell: /usr/bin/some_command
        - name: Step Two
          shell: /usr/bin/some_other_command
    ```
    
    - 第一个批处理包含 2 个主机。
    
    - 由于 30% 的 2 是 0.6，因此单个主机失败会导致执行停止。
    
    - 若第一个批处理中的两个主机都成功，则 Ansible 将继续执行包含 10 个主机的第二个批处理。
    
    - 由于 10 的 30% 是 3，因此，必须有超过 3 个主机失败才会导致 Ansible 停止 playbook 执行。
    
    - 如果三个或更少的主机在第二个批处理中遇到错误，Ansible 将继续执行第三个批处理。
  
  - 🚀 Ansible 的失败行为总结：
    
    - 若未设置 serial 关键字和 max_fail_percentage 值，则所有主机都将在一个批处理中通过 play 运行，若所有主机都失败，则 play 将失败，只要一个主机可继续执行任务，play 将继续执行。
    
    - 若设置了 serial 关键字，则主机将在多个批处理中通过 play 运行，若任何一个批处理中的所有主机都失败时，play 将会失败。
    
    - 若设置了 max_fail_percentage 关键字，则当批处理中超过该百分比的主机失败时，该 play 将失败。
    
    - play 失败，则 playbook 中剩余的所有 play 都将被中止。

- 一次运行一个任务（`run a task once`）：
  
  - `run_once: yes` 关键字：
    
    使一项任务只在每个批处理的一台主机上运行，即整个批处理中只运行一次，而不是批处理中的所有主机。
  
  - [示例](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter05/serial_run_once_demo.yaml)：
    
    ```yaml
    ---
    - name: test serial and run_once keywords
      hosts: all,!gitlab_node
      # 该示例中主机组为除了 gitlab_node 主机组外的所有主机
      order: sorted 
      # 指定的主机组中按字母顺序执行任务
      become: false
      gather_facts: no
      serial: 2
    
      tasks:
        - name: 'List: show all hosts in play'
          debug:
            var: ansible_play_hosts
    
        - name: 'List: show all hosts in each batch'
          debug:
            var: ansible_play_batch
    
        - debug:
            msg: First host in play is {{ ansible_play_hosts[0] }}
    
        - name: Report batch process on managed hosts
          shell: "echo {{ inventory_hostname }} -- {{ batch_nodes }} >> /tmp/batch_nodes.txt"
          # 使用魔法变量，若使用事实变量将报错！
          run_once: yes
          # Just run once on the first node of batch, ignore other nodes.
          delegate_to: workstation.lab.example.com
          vars:
            batch_nodes: "{{ ansible_play_batch | join(' ') }}"
          # ansible_play_batch variable is list included batch managed hosts.
          #when: inventory_hostname == ansible_play_hosts[0]
          # Just run the task on the fist node of all hosts
    ```
    
    ```bash
    ┌─[devops][workstation][±][master ✓][~/.../ansible-demo/do447-course-demo]
    └─➞ cat /tmp/batch_nodes.txt 
    servera.lab.example.com -- servera.lab.example.com serverb.lab.example.com
    serverc.lab.example.com -- serverc.lab.example.com serverd.lab.example.com
    servere.lab.example.com -- servere.lab.example.com serverf.lab.example.com
    workstation.lab.example.com -- workstation.lab.example.com
    # 结果显示任务在每个批处理的第一台主机上运行
    ```
  
  - 若只需要为 play 中的所有主机运行一次任务，并且该 play 具有多个批处理，那么可将以下条件添加到任务中。
    
    ```yaml
    when: inventory_hostname == ansible_play_hosts[0]
    # 仅针对 play 中的第一个主机运行任务（ansible_play_hosts 列表变量）。 
    ```
