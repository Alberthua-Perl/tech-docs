## 🤘 Ansible 自动化进阶实践（11）- 使用 Ansible 与 API 通信

### 文档目录：

- 使用 Ansible Tower API 启动作业
- 使用 Ansible Playbook 与 API 交互

### 使用 Ansible Tower API 启动作业

- 红帽 `Ansible Tower REST API`：
  
  - 红帽 Ansible Tower 提供了一个具象状态传输（REST）API。
  - API 可使管理员和开发人员能够在 `web UI` 之外控制其 Ansible Tower 环境。
  - 自定义脚本或外部应用使用标准 `HTTP` 消息访问 API。
  - 将 Ansible Tower 与其他程序集成时，`REST API` 非常有用，其优势之一在于，任何支持 HTTP 协议的编程语言、框架或系统都可以使用 API。
  - 这提供了一种简单的方式来自动化重复性任务，并将其他企业 IT 系统与 Ansible Tower 集成。
  
  > 💥 注意：
  > 
  > 1. 该 API 处于积极开发阶段，可能无法通过 API 访问 web UI 的所有功能。
  > 2. 目前有两种版本的 API 可用，预计将在不久的未来弃用版本 1。

- 使用 REST API：
  
  - 客户端将请求发送到位于统⼀资源标识符（`URI`）的服务器元素，并使用标准的 HTTP ⽅法（如，GET、POST、PUT 和 DELETE）执行操作。
  
  - REST 架构在客户端和服务器之间提供了无状态通信通道。
  
  - 每个客户端请求的行为都独立于任何其他请求，并且包含完成该请求所需的所有信息。
  
  - 以下示例请求使用 HTTP 获取方法检索 API 主入口点的表示法。
  
  - 可以使⽤图形 web 浏览器或 Linux 命令行工具手动发出请求。
  
  - 如下所示，`curl` 命令从发出请求：
    
    ```bash
    [student@workstation ~]$ curl -X GET -k -s https://tower.lab.example.com/api/
    {"description":"AWX REST API","current_version":"/api/v2/","available_versions":{"v1":"/api/v1/","v2":"/api/v2/"},"oauth2":"/api/o/","custom_logo":"","custom_login_info":""}
    # Ansible Tower API 同时兼容两个版本，当前使用 APIv2 版本。
    ```
  
  - API 请求的输出为 `JSON` 格式。
  
  - 红帽 Ansible Tower API 是可浏览的，如访问 https://tower.lab.example.com/api/ 的 Ansible Tower 服务器主机的 API。
  
  - 可单击该 `/api/v2/` 页面上的链接，浏览特定于 API 版本 2 的信息。
  
  - 如下所示，curl 命令执行此操作，`json_pp` 命令由 `perl-JSON-PP` rpm 软件包提供并打印 API 的 JSON 输出以便于进行阅读。
    
    ```bash
    [student@workstation ~]$ curl -X GET -k -s https://tower.lab.example.com/api/v2/ | json_pp
    {
       "jobs" : "/api/v2/jobs/",
       "activity_stream" : "/api/v2/activity_stream/",
       "schedules" : "/api/v2/schedules/",
       "notifications" : "/api/v2/notifications/",
       "project_updates" : "/api/v2/project_updates/",
       "unified_job_templates" : "/api/v2/unified_job_templates/",
       "settings" : "/api/v2/settings/",
       "dashboard" : "/api/v2/dashboard/",
       "unified_jobs" : "/api/v2/unified_jobs/",
       "applications" : "/api/v2/applications/",
       "inventory_scripts" : "/api/v2/inventory_scripts/",
       "inventory_sources" : "/api/v2/inventory_sources/",
       "config" : "/api/v2/config/",
       "ping" : "/api/v2/ping/",
       "roles" : "/api/v2/roles/",
       "workflow_job_templates" : "/api/v2/workflow_job_templates/",
       "system_job_templates" : "/api/v2/system_job_templates/",
       "users" : "/api/v2/users/",
       "groups" : "/api/v2/groups/",
       "job_templates" : "/api/v2/job_templates/",
       "metrics" : "/api/v2/metrics/",
       "teams" : "/api/v2/teams/",
       "notification_templates" : "/api/v2/notification_templates/",
       "credentials" : "/api/v2/credentials/",
       "credential_types" : "/api/v2/credential_types/",
       "workflow_job_nodes" : "/api/v2/workflow_job_nodes/",
       "organizations" : "/api/v2/organizations/",
       "projects" : "/api/v2/projects/",
       "job_events" : "/api/v2/job_events/",
       "hosts" : "/api/v2/hosts/",
       "inventory" : "/api/v2/inventories/",
       "tokens" : "/api/v2/tokens/",
       "instances" : "/api/v2/instances/",
       "me" : "/api/v2/me/",
       "labels" : "/api/v2/labels/",
       "workflow_jobs" : "/api/v2/workflow_jobs/",
       "instance_groups" : "/api/v2/instance_groups/",
       "system_jobs" : "/api/v2/system_jobs/",
       "workflow_job_template_nodes" : "/api/v2/workflow_job_template_nodes/",
       "inventory_updates" : "/api/v2/inventory_updates/",
       "credential_input_sources" : "/api/v2/credential_input_sources/",
       "ad_hoc_commands" : "/api/v2/ad_hoc_commands/"
    }
    # 查看 Ansible Tower API 的各个入口点链接
    ```
  
  - 并非 API 生成的所有信息都公开提供，需提供正确的身份验证信息才能访问指定的 API 资源，如下所示：
    
    ```bash
    [student@workstation ~]$ curl -X GET -k -s https://tower.lab.example.com/api/v2/activity_stream/ | json_pp
    {
       "detail" : "Authentication credentials were not provided. To establish a login session, visit /api/login/."
    }
    ```
    
    ```bash
    $ curl -X GET \
      --user admin:redhat \
      -k -s https://tower.lab.example.com/api/v2/activity_stream/ | json_pp
    # 使用 admin 用户进行身份验证查看指定 API 的链接  
    ```
  
  - 除以上使用 json_pp 命令读取 API 的返回之外，也可以通过图形 web 浏览器访问可浏览的 REST API。
  
  - 如下所示，使用 `Firefox web` 浏览器访问相同的 API：
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/tower-apiv2-demo-1.jpg)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/tower-apiv2-demo-2.jpg)
    
    也可通过在图形界⾯中提供 `JSON` 格式文本或文件，在特定的 API ⻚⾯上使⽤ PUT 或 POST 方法。

- 使用 API 启动作业模板：
  
  - API 的一个常见用途是启动现有的作业模板（job template）。
  
  - 本示例使用 curl 命令来快速概述如何使用 API 来查找和启动已在 Ansible Tower 中配置的作业模板。
  
  - 从红帽 `Ansible Tower 3.2` 开始，可通过 API 中的名称来引用作业模板（也可使用内部 ID 号）。
  
  - 从 API 启动作业模板分两个步骤完成：
    
    - 使用 `GET` 方法访问它，以获取有关启动该作业所需的任何参数或数据的信息。
      
      ```bash
      $ curl -X GET \
        -k -s --user admin:redhat \
        https://tower.lab.example.com/api/v2/job_templates/"<template_name>"/launch/ | \
        json_pp
      
      [student@workstation ~]$ curl -X GET -k -s --user admin:redhat \
      > https://tower.lab.example.com/api/v2/job_templates/"DEV webservers setup"/launch/ | json_pp
      {
         "ask_tags_on_launch" : false,
         "ask_verbosity_on_launch" : false,
         "ask_job_type_on_launch" : false,
         "can_start_without_user_input" : false,
         "passwords_needed_to_start" : [],
         "inventory_needed_to_start" : false,
         "ask_inventory_on_launch" : false,
         "ask_variables_on_launch" : false,
         "defaults" : {
            "job_tags" : "",
            "extra_vars" : "",
            "diff_mode" : false,
            "credentials" : [
               {
                  "name" : "Developers",
                  "passwords_needed" : [],
                  "credential_type" : 1,
                  "id" : 4
               }
            ],
            "verbosity" : 0,
            "skip_tags" : "",
            "inventory" : {
               "name" : "Dev",
               "id" : 4
            },
            "limit" : "",
            "job_type" : "run"
         },
         "variables_needed_to_start" : [
            "deployment_version"
         ],
         "job_template_data" : {
            "description" : "Setup apache on DEV webservers",
            "id" : 7,
            "name" : "DEV webservers setup"
         },
         "survey_enabled" : true,
         "credential_needed_to_start" : false,
         "ask_skip_tags_on_launch" : false,
         "ask_limit_on_launch" : false,
         "ask_credential_on_launch" : false,
         "ask_diff_mode_on_launch" : false
      }  
      ```
      
      > 👉 由于作业模板的名称包含空格，因此必须使用双引号或 `URL` 百分比编码（每个空格字符为 `%20` ）对它们进行转义。
    
    - 使用 `POST` 方法访问它以启动该作业。
      
      ```bash
      $ curl -X POST \
        -k -s --user admin:redhat \
        https://tower.lab.example.com/api/v2/job_templates/"<template_name>"/launch/ | \
        json_pp
      
      $ curl -X POST -H "Content-Type: application/json" -d '
        {
          "extra_vars": {
             "<variable>": "<value>"
           # "deployment_version": "v1.1"
          }
        }' -k -s --user admin:redhat \
        https://tower.lab.example.com/api/v2/job_templates/"DEV webservers setup"/launch/ | \
        json_pp
      # 若 Job Template 配置了 Survey，该 Survey 中具有默认定义的变量，在使用 REST API 启动
      # 时不指定变量将报错。
      # 因此，需使用如上方法传递额外变量运行作业模板。
      ```
      
      > 💥 注意：
      > 
      > 1. 也可以使⽤内部 `ID` 号而不是其名称来启动作业模板。
      > 2. 在较早版本的红帽 Ansible Tower 中，使用版本 1 API 时，必须仅使用 ID 号启动作业模板。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/apiv2-post-method-survey-variable.jpg)

- 使用 Ansible Playbook 中的 API 启动作业：
  
  - 可使用 Ansible Playbook 中的 `uri` 模块以访问 Ansible Tower API，以启动运行作业模板。
  
  - 也可以从 Ansible Tower 中的作业模板运行该 playbook，并使用它将另一作业模板作为其任务之一来
    启动。
  
  - 在 playbook 中，使用 ID 或指定的 URL，为作业模板指定正确的 URL。
  
  - 还必须为 Ansible Tower 提供凭据，以便作为拥有启动该作业的权限的用户进行身份验证。
  
  - [示例](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter11/tower-apiv2-uri.yaml)：以下 Ansible Playbook 可以使用 Ansible Tower API 从 Ansible Tower 服务器的现有作业模板之⼀中启动新的作业
    
    ```yaml
    # tower-apiv2-uri.yaml
    ---
    - name: Tower API
      hosts: localhost
      become: false
    
      vars:
        tower_user: admin
        tower_pass: redhat
        tower_host: tower.lab.example.com
        tower_job: TEST%20webservers%20setup
    
      tasks:
        - name: Launch a new Job
          uri:
            url: https://{{ tower_host }}/api/v2/job_templates/{{ tower_job }}/launch/
            method: POST
            validate_certs: no
            return_content: yes
            user: "{{ tower_user }}"
            password: "{{ tower_pass }}"
            force_basic_auth: yes
            status_code: 201
    ```
    
    > 💥 注意：
    > 
    > 1. 该示例中将访问 REST API 的用户名与密码一并定义于 `vars` 变量块中，存在安全风险。
    > 
    > 2. 可将变量存储于单独的变量文件中，使用 `ansible-vault` 命令对变量文件加密，Ansible Playbook 运行需使用该变量文件时再对其解密。
    > 
    > 3. 变量若集成于 playbook 中时，可直接加密 playbook。

- Vault 凭据：
  
  - 要使 Ansible Tower 用户使用加密的文件（playbook 或包含机密的附带变量文件），除了项目所需的任何主机凭据或其他凭据外，必须在 Ansible Tower 中为使用项目的任何作业模板设置可解密这些文件的 `Vault` 凭据。
  
  - 创建存储这些文件的 Vault 密码的 Vault 凭据。
  
  - 此凭据经过加密并存储在 Ansible Tower 服务器的数据库中，和主机凭据⼀样。
  
  - 创建 Vault 凭据（如下图所示）。
  
  - 完成后，可以使用新的作业模板来启动作业。
  
  - 在启动作业模板时，Ansible Tower 使⽤ Vault 凭据对加密的 playbook 进⾏解密。
  
  - 在运行 playbook 时，将执行访问 API 以启动其他作业模板的任务。
  
  - 这将使用由 playbook 的任务所引用的工作模板，在 Ansible Tower 服务器上启动一项新作业。
    
    > 💥 注意：
    > 
    > 1. 在 Ansible 的最新版本中，可使用不同的 Ansible Vault 密码加密不同的文件。
    > 2. Ansible Tower 可以在同一作业模板中使用多个 Vault 凭据，以确保它可以解密项目中使用 Ansible Vault 加密的所有文件。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/vault-credential-demo.jpg)
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/job-template-vault-credential.jpg)
    
    > 📌 以上示例配置流程：
    > 
    > 1. 使用明文密码文件 tower-apiv2-uri-passwd.txt 利用 ansible-vault 命令加密具有敏感信息变量的 tower-apiv2-uri.yaml playbook，并将该 playbook 同步至 GitLab 项目中，该 playbook 可通过 `APIv2` 触发另一作业模板的执行。
    > 
    > 2. 使用该 playbook 创建的模板需要 vault 类型的凭据解密此 playbook。
    > 
    > 3. 如下所示：
    >    
    >    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/tower-apiv2-job-template-triggle-another-job.jpg)

- 基于令牌（token）的身份验证：
  
  - 自红帽 Ansible Tower 3.3 起，API 使⽤ OAuth 2 提供基于令牌的身份验证。
  
  - 对请求标头中具有有效令牌的 API 的任何调用都将通过身份验证。
  
  - 有两种类型的令牌：
    
    - 应用令牌（application tokens）
    
    - 个人访问令牌（personal access tokens, PAT）
  
  - 为之前在 Tower 中创建的应用请求应用令牌，它表示将使用多个用户频繁访问 API 的客户端应用。
  
  - 个人访问令牌（`PAT`）是一种更加简单的机制，可为单个用户提供 API 访问权限。
  
  - [个人访问令牌示例](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter11/pat-apiv2-uri.yaml)：
    
    ```yaml
    # pat-apiv2-uri.yaml
    ---
    - name: Tower API
      hosts: localhost
      gather_facts: false
    
      vars:
        tower_user: admin
        tower_pass: redhat
        tower_host: tower.lab.example.com
        template_name: PROD webservers setup
    
      tasks:
        - name: Get the token
          uri:
            url: "https://{{ tower_host }}/api/v2/users/1/personal_tokens/"
            method: POST
            validate_certs: false
            return_content: true
            user: "{{ tower_user }}"
            password: "{{ tower_pass }}"
            force_basic_auth: true
            # regular authentication
            status_code: 201
          register: response
    
        - name: Use the token
          uri:
            url: "https://{{ tower_host }}/api/v2/job_templates/{{ template_name | urlencode }}/launch/"
            method: POST
            validate_certs: false
            return_content: true
            status_code: 201
            headers:
              Authorization: "Bearer {{ response['json']['token'] }}"
              Content-Type: "application/json"
          register: launch
    ```
    
    > 📌 以上示例配置流程：
    > 
    > 1. 依然使用前一个示例的作业模板进行更改，需将该 playbook 同步至项目所在的 GitLab 项目中。
    > 
    > 2. 替换作业模板中的 playbook 为 pat-apiv2-uri.yaml。
    > 
    > 3. 运行该作业模板时，由于该作业可触发已配置 survey 的 playbook，因此需定义额外的变量，否则运行将报错，如下所示：
    >    
    >    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/job-template-not-extra-var-error.jpg)

### 使用 Ansible Playbook 与 API 交互

- 与 API 交互：
  
  - Ansible Engine 可以与其他服务提供的任何 `HTTP API` 交互，包括 `RESTful API`。
  
  - Ansible 使用 `uri` 模块访问这些 API。
  
  - 🤘 `uri` 模块：
    
    > 可使用 ansible-doc 命令查看该模块的所有参数。
    
    - 功能：连接到指定的 URL，控制连接参数和对响应进行操作。
    
    - `url` 参数：唯一必须的参数，表示要连接到的完整 HTTP 或 HTTPS URL。
    
    - `method` 参数及常见选项：指定连接到服务端所用的 HTTP 方法
      
      - `GET`（默认方法）：客户端请求指定资源信息，服务端返回指定资源。
      
      - `POST`：将客户端的数据提交至服务端。
      
      - `PUT`：用请求有效载荷替换目标资源的所有当前表示。
      
      - `DELETE`：请求服务端删除所标识的资源。
      
      - `PATCH`：用于对资源应用部分修改。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/primary-http-method.jpg)
      
      👉 关于 HTTP 方法的说明请参考 [该链接](https://www.restapitutorial.com/lessons/httpmethods.html)。
    
    - `headers` 参数：控制如何连接到服务，可向请求添加自定义 HTTP 标头的字典。
    
    - 示例：
      
      ```yaml
      - name: Get an entity and set a Cookie
        uri:
          url: https://example.com/entity/1
          method: GET
          headers:
            Cookie: type=TEST
      ```

- 将信息发送到 API：
  
  - uri 模块的两个互斥参数（src 与 body）可用于向任何 API 发送信息。
  
  > 💥 不可同时使用这两个参数！
  
  - `src` 参数：提交至远程服务器的文件路径
    
    ```yaml
    - src
            Path to file to be submitted to the remote server.
            Cannot be used with `body'.
            [Default: (null)]
            type: path
            version_added: 2.7
    ```
  
  - `body` 参数：Web 服务的请求体（request body）或响应体（response body）
    
    ```yaml
    - body
            The body of the http request/response to the web service.
            If `body_format' is set to 'json' it will take an already formatted 
            JSON string or convert a data structure into JSON. If `body_format' 
            is set to 'form-urlencoded' it will convert a dictionary or list of 
            tuples into an 'application/x-www-form-urlencoded' string. 
            (Added in v2.7)
            [Default: (null)]
            type: raw
    ```
  
  - `body_format` 参数：
    
    ```yaml
    - body_format
            The serialization format of the body. When set to `json' or
            `form-urlencoded', encodes the body argument, if needed, and 
            automatically sets the Content-Type header accordingly. As of `2.3' 
            it is possible to override the `Content-Type` header, when set to 
            `json' or `form-urlencoded' via the `headers' option.
            (Choices: form-urlencoded, json, raw)[Default: raw]
            type: str
            version_added: 2.0
    ```
    
    - 支持 `raw`、`json` 与 `form-urlencoded` 格式
    
    - uri 模块对于 RESTful API：使用 `json`
    
    - uri 模块对于传统的基于表单的页面：使用 `form-urlencoded`
  
  - 示例：使用如下方法完成服务的登录
    
    ```yaml
    - name: Login to a form-based webpage
      uri:
        url: https://example.com/login.php
        method: POST
        body_format: form-urlencoded
        body:
          name: your_username
          password: your_password
          enter: Sign in
    ```

- 处理 API 响应：
  
  - 任何 HTTP 服务返回的第一种信息是响应的状态代码（status code of response）。
  
  - `status_code` 参数：
    
    - 告知 uri 模块在成功时预期的状态代码
    
    - 如果响应中的状态代码不同，则该任务将失败。
  
  - `dest` 参数：将响应保存为文件
  
  - `return_content` 参数：
    
    - 将响应正文添加到结果字典中
    
    - 可使用 register 将其保存在注册变量中
  
  - 示例 1：
    
    ```yaml
    - name: Check the contents of the response
      uri:
        url: http://www.example.com
        return_content: yes
      register: response
      failed_when: "'SUCCESS' not in response.content"
    ```
  
  - 🚀 示例 2：
    
    - 将从 `GitLab APIv4` 调用返回的数据解析为注册变量（register variable），变量 my_private_token 是在 GitLab 接口中设置的个人访问令牌，可能会从使用 Ansible Vault 保护的文件中加载到 play。
    
    > 该示例中 token 已提前创建，出于测试的目的，将其明文显示在 playbook 中。
    > 
    > ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/create-personal-access-token-demo.jpg)
    
    - API 调用返回的 `JSON` 是字典的列表（a list of dictionaries），其中每一字典都包含关于一个用户的信息，username 键包含用户的用户名。
    
    - [示例](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter11/gitlab-api-v4-connect.yaml)：
      
      ```yaml
      ---
      # Before run this playbook, please ensure gitlab.lab.example.com
      # node is running.
      # gitlab.lab.example.com is NOT in DO447v2.8 course environment 
      # which is customized node!
      - name: Test uri module to connect gitlab api v4
        hosts: localhost
        vars:
          my_private_token: TSFAyjsPszz5ggxQ3joB
          # token created for connect gitlab node
          # token in gitlab-customized-access-token.txt
      
        tasks:
          - name: Use GitLab API to get user data
            uri:
              url: http://gitlab.lab.example.com/api/v4/users/
              method: GET 
              headers:
                Private-Token: "{{ my_private_token }}"
              return_content: yes 
            register: gitlab_api_result
      
          - name: Pring gitlab_api_result variable
            debug:
              var: gitlab_api_result
      
          - name: Print user names
            debug:
              msg: A valid username is "{{ item.username }}"
            loop: "{{ gitlab_api_result.json }}"
      ```
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/gitlab-api-v4-connect-demo.jpg)

- HTTP 安全性设置：
  
  - uri 模块还⽀持摘要、基本或 WSSE 身份验证，可以通过使用 `url_username` 和 `url_password` 参数进行控制。
  
  - 若远程服务支持基本身份验证，但此模块无法进行身份验证，则尝试使用 `force_basic_auth` 参数强制执行基本身份验证。
  
  - 更安全的选择是使用私钥来建立与服务器的安全连接，使用 `client_cert` 参数，引用 `PEM` 证书链文件。
  
  - 若证书链文件不包含密钥，使用 `client_key` 参数将模块指向存储密钥的文件。
  
  - 若必须避免 TLS 证书验证，则将 `validate_certs` 参数设置为 false，此设置将降低连接的安全性。

- 使用过滤器准备和解析数据：
  
  - 在处理 HTTP 和 RESTful API 时，有几种过滤器非常有用。
  
  - `urlencode` 过滤器：
    
    - 功能：URL 支持 `US-ASCII` 字符集的一个有限子集，确保正确编码的 URL。
    
    - [示例](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/chapter11/http_restful_api_filter.yaml)：
      
      ```yaml
      ---
      - name: Use http or restful api filter
        hosts: localhost
        vars:
          entity_name: Test spaces
      
        tasks:
          - name: Output urlencode result
            debug:
              msg: >
                Format url "http://example.com?name={{ entity_name | urlencode }}" or 
                "http://example.com?name={{ entity_name }}"
      ```
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/ansible-api-test/http-restful-api-filter-1.jpg)
  
  - 其他有用的模块与过滤器：`xml` 模块、`to_json` 过滤器、`from_json` 过滤器
