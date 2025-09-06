# Kimi 提问汇总

## Linux 内核

- [源码分析] Linux 从加电启动 → start kernel → 用户空间？
- Linux 内核模块参数设置后，没有在 parameters 中显示，是什么原因？module_param 声明在哪里查看？
- 如何理解 Linux 中 dmesg 命令的 ring buffer？
- Linux kernel 自身的内存管理在物理内存上是如何组织分布的？
- Linux kernel 自身运行所在的内存与用户空间程序运行所在的内存有何区别？
- Linux kernel 如何为自己分配内存? 如何为用户空间进程分配内存?
- 请详细描述从 CR3 寄存器开始的线性地址到物理地址的寻址过程?
- 上述 CR3 寄存器是什么结构?
- Linux kernel 是如何操作 MMU 和 TLB，请详述过程？

## Linux 系统编程

- 两个二进制数如何相加?
- 如何理解补码？
- 如何理解 C 语言中 short int 类型 32767 加 1 变成 -32768?

## Linux 性能优化

- Linux 中 PCP 的 pmlogger 默认是采集所有的性能指标吗？是否可以自定义只需要的性能指标，并且采集的时间间隔能指定吗？

## 系统组件

- systemd 的 service 单元文件 [Service] 部分中的 Type 类型介绍？
- D-Bus 框架的工作原理？它与 socket 编程有何不同？
- 如何编写 2 个 python 程序利用 D-Bus 框架进行通信？
- seccomp 组件的功能？seccomp 配置文件介绍？
- xfs_undelete 工具的安装 (git, tcl, tcllib)？
- xfs 文件系统中删除文件或目录的恢复示例？
- xfs 文件系统的结构解析？
- xfs 文件系统是否存在备份的 superblock？
- superblock 损坏后，如何使用从 AG 中提取 superblock 信息修复文件系统？
- nmap 扫描端口报错？
  Note: Host seems down. If it is really up, but blocking our ping probes, try -Pn
  Nmap done: 1 IP address (0 hosts up) scanned in 0.08 seconds

## 硬件监测

- 如何理解磁盘固件中的 S.M.A.R.T 子系统？这种固件的形式或者说源码是如何实现的？
- Linux 中 smartctl 命令检查磁盘健康状态？
- Linux 中 HBA 卡队列深度 (io_depth) 过小导致系统宕机，该如何解决？
- 如何根据系统日志定位 Linux 中内存 ECC 的故障插槽，请给出具体的日志与分析过程？
- Linux kernel 的 EDAC 子系统是什么？
  - 与 MCE 的区别 (mcelog)
  - MCE (Machine Check Exception) 是 CPU 抛出异常，告诉你“有错误”。
  - EDAC 会进一步告诉你“错误发生在哪根 DIMM、哪个通道”，定位粒度更细。
  
  ```bash
  $ sudo grep "[0-9]" /sys/devices/system/edac/mc/mc*/csrow*/ch*_ce_count
  /sys/devices/system/edac/mc/mc0/csrow2/ch0_ce_count:42457
  /sys/devices/system/edac/mc/mc0/csrow2/ch1_ce_count:0
  /sys/devices/system/edac/mc/mc0/csrow3/ch0_ce_count:9630
  /sys/devices/system/edac/mc/mc0/csrow3/ch1_ce_count:0
  /sys/devices/system/edac/mc/mc1/csrow2/ch0_ce_count:0
  /sys/devices/system/edac/mc/mc1/csrow2/ch1_ce_count:20229
  /sys/devices/system/edac/mc/mc1/csrow3/ch0_ce_count:0
  /sys/devices/system/edac/mc/mc1/csrow3/ch1_ce_count:31858
  # 开启 EDAC 服务或加载 EDAC 内核模块后
  # count 不为 0 的行即代表存在内存错误
  # mc*: 第好多个CPU
  # csrow*：内存通道
  # ch*：通道内的第几根内存
  ```
     
- 什么是 Linux kernel 中的 NMI watchdog（不可屏蔽中断看门狗）？

## 系统服务

- LDAP：
  - 本身是一种协议
  - LDAP 的实现：OpenLDAP (Community), 389ds/389-ds-base (redhat)
  - Kerberos 身份认证服务：OpenLDAP+Kerberos, 389-ds-base+Kerberos
  - 集成解决方案：FreeIPA (Community) ==> RedHat IdM (redhat)
- RH342v8.4 课程环境中 sssd 实验存在问题，需要排错。
- HAProxy：
  - haproxy.cfg 配置文件参数说明 (option 参数、timeout 参数、listen 段)

## 系统安全

- OpenSSL 中的 3DES 对称加密算法中 PBKDF2 是什么？老式的 EVP_BytesToKey 是什么？

## Ceph

- 如何理解 Ceph MDS 缓存以及 MDS 客户端的缓存一致性？

## Jenkins
  
- Jenkinsfile 中的 script { ... } 块作用是什么？与直接使用 sh 有何区别？
- 安装 Pipeline Stage View 查看流程方块视图
- Jenkins 中的凭据插件：
  - Credentials Binding Plugin: withCredentials 指令 (动态使用)
- Credentials Plugin: checkout 步骤 credentialsID (静态使用)

## Ansible

- python 如何调用 ansible-playbook？ansible-runner 如何使用？

## Kubernetes

- 请给出一个 Kubernetes Operator 的实现的具体示例
- Kubernetes 从哪个版本开始支持 Tekton 与 ArgoCD？  

## 机器学习

- 函数 y=1/(1+e^(-x)) 求导？
- 超平面与法向量详细介绍？
- 如何求直线方程与平面方程的法向量？
- 点到超平面距离公式如何推导？超平面表达式如何推导？
- 向量内积的柯西·施瓦茨不等式？
- [十大回归算法与代码示例](https://mp.weixin.qq.com/mp/appmsgalbum?action=getalbum&__biz=MzkwNjY5NDU2OQ==&scene=1&album_id=3510733534568808456&count=3#wechat_redirect)
- 详细描述线性回归的训练过程？
- 最小二乘法的理解及推导过程？
- 举例说明最小二乘法的具体步骤？
- 均方根误差是不是通过最小二乘法推导而来的？
- 如何使用 python 绘制曲面与平面相切及其切点？
- 机器学习中的 AC、SN、MCC、MSE、RMSE？
- NumPy 数组表示的矩阵维度，请给出具体示例？
- 如何修改 ipynb 的字体以及安装 Git 扩展？
- 误差反向传播算法 (BP) 的详细数学推导？

## 深度学习

- tensorflow.keras.datasets 下载的数据存放路径是什么？
- 下采样操作是什么？
- CNN 训练过程中反向传播在哪些阶段实现？
- CNN 中每个卷积层中的不同卷积核的值是如何确定的？
- 为什么激活函数可以有效解决梯度消失问题？
- 深度学习中为什么会出现梯度消失与梯度爆炸，请详细说明原因？

## 大模型

- 位置编码与词向量化哪个先执行？
- LLM 提取 token 的 python 示例 (各大模型的方式存在差异)？
- tokenizer 不属于 Transformer 的一部分，而是每个模型实现的一部分，是吗？
- 为什么每个 Transformer 模型都需要一个特定的 tokenizer？
- BPE 是什么？
  