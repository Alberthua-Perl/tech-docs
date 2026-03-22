# 🗝️ Linux 中通过 PAM 控制身份认证

## 文档目录

- [🗝️ Linux 中通过 PAM 控制身份认证](#️-linux-中通过-pam-控制身份认证)
  - [文档目录](#文档目录)
  - [1. 审计 PAM 配置](#1-审计-pam-配置)
    - [1.1 描述 PAM](#11-描述-pam)
    - [配置PAM](#配置pam)
  - [修改PAM配置](#修改pam配置)
    - [准备更新配置](#准备更新配置)
    - [使用authconfig配置PAM](#使用authconfig配置pam)
    - [手动修改PAM](#手动修改pam)
  - [配置密码质量要求](#配置密码质量要求)
    - [描述pam\_pwquality模块](#描述pam_pwquality模块)
    - [配置pam\_pwquality模块](#配置pam_pwquality模块)
    - [配置具有特定字符类要求的密码策略](#配置具有特定字符类要求的密码策略)
    - [解释贡献度（credit）机制](#解释贡献度credit机制)
  - [登录失败后限制访问](#登录失败后限制访问)
    - [多次登录失败后锁定帐户](#多次登录失败后锁定帐户)
    - [配置pam\_faillock模块](#配置pam_faillock模块)
    - [管理锁定的帐户](#管理锁定的帐户)
  - [补充：PAM模块使用示例](#补充pam模块使用示例)
    - [pam\_tally2模块](#pam_tally2模块)
    - [pam\_access模块](#pam_access模块)
    - [pam\_listfile模块](#pam_listfile模块)

## 1. 审计 PAM 配置

### 1.1 描述 PAM

- 可插拔式认证模块系统（pluggable authentication modules system，PAM）为应用提供一种通用的方法以实现身份认证（authentication）和授权（authorization）。
- PAM为应用提供通用的方式，实施对身份认证和授权的支持。
- 支持PAM的应用调用PAM核心库（libpam），使应用执行所有身份认证任务，并将通过的结果（pass或fail）返回给应用。
- PAM模块可实施不同的身份认证方法，如下所示：
  - 利用Kerberos执行身份认证（pam_krb5 模块）
  - 利用LDAP服务器进行身份认证（pam_ldap 模块）
  - 使用标准的本地文件/etc/passwd和/etc/shadow进行身份认证（pam_unix 模块）
- 支持PAM的应用可以直接使用上述身份认证方法，无需进行修改或重新编译。
- 管理员可以使用PAM配置文件来选择要用于各个应用的模块。
- PAM身份认证原理示意：(此处有图片)
- 如上所示，login程序与PAM联系以进行身份认证。
- PAM读取 /etc/pam.d/login 配置文件，以检索用于身份认证的模块列表。
- PAM调用存储在 /usr/1ib64/security/ 目录中的模块以进行身份认证。
- PAM将身份认证成功状态返回给login程序。
- vsftpd FTP服务器也使用PAM验证FTP用户的身份，但在图中，用户提供了不正确的凭据，因此PAM将一个失败代码返回给vsftpd。
- ✅ PAM与系统管理员、PAM模块开发者、应用程序开发者间的关系：(此处有图片)
- 系统管理员：通过各服务或应用程序的PAM配置文件制定认证策略。
- PAM模块开发者：利用PAM SPI编写验证模块，主要引入pam_sm_*()函数供libpam调用，将不同的认证机制引入。
- 应用程序开发者：通过在程序中使用PAM API实现对认证方法的调用。
- 描述身份认证和授权:
  - 描述身份认证和授权
    - 应用必须对客户端进行身份认证。
    - 应用必须授权客户端，使其可访问应用。
  - 如，用户在登录提示符下键入正确的用户名和密码后通过了身份认证。
  - 但访问控制限制可能不授权该用户，并从交互式登录中拒绝。
  - PAM可管理身份认证和授权。

### 配置PAM

- /usr/lib64/libpam.so.*：PAM核心库文件
- /etc/pam.d/ 目录：大多数PAM配置文件存放目录
- /etc/security/ 目录：部分PAM模块的额外配置文件存放目录
- /usr/1ib64/security/ 目录：PAM模块存放目录
- 每个支持PAM的应用都在/etc/pam.d/中拥有其PAM配置文件，通常此文件的名称与应用的名称相同。
- 如，/etc/pam.d/login针对的是login程序，或/etc/pam.d/sshd针对的是sshd守护进程。
- 若缺少针对某一应用的服务文件，则PAM会默认使用 /etc/pam.d/other。
- 配置文件包含指定要调用的模块的规则，从而能进行身份认证和授权。
- 因此，管理员可以为各个应用配置不同的身份认证方式。
- 实践中通常希望所有应用都使用相同的身份认证方法。
- 因此，大多数配置文件使用 include 或 substack 指令来包含 system-auth 或 password-auth 文件（位于/etc/pam.d/）。(此处有图片)
- 利用这种组织方式，应用共享相同的身份认证方式，并且维护一致性的PAM测试也相当容易。
- 管理员仅需要管理 system-auth 与 password-auth 文件以及单独的服务配置文件。
- 网络服务配置文件（sshd或vsftpd）包含 password-auth 文件。
- 本地服务配置文件（login或sudo）包含 system-auth 文件。
- PAM配置文件语法  
  - /etc/pam.d/中应用的配置文件规则格式：
  
  ```bash
  type  control  module [module_arguments]
  ```
  
  - 如下所示：/etc/pam.d/system-auth(此处有图片)  
  - ⭐ 第一列为类型（type），将不同的测试组织在4种管理组（management groups）中，分别为auth、account、password、session。  
  - ❗ 在每个管理组中的PAM规则顺序相当重要，PAM规则将从上至下解析并执行！  
  - 位于类型（type）前的 "-" 表示PAM模块不存在时跳过规则测试。  
  - PAM规则类型：
    - auth（认证管理）：接受用户名与密码，对该用户的密码进行认证等。
    - account（帐户管理）：检查帐户是否允许登录系统，密码是否过期，账号登录是否存在时间限制等。
    - password（密码管理）：管理用户的密码更改，该管理组不执行认证与授权。
    - session（会话管理）：提供对会话的管理与记账。  
  - 此处有图片  
  - 相同的PAM模块可在不同的管理组中被多次调用，PAM可区分在不同管理组中相同模块的不同行为。  
  - ⭐ 第二列是控制（control flags），控制类型如下所示：
    - required：该模块必须返回成功才通过认证，若该模块返回失败，失败结果不立即通知用户，而是等到同一管理组中的所有模块全部执行完成后再将失败结果返回至应用程序（必要条件）。
    - requisite：与required类似，该模块必须返回成功才通过认证，区别在于，一旦模块返回失败，将不再执行同一管理组中的任何模块，而是直接将控制权返回至应用程序（必要条件）。
    - sufficient：该模块返回成功已足以通过认证，不必再执行该管理组内的其他模块，若该模块返回失败可忽略结果（充分条件）。
    - optional：该模块可选，其返回成功与否一般对身份认证不起关键作用，其执行结果不影响其他模块的执行，返回值一般被忽略。
  - 此处有图片
  - ⭐ 第三列是PAM模块本身，可使用模块的绝对路径或模块名称（推荐）指定。
  - PAM模块可具有选项和参数，但并非每个模块都具有。
  - PAM模块的选项和参数一般具有唯一性。
  - PAM控制类型（control flags）执行结果汇总：(此处有图片)
  - PAM控制类型执行结果示例：(此处有图片)
  - ❗required与requisite对应的PAM模块只要返回失败，其所在的管理组最终返回失败，区别只是在于是否继续执行其同一管理组中的后续测试。
- 使用SSSD和PAM
  - RHEL 6.0中引入的系统安全服务守护程序（system security services daemon，SSSD）允许对远程目录和身份认证服务（如LDAP、Kerberos、ActiveDirectory和红帽身份管理）进行用户身份认证。
  - 当远程用户在本地系统上进行身份认证时，SSSD会将其凭据和身份认证参数存储到本地缓存中。
  - 因此，即使远程认证服务器不可访问，远程用户仍然可以登录本地系统。
  - 此功能对于与公司网络断开连接的笔记本电脑特别有用，也有助于减轻远程认证服务的负载。
  - PAM首选通过 pam_sss 模块使用SSSD。
- 访问PAM文档
  
  ```bash
  $ man 5 pam.conf：
  # 查看PAM配置文件使用的详细信息
  ```
  
  ```bash
  $ man -k pam_：
  # 查看系统上可用的PAM模块列表与说明信息
  ```

-----

## 修改PAM配置

### 准备更新配置

- 由于PAM控制对系统的访问，因此其配置中的错误可能会产生严重的后果。
- 不正确的配置会导致无法访问系统，甚至root也可能会被拒绝访问。
- 为避免PAM的误配置，可使用如下命令备份PAM配置。

  ```bash
  $ sudo authconfig --savebackup=<backup_dir>
  # 备份PAM配置文件
  ```

  ```bash
  $ sudo authconfig --restorebackup=<backup_dir>
  # 恢复PAM配置文件
  ```

- ✅ 在另一个终端上打开root shell，并保持打开状态。
- 若将自己锁定在系统外，仍可使用该打开的shell来修复问题。
- 在关闭该救援终端前，请确认新的PAM配置是否按预期工作。
- ✅ 若遇到任何问题，请在修复系统之前不要重启系统。
- 使用提供的authconfig命令配置PAM，而不要手动修改文件。

### 使用authconfig配置PAM

- authconfig 提供命令行工具，也通过 authconfig-gtk 提供图形界面。
- 已弃用的 authconfig-tui 命令提供基于文本的界面。
- 管理员也可使用authconfig来管理名称服务交换机（NSS），以配置对用户信息数据库的访问。
- authconfig命令的两种工作模式：  
  - 更新模式（update mode）：使用 --update 选项更新配置  
  - 测试模式（test mode）：使用 --test 选项打印配置而不应用配置
- 更新模式下，authconfig命令将更改/etc/pam.d/{system-auth-ac,password-auth-ac}文件，而大多数服务的PAM配置文件包含system-auth与password-auth文件，这两个文件是前两个文件的软链接。
- 当authconfig命令更新了 *-ac 文件，PAM配置即时生效。

### 手动修改PAM

- 大多数情况下，管理员可通过authconfig命令管理PAM，而在一些特殊情况下，需要手动编辑PAM配置文件。
- ✅ 推荐做法：编辑PAM配置文件时，需确保在保存文件之前进行需要的所有更改。
- 由于PAM会立即应用修改，因此配置不全可能会导致系统无法访问。
- 手动修改/etc/pam.d/system-auth文件（软链接），若使用authconfig命令更新将覆盖该文件已更新的配置，使用以下两种方式避免此现象的发生。
- 仅允许手动配置  
  - 可创建 system-auth-local 与 password-auth-local 文件避免authconfig命令的配置覆盖。  
  - 此处有图片  
  - 因此，可手动编辑以上文件完成PAM配置，若误使用authconfig命令更新，其只影响system-auth-ac与password-auth-ac文件，不会影响现有PAM配置。
- 允许手动与authconfig配置  
  - 可能更倾向于使用authconfig命令，但也允许手动配置。  
  - 此处有图片  
  - 因此，可使用自定义的 *-local 文件用于手动配置，但也包含了通过authconfig命令更新的 *-ac 文件。

-----

## 配置密码质量要求

### 描述pam_pwquality模块

- RHEL中可强制使用密码安全策略。
- 当用户通过打开的应用会话更改其密码时，PAM使用 password 管理组中的规则。
- /etc/pam.d/sshd与/etc/pam.d/login文件中的password规则：(此处有图片)
- /etc/pam.d/password-auth与/etc/pam.d/system-auth文件中的password规则：(此处有图片)
- pam_pwquality 模块提取用户提供的新密码，对照 /usr/share/cracklib/ 中的字典，并检查它的模式和易猜组合。
- 此模块默认要求密码长度超过六个字符，并且组合有大写、小写字母、数字以及特殊字符。
- 此模块通过 requisite 控制器调用，因此若判定为不符合要求，密码更改会立即停止，并且控制权返回到启动PAM调用的应用，不进一步测试PAM规则。
- ❗pam_pwquality 模块代替了RHEL 7中的 pam_cracklib 模块。
- 若用户通过pam_pwquality测试，PAM将执行sufficient pam_unix.so规则。
- 在password管理组中，pam_unix 模块在各种后端存储机制中更新用户密码，包括/etc/passwd、/etc/shadow或网络信息系统（NIS）。
- 由于pam_unix规则使用sufficient控制器，若成功更新用户的密码，PAM将关闭并将控制权传回调用它的应用。
- 最后一条规则是 required pam_deny.so 条目。
- 若由于某种原因导致 sufficient pam_unix.so 规则未能更新用户密码，PAM才会测试此规则。
- pam_deny 模块始终返回失败结果，应用就能感知调用PAM时出现了错误。
- 由于pam_deny规则为required，因此PAM会向尝试更改用户密码的应用返回整体失败。

### 配置pam_pwquality模块

- 手动编辑PAM配置文件中的 requisite pam_pwquality.so 参数，其默认参数位于 /etc/security/pwquality.conf 中。
- 编辑pam_pwquality规则文件 /etc/security/pwquality.conf，但/etc/pam.d/system-auth和/etc/pam.d/password-auth文件中的以下参数无法在 /etc/security/pwquality.conf中指定，若需修改它们，需编辑PAM配置文件。
- try_first_pass：  
  - 提示用户输入密码之前，pam_pwquality会验证先前的模块是否已请求过密码。  
  - 若是这种情况，pam_pwquality将重用此密码。
- local_users_only：仅认证由/etc/passwd文件定义的本地用户。
- retry=N:为用户提供N次机会，来输入通过模块测试的密码。
- authtok_type= :  
  - 默认情况下，pam_pwquality在要求输入新密码时会显示 “New password” 和 “Retype new password” 提示。  
  - 使用authtok_type自定义该提示。  
  - pam_pwquality将authtok_type的值插入到 new 和 password 间。  
  - 如，将authtok_type设置为Linux会将提示更改为 “New Linux password” 和 “Retype new Linux password”。(此处有图片)
- 以下参数控制密码复杂性要求。
- 可在PAM配置文件中将其指定为pam_pwquality模块的参数，或在 /etc/security/pwquality.conf 文件中指定。
- minlen :  
  - 密码所需的最短长度。  
  - 默认情况下，所有字符对总长度分数贡献分值都是 1。  
  - 管理员可利用credit系统为某些字符类型给予更大的分值，从而只需较少的总字符数即可满足相同的长度要求。
- lcredit :  
  - 小写字母字符对密码长度的贡献分值。  
  - 若将此值设置为负数，它将指定所需的最少小写字母字符数。
- ucredit :  
  - 大写字母字符对密码长度的贡献分值。  
  - 若将此值设置为负数，它将指定所需的最少大写字母字符数。
- dcredit :  
  - 数字字符对密码长度的贡献分值。  
  - 若将此值设置为负数，它将指定所需的最少数字字符数。
- ocredit :  
  - 其他字符对密码长度的贡献分值。  
  - 其他字符包括符号，以及不包括在小写、大写和数字范围的所有字符。  
  - 若将此值设置为负数，它将指定所需的最少其他字符数。
- difok=N : 新密码与旧密码间最少的不同字符数。
- minclass=N :最少的字符类别，包括数字、大写、小写与其他字符。
- maxrepeat=N :密码中不能出现超过N个连续重复的字符。(此处有图片)
- maxsequence=N ：密码中不能出现连续N位的单调字符。(此处有图片)

```bash
$ man 8 pam_pwquality
# 查看pam_pwquality模块的详细使用信息
```

```bash
$ man 5 pwquality.conf
#查看pam_pwquality模块的配置文件信息
```

- ✅ pam_pwquality模块调整密码复杂度与密码记录示例：/etc/pam.d/system-auth (此处有图片)

### 配置具有特定字符类要求的密码策略

- 用户密码复杂度示例：(此处有图片)
- 负值表示每个类所需的最少字符数。
- 即使策略未指定有关小写字母字符的任何内容，仍需将 lcredit 参数设置为0。
- 所有这些贡献度参数的默认值为1，需要为所有参数指定合适的值，否则pam_pwquality将不能正确实施密码策略。

### 解释贡献度（credit）机制

- pam_pwquality贡献度机制当贡献度参数为正数值时触发。
- 在此情况下，minlen参数更像是密码必须达到时模块才会接受的质量级别。
- 此处有图片
- ❗若得到的分数等于或大于minlen，则接受密码。
- 此处有图片
- 此处有图片

-----

## 登录失败后限制访问

### 多次登录失败后锁定帐户

- 许多安全策略都有与多次登录尝试失败后锁定用户帐户相关的要求。
- 可使用pam_faillock模块来实施此要求。
- pam_faillock 模块：可以在特定次数的登录尝试失败后锁定帐户，定义的期限后自动解锁帐户。
- 使用 faillock 命令（PAM软件包的一部分）查看登录尝试失败的报告或重置用户失败的尝试。
- ❗pam_faillock模块已添加至RHEL 6.1中，与现有的 pam_tally2 模块一起提供。
- pam_faillock提供一些额外的功能，如它还控制本地屏幕保护程序上的失败登录尝试。
- authconfig工具可以配置pam_faillock模块，但不能配置pam_tally2模块。
- ❗启用pam_faillock时，具有拒绝服务攻击风险，恶意用户可以通过自愿输入错误密码来可轻松锁定帐户，之后锁定期间内，合法用户将被拒绝访问其帐户。

### 配置pam_faillock模块

- pam_faillock没有专用的配置文件，其配置完全通过配置参数进行。
- 要启用和配置pam_faillock，可手动编辑PAM配置文件，也可使用authconfig工具。

```bash
$ sudo authconfig --enablefaillock --faillockargs="deny=3 fail_interval=60 unlock_time=600" --update
# 使用authconfig命令更新pam_faillock模块配置
```

- --enablefaillock选项激活PAM配置中的pam_faillock模块，--faillockargs选项配置该模块。
- pam_faillock模块相关参数：  
  - deny=N :连续尝试登录失败的次数，若全部失败pam_faillock将锁定帐户。  
  - fail_interval=<second> :
    - 以秒为单位指定的时间间隔，只有在此期间发生失败的登录尝试才将锁定帐户。
    - 如前所示，当使用 deny=3 和 fail_interval=60 时，若用户在一分钟内连续输错三次密码，pam_faillock将锁定帐户。
  - unlock_time=<second> :
    - PAM解锁帐户前须等待的秒数。
    - 如前所示，当使用 unlock_time=600 时，帐户被锁定的用户必须等待10分钟才能再次登录。
  - even_deny_root :
    - 默认情况下，若不设定此参数，pam_faillock不锁定root帐户。
    - 使用 even_deny_root 参数，pam_faillock会将应用于普通用户的相同规则应用于root用户。
    - 启用此设置可能产生严重后果，恶意用户可以锁定root用户，导致合法管理员无法访问系统。
    - ✅ 缓解方法：可使用 root_unlock_time=<second> 参数为root用户指定较短的锁定时间，对于SSH而言，可在/etc/ssh/sshd_config中 PermitRootlogin 设置为no。
  - preauth :确认用户帐户是否被锁定。
  - authfail :报告用户帐户登录失败的状态。
- 此处有图片
- ✅ PAM调用auth和account管理组中的pam_faillock模块示例：(此处有图片)

```bash
$ man 8 pam_faillock：
# 查看pam_faillock模块的详细使用信息 
```

### 管理锁定的帐户

- $ sudo faillock：列出失败的登录尝试

```bash
$ sudo faillock --user <username>
# 列出指定用户的失败登录记录
```

```bash
$ sudo faillock --user <username> --reset
# 删除指定用户的失败登录记录，但同时解锁该用户帐户。
```

- 此处有图片

- ❗faillock的用户帐户锁定与usermod -L命令的用户帐户锁定方式不同！

-----

## 补充：PAM模块使用示例

### pam_tally2模块

- 与pam_faillock模块类似，该模块同样具有多次登录失败后限制用户访问及锁定用户的功能，可分别对于本地tty终端与远程SSH登录生效。
- /etc/pam.d/login 中PAM配置示例：(此处有图片)
- 限制普通用户与root用户在多次登录失败后访问本地tty终端，并锁定其帐户，但解锁帐户时间不同。(此处有图片)
- (此处有图片)
  
  ```bash
  $ sudo pam_tally2 --user <username>
  # 查看pam_tally2模块识别的登录失败的用户信息
  ```

- /etc/pam.d/sshd 中PAM配置示例：(此处有图片)
- 限制普通用户与root用户在多次密码输入失败后进行SSH登录，并锁定其帐户，但解锁帐户时间不同。(此处有图片)
- (此处有图片)
- 示例：pam_tally2模块在用户连续输入密码错误时，将用户帐户锁定的时间倒计时。(此处有图片)
  
  ```bash
  $ man 8 pam_tally2
  # 查看pam_tally2模块的详细使用信息
  ```

### pam_access模块

- 该模块提供logdaemon登录类型守护进程的访问控制。
- /etc/pam.d/sshd 中PAM配置示例：限制用户的SSH远程登录访问 (此处有图片)
- pam_access模块配置文件：/etc/security/access.conf
- (此处有图片)
- /etc/pam.d/login 中的PAM配置示例：限制用户的本地tty终端登录 (此处有图片)
- (此处有图片)
  
  ```bash
  $ man 8 pam_access
  # 查看pam_access模块的详细使用信
  ```

### pam_listfile模块

- 该模块可基于文件允许或拒绝服务。
- /etc/pam.d/sshd 中PAM配置：(此处有图片)
- 如上所示，pam_listfile模块的onerr参数在指定的文件不存在时，将返回测试fail的结果，使用user的方式进行认证，允许指定文件中的用户SSH远程登录系统，root必须添加至文件中，否则将被拒绝登录系统！
- /var/log/secure 日志：pam_listfile与pam_tally2模块同时生效，先拒绝未指定的用户SSH远程登录系统，再对允许登录系统的用户进行密码输入正确性测试。(此处有图片)

```bash
$ man 8 pam_listfile
# 查看pam_listfile模块的详细使用信息￼ (此处有图片)
```
