## kubeadm 更新 Kubernetes 集群到期证书

### 环境说明：

- Kubernetes 版本：`v1.22.1`

- kubeadm 版本：`v1.22.1`

### 处理方法：

- kubeadm 更新集群证书从 Kubernetes `v1.15` 进入 `stable` 状态，可在 GA 环境中使用。

- 默认情况下，由 kubeadm 为集群生成的所有证书在 1 年后到期。

- 更新（重新签发）集群证书需根据其部署方式而定，通过二进制部署的集群需手动更新集群证书，而通过 kubeadm 部署的集群可使用 kubeadm 更新证书，也可重新编译 kubeadm 用以生成自定义有效期的证书。

- 👉 笔者环境中直接使用 kubeadm 更新证书。

- 集群证书更新方式，步骤如下：
  
  - 检查集群证书有效期（通常于 `master` 节点执行）：
    
    ```bash
    $ kubeadm certs check-expiration
    $ openssl x509 -noout -in /etc/kubernetes/pki/apiserver.crt -dates
    # 也可通过以上命令单独查看证书是否过期
    ```
  
  - 集群 master 节点上的 `/etc/kubernetes/pki/*` 的证书在更新之前应继续保留在节点上不可删除，删除后将导致集群异常无法恢复。
  
  - 🏷 kubeadm 更新集群 master 节点上的证书，而 worker 节点上 `kubelet` 证书默认自动轮换更新，无需关心证书到期问题。`kube-apiserver` 访问 kubelet 时，并不校验 kubelet 服务端证书，kubeadm 也并不提供更新 kubelet 服务端证书的办法。
  
  - 更新延期集群证书 1 年有效期：
    
    ```bash
    $ kubeadm certs renew all --config ./kubeadm-conf.yml
      W1213 21:45:40.125346   15197 strict.go:55] error unmarshaling configuration schema.GroupVersionKind{Group:"kubelet.config.k8s.io", Version:"v1beta1", Kind:"KubeletConfiguration"}: error converting YAML to JSON: yaml: unmarshal errors:
        line 27: key "cgroupDriver" already set in map
      certificate embedded in the kubeconfig file for the admin to use and for kubeadm itself renewed
      certificate for serving the Kubernetes API renewed
      certificate the apiserver uses to access etcd renewed
      certificate for the API server to connect to kubelet renewed
      certificate embedded in the kubeconfig file for the controller manager to use renewed
      certificate for liveness probes to healthcheck etcd renewed
      certificate for etcd nodes to communicate with each other renewed
      certificate for serving etcd renewed
      certificate for the front proxy client renewed
      certificate embedded in the kubeconfig file for the scheduler manager to use renewed
    
      Done renewing certificates. You must restart the kube-apiserver, kube-controller-manager, kube-scheduler and etcd, so that they can use the new certificates.
    # 根据集群部署时使用的 kubeadm-conf.yml 配置文件更新所有集群证书
    
    $ tree -D /etc/kubernetes/pki
      /etc/kubernetes/pki
      ├── [Dec 13 21:45]  apiserver.crt
      ├── [Dec 13 21:45]  apiserver-etcd-client.crt
      ├── [Dec 13 21:45]  apiserver-etcd-client.key
      ├── [Dec 13 21:45]  apiserver.key
      ├── [Dec 13 21:45]  apiserver-kubelet-client.crt
      ├── [Dec 13 21:45]  apiserver-kubelet-client.key
      ├── [Dec 25  2021]  ca.crt
      ├── [Dec 25  2021]  ca.key
      ├── [Dec 25  2021]  etcd
      │   ├── [Dec 25  2021]  ca.crt
      │   ├── [Dec 25  2021]  ca.key
      │   ├── [Dec 13 21:45]  healthcheck-client.crt
      │   ├── [Dec 13 21:45]  healthcheck-client.key
      │   ├── [Dec 13 21:45]  peer.crt
      │   ├── [Dec 13 21:45]  peer.key
      │   ├── [Dec 13 21:45]  server.crt
      │   └── [Dec 13 21:45]  server.key
      ├── [Dec 25  2021]  front-proxy-ca.crt
      ├── [Dec 25  2021]  front-proxy-ca.key
      ├── [Dec 13 21:45]  front-proxy-client.crt
      ├── [Dec 13 21:45]  front-proxy-client.key
      ├── [Dec 25  2021]  sa.key
      └── [Dec 25  2021]  sa.pub
    
      1 directory, 22 files
    # 查看更新后的所有集群证书
    ```
    
    🔗 上述 kubeadm-conf.yml 可参考此 [链接](https://github.com/Alberthua-Perl/kani/blob/main/files/kube-utils/kubeadm-conf.yml)，可根据自身的实际情况进行修改后运行。
  
  - 更新集群证书后，需更新集群 `kubeconfig` 配置文件：
    
    ```bash
    $ mkdir ~/kubeconfig-backup
    $ mv /etc/kubernetes/*.conf ~/kubeconfig-backup/
    # 备份集群原始 kubeconfig 配置文件
    
    $ kubeadm init phase kubeconfig all --config ./kubeadm-conf.yml 
      W1213 21:53:42.328419   19385 strict.go:55] error unmarshaling configuration schema.GroupVersionKind{Group:"kubelet.config.k8s.io", Version:"v1beta1", Kind:"KubeletConfiguration"}: error converting YAML to JSON: yaml: unmarshal errors:
        line 27: key "cgroupDriver" already set in map
      [kubeconfig] Using kubeconfig folder "/etc/kubernetes"
      [kubeconfig] Writing "admin.conf" kubeconfig file
      [kubeconfig] Writing "kubelet.conf" kubeconfig file
      [kubeconfig] Writing "controller-manager.conf" kubeconfig file
      [kubeconfig] Writing "scheduler.conf" kubeconfig file
    # 根据集群部署时使用的 kubeadm-conf.yml 配置文件重新生成集群 kubeconfig 配置文件
    
    $ ls -lh /etc/kubernetes/
      total 36K
      -rw------- 1 root root 5.6K Dec 13 21:53 admin.conf
      -rw------- 1 root root 5.6K Dec 13 21:53 controller-manager.conf
      -rw------- 1 root root 5.7K Dec 13 21:53 kubelet.conf
      drwxr-xr-x 2 root root  113 Nov 22 00:40 manifests
      drwxr-xr-x 3 root root 4.0K Dec 25  2021 pki
      -rw------- 1 root root 5.5K Dec 13 21:53 scheduler.conf
    ```
  
  - 更新完成后检查集群所有证书有效期：
    
    ```bash
    $ kubeadm certs check-expiration
     [check-expiration] Reading configuration from the cluster...
     [check-expiration] FYI: You can look at this config file with 'kubectl -n kube-system get cm kubeadm-config -o yaml'
    
     CERTIFICATE                EXPIRES                  RESIDUAL TIME   CERTIFICATE AUTHORITY   EXTERNALLY MANAGED
     admin.conf                 Dec 13, 2023 13:53 UTC   364d                                    no      
     apiserver                  Dec 13, 2023 13:45 UTC   364d            ca                      no      
     apiserver-etcd-client      Dec 13, 2023 13:45 UTC   364d            etcd-ca                 no      
     apiserver-kubelet-client   Dec 13, 2023 13:45 UTC   364d            ca                      no      
     controller-manager.conf    Dec 13, 2023 13:53 UTC   364d                                    no      
     etcd-healthcheck-client    Dec 13, 2023 13:45 UTC   364d            etcd-ca                 no      
     etcd-peer                  Dec 13, 2023 13:45 UTC   364d            etcd-ca                 no      
     etcd-server                Dec 13, 2023 13:45 UTC   364d            etcd-ca                 no      
     front-proxy-client         Dec 13, 2023 13:45 UTC   364d            front-proxy-ca          no      
     scheduler.conf             Dec 13, 2023 13:53 UTC   364d                                    no      
    
     CERTIFICATE AUTHORITY   EXPIRES                  RESIDUAL TIME   EXTERNALLY MANAGED
     ca                      Dec 23, 2031 13:09 UTC   9y              no      
     etcd-ca                 Dec 23, 2031 13:09 UTC   9y              no      
     front-proxy-ca          Dec 23, 2031 13:09 UTC   9y              no
    # 集群所有证书有效期延期 1 年
    ```

- 💥 需要注意的是，此时新生成的集群 `/etc/kubernetes/admin.conf` 配置文件嵌套新的证书，在集群外部或可使用 kubectl 命令连接至集群的节点上需使用该文件更新节点 `$HOME/.kube/config` 文件，否则无法连接至集群中，直接报错 `error: You must be logged in to the server (Unauthorized)`。

### 参考文档：

- [Kubernetes Doc - 使用 kubeadm 进行证书管理](https://kubernetes.io/zh-cn/docs/tasks/administer-cluster/kubeadm/kubeadm-certs/)

- [Kubernetes Doc - PKI 证书和要求](https://kubernetes.io/zh-cn/docs/setup/best-practices/certificates/)

- [Kubeadm 证书过期时间调整](https://www.cnblogs.com/skymyyang/p/11093686.html)
