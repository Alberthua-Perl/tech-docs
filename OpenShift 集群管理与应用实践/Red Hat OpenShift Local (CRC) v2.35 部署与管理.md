# ⭕ Red Hat OpenShift Local (CRC) v2.35 部署与管理

- [⭕ Red Hat OpenShift Local (CRC) v2.35 部署与管理](#-red-hat-openshift-local-crc-v235-部署与管理)
  - [1. Lab 环境说明](#1-lab-环境说明)
  - [2. 采用 CRC 部署 Red Hat OpenShift Local（本地开发环境）](#2-采用-crc-部署-red-hat-openshift-local本地开发环境)
    - [2.1 设置 CRC 数据采集遥测（telemetry）](#21-设置-crc-数据采集遥测telemetry)
    - [2.2 设置 CRC 运行时](#22-设置-crc-运行时)
    - [2.3 查看 CRC 本地目录](#23-查看-crc-本地目录)
    - [2.4 启动 CRC 运行实例](#24-启动-crc-运行实例)
  - [3. 通过 CRC 管理 Red Hat OpenShift Local](#3-通过-crc-管理-red-hat-openshift-local)
    - [3.1 打开 Web 浏览器并登录 openshift web console](#31-打开-web-浏览器并登录-openshift-web-console)
    - [3.2 获取 openshift web console url](#32-获取-openshift-web-console-url)
    - [3.3 确认 kubeadmin 与 developer 用户信息](#33-确认-kubeadmin-与-developer-用户信息)
    - [3.4 确认所有 clusteroperator](#34-确认所有-clusteroperator)
    - [3.5 使用当前用户登录 registry、获取镜像与列举镜像](#35-使用当前用户登录-registry获取镜像与列举镜像)
    - [3.6 停止 CRC 实例](#36-停止-crc-实例)
    - [3.7 删除 CRC 实例](#37-删除-crc-实例)
    - [3.8 CRC config 子命令](#38-crc-config-子命令)
    - [3.9 预设置 CRC 运行环境](#39-预设置-crc-运行环境)
    - [3.10 登录 CRC 实例并确认 dnsmasq.service 服务](#310-登录-crc-实例并确认-dnsmasqservice-服务)
  - [🧪 4. CRC 故障排除](#-4-crc-故障排除)
  - [5. 参考链接](#5-参考链接)

## 1. Lab 环境说明

- 主机资源:
  - CPU/RAM + 存储：4C16G, 250G disk
  - 主机名：foundation0.ilt.example.com
- 虚拟机资源:
  - CPU/RAM + 存储：由 CRC 定义虚拟机实例
  - 主机名: crc
- 💪 **CRC 可用于 Red Hat OpenShift Local（本地开发环境）的管理组件，OCP 集群以单节点 CRC 实例（VM 虚拟机）的方式运行。此环境仅在开发与测试环境中使用，请勿在生产环境中使用！**

## 2. 采用 CRC 部署 Red Hat OpenShift Local（本地开发环境）

### 2.1 设置 CRC 数据采集遥测（telemetry）
  
```bash
[kiosk@foundation0 backup]$ crc config set consent-telemetry yes
Successfully configured consent-telemetry to yes
```

### 2.2 设置 CRC 运行时
  
```bash
[kiosk@foundation0 backup]$ crc setup
INFO Using bundle path /home/kiosk/.crc/cache/crc_libvirt_4.16.7_amd64.crcbundle
INFO Checking if running as non-root
INFO Checking if running inside WSL2
INFO Checking if crc-admin-helper executable is cached
INFO Caching crc-admin-helper executable
INFO Using root access: Changing ownership of /home/kiosk/.crc/bin/crc-admin-helper-linux-amd64
INFO Using root access: Setting suid for /home/kiosk/.crc/bin/crc-admin-helper-linux-amd64
INFO Checking if running on a supported CPU architecture
INFO Checking if crc executable symlink exists
INFO Creating symlink for crc executable
INFO Checking minimum RAM requirements
INFO Check if Podman binary exists in: /home/kiosk/.crc/bin/oc
INFO Checking if Virtualization is enabled
INFO Checking if KVM is enabled
INFO Checking if user is part of libvirt group
INFO Checking if active user/process is currently part of the libvirt group
INFO Checking if libvirt daemon is running
INFO Checking if a supported libvirt version is installed
INFO Checking if crc-driver-libvirt is installed
INFO Installing crc-driver-libvirt
INFO Checking crc daemon systemd service
INFO Setting up crc daemon systemd service
INFO Checking crc daemon systemd socket units
INFO Setting up crc daemon systemd socket units
INFO Checking if systemd-networkd is running
INFO Checking if NetworkManager is installed
INFO Checking if NetworkManager service is running
INFO Checking if /etc/NetworkManager/conf.d/crc-nm-dnsmasq.conf exists
INFO Writing Network Manager config for crc
INFO Using root access: Writing NetworkManager configuration to /etc/NetworkManager/conf.d/crc-nm-dnsmasq.conf
INFO Using root access: Changing permissions for /etc/NetworkManager/conf.d/crc-nm-dnsmasq.conf to 644
INFO Using root access: Executing systemctl daemon-reload command
INFO Using root access: Executing systemctl reload NetworkManager
INFO Checking if /etc/NetworkManager/dnsmasq.d/crc.conf exists
INFO Writing dnsmasq config for crc
INFO Using root access: Writing NetworkManager configuration to /etc/NetworkManager/dnsmasq.d/crc.conf
INFO Using root access: Changing permissions for /etc/NetworkManager/dnsmasq.d/crc.conf to 644
INFO Using root access: Executing systemctl daemon-reload command
INFO Using root access: Executing systemctl reload NetworkManager
INFO Checking if libvirt 'crc' network is available
INFO Setting up libvirt 'crc' network
INFO Checking if libvirt 'crc' network is active
INFO Starting libvirt 'crc' network
INFO Checking if CRC bundle is extracted in '$HOME/.crc'
INFO Checking if /home/kiosk/.crc/cache/crc_libvirt_4.16.7_amd64.crcbundle exists
INFO Getting bundle for the CRC executable
INFO Downloading bundle: /home/kiosk/.crc/cache/crc_libvirt_4.16.7_amd64.crcbundle...
4.86 GiB / 4.86 GiB [-------------------------------------------------------------------------------] 100.00% 1.46 MiB/s
INFO Uncompressing /home/kiosk/.crc/cache/crc_libvirt_4.16.7_amd64.crcbundle
crc.qcow2:  20.44 GiB / 20.44 GiB [----------------------------------------------------------------------------] 100.00%
oc:  152.50 MiB / 152.50 MiB [---------------------------------------------------------------------------------] 100.00%
Your system is correctly setup for using CRC. Use 'crc start' to start the instance
```

### 2.3 查看 CRC 本地目录
  
```bash
[kiosk@foundation0 backup]$ tree ~/.crc/
/home/kiosk/.crc/
├── bin
│   ├── crc -> /home/kiosk/bin/crc
│   ├── crc-admin-helper-linux-amd64
│   └── crc-driver-libvirt-amd64
├── cache
│   ├── crc_libvirt_4.16.7_amd64
│   │   ├── crc-bundle-info.json
│   │   ├── crc.qcow2
│   │   ├── id_ecdsa_crc
│   │   ├── kubeconfig
│   │   ├── oc
│   │   └── podman-remote
│   └── crc_libvirt_4.16.7_amd64.crcbundle
├── crc-http.sock
├── crc.json
├── crc.log
└── segmentIdentifyHash
  
3 directories, 14 files
```

### 2.4 启动 CRC 运行实例
  
```bash
[kiosk@foundation0 backup]$ crc start
INFO Using bundle path /home/kiosk/.crc/cache/crc_libvirt_4.16.7_amd64.crcbundle
INFO Checking if running as non-root
INFO Checking if running inside WSL2
INFO Checking if crc-admin-helper executable is cached
INFO Checking if running on a supported CPU architecture
INFO Checking if crc executable symlink exists
INFO Checking minimum RAM requirements
INFO Check if Podman binary exists in: /home/kiosk/.crc/bin/oc
INFO Checking if Virtualization is enabled
INFO Checking if KVM is enabled
INFO Checking if libvirt is installed
INFO Checking if user is part of libvirt group
INFO Checking if active user/process is currently part of the libvirt group
INFO Checking if libvirt daemon is running
INFO Checking if a supported libvirt version is installed
INFO Checking if crc-driver-libvirt is installed
INFO Checking crc daemon systemd socket units
INFO Checking if systemd-networkd is running
INFO Checking if NetworkManager is installed
INFO Checking if NetworkManager service is running
INFO Checking if /etc/NetworkManager/conf.d/crc-nm-dnsmasq.conf exists
INFO Checking if /etc/NetworkManager/dnsmasq.d/crc.conf exists
INFO Checking if libvirt 'crc' network is available
INFO Checking if libvirt 'crc' network is active
INFO Loading bundle: crc_libvirt_4.16.7_amd64...
CRC requires a pull secret to download content from Red Hat.
You can copy it from the Pull Secret section of https://console.redhat.com/openshift/create/local.
? Please enter the pull secret *****************************************************************************************                                                                                                                        INFO Creating CRC VM for OpenShift 4.16.7...
INFO Generating new SSH key pair...
INFO Generating new password for the kubeadmin user
INFO Starting CRC VM for openshift 4.16.7...
INFO CRC instance is running with IP 192.168.130.11
INFO CRC VM is running
INFO Updating authorized keys...
INFO Configuring shared directories
INFO Check internal and public DNS query...
INFO Check DNS query from host...
WARN Wildcard DNS resolution for apps-crc.testing does not appear to be working
INFO Verifying validity of the kubelet certificates...
INFO Starting kubelet service
INFO Waiting for kube-apiserver availability... [takes around 2min]
INFO Adding user's pull secret to the cluster...
INFO Updating SSH key to machine config resource...
INFO Waiting until the user's pull secret is written to the instance disk...
INFO Changing the password for the kubeadmin user
INFO Updating cluster ID...
INFO Updating root CA cert to admin-kubeconfig-client-ca configmap...
INFO Starting openshift instance... [waiting for the cluster to stabilize]
INFO 3 operators are progressing: console, image-registry, ingress
INFO Operator network is progressing
INFO Operator authentication is not yet available
INFO Operator authentication is not yet available
INFO All operators are available. Ensuring stability...
INFO Operators are stable (2/3)...
INFO Operators are stable (3/3)...
INFO Adding crc-admin and crc-developer contexts to kubeconfig...
Started the OpenShift cluster.
  
The server is accessible via web console at:
  https://console-openshift-console.apps-crc.testing
  
Log in as administrator:
  Username: kubeadmin
  Password: Vvusp-BpWqd-Ps9Vp-Bv2pH
  
Log in as user:
  Username: developer
  Password: developer
  
Use the 'oc' command line interface:
  $ eval $(crc oc-env)
  $ oc login -u developer https://api.crc.testing:6443
```

## 3. 通过 CRC 管理 Red Hat OpenShift Local

### 3.1 打开 Web 浏览器并登录 openshift web console
  
```bash
$ crc console
```

### 3.2 获取 openshift web console url
  
```bash
$ crc console --url
https://console-openshift-console.apps-crc.testing
```

### 3.3 确认 kubeadmin 与 developer 用户信息
  
```bash
$ crc console --credentials
To login as a regular user, run 'oc login -u developer -p developer https://api.crc.testing:6443'.
To login as an admin, run 'oc login -u kubeadmin -p Vvusp-BpWqd-Ps9Vp-Bv2pH https://api.crc.testing:6443'
```

### 3.4 确认所有 clusteroperator
  
```bash
Note: RedHat OpenShift Local disable Cluster Monitoring Operator in default
$ oc config use-context crc-admin
Switched to context "crc-admin".
$ oc whoami
kubeadmin
$ oc get [co|clusteroperator]
```

### 3.5 使用当前用户登录 registry、获取镜像与列举镜像
  
```bash
$ oc whoami
developer
$ oc registry login --insecure=true  #login registry as developer
$ oc new-project internal-registry-test
$ oc image mirror registry.access.redhat.com/ubi8/ubi:latest=default-route-openshift-image-registry.apps-crc.testing/internal-registry-test/ubi8:latest --insecure=true --filter-by-os=linux/amd64
## mirror image from external registry to internal registry
$ oc get is
NAME   IMAGE REPOSITORY                                                                      TAGS     UPDATED
ubi8   default-route-openshift-image-registry.apps-crc.testing/internal-registry-test/ubi8   latest   2 minutes ago
## verify image stream in internal registry
$ oc run myubi8 --image=ubi8 --command -- sleep 360s
## run pod to test ocp local cluster
```

### 3.6 停止 CRC 实例
  
```bash
$ crc stop
```

### 3.7 删除 CRC 实例
  
  ```bash
  $ crc delete
  ## IMPORTANT: Delete CRC instance will lost all data for ocp local cluster!
  ```

### 3.8 CRC config 子命令
  
```bash
$ crc config --help  ## get all crc config properties
$ crc config set memory 16384  ##setup crc instance memory when running
$ crc config set enable-cluster-monitoring true  ##setup cluster monitoring operator
$ crc config set consent-telemetry yes  ##enable cluster telemetry data collect
$ crc config view
```

### 3.9 预设置 CRC 运行环境

- 💥 **注意：CRC 预设置的值在当前运行的实例中不能被更改！**
- 首先必须删除当前的实例，再运行以下命令设置：
  
  ```bash
  $ crc config set preset [openshift|microshift]
  ```

### 3.10 登录 CRC 实例并确认 dnsmasq.service 服务
  
- CRC 实例中禁止使用用户名与密码登录，仅仅使用 SSH 私钥文件的方式进行登录。
- CRC 实例中 core 用户的 SSH 私钥文件位于 `$HOME/.crc/machines/crc/`。
  
  ```bash
  $ CRC_IPADDR=$(grep -i IPAddress ~/.crc/machines/crc/config.json | awk -F'"' '{print $4}')
  $ echo $CRC_IPADDR
  $ ssh -i ~/.crc/machines/crc/id_ed25519 core@${CRC_IPADDR}
  Red Hat Enterprise Linux CoreOS 416.94.202408062045-0
    Part of OpenShift 4.16, RHCOS is a Kubernetes-native operating system
    managed by the Machine Config Operator (`clusteroperator/machine-config`).
  
  WARNING: Direct SSH access to machines is not recommended; instead,
  make configuration changes via `machineconfig` objects:
    https://docs.openshift.com/container-platform/4.16/architecture/architecture-rhcos.html
  
  ---
  Last login: Mon Sep 16 07:36:54 2024 from 192.168.130.1
  [core@crc ~]$ sudo su -
  [root@crc ~]# systemctl status dnsmasq.service  ##verify dnsmasq service in crc instance
  ● dnsmasq.service - DNS caching server.
       Loaded: loaded (/usr/lib/systemd/system/dnsmasq.service; enabled; preset: disabled)
       Active: active (running) since Mon 2024-09-16 11:34:02 UTC; 29min ago
     Main PID: 1119 (dnsmasq)
        Tasks: 1 (limit: 101724)
       Memory: 1.8M
          CPU: 444ms
       CGroup: /system.slice/dnsmasq.service
               └─1119 /usr/sbin/dnsmasq
  
  Sep 16 12:03:00 crc dnsmasq[1119]: query[A] api.crc.testing from 10.217.0.32
  Sep 16 12:03:00 crc dnsmasq[1119]: config api.crc.testing is 192.168.130.11
  Sep 16 12:03:03 crc dnsmasq[1119]: query[A] oauth-openshift.apps-crc.testing.crc.testing from 10.217.0.32
  Sep 16 12:03:03 crc dnsmasq[1119]: config oauth-openshift.apps-crc.testing.crc.testing is NXDOMAIN
  Sep 16 12:03:03 crc dnsmasq[1119]: query[AAAA] oauth-openshift.apps-crc.testing.crc.testing from 10.217.0.32
  Sep 16 12:03:03 crc dnsmasq[1119]: config oauth-openshift.apps-crc.testing.crc.testing is NXDOMAIN
  Sep 16 12:03:04 crc dnsmasq[1119]: query[A] oauth-openshift.apps-crc.testing.crc.testing from 10.217.0.32
  Sep 16 12:03:04 crc dnsmasq[1119]: config oauth-openshift.apps-crc.testing.crc.testing is NXDOMAIN
  Sep 16 12:03:04 crc dnsmasq[1119]: query[AAAA] oauth-openshift.apps-crc.testing.crc.testing from 10.217.0.32
  Sep 16 12:03:04 crc dnsmasq[1119]: config oauth-openshift.apps-crc.testing.crc.testing is NXDOMAIN
  
  [root@crc ~]# cat /etc/dnsmasq.d/crc-dnsmasq.conf
  listen-address=192.168.130.11
  expand-hosts
  log-queries
  local=/crc.testing/
  domain=crc.testing
  address=/apps-crc.testing/192.168.130.11
  address=/api.crc.testing/192.168.130.11
  address=/api-int.crc.testing/192.168.130.11
  address=/crc.crc.testing/192.168.126.11
  ```

## 🧪 4. CRC 故障排除

- 1️⃣ 如何解决报错 `WARN: app-crcs.testing is not available`？
  - 以上报错暗示 `NetworkManager.service` 未正常工作或 `dnsmasq.service` 正在运行。
  - 因此，可重起 `NetworkManager.service` 或停止 `dnsmasq.service` 来解决问题。
  - dnsmasq 进程由 NetworkManager 调用，无需额外启动 dnsmasq。
- 2️⃣ CRC 实例无法解析域名（报错诸如 `serverbehaving` 或 `i/o timeout` 等），如何解决？
  - 重启 `NetworkManager.service` 后，在 CRC 实例宿主机（笔者环境为 foundation0）的 /etc/resolv.conf 中添加 `nameserver 8.8.8.8` 解决。或者直接在 CRC 实例的 /etc/resolv.conf 中添加 `nameserver 8.8.8.8` 解决。
- 3️⃣ 如何在 CRC 实例中拉取容器镜像？  
  - CRC 实例中切换为 root 用户
  - 所有的容器镜像保存在 root 用户空间中，可使用 crictl 管理镜像，podman 可用但不推荐。
  - 如果容器镜像已存在于 CRC 实例中，oc 命令直接使用它们而不再从外部仓库中拉取。

## 5. 参考链接

- [Red Hat OpenShift Local - Docs](https://docs.redhat.com/zh_hans/documentation/red_hat_openshift_local/2.35/html/getting_started_guide/index)
- [GitHub - Log on to crc-VM #770](https://github.com/crc-org/crc/issues/770)
