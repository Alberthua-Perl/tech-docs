## 💎 Ansible 自动化进阶实践（4）- 过滤器、插件与高级循环

### 文档目录：

- 使用过滤器处理变量

- 使用查找模板化外部数据

- 实施高级循环

- 使用过滤器处理网络地址

### 使用过滤器处理变量：

> 该小节涉及的 Ansible Playbook 的 demo 片段可参考以下链接：
> 
> [jinja2_vars.yml](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/jinja2_vars.yml)、[jinja2_filter.yml](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/jinja2_filter.yml)

- Ansible 过滤器：
  
  - Ansible 使用 `Jinja2` 表达式将变量值应用到 playbook 和模板。
  
  - Jinja2 表达式使用双大括号括起的变量的名称替换为其值：`{{ variable }}`
  
  - Jinja2 表达式也支持过滤器（`filter`）。
  
  - 利用过滤器，可处理变量的值来提取信息、对其进行转换或使用它来计算新值。
  
  - 👉 过滤器的功能：
    
    - 用于修改或处理在 playbook 或模板（template）中的变量的值。
  
  - 👉 过滤器的来源：
    
    - Jinja2 语言的标准过滤器
    
    - 作为插件包含在 Red Hat Ansible 引擎中的过滤器
    
    - 自定义的过滤器（不在本课程的范围）
  
  - 👉 过滤器的使用：
    
    - 在变量名称后加上竖线 `|` 字符和要应用的过滤器的名称。
    
    - 某些过滤器可能需要将可选参数或选项放在括号中。
    
    - 可在一个表达式中串联多个过滤器。

- 变量类型：
  
  - 字符串（string）：
    
    - 字符序列
    
    - 字符串无须用引号或双引号括起
    
    - Ansible 从未加引号的字符串中修剪尾部空格
    
    - YAML 格式允许定义多行字符串，使用竖线运算符 `|` 或大于运算符 `>` 保留换行符。
  
  - 数字（number）：
    
    - 当变量内容符合数字时，Ansible 将解析字符串生成数值，即整数（integer）或浮点数（float）。
    
    - 整数：包含十进制字符，可以在前面加上 `+` 或 `-` 符号。
    
    - 浮点数：整数的值中包含小数点 `.` 。
    
    - 也可使用科学记数法表示整数或浮点数。
      
      ```yaml
      scientific_answer: 0.42e+2
      ```
    
    - 十六进制数字：以 `0x` 开头，后面仅跟十六进制字符。
      
      ```yaml
      hex_answer: 0x2A
      ```
    
    - 若将数字使用引号圈引，则将被视为字符串。
  
  - 布尔值（boolean）：
    
    - 布尔值：yes、y、no、n、on、off、true、false
    
    - ✅ 值不区分大小写，但 Jinja2 文档建议使用小写以保持一致。
  
  - 日期（date）：
    
    - 若字符串符合 `ISO-8601` 标准，Ansible 会将字符串转换为日期类型的值。
      
      ```yaml
      my_date_time: 2019-05-30T21:15:32.42+02:00
      my_simple_date: 2019-05-30
      ```
  
  - null：
    
    - 特殊的 null 值将变量声明为 `undeﬁned`。
    
    - null 字符串或波形符 `~` 将 null 值分配给变量。
  
  - 列表或数组（list or array）：
    
    - 列表（也称为数组）是值的有序集合。
    
    - ✅ 列表是数据收集和循环的基本结构。
    
    - 将列表写作以逗号分隔的值序列并用方括号括起，或每行元素并加上短划线 `-` 前缀。
      
      ```yaml
      my_list: ['Douglas', 'Marvin', 'Arthur']
      my_list:
        - Douglas
        - Marvin
        - Arthur
      ```
    
    - 与大多数编程语言中的数组一样，可使用从 0 开始的索引编号来访问列表的特定元素。
  
  - 字典（dictionary）：
    
    - 字典（在其他上下文中也称为映射或哈希）是将字符串键链接到值以直接访问的结构。
    
    - 与列表一样，字典可使用冒号 `:` 表示法写在一行内或跨越多行。
      
      ```yaml
      ---
      - name: Test Jinja2 variables
        hosts: localhost
        gather_facts: no
        become: false
        vars:
          my_list:
            - Douglas
            - Marvin
            - Arthur
          my_dict:
            Douglas: Human
            Marvin: Robot
            Arthur: Human
      
        tasks:
          - name: Confirm that the second list element is "Marvin"
            assert:
              that:
                - my_list[1] == 'Marvin'
      
          - name: Confirm "Marvin" vaule of hash
            assert:
              that:
                - my_dict['Marvin'] == 'Robot'
      ```
      
      - 该 playbook 可参考 [该 jinja2_vars.yml 的链接](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/jinja2_vars.yml)。
      
      - 也可使用语法 `my_dict.Marvin` 来访问 `my_dict['Marvin']`。
      
      - ✅ 不建议使用点表示法，它可能会与 python 字典的属性和方法的保留名称冲突！

- 常用过滤器概述：
  
  - Jinja2 过滤器的实用公用函数，可参考 [该链接](https://jinja.palletsprojects.com/en/2.10.x/templates/#builtin-filters)。
    
    👉 [Jinja v2.10 引擎的 changelog](https://jinja.palletsprojects.com/en/2.11.x/changelog/#version-2-10-1)：
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/jinja2-v2.10-changelog.jpg)
  
  - 检查变量是否已定义：
    
    - `mandatory` 过滤器：
      
      - 若变量未使用值定义，则会失败并中止 Ansible playbook。
      
      - 使用该过滤器后，即使变量未定义 playbook 将继续运行。
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/mandatory-filter-1.jpg)
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/mandatory-filter-2.jpg)
    
    - `default` 过滤器：
      
      - 若变量未使用值定义，则此过滤器会将其设置为圆括号中指定的值。
      
      - 若括号中的第二个参数为 `True`，那么当变量的初始值是空字符串或布尔值 False 时，过滤器也会将变量设置为默认值。
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/default-filter-1.jpg)
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/default-filter-2.jpg)
      
      - default 过滤器也可以取特殊值 `omit`，将导致值在没有初始值时保留为未定义状态。
      
      - 若变量已具有值，则 omit 不会更改值。
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/default-filter-3.jpg)
  
  - 执行数学计算：
    
    - Jinja2 提供多个数学过滤器，可对数字进行运算。
    
    - 可对数字进行基本的数学计算：
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/arithmetic-operator.jpg)
    
    - `int` 过滤器：将值转换为整数
      
      示例：Jinja2 表达式在当前的小时数上加上一，该数字作为事实收集，并作为字符串而不是整数存储。
      
      ```yaml
      - name: Operate number calculate
        debug:
          msg: >
            The result is {{ ( ansible_facts['date_time']['hour'] | int ) + 1 }}
      ```
    
    - `float` 过滤器：将值转换为浮点数
    
    - 其他数学计算的过滤器：
      
      - log：对数计算
      
      - pow：幂计算
      
      - root：二分求幂
      
      - abs：绝对值计算
      
      - round：四舍五入
  
  - 操作列表：
    
    - `max`、`min`、`sum` 过滤器：查找所有列表项数字的最大数、最小数和总和
      
      ```jinja2
      {{ [2, 4, 6, 8, 10, 12] | sum }}
      ```
    
    - 提取列表元素：
      
      - `first`、`last`、`length` 过滤器：
        
        提取列表的第一个元素、最后一个元素，以及获取列表长度。
        
        ```yaml
        - name: Test multiple filters for list
          assert:
            that:
              - "{{ [2, 4, 6, 8, 10, 12] | length }} is eq( 6 )"
              - "{{ [2, 4, 6, 8, 10, 12] | first }} is eq( 2 )"
              - "{{ [2, 4, 6, 8, 10, 12] | last }} is eq( 12 )"
        ```
      
      - `random` 过滤器：返回列表中的一个随机元素
        
        ```jinja2
        {{ [2, 4, 6, 8, 10, 12] | random }}
        ```
    
    - 修改列表元素的顺序（排序）：
      
      - `unique` 过滤器：去除列表中的重复元素
        
        示例：去除收集的事实列表中重复条目的用户名或主机名
      
      - `sort` 过滤器：
        
        - 按照元素的自然顺序对列表进行排序
        
        - 可使用该过滤器来操作 `硬编码数据`，而不是变量的值。
        
        - `eq` Jinja2 测试比较 sort 过滤器的输出，查看其是否与预期的列表相等。
        
        - 由于结果和预期值相等，因此 `assert` 模块成功。
        
        ```yaml
        - name: Test multiple filters for list
          assert:
            that:
              - "{{ [1, 4, 2, 2] | unique | sort }} is eq( [1, 2, 4] )"
              - "{{ [4, 8, 10, 6, 2] | sort }} is eq( [2, 4, 6, 8, 10] )"
        ```
      
      - `reverse` 过滤器：返回顺序与原始顺序相反的列表
      
      - `shuffle` 过滤器：返回列表，其中的元素与原列表相同，但顺序随机。
        
        ```yaml
        - name: Test multiple filters for list
          assert:
            that:
              - "{{ [2, 4, 6, 8, 10] | reverse | list }} is eq( [10, 8, 6, 4, 2] )"
              - "{{ [4, 8, 10, 6, 2] | sort }} is eq( [2, 4, 6, 8, 10] )"
        
        - name: Output variable value of filter
          debug:
            msg:
              another list is "{{ [5, 9, 13, 27] | shuffle }}"
        ```
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/reverse-list-filter-error.jpg)
        
        👉 reverse 过滤器必须与 list 过滤器搭配使用，否则报错！
    
    - 合并列表：
      
      - 可将多个列表合并为单个列表以简化迭代。
      
      - `flatten` 过滤器：
        
        以递归方式取输入列表值中的任何内部列表，并将内部值添加到外部列表中（嵌套子列表的扁平化处理）。
      
      - 使用 flatten 来合并从迭代父列表而获得的列表值。
        
        ```yaml
        - name: Test multiple filters for list
          assert:
            that:
              - "{{ [2, [4, [6, 8]], 10] | flatten }} is eq( [2, 4, 6, 8, 10] )"
        ```
    
    - 以集合（set）形式操作列表：
      
      > 注意：类似于 python 中的 set() 函数定义集合
      
      - `union` 过滤器：
        
        返回一个集合，包含两个输入集合中的元素。
      
      - `intersect` 过滤器：
        
        返回一个集合，包含两个集合中共有的元素。
      
      - `difference` 过滤器：
        
        返回一个集合，包含存在于第一个集合、但不存在于第二个集合的元素。
        
        ```yaml
        - name: Test multiple filters for list
          assert:
            that:
              - "{{ [2, 4, 6, 8, 10] | difference([2, 4, 6, 16]) }} is eq( [8, 10] )"
              - "{{ [2, 4, 6] | union([10, 16]) }} is eq( [2, 4, 6, 10, 16] )"
              - "{{ [2, 4, 6] | intersect([4, 12, 19]) }} is eq( [4] )"
        ```
  
  - 操作字典：
    
    - 字典的操作方式：
      
      - 过滤器构建字典
      
      - 字典转换为列表
      
      - 列表转换为字典
    
    - 与列表不同，字典不会以任何方式进行排序，它们仅仅是键值对的集合。
    
    - 连接字典（join dictionary）：
      
      - `combine` 过滤器：
        
        - 连接两个字典
        
        - 第二个字典中的条目的优先级高于第一个字典中的条目        
    
    - 重塑字典（reshape dictionary）：
      
      - `dict2items` 过滤器：重塑字典为项列表
      
      - `items2dict` 过滤器：重塑列表为字典
        
        ```yaml
        - name: Test multiple filters for dictionary
          assert:
            that:
              - "{{ {'A':1, 'B':2} | combine({'B':4, 'C':5}) }} is eq( expected )"
              - "{{ characters_dict | dict2items }} is eq( characters_items )"
              - "{{ characters_items | items2dict }} is eq( characters_dict )" 
          vars:
            expected:
              A: 1
              B: 4
              C: 5
            characters_dict:
              Douglas: Human
              Marvin: Robot
              Arthur: Human
            characters_items:
              - key: Douglas
                value: Human
              - key: Marvin
                value: Robot
              - key: Arthur
                value: Human
        ```
  
  - 散列、编码与操作字符串：
    
    - 字符串的大小写转换：
      
      - `capitalize` 过滤器：将字符串变量值的第一个字母转换成大写，其余全转换成小写。
      
      - `upper` 过滤器：将字符串变量值全转换成大写字母
      
      - `lower` 过滤器：将字符串变量值全转换成小写字母
        
        ```yaml
        - name: Output variable value of filter
          debug:
            msg: >
              The capitalize myname is {{ myname | capitalize }}
              and lower myname is {{ myname | lower }}
              and upper myname is {{ myname | upper }}
          vars:    
            myname: albertHua
            mynumber: 447
        ```
    
    - 散列字符串与密码：
      
      - `hash` 过滤器：利用提供的哈希算法，返回输入字符串的哈希值。
        
        ```yaml
        - name: the string's SHA-1 hash
          vars:
            expected: '8bae3f7d0a461488ced07b3e10ab80d018eb1d8c'
          assert:
            that:
              - "'{{ 'Arthur' | hash('sha1') }}' is eq( expected )"
        ```
      
      - `password_hash` 过滤器：生成密码哈希
        
        ```jinja2
        {{ 'secret_password' | password_hash('sha512') }}
        # 对明文密码进行 SHA512 哈希加密
        ```
    
    - 编码字符串：
      
      - `b64encode` 过滤器：将二进制数据转换为 base64
      
      - `b64decode` 过滤器：将 base64 转换为二进制格式
        
        ```yaml
        - name: Base64 encoding and decoding of values
          assert:
            that:
              - "'{{ 'âÉïôú' | b64encode }}' is eq( 'w6LDicOvw7TDug==' )"
              - "'{{ 'w6LDicOvw7TDug==' | b64decode }}' is eq( 'âÉïôú' )"
        ```
      
      - 在将字符串发送到 shell 之前，为了避免解析或代码注入问题，最好使用 `quote` 过滤器清理字符串。
        
        ```yaml
        - name: Put quotes around 'my_string'
          shell: echo {{ my_string | quote }}
        ```
    
    - 替换文本：
      
      - `replace` 过滤器：替换输入字符串内的某个子字符串的所有匹配
      
      - `regex_search`、`regex_replace` 过滤器与正则表达式实现复杂搜索和替换。
        
        ```yaml
        - name: Test results of regex search and search-and-replace
          assert:
            that:
              - "'{{ 'marvin, arthur' | replace('ar', '**') }}' is eq( 'm**vin, **thur' )"
              - "'{{ 'marvin, arthur' | regex_search('ar\\S*r') }}' is eq( 'arthur' )"
              - "'{{ 'arthur up' | regex_replace('ar(\\S*)r','\\1mb') }}' is eq( 'thumb up' )"
        ```
      
      > 💥 注意：教材上示例存在错误！
      > 
      > 1. 使用正则表达式时，需额外在反斜杠前再添加反斜杠，否则 playbook 语法报错！
      >    
      >    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/replace-regular-expression-error-1.jpg)
      > 
      > 2. 由过滤器处理的字符串需使用单引号或双引号圈引，否则使用 eq 测试将报错！
      >    
      >    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/replace-regular-expression-error-2.jpg)
  
  - 操作 JSON 数据：
    
    - JSON 和 YAML 表示法密切相关，Ansible 数据结构则可作为 JSON 来处理。
    
    - 类似地，Ansible Playbook 可能与之交互的许多 API 都会使用或提供 JSON 格式的信息。
    
    - JSON 查询：
      
      `json_query` 过滤器：从 Ansible 数据结构中提取信息
      
      ```yaml
      - name: Get the 'name' elements from the list of dictionaries in 'hosts'
        assert:
          that:
            - "{{ hosts | json_query('[*].name') }} is eq( ['bastion', 'classroom'] )"
        vars:
          hosts:
            - name: bastion
              ip:
                - 172.25.250.254
                - 172.25.252.1
            - name: classroom
              ip:
                - 172.25.252.254
      ```
    
    - 分析和编码数据结构：
      
      - `to_json`、`to_yaml` 过滤器：将数据结构序列化为 JSON 或 YAML 格式。
      
      - `to_nice_json`、`to_nice_yaml` 过滤器：获取人类可读的格式化输出。
        
        ```yaml
        - name: Convert between JSON and YAML format
          assert:
            that:
              - "'{{ hosts | to_json }}' is eq( hosts_json )"
          vars:
            hosts:
              - name: bastion
                ip:
                  - 172.25.250.254
                  - 172.25.252.1
            hosts_json: '[{"name": "bastion", "ip": ["172.25.250.254", "172.25.252.1"]}]'
        ```

### 使用查找模板化外部数据：

> 1. 该小节涉及的 Ansible Playbook 的 demo 片段可参考以下链接：[jinja2_plugins.yml](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/jinja2_plugins.yml)
> 
> 2. 可使用 `lookup 函数 + 过滤器` 重构 `with_*` 关键字！

- 查找插件（`lookup plug-in`）：
  
  - 查找插件是 Jinja2 模板化语言的 Ansible 扩展。
  
  - 这些插件使 Ansible 能够使用外部来源的数据（非 ansible 结构化文件），如文件和 shell 环境。

- 调用查找插件：
  
  - 可使用两个 Jinja2 模板函数（`lookup` 或 `query`）中的一个来调用查找插件。
  
  - 这两种方法都具有与过滤器相似的语法。
  
  - 指定函数的名称，并在圆括号中指定要调用的查找插件的名称和插件需要的任何参数。
  
  - 调用方法示例：
    
    `file` 插件：将一个或多个文件以逗号分隔，返回每个文件中的内容。
    
    - lookup 函数调用：
      
      ```yaml
      - name: lookup function calls file plug-in
        debug:
          msg: "Variable hosts is: {{ hosts }}"
        vars:
          hosts: "{{ lookup('file', '/etc/hosts', '/etc/issue') }}"
      ```
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/lookup-file-plugin-result.jpg)
    
    - query 函数调用：
      
      Ansible `2.5` 版本及以上，可使用 query 函数，而不是 lookup 来调用查找插件，根据以上的代码片段可更改为如下所示：
      
      ```yaml
      vars:
        hosts: "{{ query('file', '/etc/hosts', '/etc/issue') }}"
      ```
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/query-file-plugin-result.jpg)
    
    - 两种调用方法的区别：
      
      无论使用何种插件，query 函数回 `列表`，lookup 函数返回 `逗号分隔的值`（字符串）。

- 选择查找插件：
  
  ```bash
  $ ansible-doc -t lookup -l
  # 获取可用查找插件的完整列表
  $ ansible-doc -t lookup <plugin_name>
  # 查看特定插件用途和使用方法的详细信息
  ```
  
  ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/ansible-doc-lookup.jpg)

- `file` 插件：
  
  - 功能：读取文件的内容
  
  - file 插件允许 Ansible 将本地文件的内容加载到变量中。
  
  - 若提供相对路径，则插件将在 playbook 的 `files` 目录中查找文件。
  
  - file 插件读取控制节点上的文件，而不是受管主机上的文件。
  
  - 示例：
    
    - 读取用户的 SSH 公钥文件内容，并使用 `authorized_key` 模块将授权的密钥添加到受管主机。
    
    - 以下示例使用循环和 `+` 运算符将字符串附加到模板中，以便查找 files/fred.key.pub 和 files/naoko.key.pub 文件。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/authorized_key-lookup-loop.jpg)
    
    > 💥 注意：
    > 
    > 可直接在 ansible 命令行中使用 authorized_key 模块，以实现指定用户的 SSH 公钥分发，但必须在交互式命令行中输入登录用户的密码。
    > 
    > ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/authorized_key-deliver-user-ssh-pubkey.jpg)
  
  - 若文件为 YAML 或 JSON 格式，可使用 `from_yaml` 或 `from_json` 过滤器将其解析为正确结构化的数据。
    
    ```yaml
    - name: Transfer YAML file format
      debug:
        msg: "Transformation format of file is: {{ my_yaml }}"
      vars:
        my_yaml: "{{ lookup('file', 'user_list.yml') | from_yaml }}"
        # from_yaml filter used to transfer yaml format to structure file  
    ```

- `template` 插件：
  
  - 功能：使用模板应用数据
  
  - template 插件也返回文件的内容，预期文件内容为 Jinja2 模板，并在应用内容之前评估该模板。
  
  - 若将相对路径传递给模板文件，则插件将在 playbook 的 `templates` 目录中查找该文件。
    
    ```yaml
    - name: Print "Hello class"
      debug:
        msg: "{{ lookup('template', 'mytemplate.j2') }}"
      vars:
        name: class
    ```
  
  - template 插件允许额外的参数，如定义开始和结束标记序列。
  
  - 若输出字符串为 YAML 值，则 `convert_data` 选项将解析字符串以提供结构化数据。
  
  > 💥 注意：
  > 
  > 1. 插件（plug-in）工作于控制节点，而模块（module）工作于受管主机。 
  > 
  > 2. 因此，template 查找插件不可使用事实变量，否则报错，只可使用自定义变量！
  >    
  >    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/template-plugin-error.jpg)

- `env` 插件：
  
  - 功能：从控制节点读取查询环境变量
    
    ```jinja2
    home: "{{ lookup('env', 'HOME') }}"
    ```

- `pipe` 插件：
  
  - 功能：控制节点上返回命令生成的原始输出
  
  - 调用方法：`query + pipe` 返回字符串
    
    ```jinja2
    {{ query('pipe', 'ls files') }}
    # 以字符串形式返回 ls 命令的原始输出
    ```

- `lines` 插件：
  
  - 功能：控制节点上将命令的输出拆分为行
  
  - 调用方法：`query + lines` 返回列表
    
    ```jinja2
    {{ query('lines', 'ls files') }}
    # 使用 query 函数返回列表，每行输出由 ls 作为列表项返回。
    ```
  
  - 示例：分别提取不同文件的指定行（列表的列表）
    
    ```yaml
    - name: Prints the specfied line of some files
      debug:
        msg: "{{ item[2] }}"
      loop:
      # loop 循环的每个列表项为一个列表
        - "{{ query('lines', 'cat /etc/services') }}"
        - "{{ query('lines', 'cat /etc/passwd') }}"
        - "{{ query('lines', 'cat /etc/issue') }}"
    ```

- `url` 插件：
  
  - 功能：从 URL 获取内容
  
  - 有许多选项可用于控制身份验证、选择 Web 代理或将返回的内容拆分为行。
    
    ```jinja2
    {{ lookup('url', 'https://my.site.com/my.file') }}
    # 获取的返回数据可先通过过滤器对其进行处理
    ```

- `error` 参数：
  
  - 功能：处理查找错误
  
  - 大多数 Ansible 插件可在失败时中止 playbook。
  
  - 但 lookup 功能将执行委托给其他插件，这些插件可能不需要在失败时中止 playbook。
  
  - file 插件若找不到文件可能无需中止 playbook，但可能需要通过创建缺少的文件来恢复。
  
  - 为了适应不同的插件需求，lookup 插件接受 `error` 参数。
    
    ```yaml
    - name: Verify lookup plugin file ERROR
      debug:
        msg: Message is {{ message }}
      vars:
        message: "{{ lookup('file', 'myfile.txt', errors='warn') }}"
    ```
    
    - error 参数：
      
      - `strict` 值：默认值，即基础脚本失败，lookup 插件会引发严重错误。
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/lookup-error-strict.jpg)
      
      - `warn` 值：lookup 插件会在基础脚本失败时记录警告并返回空字符串或空列表。
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/lookup-error-warn.jpg)
      
      - `ignore` 值：lookup 插件会以静默方式忽略错误，并返回空字符串或空列表。
        
        ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/lookup-error-ignore.jpg)

### 实施高级循环：

> 该小节涉及的 Ansible Playbook 的 demo 片段可参考以下链接：
> 
> [advanced_loop.yml]()

- `with_list` 关键字与 `loop` 关键字：
  
  - 功能：迭代扁平化列表
  
  - Ansible `2.5` 及以上版本：
    
    - `loop` 关键字在项目的扁平列表上循环。
    
    > ✅ 扁平化列表：无嵌套列表的列表
    
    - 与查找插件结合使用时，可在列表中构建更复杂的数据以用于循环。
  
  - Ansible `2.5` 以下版本：
    
    - 任务迭代通过使用以 `with_` 开头并以查找插件的名称结尾的关键字来实施。
    
    - 此语法中与 loop 等效的是 `with_list`，设计用于在简单的扁平列表中迭代。
    
    - 对于简单的列表，loop 是可使⽤的最佳语法。
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/loop-with_list.jpg)
    
    > 👉 [loop 关键字与 with_* 关键字的语法转化](https://docs.ansible.com/ansible/latest/user_guide/playbooks_loops.html#migrating-to-loop)
  
  - loop 关键字代替 with_* 样式循环的优点：
    
    - 无需记住或查找 with_* 样式关键字来适合迭代场景。
    
    - 使用插件和过滤器使 loop 关键字任务适应用例。
    
    - 专注于学习 Ansible 中提供的插件和过滤器，其使用性比单纯的迭代更广泛。
    
    - 可通过命令行使用 `ansible-doc -t lookup` 命令访问查找插件文档。

- `with_items` 关键字与 `flatten` 过滤器：
  
  - 功能：迭代列表组成的列表
  
  - 可将各变量定义的列表组成的嵌套列表转换成扁平化列表，逐一迭代各列表项。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/with_items-demo.jpg)
  
  > 💥 注意：
  > 
  > 1. 该示例中，直接简单地将 with_items 关键字替换为 loop 关键字，执行 playbook 时，无法迭代各个变量定义的列表项。
  >    
  >    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/loop-replace-with_items-no-result.jpg)
  > 
  > 2. 由于 loop 不执行隐式的一级扁平化，因此它不完全等效于 with_items，而与 with_list 的行为相同。
  > 
  > 3. 只传递到循环的列表是简单列表，这两种方法的行为都相同。
  
  - 👉 使用 loop 关键字与 flatten 过滤器重构 with_items。
  
  - `flatten` 过滤器：
    
    - 以递归方式搜索嵌入式列表，并从发现的值创建列表。
    
    - levels 参数：指定要在用于搜索嵌入式列表的整数级别数
    
    - levels=1 参数：指定仅通过下降到初始列表中每一项的另一个列表来获取值
    
    - 重构时指定 `flatten(levels=1)` 过滤器，实现 with_items 隐式的相同的一级扁平化。
      
      ```yaml
      - name: Regenerate test files
        command: bash prepare_test_files.sh
      
      - name: Remove build files through loop
        file:
          path: "{{ item }}"
          state: absent
        loop: "{{ list_of_lists | flatten(levels=1) }}"
        vars:
          list_of_lists:
            - "{{ app_a_tmp_files }}"
            - "{{ app_b_tmp_files }}"
            - "{{ app_c_tmp_files }}"
      ```

- `with_subelements` 关键字与 `subelements` 过滤器：
  
  - 功能：迭代字典组成的列表
  
  - `subelements` 过滤器：
    
    - 从具有嵌套列表的列表中创建一个列表
    
    - 过滤器处理字典的列表，每个字典都包含一个引用列表的键。
    
    - 若要使用该过滤器，必须在对应于列表的每个字典上提供键名称。
      
      ```yaml
      vars:
        users_list:
          - name: bob
            gender: male
            hobby:
              - Skateboard
              - Videogame
          - name: alice
            gender: female
            hobby:
              - Music
      ```
      
      ```yaml
      # 两种不同的实现方式，但效果相同。
      # 方式 1：
      - name: Test complex data structure through with_subelements
        debug:
          msg: "{{ item.0.name }}'s hobby is {{ item.1 }}"
        with_subelements:
          - "{{ users_list }}"
          - hobby
      
      # 方式 2：
      - name: Test complex data structure through loop
        debug:
          msg: "{{ item.0.name }}'s hobby is {{ item.1 }}"
        loop: "{{ users_list | subelements('hobby') }}"  
      ```
      
      - subelements 过滤器从 users_list 变量数据创建一个新列表。
      
      - 列表中的每一项本身是一个两元素列表。
      
      - 第一个元素包含对每个用户的引用。
      
      - 第二个元素包含对该用户的 hobby 列表中的单个条目的引用。

- `with_dict` 关键字与 `dict2items` 过滤器：
  
  - 功能：迭代字典组成的字典
  
  - 字典组成的字典，如下所示：
    
    ```yaml
    users_dict:
      demo1:
        name: Demo user 1
        mail: demo1@example.com
      demo2:
        name: Demo user 2
        mail: demo2@example.com
      demo3:
        name: Demo user 3
        mail: demo3@example.com
    ```
  
  - Ansible 2.5 之前，必须使用 `with_dict` 关键字来迭代以上字典的键值对。
  
  - 也可使用 `dict2items` 过滤器将字典转换为列表。
  
  - 此列表中的项目的结构与 with_dict 关键字生成的项目相同。
    
    ```yaml
    # 两种不同的实现方式，但效果相同。
    # 方式 1：
    - name: Verify users key and value through with_dict
      debug:
        msg: >
          Users dictionary's key is {{ item.key }} and 
          value is {{ item.value.name }} and 
          user's mail is {{ item.value.mail }}
      with_dict: "{{ users_dict }}"
    
    # 方式 2：
    - name: Verify users key and value through loop
      debug:
        msg: >
          Users dictionary's key is {{ item.key }} and
          value is {{ item.value.name }} and
          user's mail is {{ item.value.mail }}
      loop: "{{ users_dict | dict2items }}"
    ```
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/witch_dict-demo.jpg)

- `with_fileglob` 关键字与 `fileglob` 查找插件：
  
  - 功能：迭代文件通配模式
  
  - 可通过 `fileglob` 查找插件构建循环，以迭代与所提供的文件通配模式匹配的文件列表。
  
  - 👉 构建列表需使用 `query` 函数，而 lookup 函数返回字符串不可迭代。
    
    ```yaml
    ## loop + query + fileglob plug-in equal to with_fileglob
    - name: Test fileglob lookup plugin through loop
      debug:
        msg: Bash file is {{ item }}
      loop: "{{ query('fileglob', '~/.bash*') }}"
      # 返回列表
    
    - name: Test fileglob lookup plugin through with_fileglob
      debug:
        msg: Bash file is {{ item }}
      with_fileglob:
        - "~/.bash*"
    ```
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/with_fileglob-demo.jpg)

### 使用过滤器处理网络地址：

> 该小节涉及的 Ansible Playbook 的 demo 片段可参考以下链接：
> 
> [filter_network.yml](https://github.com/Alberthua-Perl/ansible-demo/blob/master/do447-course-demo/filter_network.yml)

- 收集和处理网络信息：
  
  - 有多个过滤器和查找插件可用于收集和处理 Ansible 自动化的网络信息。
  
  - 👉 在受管主机上配置网络设备时，这些过滤器和插件可与收集事实结合使用。
  
  - 标准 `setup` 模块可在多个 play 开头自动收集受管主机的网络事实。
  
  - `ansible_facts['interfaces']` 事实：系统上所有网络接口名称的列表
  
  - 若 enp11s0 是系统上的接口，则具有名为 ansible_facts['enp11s0'] 的事实，其为包含 MAC 地址、IPv4 和 IPv6 地址、内核模块等值的字典。
    
    ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/ansible-network-facts.jpg)

- 网络信息过滤器：
  
  - `ipaddr` 过滤器：
    
    - 操作和验证与网络相关的事实。
    
    - 可用于检查 IP 地址的语法，从 `VLSN` 子网掩码转换为 `CIDR` 子网前缀表示法，执行子网计算，并查找网络范围内的下一个可用地址。
    
    - 💥 该过滤器需要 `netaddr` Python 模块，该模块由 RHEL 8 中的 `python3-netaddr` 软件包提供（区分 ansible 模块与 python 模块）。
    
    - 不带参数的 ipaddr 过滤器接受单个值：
      
      - 值为 IP 地址，则过滤器返回 IP 地址。
      
      - 值不为 IP 地址，则过滤器将返回 False 。
      
      - 值为列表，则过滤器返回有效的 IP 地址，而不是无效的地址。
      
      - 所有项目都无效，则返回空列表。
    
    - ipaddr 过滤器支持接受参数的传递。
    
    - 参数可以使过滤器根据数据输入返回不同的信息。
    
    - 示例：VLSN 子网掩码与 CIDR 子网掩码的转换
      
      ```yaml
      - name: Verify ipaddr filter network parameter
        debug:
          msg: >
            VLSN netmask is {{ '10.0.0.1/23' | ipaddr('netmask') }}
            CIDR netmask is {{ '10.0.0.1/255.255.254.0' | ipaddr('prefix') }}
      ```
      
      ![](https://github.com/Alberthua-Perl/tech-docs/blob/master/images/ansible-advanced-practice/filter-plugin-loop/ipaddr-filter-demo.jpg)
  
  - ipaddr 过滤器的部分（验证用）参数：
    
    - `address`：
      - 验证输入值是否为有效的 IP 地址。
      
      - 若输入中包含网络前缀，它将被剥离。
    
    - `net`：
      
      验证输入值是否为网络范围，并以 `CIDR` 格式返回。
    
    - `host`：
      
      确保 IP 地址符合等效的 CIDR 前缀格式。
    
    - `prefix`：
      
      验证输入主机是否满足主机/前缀或 CIDR 格式，并返回其前缀。
    
    - `host/prefix`：
      
      - 验证输入值是否为主机/前缀格式。
      
      - 主机部分是网络上的可用 IP 地址，前缀是有效的 CIDR 前缀。
    
    - `network/prefix`：
      - 验证输入值是否为网络/前缀格式。
      
      - 网络部分是网络地址，前缀是有效的 CIDR 前缀。
    
    - `public` 或 `private`：
      
      验证输入 IP 地址或网络范围是否在由 IANA 分别预留为公共或私有的范围内。
    
    - `size`：
      
      将输入网络范围转换为该范围内的 IP 地址数。
    
    - 任何整数（n）：
      
      - 将网络范围转换为该范围内的第 n 个元素。
      
      - 负数返回从最后一个数的第 n 个元素。
    
    - `network`、`netmask` 和 `broadcast`：
      
      验证输入主机是否满足主机/前缀或 CIDR 格式，并将其分别转换为网络地址（将子网
      
      掩码应用到主机）、子网掩码或广播地址。
    
    - `subnet`：
      
      验证输入主机是否满足主机/前缀或 CIDR 格式，并返回包含该主机的子网。
    
    - `ipv4` 和 `ipv6`：
      
      验证输入是有效的网络范围，并将它们分别转换为 IPv4 和 IPv6 格式。

- 查找 DNS 信息：
  
  - `dig` 查找插件：
    
    - 功能：dig 命令的接口，可针对 DNS 服务器进行查询，并返回生成的记录。
    
    - 依赖：控制节点中的 `python3-dns` 软件包的 `dnspython` Python 库
    
    - 示例：
      
      - 使用控制节点中配置的 DNS 服务器直接查询：
        
        ```jinja2
        "{{ lookup('dig', 'example.com') }}"
        # 确认是否存在提供的 FQDN 的 A 记录
        ```
      
      - 使用与控制节点中的配置不同的 DNS 服务器：
        
        ```jinja2
        # 示例 1：
        "{{ lookup('dig', 'example.com', '@4.4.8.8,4.4.4.4) }}"
        # 使用额外参数，以 @ 符号为前缀紧跟要使用的 DNS 服务器的逗号分隔列表。
        
        # 示例 2：
        - name: Query specfied host A record
          debug:
            msg: Host A record is {{ lookup('dig', 'servera.lab.example.com', '@172.25.250.254') }}
        ```
      
      - `qtype` 参数：
        
        - 功能：获取其他类型的记录，将斜杠 `/` 和记录类型附加到 FQDN。
          
          ```jinja2
          # 两种方式均可
          "{{ lookup('dig', 'example.com', 'qtype=TXT') }}"
          "{{ lookup('dig', 'example.com/TXT') }}"
          # dig 查找插件可以提供 DNS 文本记录
          
          "{{ lookup('dig', 'example.com', 'qtype=MX') }}"
          "{{ lookup('dig', 'example.com/MX') }}"
          ```
  
  - `dnstxt` 查找插件：
    
    - 功能：只需要使用 `TXT` 方式记录 dig 查找的内容
    
    - 依赖：控制节点中的 `python3-dns` 软件包的 `dns/dns.resolver` Python 库
    
    - 示例：
      
      ```yaml
      - name: Determinte if host's ip address is private
        debug:
          msg: "{{ lookup('dig', ansible_facts['hostname']) | ipaddr('private') }}"
      ```
