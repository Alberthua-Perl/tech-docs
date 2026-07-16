# Milvus 向量数据库管理与集成

## 文档说明

## 文档目录

- [Milvus 向量数据库管理与集成](#milvus-向量数据库管理与集成)
  - [文档说明](#文档说明)
  - [文档目录](#文档目录)
  - [Milvus 数据库架构与数据写入流程](#milvus-数据库架构与数据写入流程)
  - [Milvus 数据库部署](#milvus-数据库部署)
    - [Standalone 模式：Docker 单容器运行](#standalone-模式docker-单容器运行)
    - [多容器模式：Docker Compose 运行](#多容器模式docker-compose-运行)
  - [Python SDK 连接测试 Milvus](#python-sdk-连接测试-milvus)
  - [参考链接](#参考链接)

## Milvus 数据库架构与数据写入流程

## Milvus 数据库部署

### Standalone 模式：Docker 单容器运行

以 `Milvus Standalone` 模式部署数据库，但采用 [官方脚本](https://raw.githubusercontent.com/milvus-io/milvus/master/scripts/standalone_embed.sh) 无法正确启动 Milvus 数据库，需更改以下内容修复：

<img src="images/standalone_milvus_bugfix.png" width=80%>

```bash
$ wget -O standalone_embed.sh https://raw.githubusercontent.com/milvus-io/milvus/master/scripts/standalone_embed.sh
$ mkdir milvus
# 创建 Milvus 数据库目录
$ chmod -R 0777 milvus
# 修改 Milvus 数据库目录权限
$ mv standalone_embed.sh milvus/
$ cd milvus/
$ vim standalone_embed.sh
# 根据上图修改脚本完成部署
$ bash standalone_embed.sh start
Unable to find image 'milvusdb/milvus:2.6-20260516-1d584f5e' locally
2.6-20260516-1d584f5e: Pulling from milvusdb/milvus
7646c8da3324: Already exists
5f94a9591bfb: Pull complete
c17e2914f747: Pull complete
ee4a5c473250: Pull complete
90b5180edda9: Pull complete
c3d515cd17cd: Pull complete
a5626e2c21c5: Pull complete
56391d0d194c: Pull complete
4f4fb700ef54: Pull complete
Digest: sha256:a1cde8304c130d8613c4be60a49f8e4b69ad236e4c08f9dc6df5b80302211c36
Status: Downloaded newer image for milvusdb/milvus:2.6-20260516-1d584f5e
Wait for Milvus Starting...
Start successfully.
To change the default Milvus configuration, add your settings to the user.yaml file and then restart the service.
# 首次启动将拉取 Milvus 数据库容器镜像

$ docker ps --format="table {{ .Names }} {{ .Status }}" | grep milvus
milvus-standalone Up 54 minutes (healthy)
```

### 多容器模式：Docker Compose 运行

```bash
$ sudo mkdir -p /opt/milvus/volumes
# 创建 Milvus 数据库挂载目录
$ sudo chmod -R 0777 /opt/milvus/volumes
$ sudo cd /opt/milvus/
$ sudo vim milvus-standalone-docker-compose.yml
# 编辑 Milvus 的 docker-compose 文件
```

```yaml
# file: /opt/milvus/milvus-standalone-docker-compose.yml
version: '3.5'

services:
  etcd:
    container_name: milvus-etcd-0
    image: quay.io/coreos/etcd:v3.5.25
    environment:
      - ETCD_AUTO_COMPACTION_MODE=revision
      - ETCD_AUTO_COMPACTION_RETENTION=1000
      - ETCD_QUOTA_BACKEND_BYTES=4294967296
      - ETCD_SNAPSHOT_COUNT=50000
    volumes:
      - ${DOCKER_VOLUME_DIRECTORY:-.}/volumes/etcd:/etcd
    command: etcd -advertise-client-urls=http://etcd:2379 -listen-client-urls http://0.0.0.0:2379 --data-dir /etcd
    healthcheck:
      test: ["CMD", "etcdctl", "endpoint", "health"]
      interval: 30s
      timeout: 20s
      retries: 3

  minio:
    container_name: milvus-minio-0
    image: minio/minio:RELEASE.2024-12-18T13-15-44Z
    environment:
      MINIO_ACCESS_KEY: minioadmin
      MINIO_SECRET_KEY: minioadmin
    ports:
      - "9001:9001"
      - "9000:9000"
    volumes:
      - ${DOCKER_VOLUME_DIRECTORY:-.}/volumes/minio:/minio_data
    command: minio server /minio_data --console-address ":9001"
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:9000/minio/health/live"]
      interval: 30s
      timeout: 20s
      retries: 3

  standalone:
    container_name: milvus-standalone-0
    image: milvusdb/milvus:v2.6.16
    command: ["milvus", "run", "standalone"]
    security_opt:
    - seccomp:unconfined
    environment:
      ETCD_ENDPOINTS: etcd:2379
      MINIO_ADDRESS: minio:9000
      MQ_TYPE: woodpecker
    volumes:
      - ${DOCKER_VOLUME_DIRECTORY:-.}/volumes/milvus:/var/lib/milvus
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:9091/healthz"]
      interval: 30s
      start_period: 90s
      timeout: 20s
      retries: 3
    ports:
      - "19530:19530"
      - "9091:9091"
    depends_on:
      - "etcd"
      - "minio"

networks:
  default:
    name: milvus
```

启动 Milvus 数据库：

```bash
$ sudo docker compose -f ./milvus-standalone-docker-compose.yml up -d
WARN[0000] /home/godev/backup/milvus/milvus-standalone-docker-compose.yml: the attribute `version` is obsolete, it will be ignored, please remove it to avoid potential confusion 
WARN[0000] a network with name milvus exists but was not created for project "milvus".
Set `external: true` to use an existing network 
[+] up 3/3
 ✔ Container milvus-minio-0      Started      0.4s
 ✔ Container milvus-etcd-0       Started      0.4s
 ✔ Container milvus-standalone-0 Started      0.2s

 $ sudo docker ps
 CONTAINER ID   IMAGE                                      COMMAND                  CREATED      STATUS                 PORTS                                                                                          NAMES
bd7a6410b440   milvusdb/milvus:v2.6.16                    "/tini -- milvus run…"   2 days ago   Up 4 hours (healthy)   0.0.0.0:9091->9091/tcp, [::]:9091->9091/tcp, 0.0.0.0:19530->19530/tcp, [::]:19530->19530/tcp   milvus-standalone-0
b72bdcdbff9c   quay.io/coreos/etcd:v3.5.25                "etcd -advertise-cli…"   2 days ago   Up 4 hours (healthy)   2379-2380/tcp                                                                                  milvus-etcd-0
b5c830a39edc   minio/minio:RELEASE.2024-12-18T13-15-44Z   "/usr/bin/docker-ent…"   2 days ago   Up 4 hours (healthy)   0.0.0.0:9000-9001->9000-9001/tcp, [::]:9000-9001->9000-9001/tcp                                milvus-minio-0
# 查看 Milvus 数据库各组件容器运行状态
```

## Python SDK 连接测试 Milvus

```python
from pymilvus import MilvusClient

client = MilvusClient(uri="http://localhost:19530", db_name="default")
print(client.list_collections())
```

## 参考链接

- [milvusdb/milvus | DockerHub](https://hub.docker.com/r/milvusdb/milvus)
- [容器化部署 Milvus 向量数据库：从开发到生产环境的完整指南 | 博客园](https://www.cnblogs.com/ycfenxi/p/19926766)
