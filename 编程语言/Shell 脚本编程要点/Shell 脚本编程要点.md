# 🧩 Shell 脚本编程要点

## 文档目录

- [🧩 Shell 脚本编程要点](#-shell-脚本编程要点)
  - [文档目录](#文档目录)
  - [1. grep 命令示例](#1-grep-命令示例)
  - [2. 用户登录时加载 bash 配置文件的过程](#2-用户登录时加载-bash-配置文件的过程)
  - [3. 关于重定向说明](#3-关于重定向说明)
  - [4. Shell 内部字段分隔符（Internal Field Separator）](#4-shell-内部字段分隔符internal-field-separator)
  - [5. Shell 脚本多行注释](#5-shell-脚本多行注释)
  - [6. 设置自定义 PS1 交互式命令提示符](#6-设置自定义-ps1-交互式命令提示符)
  - [7. Shell 常用命令汇总](#7-shell-常用命令汇总)
    - [7.1 管道中使用 sudo 命令](#71-管道中使用-sudo-命令)
    - [7.2 重要的配置文件](#72-重要的配置文件)
    - [7.3 调试 Shell 脚本](#73-调试-shell-脚本)
    - [7.4 systemctl 命令选项](#74-systemctl-命令选项)
    - [7.5 base64 命令使用](#75-base64-命令使用)
    - [7.6 read 命令使用](#76-read-命令使用)
    - [🎯 7.7 sh/source/exec 命令的区别](#-77-shsourceexec-命令的区别)
  - [8. Shell 数组](#8-shell-数组)
  - [9. Shell 中的正则匹配](#9-shell-中的正则匹配)
    - [9.1](#91)
    - [9.2](#92)
  - [10. 浮点数计算](#10-浮点数计算)
  - [11. awk 编程示例](#11-awk-编程示例)
  - [❓待解决语法问题](#待解决语法问题)

## 1. grep 命令示例
  
```bash
$ grep -n "pattern" /path/to/file
# 显示匹配行的行号
  
$ grep -r "pattern" /path/to/directory
# 递归搜索目录中的所有文件
  
$ grep -v "pattern" /path/to/file
# 反向搜索，显示不包含模式的行。
  
$ grep -E "regex_pattern" /path/to/file
# 使用正则表达式搜索
  
$ grep "pattern1\|pattern2" /path/to/file
# 查找多个模式之一
  
$ grep -c "pattern" /path/to/file
# 统计匹配次数
  
$ grep -w "pattern" /path/to/file
# 查找时匹配整个单词而不是部分单词
  
$ grep "pattern1.*pattern2" /path/to/file
# 查找同时包含两个模式的行
  
$ find /path/to/search -type f -exec grep -l "pattern" {} \;
# grep 结合 find：在目录及其子目录中查找包含特定关键词的文件
  
$ sed -i 's/pattern/replacement/g' /path/to/file
# grep 结合 sed：使用正则表达式替换匹配的内容
  
$ awk '/pattern/ {print $2}' /path/to/file
# grep 结合 awk：根据特定字段进行过滤或操作
  
$ cut -d',' -f2 /path/to/file.csv | grep "pattern"
# grep 结合 cut：通过定界符截取特定字段
  
$ grep "pattern" /path/to/file | xargs command
# grep 结合 xargs：将 grep 结果作为参数传递给另一个命令
```

## 2. 用户登录时加载 bash 配置文件的过程

- 登录式 shell（login shell）加载配置文件过程：

  ```mermaid
  graph LR
    A(["/etc/profile"]) --> B(["/etc/profile.d/*.sh"])
    B --> C(["~/.bash_profile"])
    C --> D(["~/.bashrc"])
    D --> E(["/etc/bashrc"])
  ```

- 非登录式 shell（non-login shell）加载配置文件过程：

  ```mermaid
  graph LR
    A(["~/.bashrc"]) --> B(["/etc/bashrc"])
    B --> C(["/etc/profile.d/*.sh"])
  ```

## 3. 关于重定向说明

- `<`：输出重定向（将原来需要由键盘输入的数据，改由文件内容来取代）

  ```bash
  tr 'a-z' 'A-Z' < /path/to/file
  ```

- `<<`：代表结束的输入字符

  ```bash
  cat <<END
  cat > /path/to/file <<EOF
  cat <<EOF > /path/to/file
  ```

## 4. Shell 内部字段分隔符（Internal Field Separator）

- 字段分隔符为 `IFS`
- IFS 默认定义为空格、制表符与换行符，若 Shell 处理数据时出现以上字符，将以其作为内部字段分隔符。
- 默认情况下，使用 `for` 循环读取空格分隔的单行文本时，将以单词的方式输出字符串，而非单行文本。
- ✨ 此时默认以空格作为 IFS，需要重新定义 IFS 环境变量，即 `IFS=$'\n'`。

## 5. Shell 脚本多行注释

使用 `:<<!` 或 `:<<EOF` 与 `!` 或 `EOF` 注释代码块

## 6. 设置自定义 PS1 交互式命令提示符
  
```bash
$ vim ~/.bashrc
  export COURSE_ID=$(grep COURSE /etc/rht | awk -F'=' '{print $2}' | tr '[a-z]' '[A-Z]')
  export PS1="(${COURSE_ID})\033[1;36m[\u@\h\033[0m \033[1;33m\W\033[0m\033[1;36m]$ \033[0m"
```

## 7. Shell 常用命令汇总

### 7.1 管道中使用 sudo 命令

```bash
$ echo "<password>" | sudo -S <username>
# 命令行明文传递密码，sudo 直接为指定用户提权。
```

### 7.2 重要的配置文件

```bash
$ sudo vim /etc/profile
  echo "##### Welcome to login workstation.lab.example.com! #####"
  export HISTSIZE=500
  # 配置历史命令最多显示条数
  export HISTTIMEFORMAT="%F %T "
  # 配置历史命令时间的显示格式
  export TMOUT=1800
  # 配置所有用户登陆超时时间。若用户无任何操作，在配置的时间后将自动登出系统，单位为秒（s）。
  # 该超时时间也用于 SSH 远程登陆，若远程连接没有任何操作，在配置的时间后将自动登出。

$ echo "export TMOUT=1800" >> $HOME/.bash_profile
# 配置相关用户登陆系统的超时时间
# 避免自动登出系统的解决方法：
#   $ unset TMOUT  # 取消登陆超时时间
#   $ read         # read 命令搭配 TMOUT 一起使用，使 read 等待无限长时间。

$ sudo vim /etc/nologin.txt    
# 使用 /sbin/nologin 作为登陆 shell 而无法登陆系统
# 若使用该用户登陆时，系统显示的自定义说明文件。

$ sudo vim /etc/nologin
# pam_nologin.so 的配置文件
# 该配置文件内的用户将被拒绝登录系统（本地或远程登录），每个用户占一行。

$ sudo cat /etc/securetty
# pam_securetty.so 的配置文件
# telnet 远程登录时，限制 root 的登录终端类型。
# 包括：本地控制台（console）、本地终端（ttyX）、远程虚拟终端（pts/X）
```

### 7.3 调试 Shell 脚本

```bash
$ sh -x /path/to/<shell_script>
# 跟踪 shell 脚本执行过程      
  
$ sh -n /path/to/<shell_script>
# 检查 shell 语法

$ set -C
# 禁止对已经存在文件使用覆盖重定向（>）；强制覆盖重定向，则使用 >|。
$ echo "Hello world!" > ./testfile
# 设置以上命令后，执行覆盖重定向操作将返回 "cannot overwrite existing file" 报错。
$ echo "Hello world!" >| ./testfile
# 强制重定向覆盖
$ set +C
# 关闭上述功能
```

### 7.4 systemctl 命令选项

```bash
$ sudo systemctl status -l --no-pager <unit_name>.service
# -l, --full 选项：完全显示单元文件的所有内容
# --no-pager 选项：不截断输出，在整页中显示结果。
# 注意：相关的 systemd 环境变量如下所示，
#   $SYSTEMD_PAGER
#          Pager to use when --no-pager is not given; overrides $PAGER. If neither $SYSTEMD_PAGER nor $PAGER are set,
#          a set of well-known pager implementations are tried in turn, including less(1) and more(1), until one is
#          found. If no pager implementation is discovered no pager is invoked. Setting this environment variable to
#          an empty string or the value "cat" is equivalent to passing --no-pager.
```

### 7.5 base64 命令使用

- 基本使用：

```bash
$ cat /path/to/file | base64
# 使用 base64 编码原文件，76 个字符处断行。

$ cat /path/to/file | base64 -w0
# 完整输出编码后的整行
# 场景：适合脚本变量、HTTP 头、JWT、Kubernetes secret
```

- 文件编码与加密：

```bash
$ base64 /path/to/file > /path/to/base64_file
$ base64 -d /path/to/base64_file > /path/to/file

$ tar -zcvf - /path/to/file_or_dir | \
  openssl des3 -salt -pbkdf2 -iter 10000 -k "<password>" -out /path/to/file_or_dir.tar.gz
# 使用 3DES 对称加密算法、PBKDF2 标准以及 10000 次迭代生成密钥进行 tar.gz 压缩包加密
# 说明：
#   des3：对称加密算法，Triple-DES（3DES），密钥长度 168-bit。
#   -salt：在加密前随机生成 8-byte 的 salt，用于抵抗字典/彩虹表攻击。
#   -pbkdf2：使用 PBKDF2 算法从口令派生密钥，而不是旧的 EVP_BytesToKey 算法。
#            PBKDF2 是 “基于密码的密钥派生函数2”（Password-Based Key Derivation Function 2）的算法，
#            用于安全地从用户提供的密码中派生出加密密钥，而不是直接存储明文密码。
#            它通过对密码进行多次迭代哈希运算（使用随机 salt 来增强安全性），以生成固定长度的密钥，
#            从而大大增加了暴力破解的难度，确保了密码的安全性。
#            PBKDF2 是一个被广泛推荐的标准，常用于存储和验证用户密码。
#   -iter 10000：PBKDF2 的迭代次数设为 10000，提高暴力破解成本（可再调高）。
$ openssl des3 -salt -pbkdf2 -iter 10000 -d -k "redhat" -in /path/to/file_or_dir.tar.gz | \
  tar -zxvf -
# 使用 3DES 对称加密算法对 tar.gz 压缩包解密
# -d 选项：解密文件
# -k 选项：指定解密的明文密码
```

### 7.6 read 命令使用

| 场景 | 代码 | 说明 |
| ----- | ----- | ----- |
| 简单交互 | `read -p "姓名: " name` | 打印提示语，把输入存 `$name` |
| 超时自动退出 | `read -t 3 -p "继续吗[y/N]: " ans` | 3 秒内无输入则 `read` 返回非 0 |
| 不回显（密码） | `read -s -p "密码: " pwd` | 关闭回显，适合读密码 |
| 整行含空格 | `read -r line` | 禁用反斜杠转义，原样读整行 |
| 分割到数组 | `read -a arr`  | 按 IFS 拆分，元素存 `${arr[@]}` |
| 指定分隔符 | `IFS=, read -r f1 f2 f3` | 用逗号分割，一次赋 3 个变量 |

1️⃣ 交互场景：

```bash
$ read -p "输入文件名：" file    # -p 选项：交互式输入
输入文件名：loop.sh
$ [[ -f $file ]] && echo "存在" || (echo "不存在"; exit 2)    # 小括号将多条命令组合为整体
存在
$ read -p "输入文件名：" file
输入文件名：testfile
$ [[ -f $file ]] && echo "存在" || (echo "不存在"; exit 2)
不存在
$ echo $?
2
```

2️⃣ 超时自动默认值：

```bash
$ read -t 5 -p "继续？[y/N] " ans || ans="N"    # -t 选项：指定交互输入超时时间
继续？[y/N] y
$ [[ $ans =~ ^[yY]$ ]] && echo "Go on" || echo "Stop"    # 字符串匹配正则表达式
Go on
$ read -t 5 -p "继续？[y/N] " ans || ans="N"    # 超时 5 秒默认为 N
继续？[y/N] 
$ [[ $ans =~ ^[yY]$ ]] && echo "Go on" || echo "Stop"
Stop
```

3️⃣ 读密码：

```bash
$ read -s -p "请输入密码：" password    # -s 选项：隐藏输入的内容
请输入密码：
$ printf "您输入了 %d 个字符\n" ${#password}    # printf 命令格式化输出；${#变量名} 表示变量的字符数
```

4️⃣ 指定分隔符拆分：

```bash
### 按逗号拆分 ###
$ IFS=,
$ read -r -p "请输入姓名，年龄，城市：" name age city    # -r 选项：取消反斜杠转义
请输入姓名，年龄，城市：Foo,20,Shanghai
$ echo "$name | $age | $city"    # 根据 IFS 分隔符拆分字符串，为 3 个变量赋值。
Foo | 20 | Shanghai

### 按空格拆分 ###
$ IFS=' '
$ read -r name age city <<< "Foo 23 Shanghai"    # 内联输入重定向
$ echo "$name | $age | $city"
Foo | 23 | Shanghai
```

5️⃣ 整行读文件：

| 写法 | 含义 | 效果 |
| ----- | ----- | ----- |
| `IFS= read` | 临时清空 IFS | **保留行首行尾的所有空白**（包括空格、Tab） |
| `IFS= read -r line` | 标准安全写法 | 保留空白 + 不解释反斜杠转义 |
| `read line`（无 IFS=） | 使用默认 IFS | 会 **去除行首行尾的空白字符** |

IFS= 清空字段分隔符，使得 read 不会修剪行首行尾的空白，保证数据完整性。

```bash
#!/bin/bash

i=1
while IFS= read -r line; do
    echo "Line $i: $line"
    (( i += 1 ))
done < /path/to/file
```

### 🎯 7.7 sh/source/exec 命令的区别

| 特性 | `sh script.sh` | `source script.sh` (或 `. script.sh`) | `exec script.sh` |
| ----- | ----- | ----- | ----- |
| **进程关系** | 创建子 Shell 进程 | 在当前 Shell 中执行 | 替换当前 Shell 进程 |
| **PID 变化** | 新 PID（子进程） | 不变（当前 PID） | 不变（继承当前 PID） |
| **环境变量** | 子进程独立，不影响父 Shell | 直接影响当前 Shell | 替换后原 Shell 环境丢失 |
| **执行结束后** | 返回父 Shell 继续执行 | 继续执行后续命令 | **当前 Shell 终止** |
| **脚本权限** | 不需要执行权限（读即可） | 不需要执行权限 | 需要执行权限 |

## 8. Shell 数组

## 9. Shell 中的正则匹配

### 9.1

### 9.2

## 10. 浮点数计算

## 11. awk 编程示例

- 统计文件中指定列的值

```bash
$ cat results.txt
N40  1
G39  0
F30   1

$ awk '{count[$2]++}END{print "0 的数量：", count[0]; print "1 的数量：", count[1]}' results.txt    #1
0 的数量： 1
1 的数量： 2
# 统计第二列中 0 与 1 的数量（count 为关联数组，类似哈希）
$ awk '{count[$2]++}END{for (v in count) print v "的数量: " count[v]}' results.txt    #2：1与2等效
```

## ❓待解决语法问题

- set 命令如何调整 shell 行为：`-o` 选项、`-e` 选项？
- 如何使用 expr 命令与 bc 命令联合处理浮点数？
- 如何使用 sed 命令删除指定行、在指定行前后插入新行？
- awk 脚本如何引入 shell 中的变量？
- awk 脚本中如何使用正则表达式匹配？
- shellcheck 软件包如何检查 shell 语法？
- shell 中如何截取字符串、统计字符串长度？
- shell 变量中包含多个字符串时匹配空行使用 `[[ $var == [[:space:]] ]]`
- shell 中 if 条件判断如何使用正则表达式匹配？
- while 条件判断的无限循环使用？
- 如何定义 shell 空数组？并在数组中添加或删除元素？
- 如何使用 shell 数组，并且循环迭代？
- shell 中的 += 如何实现？
  - 数组追加元素：array+=(新项1 新项2 …)，带括号才是数组。
  - 字符串拼接：str+=后缀
  - 数字累加：((num+=增量))
- shell 中数字累加有浮点数如何处理？
- shell 中如何转换浮点数为整形数（浮点数截断）？
- shell 中 `trap "exit" INT` 的含义是（信号捕获）？
