# 🔑 OpenSSL 体系与证书管理

## 文档目录

- [🔑 OpenSSL 体系与证书管理](#-openssl-体系与证书管理)
  - [文档目录](#文档目录)
  - [1. openssl 命令说明](#1-openssl-命令说明)
  - [2. openssl req 命令](#2-openssl-req-命令)
  - [3. openssl rsa 命令](#3-openssl-rsa-命令)
  - [4. openssl x509 命令](#4-openssl-x509-命令)
  - [5. 创建自签名数字证书的方法](#5-创建自签名数字证书的方法)
  - [🎯 6. 创建 CA RSA 私钥与 CA 根证书（root-ca）](#-6-创建-ca-rsa-私钥与-ca-根证书root-ca)
  - [🎉 7. 基于 CA 根证书创建 server 端数字签名证书](#-7-基于-ca-根证书创建-server-端数字签名证书)
  - [🎉 8. 基于 CA 根证书创建 client 端数字签名证书](#-8-基于-ca-根证书创建-client-端数字签名证书)
  - [🍵 9. 使用数字签名证书实现单双向连接测试](#-9-使用数字签名证书实现单双向连接测试)
    - [9.1 使用 server 端数字签名证书进行单向连接测试](#91-使用-server-端数字签名证书进行单向连接测试)
    - [9.2 使用 server 端与 client 端数字签名证书进行双向连接测试](#92-使用-server-端与-client-端数字签名证书进行双向连接测试)
  - [🎯 10. 故障排查示例](#-10-故障排查示例)
    - [10.1 撤销证书签发异常的证书](#101-撤销证书签发异常的证书)
    - [10.2 总结](#102-总结)

## 1. openssl 命令说明

证书私钥（RSA 私钥）、证书签名请求（CSR）与证书（X509）均存在 **编码形式**（encoded）与人类可读的 **纯文本形式**（text），因此可使用 `-noout` 选项不打印编码形式，`-text` 选项打印纯文本形式。仅使用 `-noout` 选项时，编码形式与纯文本形式均不打印，而两者搭配使用仅返回纯文本形式内容。

## 2. openssl req 命令

- 功能：创建或查询 `csr` 证书签名请求与数字签名证书（或自签名证书）
- 常用选项：
  
  ```bash
  -noout             不打印证书签名请求的编码（encoded）版本
  -text              纯文本形式打印证书签名请求
  -verify            验证请求中的自签名信息

  -key               指定用于创建 csr 证书签名请求的私钥
  -newkey alg:nbits  创建新的 csr 证书签名请求与私钥，指定加密算法与加密位数（通常为 rsa:2048）。             
  -nodes             不使用密码为新创建的私钥加密
  -keyout            指定新创建私钥的文件名
  -sha256            使用 SHA-256 摘要（创建自签名数字证书时使用）
  -subj              指定创建 csr 证书签名请求与数字签名证书所需的详细信息如果未指定该选项，将进入交互模式。
  -new               生成新的 csr 证书签名请求
  -x509              生成数字签名证书（不生成证书签名请求）
  -days              指定数字签名证书的合法时间（有效期），默认 30 天。
  -out               指定输出的 csr 证书签名请求或数字签名证书的名称
  ```

- 常用命令：

  ```bash
  $ openssl req -in /path/to/<req_file>.csr -noout -verify
  # 仅验证请求中的自签名信息
  $ openssl req -in csr/inter-ca.csr.pem -noout -verify
  Certificate request self-signature verify OK

  $ openssl req -in /path/to/<req_file>.csr -noout -text -verify
  # 验证请求中的自签名信息，并返回文本形式的请求内容。
  ```

## 3. openssl rsa 命令

- 常用选项：

  ```bash
  -noout           不打印密钥的编码版本
  -text            纯文本形式打印出各种公钥或私钥的组成部分
  -check           检测证书私钥的一致性
  ```

- 常用命令：

  ```bash
  $ openssl rsa -in /path/to/<key_file>.key
  # 仅返回编码形式的 RSA 私钥内容（-----BEGIN/END----- 块）
  # 注意：如果此私钥已使用密码（passphrase）加密，那么需输入密码才能查看。
  $ openssl rsa -in root-ca/private/root-ca.key.pem
  Enter pass phrase for root-ca/private/root-ca.key.pem: <passphrase>
  writing RSA key
  ...
  # 查看 RSA 私钥时需输入解密密码

  $ openssl rsa -in /path/to/<key_file>.key -noout -text
  # 不返回编码形式的证书私钥内容：仅返回 RSA 私钥摘要内容

  $ openssl rsa -in /path/to/<key_file>.key -noout -check
  # 检测 RSA 私钥的一致性
  $ openssl rsa -in root-ca/private/root-ca.key.pem -noout -check
  Enter pass phrase for root-ca/private/root-ca.key.pem:
  RSA key ok
  ```  

## 4. openssl x509 命令

- 功能：创建与查询数字签名证书
- 常用选项：
  
  ```bash
  -in              指定输入的证书文件  
  -noout           不打印编码的 PEM 证书本身（省去 -----BEGIN/END----- 块）
  -text            纯文本形式打印证书详情（版本、序列号、有效期、公钥、扩展等）

  -subject         打印主题的名称
  -dates           打印输出证书起始与过期的日期（有效期）
  -req             指定 csr 证书签名请求，与 -in 选项合用。                
  -CAkey           指定用于签署证书的 CA 私钥
  -CA              指定用于签署证书的 CA 根证书
  -CAcreateserial  创建 CA 序列号文件，扩展名为 ".srl"，该选项必须与 -CA 选项合用。
  -days            指定数字签名证书的合法时间（有效期），默认 30 天，不与 -preserve_dates 选项合用。
  -out             指定输出的数字签名证书名称
  ```

- 常用命令：

  ```bash
  $ openssh x509 -in /path/to/<cert_file>.crt
  # 仅返回编码的 PEM 证书本身（-----BEGIN/END----- 块）

  $ openssl x509 -in /path/to/<cert_file>.crt -noout -text 
  # 不打印编码的 PEM 证书：纯文本形式打印证书详情（版本、序列号、有效期、公钥、扩展等）

  $ openssl x509 -in /path/to/<cert_file>.crt -noout -subject
  # 不打印编码的 PEM 证书：打印证书的主题信息

  $ openssl x509 -in /path/to/<cert_file>.crt -noout -dates
  # 不打印编码的 PEM 证书：打印证书的起始与过期的日期（有效期）

  $ openssl x509 -in /path/to/<cert_file>.crt -noout -serial
  # 不打印编码的 PEM 证书：查看证书的序列号

  $ openssl x509 -in /path/to/<cert_file>.crt -noout -pubkey
  # 不打印编码的 PEM 证书：查看证书中的公钥信息，该公钥与 RSA 私钥中提取的公钥一致。

  $ openssl verify -CAfile root-ca.cert.pem inter-ca.cert.pem 
  inter-ca.cert.pem: OK
  # 验证证书链
  ```

## 5. 创建自签名数字证书的方法

- 1️⃣ 先创建私钥再创建自签名数字证书
- 2️⃣ 同时创建私钥与自签名数字证书

  ```bash
  $ openssl req \
    -newkey rsa:4096 -nodes -keyout server.key \
    -sha256 -subj "/C=CN/ST=Shanghai/L=Shanghai/O=RedHat/OU=GLS/CN=cloud-ctl.lab.example.com" \
    -x509 -days 3650 \
    -out server.crt
  ```

## 🎯 6. 创建 CA RSA 私钥与 CA 根证书（root-ca）
  
```bash
#步骤1
$ openssl genrsa [-des3] -out ca.key [1024|2048|4096]
# 创建 CA RSA 私钥
# -des3 选项：交互输入密码为 RSA 私钥加密

#步骤2
$ openssl req \
  -key ca.key \
  -subj "/C=CN/ST=Shanghai/L=Shanghai/O=RedHat/OU=GLS/CN=CA-center.lab.example.com" \
  -new -x509 -days 3650 \
  -out ca.crt
# 创建 CA 自签名根证书

#步骤1+步骤2
$ openssl req \
  -x509 \
  -newkey rsa:2048 -nodes \
  -keyout ca.key -out ca.crt \
  -days 3650 \
  -subj "/C=CN/ST=Shanghai/L=Shanghai/O=RedHat/OU=GLS/CN=CA-center.lab.example.com"
# 同时创建 CA RSA 私钥与 CA 自签名根证书
  
$ openssl rsa -in ca.key -noout -text
# 查看 CA RSA 私钥的纯文本信息
  
$ openssl x509 -in ca.crt -noout -text
# 查看 CA 根证书的纯文本摘要信息
```

## 🎉 7. 基于 CA 根证书创建 server 端数字签名证书
  
```bash
#步骤1
$ openssl genrsa [-des3] -out server.key [1024|2048|4096]
# 创建 server 端 RSA 私钥
  
$ openssl rsa -in server.key -pubout -out server.pub
# 提取 server 端 RSA 私钥对应的公钥（公钥由私钥中提取）

#步骤2
$ openssl req \
  -key server.key \
  -subj "/C=CN/ST=Shanghai/L=Shanghai/O=RedHat/OU=GLS/CN=cloud-ctl.lab.example.com" \
  -new \
  -out server.csr
# 创建 server 端证书签名请求（certificate signing request）
# 注意：
#   1. 创建 csr 不使用 -x509 与 -days 选项
#   2. 在创建生成服务端与客户端证书签名请求时均要注意以下三点：
#      a. CA 根证书的 Common Name (CN) 填写 root 即可，所有服务端和客户端的证书该字段需要填写 IP 或域名。
#      b. 一定要注意的是，CA 根证书的该字段和服务端证书、客户端证书不能一样。
#      c. 其他所有字段的填写，CA 根证书、服务端证书、客户端证书需保持一致，最后的密码可直接回车跳过。

#步骤1+步骤2：不使用 -x509 与 -days 选项
$ openssl req \
  -newkey rsa:2048 -nodes \
  -keyout server.key -out server.csr \
  -subj "/C=CN/ST=Shanghai/L=Shanghai/O=RedHat/OU=GLS/CN=cloud-ctl.lab.example.com"

$ openssl req -in server.csr -noout -text
# 查看 server 端纯文本证书签名请求信息
  
$ openssl x509 \
  -req -in server.csr \
  -CAkey ca.key -CA ca.crt -CAcreateserial \
  -days 3650 \
  -out server.crt
# 创建 server 端数字签名证书
#            -req 选项：输入 CSR 证书签名请求
#             -in 选项：待签名的 CSR 文件
#          -CAkey 选项：CA 的私钥（用来签名）
#             -CA 选项：CA 的自身证书（签名者身份）
# -CAcreateserial 选项：自动生成序列号文件（ca.srl）
#           -days 选项：证书有效期 10 年
#            -out 选项：输出服务器证书
```

## 🎉 8. 基于 CA 根证书创建 client 端数字签名证书
  
```bash
$ openssl genrsa [-des3] -out client.key [1024|2048|4096]
# 创建 client 端 RSA 私钥
  
$ openssl req \
  -key client.key \
  -subj "/C=CN/ST=Shanghai/L=Shanghai/CN=sec-srv.lab.example.com" \
  -new \
  -out client.csr
# 创建 client 端证书签名请求
  
$ openssl x509 \
  -req -in client.csr \
  -CAkey ca.key -CA ca.crt -CAcreateserial \
  -days 3650 \
  -out client.crt
# 创建 client 端数字签名证书
```

## 🍵 9. 使用数字签名证书实现单双向连接测试

### 9.1 使用 server 端数字签名证书进行单向连接测试
  
```bash
$ openssl s_server -accept <port> -key server.key -cert server.crt    
# server 端：启动单向安全连接，启动后将等待 client 端发送信息并回显。

$ openssl s_client -connect <host_ip>:<port>
# client 端：连接 server 端，如果连接成功后将在任意一端输入信息后会在另一端显示该信息。
```

### 9.2 使用 server 端与 client 端数字签名证书进行双向连接测试
  
```bash
$ openssl s_server \
  -accept <port> \
  -key server.key -cert server.crt \
  -Verify <N>
# server 端：强制要求 client 端提供私钥与 client 端数字签名证书进行安全连接
### 示例 ###
$ openssl s_server \
  -accept 10001 \
  -key server.key -cert server.crt \
  -Verify 5
  
$ openssl s_client \
  -connect <host_ip>:<port> \
  -key client.key -cert client.crt
# client 端：连接 server 端，如果连接成功后将在任意一端输入信息后会在另一端显示该信息。
### 示例 ###
$ openssl s_client \
  -connect 10.197.11.100:10001 \
  -key client.key -cert client.crt
```

- 💥 注意：
  - 以上命令中的 `-Verify <N>` 选项表示验证证书链深度，即从客户端证书（不包含）至 CA 根证书（包含）之间的证书签发数量。
  - **末端实体证书（End-entity certificate）**：客户端（或服务端）申请的证书
  - 因此，`N = 中间证书数量 + 1 个 CA 根证书`。
  - 可参考如下所示：5 层深度
    - 客户端证书（末端实体证书） → 不计入深度 <br>
      ↓ 1️⃣ 第一层中间 CA <br>
      ↓ 2️⃣ 第二层中间 CA <br>
      ↓ 3️⃣ 第三层中间 CA <br>
      ↓ 4️⃣ 第四层中间 CA <br>
      ↓ 5️⃣ 根 CA（Top-level）
    - 以上表示 5 层证书链深度，如果再增加一层中间层 CA，那么客户端连接是将验证失败，返回报错 `X509_V_ERR_CERT_CHAIN_TOO_LONG`。

## 🎯 10. 故障排查示例

> 说明：参考 RH362v9.1 课程 Chapter2 Section3 Lab

### 10.1 撤销证书签发异常的证书

root-ca/ 目录与 inter-ca/ 目录中分别存储了根 CA 证书与中间 CA 证书的相关文件，当前的证书链可验证通过，即 `root-ca.cert.pem -> inter-ca.cert.pem -> server001.lab.example.com.cert.pem`。但通过 openssl ca 命令的 `-extensions v3_inter_ca` 选项重新生成中间 CA 证书后，其 subject 主体发生改变，导致 server001.lab.example.com.cert.pem 在证书链验证过程中失败。因此，撤销此服务端证书（末端实体证书）再通过 inter-ca.cert.pem 重新签发此证书即可完成证书链的验证。新的证书链验证过程：`root-ca.cert.pem -> inter-ca.cert.pem -> service.lab.example.com.cert.pem`

1️⃣ 证书信息查询与验证：

```bash
## root@workstation
$ cd /root/inter-ca
$ openssl x509 -in certs/inter-ca.cert.pem -noout -text
#查看证书摘要，不显示 PEM 块。
$ openssl rsa -in private/inter-ca.key.pem -noout -check
Enter pass phrase for private/inter-ca.key.pem: InterCA123^
RSA key ok
#检查 RSA 私钥的加密密钥，不显示 PEM 块。

$ openssl req -in csr/inter-ca.csr.pem -noout -text -verify
Certificate request self-signature verify OK
#验证证书签名请求

$ cd /root/root-ca
$ openssl ca -config openssl.cnf \
  -notext -md sha256 -extensions v3_inter_ca -days 1825 \
  -in ../inter-ca/csr/inter-ca.csr.pem \
  -out ../inter-ca/certs/inter-ca.cert.pem
Using configuration from openssl.cnf
Enter pass phrase for /root/root-ca/private/root-ca.key.pem:
Check that the request matches the signature
Signature ok
Certificate Details:
        Serial Number: 4096 (0x1000)
        Validity
            Not Before: Jan  2 09:06:14 2026 GMT
            Not After : Jan  1 09:06:14 2031 GMT
        Subject:
            countryName               = US
            stateOrProvinceName       = North Carolina
            organizationName          = Red Hat
            organizationalUnitName    = Learning
            commonName                = RH362 intermediate CA
        X509v3 extensions:
            X509v3 Subject Key Identifier:
                43:F5:73:DB:EF:BF:81:14:54:72:73:C5:43:C1:32:EA:BC:5F:AA:56
            X509v3 Authority Key Identifier:
                AB:28:1A:34:21:FD:B1:DA:3F:05:21:47:5A:AB:08:6F:07:04:92:73
            X509v3 Basic Constraints: critical
                CA:TRUE, pathlen:0
            X509v3 Key Usage: critical
                Digital Signature, Certificate Sign, CRL Sign
Certificate is to be certified until Jan  1 09:06:14 2031 GMT (1825 days)
Sign the certificate? [y/n]:y


1 out of 1 certificate requests certified, commit? [y/n]y
Write out database with 1 new entries
Data Base Updated
#根 CA 目录中更新中间 CA 证书（subject 主体发生变化）
#各选项说明：
#  -config openssl.cnf：指定 CA 配置文件（含数据库、扩展段、路径等）
#  -md sha256：用 SHA-256 做签名摘要（默认 SHA-1 已不安全）
#  -extensions v3_inter_ca：引用配置文件中 `[ v3_inter_ca ] 扩展段，把 KeyUsage、BasicConstraints 等扩展写进证书，使其成为「中间 CA」。
#  -days 1825：证书有效期 1825 天（≈ 5 年）
#  -notext：不输出人类可读的文本形式（只生成 DER/PEM 编码，减少体积）
#  -in ../inter-ca/csr/inter-ca.csr.pem：待签名的中间 CA 证书请求文件
#  -out ../inter-ca/certs/inter-ca.cert.pem：输出最终签好的中间 CA 证书

$ chmod 0444 ../inter-ca/certs/inter-ca.cert.pem
#更改中间 CA 证书权限
$ openssl verify -CAfile certs/root-ca.cert.pem ../inter-ca/certs/inter-ca.cert.pem
../inter-ca/certs/inter-ca.cert.pem: OK
#根据 CA 根证书验证中间 CA 证书（证书链完整），即根 CA 证书签发中间 CA 证书。
```

2️⃣ 撤销服务端证书（末端实体证书）并新建服务端证书：

```bash
## root@workstation
$ cd /root/inter-ca
$ echo 1000 > crlnumber  #创建 CRL（证书撤销列表）的版本号计数器文件，即 openssl 每次使用 -gencrl 选项时，此文件版本号 +1。
$ openssl ca -config openssl.cnf -gencrl -out crl/inter-ca.crl.pem  #生成 CRL 文件记录撤销的证书
$ openssl crl -in crl/inter-ca.crl.pem -noout -text  #查看 CRL 文件中的撤销证书列表，当前无任何撤销证书。
...
No Revoked Certificates.                
...

$ openssl ca -config openssl.cnf -revoke certs/server001.lab.example.com.cert.pem  #撤销指定的服务端证书
$ openssl ca -config openssl.cnf -gencrl -out crl/inter-ca.crl.pem  #重新生成 CRL 文件，此时记录了已撤销的证书，同时记录在数据库文件 index.txt 中。
$ openssl crl -in crl/inter-ca.crl.pem -noout -text
...
Revoked Certificates:
    Serial Number: 080B43735788EC331DC000F945C6140104BB827B
...
$ cat index.txt
R       260629170552Z   260102170714Z   080B43735788EC331DC000F945C6140104BB827B        unknown /C=US/ST=North Carolina/L=Raleigh/O=Red Hat/OU=Learning/CN=server001.lab.example.com
#以上两条命令的输出中均包含已撤销的证书序列号

$ openssl genrsa -out private/service.lab.example.com 2048  #生成新证书的密钥
$ chmod 0400 private/service.lab.example.com  #更改密钥文件权限
$ openssl req -config openssl.cnf \
  -new -sha256 \
  -key private/service.lab.example.com \
  -out csr/service.example.com.csr.pem
#创建新证书的 CSR 请求
$ openssl ca -config openssl.cnf \
  -notext -md sha256 -extensions end_entity_cert -days 365 \
  -in csr/service.example.com.csr.pem \
  -out certs/service.lab.example.com.cert.pem
#创建服务端证书（末端实体证书）
$ chmod 0444 certs/service.lab.example.com.cert.pem  #更改证书权限
$ openssl verify -CAfile certs/ca-chain.cert.pem certs/service.lab.example.com.cert.pem  #使用证书链文件（包含根 CA 证书在内的所有证书）验证服务端证书
```

### 10.2 总结

证书撤销与重新生成过程：

- 创建 crlnumber（版本号计数器文件）
- 创建 *.crl.pem 撤销证书列表
- 使用 openssl ca -revoke 选项撤销证书
- 上述命令自动更新 index.txt，写入 R (撤销) 状态的证书序列号。
- 自动根据 index.txt 重新生成 *.crl.pem 撤销证书列表（此文件中 Revoked 列表中显示）
- crlnumber 文件版本递增 +1，*.crl.pem 文件中存在撤销的证书。
