# Jenkins 的 CI/CD 之旅（）—— Nexus3 的部署与常规设置

## 文档说明



## 文档目录

- []()
- []()
- []()
- []()
- []()
- []()
- []()

## 容器化部署 Nexus3



```bash
$ podman run --detach \
  --name nexus3 \
  --publish 8081:8081 --publish 8082:8082 --publish 8083:8083 \
  --volume /home/kiosk/backup/middleware/nexus-data:/nexus-data:Z \
  --memory 4096m \
  docker.io/sonatype/nexus3:3.9.0
```

## 容器镜像仓库

### 创建 docker-hosted 类型的容器镜像仓库

### 登录与推送容器镜像

podman 登录此类仓库默认使用 SSL 认证，若本地未设置 SSL 证书将认证失败而无法登录。如下所示：

```bash
$ podman login nexus3.lab.example.com:8882
Authenticating with existing credentials for nexus3.lab.example.com:8882
Existing credentials are invalid, please enter valid username and password
Username (devuser0): devuser0
Password:
Error: authenticating creds for "nexus3.lab.example.com:8882": pinging container registry nexus3.lab.example.com:8882: Get "https://nexus3.lab.example.com:8882/v2/": EOF
```

根据 Nexus3 容器启动的端口映射可知，访问容器所在的宿主机端口 8882 即可访问容器镜像服务。

```bash
$ podman login --tls-verify=false nexus3.lab.example.com:8882
Username: devuser0  #Nexus3 中创建的常规用户
Password:    #以上用户的密码
Login Succeeded!
```

Nexus3 中的容器镜像存储在扁平化的空间中，镜像不从属于用户或组织，而是直接进行管理。因此，在为容器镜像设置标签时，可遵循以下方式，此处以 `docker.io/library/registry:2` 为例：

```bash
$ podman tag docker.io/library/registry:2 nexus3.lab.example.com:8882/registry:2
$ podman push --tls-verify=false nexus3.lab.example.com:8882/registry:2
Getting image source signatures
Copying blob 3dc78a2f4e56 skipped: already exists
Copying blob 1a0007403c9e skipped: already exists
Copying blob 11d475ff1a07 skipped: already exists
Copying blob 8d881b642817 skipped: already exists
Copying blob 0fcc9ad4e206 skipped: already exists
Copying config 26b2eb0361 done
Writing manifest to image destination
```

容器镜像推送完成后，Nexus3 中将显示此镜像：


