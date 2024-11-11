# ⭕ Red Hat OpenShift Local (CRC) v2.35 Setup Info

## Environment

- Host:
  - CPU/RAM + Storage: 4C16G, 250G disk
  - hostname: foundation0.ilt.example.com
- VM Guest:
  - CPU/RAM + Storage: defined by CRC instance
  - hostname: crc
- 💥 Note: CRC instance is JUST used to development and test environment, NOT for production!  

## Deploy Red Hat OpenShift Local through CRC

- Setup telemetry data collection
  
  ```bash
  [kiosk@foundation0 backup]$ crc config set consent-telemetry yes
  Successfully configured consent-telemetry to yes
  ```

- Setup CRC runtime
  
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
  INFO Checking if libvirt is installed
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

- View CRC local directory
  
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

- Start CRC running instance
  
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

## Manage Red Hat OpenShift Local through CRC

- Open web browser to login openshift web console
  
  ```bash
  $ crc console
  ```

- Get openshift web console url
  
  ```bash
  $ crc console --url
  https://console-openshift-console.apps-crc.testing
  ```

- Verify kubeadmin and developer users info
  
  ```bash
  $ crc console --credentials
  To login as a regular user, run 'oc login -u developer -p developer https://api.crc.testing:6443'.
  To login as an admin, run 'oc login -u kubeadmin -p Vvusp-BpWqd-Ps9Vp-Bv2pH https://api.crc.testing:6443'
  ```

- Verify all clusteroperator
  
  ```bash
  Note: RedHat OpenShift Local disable Cluster Monitoring Operator in default
  $ oc config use-context crc-admin
  Switched to context "crc-admin".
  $ oc whoami
  kubeadmin
  $ oc get [co|clusteroperator]
  ```

- Use current user to login registry, mirror images, list images
  
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

- Stop CRC instance (ocp local cluster)
  
  ```bash
  $ crc stop
  ```

- Delete CRC instance (ocp local cluster)
  
  ```bash
  $ crc delete
  ## IMPORTANT: Delete CRC instance will lost all data for ocp local cluster!
  ```

- CRC config commands
  
  ```bash
  $ crc config --help  ## get all crc config properties
  $ crc config set memory 16384  ##setup crc instance memory when running
  $ crc config set enable-cluster-monitoring true  ##setup cluster monitoring operator
  $ crc config set consent-telemetry yes  ##enable cluster telemetry data collect
  $ crc config view
  ```

- Setup CRC preset
  
  - The value of CRC preset CAN'T be changed on current instance.
  - You must delete the instance first. Then run `crc config set preset microshift` to set.
  
  ```bash
  $ crc config set preset [openshift|microshift]
  ```

- Login CRC instance and verify dnsmasq.service in crc instance
  
  - Using user and password to login CRC instance isn't been allowed, so JUST to use ssh identity file to login.
  - The identity file for core user is located in ~/.crc/machines/crc/.
  
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

## 🧪 Troubleshooting

- 1.How to resolve the issue `WARN: app-crcs.testing is not available`?
  
  - Indicating that `NetworkManager.service` works not well or `dnsmasq.service` is running.
  - So you can restart `NetworkManager.service` or stop `dnsmasq.service` to resolve this issue.
  - Dnsmasq progress is called by NetworkManager.

- 2.CRC instance can't resolv domain name (errors like `serverbehaving` or `i/o timeout`, etc), so how to resolve it?
  
  - Just add `nameserver 8.8.8.8` into /etc/resolv.conf on foundation0 after restarting `NetworkManager.service`.

- 3.How to pull container image on CRC instance local?
  
  - switch user from crc to root
  - All container images storaged in root user namespace, and podman or crictl tool can manage images in local.
  - If container images existing on crc intance, associated oc commands use them directly not to pull from external registry.

## Reference

- [Red Hat OpenShift Local - Docs](https://docs.redhat.com/zh_hans/documentation/red_hat_openshift_local/2.35/html/getting_started_guide/index)
- [GitHub - Log on to crc-VM #770](https://github.com/crc-org/crc/issues/770)
