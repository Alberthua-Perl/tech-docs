## Linux 系统安装与启动引导及故障排查

### 文档目录：

- 常见物理服务器远程带外口

- 不同型号物理服务器安装 Linux 的故障排查

- RHEL 中 grub 引导配置文件的说明

- RHEL 配置开机启动方式

- 系统开机自启动定义文件

- 多种系统破解 root 密码的方式

- 系统无故关机并重启原因排查方法

- 案例：DELL R730 物理机 CentOS 7.2 开机启动恢复

- 创建兼容 UEFI 与 BIOS 引导的 RHEL 6 自动化安装 ISO 镜像

- 创建兼容 UEFI 与 BIOS 引导的 CentOS 7 自动化安装 ISO 镜像

- CentOS 7 的 kernel 升级方法

### 常见物理服务器远程带外口：

- `Lenovo System x3650 M5` 服务器 `iLO 4` 管理口登录：
  
  - 服务器使用 `IBM ServeRAID M5210 RAID` 卡，iLO 管理模块为 Lenovo Intergrated Management Module II。
  
  - 生产运维接入平台登录 iLO地址，若一种类型浏览器无法打开，更换另一种浏览器打开。
  
  - 默认用户名：USERID
  
  - 默认密码：PASSW0RD

- `Huawei FushionServer RH2288 V3/2488H V5` 服务器 `iBMC` 管理口登录：
  
  - 默认用户名：root/Administrator
  
  - 默认密码：Huawei12#$
  
  - 默认 V3 BIOS 密码：Huawei12#$
  
  - 默认 V5 BIOS密码：Admin@9000

- HPE 服务器 iLO 4/5 管理口登录：
  
  - 默认用户名：bocomsimuser
  
  - 默认密码：Bocom$sim2012
  
  - `HP ProLiant DL380 Gen9` 物理服务器使用 `iLO 4` 管理接口
  
  - `HPE ProLiant DL560 Gen10` 物理服务器使用 `iLO 5` 管理接口
  
  > 👉 iLO 5 管理模式：使用 HTML 5 远程管理界面，`Ctrl+Alt+Del` 组合键重启系统。

- 浪潮服务器（inspur）管理口登录：
  
  - 默认用户名：admin
  
  - 默认密码：admin

- `H3C` 服务器 `HDM` 管理口登录：
  
  - 默认用户名：admin
  
  - 默认密码：Password@_
  
  > 👉 使用 HTML 5 远程管理界面登录

### 不同型号物理服务器安装 Linux 的故障排查：

- 通用 troubleshooting 方法：
  
  - 若物理服务器主板的引导方式为 `UEFI`，则磁盘分区方式使用 `GPT` 分区，引导分区的挂载点为 `/boot/efi`（FAT 文件系统，EFI System Partition/EFI Boot Partition），挂载设备为 `/dev/sda1`。
  
  - 若物理服务器主板的引导方式为 `BIOS`，则磁盘分区方式使用 `MSDOS` 分区，引导分区的挂载点为 `/boot`（ext3/4/xfs文件系统）。
  
  - ✅ 在新系统安装后首次引导可能无法正常完成，可将引导方式更改为 `BIOS`（Legacy Mode）或 `UEFI`（UEFI Mode）进行尝试引导。
  
  - UEFI 方式引导 RHEL 7.x 或 SLES 12 SPx，重新封装系统引导镜像：
    
    ```bash
    $ sudo grub2-mkconfig -o /boot/efi/EFI/redhat/grub.cfg
    ```
  
  - 每台物理服务器均具有一个唯一的可识别序列号，如下所示：
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\physical-server-serial-number.jpg)

- `Huawei FushionServer`：
  
  > BOOT 引导过程中按 `Delete` 进入引导配置界面。
  
  - `2488H V5` 服务器安装 RHEL 6.7 报错：
    
    - 该服务器型号不支持 RHEL 6.7，可支持 `RHEL 6.9` 以上与 `RHEL 7.3` 以上等版本。
    
    - 由于 RHEL 6.7 缺少该服务器型号的 `RAID 阵列卡驱动` 而无法识别 RAID 阵列卡，导致无法识别相应磁盘。
  
  - `2288H V5` 服务器安装 `Debian 8.11.0` 报错：
    
    - 该服务器的磁盘、RAID 阵列卡与物理网卡信息：
      
      - 磁盘阵列如下所示：
        
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\huawei-fushionserver-2288h-v5-server-disk-1.jpg)
        
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\huawei-fushionserver-2288h-v5-server-disk-2.jpg)
      
      - RAID 阵列卡型号：
        
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\huawei-fushionserver-2288h-v5-server-megaraid.jpg)
      
      - 物理网卡型号与芯片类型：
        
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\huawei-fushionserver-2288h-v5-server-nic.jpg)
        
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\huawei-fushionserver-2288h-v5-server-nic-1GbE-chip.jpg)
        
        ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\huawei-fushionserver-2288h-v5-server-nic-10GbE-chip.jpg)
      
      > ✅ 同一类 Linux 驱动程序识别相应的物理网卡芯片组（chipset）。
    
    - 导致的原因：
      
      - 该服务器上未配置额外的 `PCIe` 扩展插槽的物理网卡，因此 Debian 8.11.0 直接使用板载物理网卡连接。
      
      - 由于 Debian 8.11.0 无对应的物理网卡驱动（`i40e`），而无法识别物理网卡。
      
      - Debian 8.11.0 由于其缺少相应的 `RAID 阵列卡驱动` 而无法识别该服务器的磁盘，无法安装操作系统。
      
      > 🤔 解决思路：
      > 
      > 下载相应版本的驱动程序源代码并进行编译，将生成的 RAID 阵列卡驱动程序模块（.ko）在安装过程中导入待操作系统安装完成后，使用 update-initramfs 命令将该 RAID 卡驱动程序模块封装入 Linux 内核镜像中，重启系统即可正确识别。
  
  - `2288H V5` 服务器安装 RHEL 6.8 报错：
    
    - 该服务器已使用 BIOS 引导 MSDOS 磁盘分区格式成功安装 RHEL 6.8，但安装后首次引导无法找到引导设备。
    
    > 👉 此处的成功安装只是安装程序显示的安装完成，成功安装的标志为可正常引导进入系统。
    
    - 因此，尝试进入 BOOT 引导界面更改为 `UEFI` 引导方式，在系统安装磁盘分区过程中，切换为 `anaconda` 文本交互模式，以 UEFI 引导方式重新安装。
      
      ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\2288H-V5-disk-1.jpg)
      
      ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\2288H-V5-disk-2.jpg)
      
      ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\2288H-V5-disk-3.jpg)
    
    - 重新安装后即可正常引导启动。

- `H3C Server`：
  
  > BOOT 引导过程中按 `F7` 进入引导配置界面。
  
  - RAID 阵列卡故障后在引导设备中无法查看到系统盘。
  
  - 由于 RAID 阵列卡故障导致系统 `I/O` 错误并只读（read-only），更换 RAID 阵列卡后即可恢复正常。
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\H3C-server-disk-1.jpg)
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\H3C-server-disk-2.jpg)

- `HPE ProLiant DL560 Gen10`：
  
  BOOT 引导过程中按 `F11` 进入引导配置界面，`F9` 进入系统配置界面。

### RHEL 中 grub 引导配置文件的说明：

- RHEL 6.x 中的 grub 引导配置文件：
  
  - `/boot/grub/grub.conf`
  
  - 该配置文件的两个软链接：`/etc/grub.conf`、`/boot/grub/menu.lst`

- RHEL 7.x 中的 grub 引导配置文件：
  
  ```bash
  $ cat /boot/grub2/grub.cfg
  # RHEL 7.x grub2 引导配置文件切勿手动修改。
  $ vim /etc/default/grub
  # grub2 引导配置文件的参数
  $ grub2-mkconfig -o /boot/grub2/grub.cfg
  # 更改 /etc/default/grub 参数后，再使用 grub2-mkconfig -o 生成 grub2 配置文件。
  ```

### RHEL 配置开机启动方式：

- RHEL 6.x 的方式：
  
  ```bash
  $ vim /etc/inittab
    id:3:initdefault:
  # RHEL 6.x 以命令行方式开机
    id:5:initdefault:
  # RHEL 6.x 以图形化方式开机，必须提前安装图形化界面。
  ```

- RHEL 7.x 的方式：
  
  ```bash
  $ systemctl -f enable multi-user.target
  # RHEL 7.x 以命令行方式开机，代替 RHEL 6.x 的 /etc/inittab。
  $ systemctl -f enable graphical.target
  # RHEL 7.x 以图形化方式开机，代替 RHEL 6.x 的 /etc/inittab。
  ```

- RHEL 8.x 的方式：
  
  ```bash
  $ systemctl set-default multi-user.target
  # 以命令行方式开机登录
  $ systemctl set-default graphical.target
  # 以图形化方式开机登录
  ```

### 系统开机自启动定义文件：

- RHEL 6.x 与 RHEL 7.x 的方式：
  
  ```bash
  $ vim /etc/rc.d/rc.local    
  # RHEL 6.x 与 7.x 开机自启动定义文件，自定义脚本写入即可。
  # 注意：
  #   1. /etc/init.d：该目录存放开机自启动的自定义脚本，包括守护进程（daemon）脚本。
  #   2. RHEL 6.x/7.x 的 /etc/init.d 目录中的脚本，在系统开机过程中都将忽略其属组，
  #      均已 root 用户运行脚本，因此必须为该脚本添加可执行权限，并且该脚本在开机过程
  #      的最后执行。  
  ```

- SLES 11 SP4 的方式：
  
  ```bash
  mysuse:~ $ sudo vim /etc/init.d/after.local
  # SLES 11 SP4 开机自启动定义文件，需要自行创建，自定义脚本写入即可。
  ```

### 多种系统破解 root 密码的方式：

- RHEL 6.x 的方式（进入单用户模式）：
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel6-break-root-password-1.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel6-break-root-password-2.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel6-break-root-password-3.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel6-break-root-password-4.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel6-break-root-password-5.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel6-break-root-password-6.jpg)

- RHEL 7.x 与 RHEL 8.x 的方式：
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel7-break-root-password-1.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel7-break-root-password-2.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel7-break-root-password-3.jpg)

- SLES 11 SP4 的方式（进入单用户模式）：
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\sles11sp4-break-root-password-1.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\sles11sp4-break-root-password-2.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\sles11sp4-break-root-password-3.jpg)

- SLES 12 SP3 的方式：
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\sles12sp3-break-root-password-1.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\sles12sp3-break-root-password-2.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\sles12sp3-break-root-password-3.jpg)

### 系统无故关机并重启原因排查方法：

- 使用 `last` 命令与 `uptime` 命令查看系统开关机历史及运行时间

- 查看 `/var/log/messages` 日志确认系统开关机时间戳及发生的事件，该日志文件包含开关机过程中的所有事件记录。

- 该系统为 VMware 虚拟机，可通过虚拟化平台的日志记录进行判断。

### 案例：DELL R730 物理机 CentOS 7.2 开机启动恢复

- 问题描述：
  
  - CentOS 7.2 由于误删 `libselinux-2.5-12.el7.x86_64` 软件包而导致无法正常开机。
  
  - 使用 CentOS 7.2 ISO 镜像进入 `rescue` 救援模式依然无法恢复。

- 排查过程：
  
  - `DELL R730` 物理机使用 `F11` 按键进入 BIOS 调整引导方式：系统 ISO 镜像引导
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\dell-r730-boot-method-1.jpg)
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\dell-r730-boot-method-2.jpg)
  
  - 使用 `rescue` 救援模式的正确 shell 类型：`sh-4.2#`
    
    > 💥 注意：此时所在的环境为 ISO 镜像的 shell 环境！
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rescue-shell-1.jpg)
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rescue-shell-2.jpg)
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rescue-shell-3.jpg)
  
  - 从其他正常运行的节点上拷贝 `libselinux` 软件包文件至故障系统的 `/mnt/sysimage` 中的相应目录。
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\recover-boot-error-1.jpg)
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\recover-boot-error-2.jpg)
  
  - 切换至故障系统环境中验证系统是否恢复
  
  - 系统恢复后需重新安装 libselinux-2.5-12.el7.x86_64 软件包

### 创建兼容 UEFI 与 BIOS 引导的 RHEL 6 自动化安装 ISO 镜像：

- 使用场景：
  
  由于客户的现场环境中无法提供 PXE 与 DHCP 的方式进行自动化的安装，因此封装可自动化安装系统的 ISO 镜像实现大规模系统的自动化部署。

- 由于 `genisoimage` 软件版本问题，需使用 `CentOS 7.x` 或 `RHEL 7.x` 封装 RHEL 6.x 自动化安装 ISO 镜像，否则存在操作系统自动化安装后，无法使用 UEFI 引导的问题。

- 以 `RHEL 6.8/6.9` 为例进行创建与测试安装。

- 创建 RHEL 6.x 自动化安装 ISO 镜像：
  
  ```bash
  $ mkdir /mnt/{rhel6.8-tmp-mp,rhel6.8-auto-inst-iso}
  # 创建 RHEL 6.8 ISO 镜像挂载点与自动化安装 ISO 镜像的编辑目录
  $ mount -o loop -t iso9660 \
    /root/RHEL-6.8-Server-dvd-x86_64.iso /mnt/rhel6.8-tmp-mp
  # 挂载 RHEL 6.8 ISO 镜像
  $ cp -avr /mnt/rhel6.8-tmp-mp/* /mnt/rhel6.8-auto-inst-iso
  # 将 RHEL 6.8 ISO 镜像中的所有文件全部拷贝至编辑目录中，隐藏文件无法拷贝。
  $ cd /mnt/rhel6.8-auto-inst-iso
  # 切换至自动化安装 ISO 镜像的编辑目录中
  $ cp -avr /mnt/rhel6.8-tmp-mp/.discinfo /mnt/rhel6.8-tmp-mp/.treeinfo .
  # 拷贝隐藏文件至编辑目录中
  $ cp -av /root/anaconda-ks-custom-rhel6.cfg ks.cfg
  # 拷贝自定义安装的 Kickstart 脚本至编辑目录中（UEFI 与 BIOS 引导安装均可使用该脚本）
  # 该 Kickstart 脚本需提前配置完成
  ```
  
  以上的 Kickstart 脚本可参考 [该 GitHub 链接](https://github.com/Alberthua-Perl/scripts-confs/blob/master/kickstart/ks-rhel6-bankcomm.cfg)。
  
  ```bash
  $ vim isolinux/isolinux.cfg
  # 编辑 BIOS 引导安装的配置文件
  ```
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\ks-bios-isolinux-cfg.jpg)
  
  ```bash
  $ vim EFI/BOOT/BOOTX64.conf
  # 编辑 UEFI 引导安装的配置文件
  ```
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\ks-uefi-isolinux-cfg.jpg)
  
  ```bash
  $ genisoimage -U -r -v -T -J -joliet-long \
    -b isolinux/isolinux.bin -c isolinux/boot.cat \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    -eltorito-alt-boot -e images/efiboot.img -no-emul-boot \
    -o ../RHEL-6.8-Server-auto-inst.x86_64.iso .
  # 使用 genisoimage 命令封装 RHEL 6.8 自动化安装 ISO 镜像，该封装过程未指定
  # ISO 镜像 LABEL 标签。
  ```
  
  👉 若使用 `efi` 引导启动，genisoimage 封装必须配置如下参数：
  
   `-eltorito-alt-boot -e images/efiboot.img -no-emul-boot`

- 测试 RHEL 6.8 自动化安装 ISO 镜像：
  
  - 配置 RHEL 6.8 以 UEFI 引导方式启动：
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel68-uefi-autoboot-test-1.jpg)
  
  - RHEL 6.8 自动化安装过程：UEFI 引导方式为例
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel68-uefi-autoboot-test-2.jpg)
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel68-uefi-autoboot-test-3.jpg)
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\rhel68-uefi-autoboot-test-4.jpg)

### 创建兼容 UEFI 与 BIOS 引导的 CentOS 7 自动化安装 ISO 镜像：

- 使用场景：
  
  由于客户的现场环境中无法提供 PXE 与 DHCP 的方式进行自动化的安装，因此封装可自动化安装系统的 ISO 镜像实现大规模系统的自动化部署。

- 以 `CentOS 7.4` 为例进行创建与测试安装。

- 创建 CentOS 7.x 自动化安装 ISO 镜像：
  
  ```bash
  $ mkdir /mnt/{centos7.4-tmp-mp,centos7.4-auto-inst-iso}
  # 创建 CentOS 7.4 ISO 镜像挂载点与自动化安装 ISO 镜像的编辑目录
  $ mount /root/CentOS-7-x86_64-DVD-1708.iso /mnt/centos7.4-tmp-mp
  # 挂载 CentOS 7.4 ISO 镜像
  $ cp -avr /mnt/centos7.4-tmp-mp/* /mnt/centos7.4-auto-inst-iso
  # 将 CentOS 7.4 ISO 镜像中的所有文件全部拷贝至编辑目录中，不拷贝隐藏文件。
  $ cd /mnt/centos7.4-auto-inst-iso
  # 切换至自动化安装 ISO 镜像的编辑目录中
  $ cp -av /root/anaconda-ks-custom-centos7.cfg ks.cfg
  # 拷贝自定义安装的 Kickstart 脚本至编辑目录中（UEFI 与 BIOS 引导安装均可使用该脚本）
  # 该 Kickstart 脚本需提前配置完成
  ```
  
  以上的 Kickstart 脚本可参考 [该 GitHub 链接](https://github.com/Alberthua-Perl/scripts-confs/blob/master/kickstart/ks-centos7.cfg)。
  
  ```bash
  $ vim isolinux/isolinux.cfg
  # 编辑 BIOS 引导安装的配置文件
  ```
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\ks-centos74-bios-isolinux-cfg.jpg)
  
  ```bash
  $ vim EFI/BOOT/grub.cfg
  # 编辑 UEFI 引导安装的配置文件
  ```
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\ks-centos74-uefi-isolinux-cfg.jpg)
  
  ```bash
  $ genisoimage -U -r -v -T -J -joliet-long \
    -b isolinux/isolinux.bin -c isolinux/boot.cat \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    -eltorito-alt-boot -e images/efiboot.img -no-emul-boot \
    -V 'CentOS-74-ks-auto-inst' -o ../CentOS-7.4-Server-auto-inst-x86_64.iso .
  # 使用 genisoimage 命令封装 CentOS 7.4 自动化安装ISO镜像，该封装过程需指定 ISO 镜像
  # LABEL 标签。 
  ```
  
  👉 若使用 `efi` 引导启动，genisoimage 封装必须配置如下参数：
  
  `-eltorito-alt-boot -e images/efiboot.img -no-emul-boot`

- CentOS 7.4 自动化安装过程：UEFI 引导方式为例
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\centos74-uefi-autoboot-test-1.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\centos74-uefi-autoboot-test-2.jpg)
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\centos74-uefi-autoboot-test-3.jpg)

- 使用 UEFI/BIOS 引导的 CentOS 7.x 自动化安装 ISO 镜像，`dracut` 报错如下：
  
  由于未发现安装引导介质所致，可使用 `inst.repo` 参数或添加 ISO 镜像标签解决。
  
  ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Linux系统安装与启动引导要点及故障排查\kickstart-autoinstall-dracut-error.jpg)

### 🚀 CentOS 7 的 kernel 升级方法：

- 该场景中将 CentOS 7.4 的 kernel `3.10.x` 升级至 `4.4.x`。

- kernel 软件包类型：
  
  - 主分支：`mainline`（ml）
  
  - 稳定版：`stable`
  
  - 长期维护：`longterm`（lt）

- kernel 版本命名格式：`A.B.C`
  
  - A：内核版本号
  
  - B：内核主版本号
  
  - C：内核次版本号

- 使用 RPM 软件包方式的 kernel 升级过程：
  
  ```bash
  $ rpm -import https://www.elrepo.org/RPM-GPG-KEY-elrepo.org
  # 将 RPM GPG 公钥导入 RPM 本地数据库 
  $ wget -O /etc/pki/rpm-gpg/RPM-GPG-KEY-elrepo.org \
    https://www.elrepo.org/RPM-GPG-KEY-elrepo.org
  # 下载 RPM GPG 公钥至 RPM GPG 公钥目录中  
  $ rpm -Uvh http://www.elrepo.org/elrepo-release-7.0-2.el7.elrepo.noarch.rpm
  # 升级安装 elrepo 软件源 
         
  $ yum --disablerepo="*" --enablerepo=elrepo-kernel list available
  # 启用 elrepo-kernel 软件源查看可用的 kernel 软件包 
  $ yum --enablerepo=elrepo-kernel install -y kernel-lt kernel-lt-devel
  # 启用 elrepo-kernel 软件源安装 kernel-lt 软件包   
  
  $ awk -F\' '$1=="menuentry " {print $2}' /boot/grub2/grub.cfg
  # 查看可引导的 kernel 条目 
  $ vim /etc/default/grub
    GRUB_DEFAULT=0
  # 设置最新的 kernel 为默认引导项
   
  $ grub2-mkconfig -o /boot/grub2/grub.cfg
  # 重新生成 GRUB 引导项 
  $ reboot
  ```
