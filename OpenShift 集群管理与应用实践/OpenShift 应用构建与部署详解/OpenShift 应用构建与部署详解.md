# 🚀 OpenShift 应用构建与部署详解

## 文档目录

- [🚀 OpenShift 应用构建与部署详解](#-openshift-应用构建与部署详解)
  - [文档目录](#文档目录)
  - [OCP Project 相关命令](#ocp-project-相关命令)
  - [使用 `容器镜像` 创建应用 Pod 流程](#使用-容器镜像-创建应用-pod-流程)
  - [使用 `Containerfile` 或 `Dockerfile` 构建应用容器镜像并创建 Pod](#使用-containerfile-或-dockerfile-构建应用容器镜像并创建-pod)
  - [使用 `应用源代码` 以 `S2I` 的方式注入构建镜像创建应用 Pod](#使用-应用源代码-以-s2i-的方式注入构建镜像创建应用-pod)
  - [使用 `template` 模板定义文件创建各应用资源](#使用-template-模板定义文件创建各应用资源)
  - [Helm 常用命令汇总](#helm-常用命令汇总)
  - [参考链接](#参考链接)

## OCP Project 相关命令
  
```bash
$ oc projects
# 查看所有已存在的可用项目，当前所在的项目前具有 "*" 标识。
# 该命令只能由 master 节点的 system:admin 管理员用户或具有 cluster-role 角色的用户执行。
  
$ oc project
# 查看当前所在的项目
  
$ oc project <project_name>
# 切换至指定的项目
  
$ oc new-project <project_name> --description='<description_string>'
# 创建项目与相应描述内容
# OCP 使用项目进行资源隔离，即项目为命名空间（namespace）。
  
$ oc status -v
# 查看当前所在项目的详细状态，包括 bc 与 dc 状态等。
```

## 使用 `容器镜像` 创建应用 Pod 流程
  
```bash
$ oc new-app --list
# 查看 OCP 集群中已存在的模板（template）与镜像流（imagestream）
  
$ oc new-app \
  --name=<application_name> \
  --docker-image=<container_registry_url>/<repository>/<image_name>:[tag]
# 使用已有的容器镜像创建应用，并指定应用名称。
```
  
![oc-new-app-container-image-stratgy-1](images/oc-new-app-container-image-stratgy-1.jpg)
  
![oc-new-app-container-image-stratgy-2](images/oc-new-app-container-image-stratgy-2.jpg)
  
- 💎 补充：
  
  - 由于 OCP 4.x 已兼容 Kubernetes 的 `deployment` 资源（`api-resources` 中可获知），因此在使用 `--docker-image` 选项直接构建应用时使用 `deployment.apps`。

    ![ocp4-oc-new-app-dockerimage](images/ocp4-oc-new-app-dockerimage.jpg)
  
  - 而 OCP 3.x 未集成 deployment 资源，使用以上选项构建应用时依然使用 `deploymentconfig` 资源实现。

## 使用 `Containerfile` 或 `Dockerfile` 构建应用容器镜像并创建 Pod
  
```bash
$ oc new-app \
  --name=<application_name> \
  --strategy docker \
  <git_repository_url_of_dockerfile> [--insecure-registry]
# 以 Dockerfile 所在的 Git 仓库的方式创建应用
# 注意：
#   1. Dockerfile 中 FROM 指令指向的容器镜像仓库，需注意是否使用 SSL 协议，即是否使用
#      --insecure-registry 命令行选项。
#   2. 如下所示：
#      $ oc new-app --name=echo --strategy docker \
#        --insecure-registry http://services.lab.example.com/rhel7-echo
```

## 使用 `应用源代码` 以 `S2I` 的方式注入构建镜像创建应用 Pod
  
- 📌 方式 1：
  
  > 使用非 openshift 项目中的 imagestream 时，只能使用 `~` 方法创建应用！
  
  ```bash
  $ oc new-app \
    --name=<application_name> \
    <imagestream_name>:[tag]~<git_repository_url_of_source> \
    --labels=app=<application_name>
  # 以 S2I 源代码注入的方式创建应用
  
  ### 示例 ###
  $ oc new-app \
    --name=hello \
    php:5.5~http://workstation.lab.example.com/php-helloworld \
    --labels=app=hello \
    -o json > /tmp/app-s2i.json
  # 可将其定义的资源模板重定向至指定的 JSON 文件而不创建资源
  # 之后可直接使用该 JSON 文件创建各资源
  
  $ oc new-app \
    --name=hello \
    workstation.lab.example.com:5000/openshift3/php-55-rhel7:latest~
    http://workstation.lab.example.com/php-helloworld \
    --labels=app=hello
  # 使用外部容器镜像仓库的镜像与源代码创建应用
  
  $ oc new-app \
    --name=<application_name> \
    <imagestream_name>:[tag] <git_repository_url_of_source>#<branch>
    --context-dir <application_dir_of_branch>
  # 指定构建 imagestream、tag 及 GitHub 的代码库分支，使用该分支中的一个应用目录进行构建。
  
  ### 示例 ###
  $ oc new-app \
    --name=php-helloworld \
    php:7.3 https://github.com/Alberthua-Perl/DO180-app#s2i \
    --context-dir php-helloworld
  # 使用 GitHub 仓库中的 DO180-app s2i 分支构建 php-helloworld 应用
  # 该方法已在 OCP 4.x 中验证
  ```
  
  - 💎 补充：
  
  - 👨‍💻 示例：在 OCP 4.x 中使用 `--context-dir` 选项

    ![ocp4-oc-new-app-context-dir](images/ocp4-oc-new-app-context-dir.jpg)
  
  - 👨‍💻 示例：在 OCP 4.x 中使用 `--build-env` 选项

    ```bash
    $ oc new-app --name myapp \
      --build-env npm_config_registry=http://${RHT_OCP4_NEXUS_SERVER}/repository/nodejs \
      nodejs:12~https://github.com/${RHT_OCP4_GITHUB_USER}/DO288-apps#app-config \
      --context-dir app-config
    # 将环境变量注入至 S2I 构建镜像中用于构建应用
    # --build-env 选项指定环境变量用于注入至 S2I 构建镜像中，此处 npm_config_registry
    # 环境变量将被注入至 nodejs:12 imagestream 指向的 S2I 构建镜像中。
    # 同时需指定源代码地址及 app-config 分支，并且可通过 --context-dir 选项指定源代码目录
    # 上下文中的指定构建目录。 
    ```

    ![ocp4-oc-new-app-build-env](images/ocp4-oc-new-app-build-env.jpg)

    以上示例中使用的 `nodejs:12 imagestream`，其指向 S2I 构建镜像 `registry.redhat.io/rhscl/nodejs-12-rhel7`，其中包含的环境变量部分来自于基础构建镜像 `quay.io/centos7/s2i-base-centos7`，而 `npm_config_registry` 环境变量将直接在构建镜像中设置 nodejs registry 地址。

    ![s2i-base-centos7-env](images/s2i-base-centos7-env.jpg)

    ![nodejs-12-builder-image-env](images/nodejs-12-builder-image-env.jpg)

    ![set-npm_config_registry-env-in-nodejs12-builder-image](images/set-npm_config_registry-env-in-nodejs12-builder-image.jpg)
  
  > ✅ 以上的 nodejs 构建镜像的构建方式可参考 [该 RHSCL 的 GitHub 链接](https://github.com/sclorg/s2i-nodejs-container/tree/master/12)。
  
- 📌 方式 2：
  
  ```bash
  $ oc new-app \
    --name=<application_name> \
    --strategy source \
    -i <imagestream_name>:[tag] \
    --code <git_repository_url_of_source> 
  # 以 S2I 源代码注入的方式创建应用
  ```
  
- 📌 方式 3：
  
  ```bash
  $ oc new-app \
    --name=<application_name> \
    --strategy source \
    <git_repository_url_of_source> 
  # 以 S2I 源代码注入的方式创建应用
  # 未指定 imagestream 时，将自动识别 openshift 项目中相应的 S2I 构建镜像（builder image）。 
  # 该命令将尝试根据 Git 代码仓库根目录中存在的特定文件来确定使用哪一种语言的构建镜像。
  ```
  
- 📌 `oc new-app` 命令常用选项：
  
  ```bash
  --name                        指定构建的应用及其相关资源的名称
  --strategy [docker|source]    指定 S2I 构建过程中使用的构建策略，
                                包括 Dockerfile 文件（docker）、源代码注入（source）。
  --docker-image                指定构建应用所使用的容器镜像    
  -i, --image-stream            指定构建应用时的 imagestream 名称
  --code                        使用源代码方式构建应用
  --labels                      指定构建的应用具有的标签
  --template                    使用模板文件创建各应用资源，见下图示例。
  -p, --param                   指定模板文件中定义的环境变量，该变量可覆盖模板文件
                                中定义的变量。
  --insecure-registry           指定使用的容器镜像仓库未使用 SSL 连接
  ```
  
- 📌 使用 S2I 源代码注入创建应用 pod 流程：
  
  ![s2i-application-build](images/s2i-application-build.jpg)

## 使用 `template` 模板定义文件创建各应用资源

- 该方法常用于部署构建多应用的项目中，需处理好多个应用之间的服务发现问题。
- oc new-app 命令行中可指定应用的名称（`--name` 选项）、模板名称（`--template` 选项）与命令行参数（`--param`, -p 选项）。
- `-p` 选项：指定命令行中的额外参数，可覆盖模板文件中定义的 parameters 参数（模板中的参数可被作为模板中 env 环境变量使用）。
- 👉 模板可在集群的命名空间中，也可以资源定义文件的方式存在。
- 使用 template 模板相关命令：

  ```bash
  ### 查看、上传模板 ###
  $ oc get templates -n openshift
  $ oc get templates [-n <project>]
  # 查看指定项目中的模板，使用 oc get all 命令无法获取。

  $ oc describe templates <template> [-n <project>]
  # 查看指定项目中模板的详细参数定义

  $ oc create -f /path/to/<template>.yaml [-n <project>]
  # 在指定项目中根据模板定义文件上传模板

  ### 查看模板参数、创建模板资源
  $ oc process --parameters <template> [-n <project>]
  # 查看指定项目中模板的参数列表

  $ oc process --parameters <template>.yaml [-n <project>]
  # 查看指定项目中模板定义文件中的参数列表

  $ oc process <template> \
    -p <parameter_name1>=<parameter_value1> ... -p <parameter_nameN>=<parameter_valueN> \
    -o yaml > /path/to/<manifest_file>.yaml
  # 根据当前命名空间中指定的参数替换模板中的默认值，生成清单文件（manifest file）。
  # 注意：模板需已存在于命名空间中，生成的清单文件可直接用于创建资源对象。

  $ oc process <template> \
    --param-file=<parameters_file> \
    -o yaml > /path/to/<manifest_file>.yaml
  # 根据模板与参数定义文件生成清单文件（manifest file）
  # 其中参数定义文件格式形如：<parameter_name>=<parameter_value>

  $ oc process <template> \
    -p <parameter_name1>=<parameter_value1> ... -p <parameter_nameN>=<parameter_valueN> | \
    oc apply -f -
  # 根据模板与指定的参数创建资源对象

  $ oc process -f /path/to/<template>.yaml \
    -p <parameter_name1>=<parameter_value1> ... -p <parameter_nameN>=<parameter_valueN> \
    -o yaml > /path/to/<manifest_file>.yaml
  # 根据模板定义文件与指定的参数值生成清单文件（manifest file）

  $ oc process -f /path/to/<template>.yaml \
    -p <parameter_name1>=<parameter_value1> ... -p <parameter_nameN>=<parameter_valueN> | \
    oc apply -f -
  # 根据模板定义文件与指定的参数创建资源对象
  # 注意：template 模板文件中可直接定义 pvc 与 route
  ```
  
- 💎 补充：
  - `OCP 4.x` 中使用 `template` 模板构建 MySQL 数据库应用（单 pod 应用）：

    ![ocp4-oc-new-app-template](images/ocp4-oc-new-app-template.jpg)

  - `OCP 4.x` 中使用 template 模板构建 `php` 与 `mysql` 的多 pod 应用：

    ![ocp4-template-php-mysql-ephemeral-1](images/ocp4-template-php-mysql-ephemeral-1.jpg)

    ![ocp4-template-php-mysql-ephemeral-2](images/ocp4-template-php-mysql-ephemeral-2.jpg)

    前端 php 应用发现后端 mysql 应用的方式，CoreDNS 解析过程见前文所述：

    ![php-frontend-discover-mysql-backend](images/php-frontend-discover-mysql-backend.jpg)

    若 pod 中不具有相应的网络调试工具，可使用如下方式进行前后端 pod 的网络调试：

    ![test-frontend-pod-to-backend-pod-connection](images/test-frontend-pod-to-backend-pod-connection.jpg)

    定义以上应用的 template 模板（JSON 格式）部分如下所示：

    ![ocp4-template-php-mysql-ephemeral-3](images/ocp4-template-php-mysql-ephemeral-3.jpg)

## Helm 常用命令汇总

```bash
### Helm Chart Repository 相关操作 ###
$ helm repo add <repo_name> <repo_url>
# 添加 helm charts 仓库信息并写入 $HOME/.config/helm/repositories.yaml 中
$ helm repo add do280-repo http://helm.ocp4.example.com/charts

$ helm repo list
# 查看 helm charts 仓库的列表

$ helm repo update

$ helm repo remove <repo1_name> <repo2_name> ... <repoN_name>
# 移除指定的一个或多个 helm charts 仓库

$ helm search repo [<repo_name>] [--versions]
# 若不指定仓库时，将返回所有仓库中的 charts；若指定仓库时，将返回指定仓库中的 charts。
# --versions 选项：查看所有 charts 的全部版本信息
$ helm search repo do280-repo
# 指定仓库搜索其中最新的 charts 版本
$ helm search repo do280-repo --versions
# 指定仓库搜索其中所有的 charts 版本

$ helm search repo [<repo_name>/<chart_name>] [--versions]
# 指定仓库搜索指定 chart 的版本信息

### Helm Chart 相关操作 ###
# 注意：<chart_reference> 为 <repo_name>/<chart_name>
$ helm show chart <chart_reference> [--version <version_number>]
# 查看指定版本的 chart 信息
$ helm show chart do280-repo/etherpad --version 0.0.7

$ helm show values <chart_reference> [--version <version_number>]
# 查看指定版本的 chart 中默认定义的变量值
$ helm show values do280-repo/etherpad --version 0.0.7

$ helm install <release_name> <chart_reference> -f ./values.yaml [--version <version_number>] [--dry-run]
# 指定 chart 与自定义的 values.yaml 中的值创建应用，其中 <release_name> 为应用的名称。
# --dry-run 选项：用于模拟安装而实际不真实部署应用
$ helm install example-app do280-repo/etherpad -f ./values.yaml --version 0.0.7

$ helm upgrade <release_name> <chart_reference> -f ./values.yaml
# 指定 chart 与自定义的 values.yaml 中的值更新应用
$ helm upgrade example-app do280-repo/etherpad -f ./values.yaml

$ helm list
# 查看当前项目中已安装的应用 release 信息
# --all-namespaces 选项指定所有项目中的 release；--namespace 选项指定项目中的 release

$ helm history <release_name>
# 查看指定应用 release 的历史版本
```

## 参考链接

- [⭕ Red Hat OpenShift 基础架构与原理详解](https://github.com/Alberthua-Perl/tech-docs/blob/master/Red%20Hat%20OpenShift%20Container%20Platform/Red%20Hat%20OpenShift%20%E5%9F%BA%E7%A1%80%E6%9E%B6%E6%9E%84%E4%B8%8E%E5%8E%9F%E7%90%86%E8%AF%A6%E8%A7%A3/Red%20Hat%20OpenShift%20%E5%9F%BA%E7%A1%80%E6%9E%B6%E6%9E%84%E4%B8%8E%E5%8E%9F%E7%90%86%E8%AF%A6%E8%A7%A3.md)
