# 流水线作业 (Pipeline Job) 与 Pipeline 语法

## 文档说明

本文档中涉及的 Plugin 插件：

- Pipeline: Stage View
- Workspace Cleanup Plugin

## 文档目录

- [1. 流水线 (Pipeline) 是什么？](#1-流水线-pipeline-是什么)
- [2. 流水线语法初步](#2-流水线语法初步)
- [3. Pipeline 的组成](#3-pipeline-的组成)
- [4. Pipeline 编辑器的特性](#4-pipeline-编辑器的特性)
- [5. Pipeline 基础及代码示例](#5-pipeline-基础及代码示例)
  - [5.1 运行一条 Pipeline](#51-运行一条-pipeline)
  - [5.2 控制台输出](#52-控制台输出)
  - [5.3 运行失败的视图](#53-运行失败的视图)
  - [5.4 流水线步骤](#54-流水线步骤)
  - [5.5 回放（Replay）](#55-回放replay)
- [6. 声明式 Pipeline 的结构](#6-声明式-pipeline-的结构)
- [7. Pipeline 支持的 Section](#7-pipeline-支持的-section)
  - [7.1 Section: agent](#71-section-agent)
  - [7.2 Section: post](#72-section-post)
  - [7.3 Section: stages 和 steps](#73-section-stages-和-steps)
- [8. Pipeline 支持的 Directive](#8-pipeline-支持的-directive)
- [9. Pipeline 的常用步骤](#9-pipeline-的常用步骤)
- [10. 参考链接](#10-参考链接)

## 1. 流水线 (Pipeline) 是什么？

- “从某种抽象层次上讲，部署流水线（Deployment Pipeline）是指从软件版本控制库到用户手中这一过程的自动化表现形式。” ——《持续交付——发布可靠软件的系统方法》
- Jenkins 原本就支持 pipeline，只不过最初该功能被称为 “任务”。
  - Jenkins 1.x 仅支持于界面中手动配置流水线（Freestyle Job），而 2.x 则实现了 “流水线即代码（Pipeline as a Code）” 的机制，它支持通过 “代码” 来描述部署整条流水线。
  - 使用 “代码” 而非 UI 完成 pipeline 定义的意义在于：
    - 更好地版本化：支持版本控制
    - 更好地协作：pipeline 的修改对所有人可见，且支持代码审查。
    - 更好的可重用性
- 在 Jenkins 2.x 中，用于保存 pipeline 代码并可被 Jenkins 加载的文件称为 Jenkinsfile。
  - 流水线既可以在 pipeline 类型的任务中创建，也可以定义在一个称为 Jenkinsfile 的外部文件中，它可以同代码保存在一起。
  - Jenkinsfile 就是一个文本文件，它是部署流水线概念在 Jenkins 中的表现形式。

## 2. 流水线语法初步

Jenkins 2.x 支持两种 pipeline 语法：

- 脚本式（scripted）语法：Jenkins 最先支持的 pipeline 语法，它采用命令式风格，直接在流水线脚本中定义逻辑和程序流程。

```groovy
//脚本式流水线：node 用于脚本式流水线，从技术层面上来说，它是一个步骤，代表可以用于流水线中执行活动的资源。
node('node01') {
  stages {    
    stage('Build') {
      steps {
        echo 'Building...'
      }
    }

    stage('Test') {
      steps {
        echo 'Testing...'
      }
    }
  }
}
```

- 声明式（declarative）语法：CloudBees 公司为 Jenkins 引入的一种 “流水线即代码” 的 pipeline 语法，它允许用户在 pipeline 的定义中将更多的精力关注于期望 pipeline 的状态和输出之上，而非实现逻辑。

```groovy
//声明式流水线：agent 用于声明式流水线，它作为一个指令用于分配节点。
pipeline {
  agent {
      label 'node01'
  }

  stages {    
    stage('Build') {
      steps {
        echo 'Building...'
      }
    }

    stage('Test') {
      steps {
        echo 'Testing...'
      }
    }
  }    
}
```

## 3. Pipeline 的组成

- pipeline 实际上就是基于 Groovy 语言实现的一种 DSL（Domain-Specific Language），用于描述代码编译到打包发布的整个流水线是如何进行的。
- pipeline 的定义有一个明确的、必须遵循的结构，它由一些指令及嵌套的代码块组成，该结构存在以下几个基本元素：
  - `pipeline`：流水线的最外层结构，代表整条 pipeline，包含着 pipeline 的完整逻辑。
  - `stages`：用于包含所有 stage 的定义
  - `stage`：阶段，代表流水线的一个单独的功能完成时期，例如编译等。
  - `steps`：用于在 stage 中定义完成该阶段功能所需要经历的一系列步骤，并能够把这些步骤同该 stage 中的其它定义（如环境的定义等）分隔开。
  - `agent`：pipeline 中的单独指令，用于指定流水线的执行位置，它可能是代表着 Agent 主机的某个物理机、虚拟机或者容器。
  - `post`：用在 stage 代码块，或者是整个 pipeline 执行完成后附加的步骤。
- 在上面给出的几个配置中，post 是配置语法中惟一可省略的，但在实际应用中，通常不能省略。

```groovy
pipeline {
  agent any

  stages {
    stage('name1') {
      steps {
        ...
      }
      post {
        ...
      }
    }

    stage('name2') {
      ...
    }
  }

  post {
    ...
  }
}
```

## 4. Pipeline 编辑器的特性

在创建的或已存在的流水线风格作业中，pipeline 编辑器如下所示：

<center><img src="images/pipeline-editor.png" style="width:80%"></center>

- 语法检查：
  - 编辑器会尝试检查 Groovy 语法和引用的有效性，所有问题都会在对应代码行前面通过红色 “X” 方框标记提示出来。
  - 然而，并非所有的错误标记都代表真正的错误，在某些时候脚本可能无法解析依赖或最近创建的输入，这是一个异常而非规则。
- 完整的错误信息：
  - X 标记在提供快速识别问题行的同时，还支持鼠标悬停时展示完整的错误信息。
- 自动补全：
  - 编辑器还提供了一定程度上的自动补全功能，比如自动补全括号。
- 提供了 “代码片断生成器”：
  - 代码片断生成器提供了一种搜索可用的 DSL 步骤的方法，以帮助用户随时借鉴和查阅感兴趣步骤的主义和语法。

## 5. Pipeline 基础及代码示例

- 流水线一般由多个阶段组成，包括获取源代码、编译、集成测试、代码分析、应用打包和部署等。
  - Jenkins 2.x 能够代码化整个流水线，可参考如下 [Jenkinsfile](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/jenkins/pipeline-syntax-test/Jenkinsfile.base-structure)：

  ```groovy
  pipeline {
    agent any

    stages {
      stage('Source') {
        steps {
          echo 'Get code from Git Repository...'
        }
      }

      stage('Build') {
        steps {
          echo 'Building...'
        }
      }

      stage('Test') {
        steps {
          echo 'Testing...'
        }
      }

      stage('Deploy') {
        steps {
          echo 'Deploying...'
        }
      }
    }    
  }
  ```

  > 💥 注意：以下流水线执行过程需安装 `Pipeline: Stage View` 插件，用以显示方块形的阶段视图。

  - 第一次保存流水线定义时，Dashboard 会提示流水线尚未执行，在项目上通过 “立即构建”，可手动触发构建过程。
    - 任务执行结果在阶段视图中以方块的形式显示
      - 一次构建用一行方块来表示，其中每个方块代表流水线中的一个阶段（stage）。
      - 每个方块都代表了一个特定阶段的一次执行结果

  <center><img src="images/pipeline-not-run-yet.png" style="width:80%"></center>

  - 方块颜色的意义：
    - 蓝色条纹：运行中
    - 白色：stage 尚未执行
    - 红色条纹：state 执行失败
    - 绿色：stage 执行成功
    - 浅红色：stage 执行成功，但是下游的某个 stage 出现失败。

### 5.1 运行一条 Pipeline

以前面示例中的代码保存为一个 pipeline 的定义，通过左边菜单中的 “立即构建” 便可触发 pipeline 的一次运行，具体结果如下图所示：

<center><img src="images/run-pipeline-demo.png" style="width:80%"></center>

### 5.2 控制台输出

- 在传统的 Jenkins 中，通过点出控制台输出链接或者构建历史窗口中的构建旁边状态球来查看控制台输出，新式的 stage 视图同样支持此功能。
- 另外，将鼠标悬停于 stage 视图中的某个方块上，还会出现该阶段相关的日志信息的按钮，点击该按钮，即可显示此 stage 的日志。

<center><img src="images/pipeline-stage-log-demo.png" style="width:80%"></center>

### 5.3 运行失败的视图

多次运行构建任务时，最新一次的运行位于视图中的最上方。

- 当某个 stage 因任何原因运行失败时，Jenkins 会将其标记为红色条纹背景。
- 鼠标悬停于 stage 方块上方，会有错误信息提示。
- 如，将此前 pipeline 示例代码的 “Build” 阶段有意改为错误格式，将 echo 换成 bat 命令等，其构建结果将输出为类似如下视图：

<center><img src="images/pipeline-run-error-demo.png" style="width:80%"></center>

### 5.4 流水线步骤

流水线作业中的 Pipeline Steps 可以分解显示每个步骤的执行状态：

<center><img src="images/pipeline-steps.png" style="width:80%"></center>

### 5.5 回放（Replay）

- 对于错误的构建任务，Jenkins 提供了一种称为 “回放（Replay）” 的机制，它允许用户无须改变已保存的原有代码的基础上进行试验和调试。
- 回放为用户提供了一种在原有代码基础上修改代码并再次触发 pipeline 的功能，以便于在正式提交代码之前进行一次变更的快速验证并查看效果。
  - 点击构建菜单中的 “回放”，会弹出编辑窗口，并允许用户任意修改程序，而后点击 “运行” 按钮来验证变更效果。
  - Jenkins 会在回放窗口中运行编辑后的代码，并保存一次全新的构建记录，但原始代码依然保持从前的状态。
  - 因此，回放操作能帮用户验证变更，但真正的变更依然需要用户手动更新 pipeline 的代码完成。

<center><img src="images/pipeline-replay-demo-1.png" style="width:80%"></center>

<center><img src="images/pipeline-replay-demo-2.png" style="width:80%"></center>

## 6. 声明式 Pipeline 的结构

pipeline 的定义有一个明确的、必须遵循的结构，它由一些 `directive` 和 `section` 组成，每一个 section 又可包含其它的 section、directive 和 step，以及一些 `condition` 的定义。

- Section：用于将那些在某个时间点需要一同运行的条目（item）组织在一起
  - `agent section`：指定负责运行代码的节点
  - `stages section`：组织一到多个 stage
  - `steps section`：组织一至多个 DSL 格式的步骤
  - `post section`：在 stage 或整个 pipeline 的尾部封装一些需要被执行的步骤或者检验条件
- Directive（指令）：负责完成特定功能的语句或代码块，如 `environment`、`tools`、`triggers`、`input` 和 `when` 等。
- Steps：steps 本身就是一个标识特定 section 的名称，其内部可以使用任何合法的 DSL 语句，如 `git`、`sh`、`bat` 和 `echo` 等。

下图中的实线部分，为必选的配置参数。

<center><img src="images/declarative-pipeline-structure.png" style="width:30%"></center>

## 7. Pipeline 支持的 Section

### 7.1 Section: agent

- agent 用于指明使用哪一个节点去执行 pipeline 或 stage 中的代码
  - 在 pipeline 代码块的顶部，必须要有一个 agent 来指定 “默认” 的执行节点。
  - 一个 stage 的顶部也可以有一个 agent 的定义，用来指定负责运行该 stage 中的代码的节点。
- agent 可接受多种形式的参数：
  - `any`：任何可用节点
  - `none`：用于 pipeline 顶端时表示不定义默认的 agent，这就需要为每个 stage 单独指定。
  - `label { label "<label>" }`：具有指定的标签的节点均为可用节点
  - `node { label "<label>" }`：与 label 相似，但可以指定额外的参数 `customWorkspace`。
  - `docker`：
    - 在指定的容器中运行 pipeline 或 stage 代码，该容器动态创建并运行于预配置的可运行容器的 node 上，或能够匹配到指定 label 的 node 上。
    - 可用参数如下：`image`、`label`、`args`、`rgistryUrl` 和 `registryCredentialsId` 等。

    ```groovy
    //适合私有仓库或需要额外挂载/环境变量的场景
    pipeline {
      agent {
        docker {
          image 'maven:3.9.3-eclipse-temurin-17'
          label 'docker-enabled-node'  //只在打了该 label 的节点上运行
          args  '-v /tmp:/tmp -e MAVEN_OPTS="-Xmx2g"'  //参数传递给 docker run
          registryUrl 'https://registry.lab.example.com/'  //私有容器镜像仓库
          registryCredentialsId 'registry-cred'   //Jenkins 里预存的凭据 ID
        }
      }

      stages {
        stage('Build') {
          steps {
            sh 'mvn -B clean package'
          }
        }
      }
    }
    ```

  - `dockerfile`：
    - 功能上类似于上面 docker 参数，但容器镜像通过指定的 Dockerfile 进行构建。
    - 该参数要求 Jenkinsfile 必须从 **Multibranch Pipeline** 或者 **Pipeline from SCM** 中加载。
    - 可用参数如下：`filename`、`dir`、`label`、`additionalBuildArgs`、`args`、`registryUrl` 和 `registryCredentialsId`。

    ```groovy
    pipeline {
      agent {
        dockerfile {
          filename 'Dockerfile.ci'   //源码中的 Dockerfile 文件名
          dir 'build'                //构建上下文目录
          label 'docker-enabled-node'
          additionalBuildArgs '--build-arg VERSION=1.0.2'
          args '-v /tmp:/tmp'
        }
      }

      stages {
        stage('Test') {
          steps {
            sh 'pytest'
          }
        }
      }
    }
    ```  

- `kubernetes`：
  - 于 Kubernetes 集群上指定的 Pod 中运行 stage 或 pipeline 代码，该参数同样要求 Jenkinsfile 必须从 **Multibranch Pipeline** 或者 **Pipeline from SCM** 中加载。
  - 需要在 kubernetes 参数中指定 Pod 模板

```groovy
//Declarative Pipeline（推荐，语法简洁）
pipeline {
  /* 1. 整个流水线都跑在同一个 Pod 里 */
  agent {
    kubernetes {
      cloud 'kubernetes'          //Jenkins 系统里配置的 K8s Cloud 名称
      slaveConnectTimeout 1200    //Jenkins 等待 Pod Ready 的超时时间（秒）
      workspaceVolume emptyDirWorkspaceVolume()    //工作目录使用 emptyDir

      /* 2. 直接内嵌 Pod YAML（最灵活） */
      yaml '''
apiVersion: v1
kind: Pod
metadata:
  labels:
    jenkins: agent
spec:
  restartPolicy: Never
  containers:
  - name: jnlp    //JNLP 容器，必须存在
    image: jenkins/inbound-agent:4.15-1-alpine
    imagePullPolicy: IfNotPresent
    args: ['\$(JENKINS_SECRET)', '\$(JENKINS_NAME)']

  - name: maven   //额外工具容器
    image: maven:3.9-eclipse-temurin-17
    imagePullPolicy: IfNotPresent
    command: ['cat']
    tty: true
    resources:
      limits:
        memory: "1Gi"
        cpu: "500m"
      requests:
        memory: "512Mi"
        cpu: "250m"
  volumes:
  - name: maven-cache
    emptyDir: {}
'''
    }
  }

  environment {
    //把 kubeconfig 作为凭据注入
    KUBECONFIG = credentials('my-kubeconfig')
  }

  stages {
    stage('Maven Build') {
      steps {
        container('maven') {    //在 maven 容器里执行
          sh 'mvn -B clean package -DskipTests'
        }
      }
    }

    stage('Deploy via kubectl') {
      steps {
        container('maven') {    //同一 Pod 里复用即可
          sh '''
            kubectl --kubeconfig=$KUBECONFIG \
                    set image deployment/myapp myapp=myrepo/myapp:$BUILD_NUMBER
          '''
        }
      }
    }
  }
}
```

### 7.2 Section: post

- post section 在 pipeline 或 stage 的尾部定义一些 step，并根据其所在 pipeline 或 stage 的完成情况来判定是否
运行这些 step。
- post section 支持的 condition 如下所示：
  - `always`：总是运行
  - `changed`：其所处的 pipeline 或 stage 同前一次运行具有不同状态时，才运行该 post。
  - `fixed`：pipeline 或 stage 本次运行成功，但前一次为 failed 或 unstable 时，才运行该 post。
  - `regression`：pipeline 或 stage 前一次运行成功，但本次为 failure、unstable 或 aborted 时，才运行该 post。
  - `success`：pipeline 或 stage 的运行状态为 success 时，才运行该 post。
  - `failure`：pipeline 或 stage 的运行状态为 failed 时，才运行该 post。
  - `unstable`：因测试失败或代码冲突导致 pipeline 或 stage 的运行状态为 unstable 时，才运行该 post。在 Dashboard 中以黄色显示。
  - `aborted`：pipeline 或 stage 的运行状态为 aborted 时，才运行该 post。在 Dashboard 中以灰色显示。  
  - `unsuccessful`：pipeline 或 stage 的运行不成功时，才运行该 post。
  - `cleanup`：在其它所有的 post 的条件均被评估后（无论 pipeline 或 stage 的状态如何）才运行该 post。

  | Condition      | 触发时机 |
  | -------------- | ------------------------------- |
  | `always`       | **无论构建结果如何**，都会执行。 |
  | `changed`      | **构建结果与上一次不同**（如从成功变为失败，或反之） |
  | `success`      | **构建成功**时执行 |
  | `failure`      | **构建失败**时执行 |
  | `unstable`     | **构建结果不稳定**（如测试失败但未完全失败） |
  | `aborted`      | **构建被手动中止**时执行 |
  | `unsuccessful` | **构建未成功**（即失败、不稳定或中止） |
  | `cleanup`      | **无论结果如何**，在所有其他 `post` 条件执行后运行 |

- [Jenkinsfile](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/jenkins/pipeline-syntax-test/Jenkinsfile.post-condition) 示例：

```groovy
pipeline {
  agent any

  stages {
    stage('Build') {
      steps {
        script {
          //模拟构建失败
          error('构建失败！')  //测试1：失败构建
          //echo '构建完成！'  //测试2：成功构建
        }
      }
    }
  }
  
  post {
    always {
      echo '无论成功或失败，我都会运行！'
    }
    changed {
      echo '构建结果与上一次不同！'
    }
    success {
      echo '构建成功！'
    }
    failure {
      echo '构建失败！'
      emailext subject: '构建失败', body: '请检查 Jenkins 日志', to: 'hualongfeiyyy@163.com'
    }
    unstable {
      echo '构建不稳定！可能有测试失败。'
    }
    aborted {
      echo '构建被中止！'
    }
    unsuccessful {
      echo '构建未成功（失败/不稳定/中止）。'
    }
    cleanup {
      echo '清理工作，比如删除临时文件。'
      deleteDir()  //清理工作区
    }
  }
}
```

- 根据以上 Jenkinsfile，分别测试构建成功与构建失败触发 post 执行的过程，如下所示：

<center><img src="images/pipeline-post-condition-error.png" style="width:80%"></center>

上图中以 “测试1：失败构建” 进行流水线运行显示 post 执行日志。

<center><img src="images/pipeline-post-condition-success.png" style="width:80%"></center>

上图中以 “测试2：成功构建” 进行流水线运行显示 post 执行日志。

- 常见用法场景：

  | 场景 | 推荐 `post` 条件 |
  | ----- | ----- |
  | 发送构建通知邮件 | `failure`, `success`, `changed` |
  | 清理临时文件 | `cleanup` |
  | 记录构建状态变化 | `changed` |
  | 构建失败后回滚  | `failure` |
  | 构建被中止时报警 | `aborted` |

### 7.3 Section: stages 和 steps

pipeline 中 stages 和 steps 结构的关系：

<center><img src="images/pipeline-stages-structure.png" style="width:30%"></center>

- stages：
  - 显然，states 是 pipeline 中最重要的 section，Jenkins 会按照 stages 中定义的顺序自下而后执行各个 stage。
  - 封装了用于定义 pipeline 主体和逻辑的所有 stage 的定义，它包含一个或多个 stage 指令。
  - stages 负责描述 pipeline 中绝大部分的实际工作（work）
  - 事实上，stages 中至少需要包含一个 stage 指令来定义 CD 过程的每个离散部分，如构建、测试和部署等。
  - 🔥 stage 内部还能再嵌套一个 `stages{}` 或一个 `parallel{}` 代码块，而后在这些代码块内部再嵌套 stage，以指定 stage 的运行顺序。
    - stage 内部嵌套 `stages{}`，用于指定以**顺序（串行）方式**依次运行该 `stages{}` 内部的各 stage。
    - stage 内部嵌套的 `parallel{}`，用于指定以**并行方式**运行该 `parallel{}` 内部的各 stage。

  > 注意：
  >
  > 1. stage 内部仅能定义 steps、stages、parallel 或 matrix 四者其中之一，且多层嵌套只能用在最后一个 stage 中。
  > 2. 对于本身已经嵌套在 parallel 或 matrix 内部的 stage 来说，不支持在其内部再使用 parallel 或 matrix，但仍能使用 agent、tools 和 when 等其它指令，甚至是 stages{} 配置段以顺序运行stage。

  <center><img src="images/pipeline-test-parallel.png" style="width:80%"></center>

  上述示例可参考 [Jenkinsfile](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/jenkins/pipeline-syntax-test/Jenkinsfile.parallel)：

  ```groovy
  pipeline {
    agent {
      label 'maven'
    }

    stages {
      stage('In Sequential 1') {
        steps {
          echo "In Sequential 1"
        }
      }

      stage('In Sequential 2') {
        steps {
          echo "In Sequential 2"
        }
      }

      stage('Parallel in Sequential') {
        parallel {
          stage('In Parallel 1') {
            steps {
              echo "In Parallel 1"
            }
          }

          stage('In Parallel 2') {
            steps {
              echo "In Parallel 2"
            }
          }
        }
      }
    }
  }
  ```

- steps：
  - 负责在 stage 中定义一到多个 DSL 语句，这些语句负责完成该 stage 中特定的功能，但能够同其它的语句分隔开，如 environment 等。
  - pipeline 中真正 “做事” 的还是其内部一个个具体的 step，因而 steps 是 pipeline 中最核心的组成部署。
    - 除了 script，几乎所有的 step 在 pipeline 中都是不可拆分的原子操作。
    - pipeline 内置了大量的 step，具体请参考 [Pipeline Steps Reference](https://www.jenkins.io/doc/pipeline/steps)。
    - 除此之外，有相当一部分插件可直接当作 step 来用。
  - `script {}` 步骤负责将脚本引入到 `steps {}` 配置段中，但它为非必要的步骤，且复杂的脚本应该单独组织为 `Shared Libraries`，并由Pipeline 导入后使用。

```groovy
pipeline {
  agent any
  
  stages {
    stage('Example') {
      steps {
        echo 'Hello World'
        
        script {
          def distributions = ['RedHat', 'SUSE']
          for (int i = 0; i < distributions.size(); ++i) {
            echo "Testing the ${distributions[i]} system distribution"
          }
        }
      }
    }
  }
}
```

- 💥 在 Jenkins 的 Declarative Pipeline（声明式流水线）里，关于 `step { sh '...' }` 与 `step { script { 'sh ...' } }` 的结构，它们表面上都能执行 shell，但它们在**作用域**、**返回值**、**语法限制**、**生命周期**等方面有本质区别：

  - 1️⃣ 语法位置 & 可读性：
    - `steps { sh '...' }`：
      - 直接位于 Declarative 的 steps 块中，语法简洁、更符合声明式风格。
      - 只能写 “一行” 或 “一块” 简单脚本，不能出现 Groovy 逻辑代码。
    - `steps { script { sh '...' } }`：
      - 把脚本包在 `script { sh '...' }` 里，相当于临时切换到 Scripted Pipeline（脚本式流水线）语法。
      - 可以写完整的 Groovy 逻辑（循环、条件、变量、方法调用等）。
  - 2️⃣ 返回值与后续处理：
    - 直接 `sh`：
      - 返回值默认丢弃，只能通过 `returnStatus: true` 或 `returnStdout: true` 捕获。
      - 不能直接把结果赋值给外部变量（因为声明式作用域不允许赋值）。

      ```groovy
      steps {
        sh 'echo hello'  //只执行，获取不到结果。
      }
      ```

    - `script { sh ... }`：
      - 可以显式接收返回值，再在 Groovy 里继续处理。
      - 可以把结果存到变量，供后面的步骤使用。

      ```groovy
      steps {
        script {
          def msg = sh(script: 'echo hello', returnStdout: true).trim()
          echo "msg=$msg"  //可以继续用 Groovy 处理
        }
      }
      ```

  - 3️⃣ 变量作用域：
    - 直接 `sh`：
      - 无法定义 Groovy 变量
      - 只能使用环境变量、参数或 Credentials
    - `script { sh ... }`：
      - 可以定义 Groovy 变量/闭包/方法，生命周期仅限于该 script 块。
      - 变量不会泄漏到 pipeline 顶层，也不会影响其他 steps。

  - 4️⃣ 错误处理：
    - 直接 `sh`：脚本返回非 0 时，会立即抛异常，导致步骤失败，整个 stage 标红。
    - `script { sh ... }`：可以用 Groovy 的 `try-catch` 捕获返回码，做更细粒度的失败处理。
    - [Jenkinsfile](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/jenkins/pipeline-syntax-test/Jenkinsfile.stages-steps-error) 示例：

    ```groovy
    pipeline {                                
      agent any                               
                                          
      stages {                                
        stage('Example') {                    
          steps {                             
            echo 'Example for building...'                
                                          
            script {                          
              try {      
                //def prefix = sh(script: 'echo \\>', returnStdout: true).trim()  //正确语法：">" 需要双重转义
                def prefix = sh(script: 'echo >', returnStdout: true).trim()  //错误语法：此处将触发异常处理
                def distributions = ['RedHat', 'SUSE']
                for (int i = 0; i < distributions.size(); ++i) {
                  echo "${prefix} Testing the ${distributions[i]} system distribution"
                }                             
              } catch(error) {  //构建失败时抛出的异常实例，实例名称可自定义。
                echo "捕获异常: ${error}"  //完整的异常处理实例，相当于 error.toString()。
                echo "异常消息：${error.getMessage()} and ${error.toString()}"  //error.getMessage() 只返回 error.toString() 的部分内容
                currentBuild.result = "UNSTABLE"  //此阶段将处于 FAILURE 状态（红色颜色块），但此处修改为 UNSTABLE 状态，颜色块将显示为黄色（如下图所示）。
              }                               
            }                                 
          }                                   
        }                                     
      }                                       
    }
    ```

    <center><img src="images/pipeline-error-catch-demo.png" style="width:80%"></center>

    - 上述示例中，`currentBuild.result` 是 pipeline 的一个只写（write-only）的全局变量，用来 “覆盖” 本次构建在 Dashboard 里最终显示的总体结果（颜色块）。需注意以下几点：
      - 只能往更差的方向修改：
        - 初始值是 SUCCESS。
        - 允许把它改成 `UNSTABLE` 或 `FAILURE`，但不能改回 `SUCCESS`。
      - 不会中断流水线：
        - 仅仅改变 Dashboard 图标和 REST/JSON API 中看到的 result 字段。
        - 流水线会继续往下跑，除非自己再抛异常或 error()。
      - 优先级最低：若后面某个步骤又抛异常导致 Jenkins 内部把结果设为 FAILURE，则最终显示仍然是 FAILURE，之前设的 UNSTABLE 会被覆盖。

- pipeline 常用的内置 step：
  - 文件/目录相关：
    - `deleteDir`：删除当前目录
    - `dir("/path/to/dir")`：切换到指定目录
    - `fileExists("/path/to/dir")`：判断文件是否存在
    - `isUnix`：判断是否为类 Unix 系统
    - `pwd`：打印当前目录
    - `writeFile`：将内容写入指定的文件中，支持如下几个参数：
      - `file`：文件路径，支持相对路径和绝对路径。
      - `text`：要写入的内容
      - `encoding`：目标文件的编码，空值为系统默认的编码；支持 base64 编码格式；可选参数。
    - `readFile`：读取文件的内容，支持如下几个参数：
      - `file`：文件路径，支持相对路径和绝对路径。
      - `encoding`：读取文件内容时使用的编码格式；可选参数。
  - [Jenkinsfile](https://github.com/Alberthua-Perl/ansible-demo/blob/master/jenkins-ci-plt/jenkins/pipeline-syntax-test/Jenkinsfile.step-for-file) 示例：

  ```groovy
  pipeline {
    agent any
 
    stages {
      stage('File and Directory Operations') {
        steps {
          //1. 创建文件
          script {
          //使用 sh 命令创建一个测试文件
            sh 'touch testfile.txt'
          }
          //也可以使用 write 命令
          writeFile file: 'testfile2.txt', text: 'Hello Jenkins!'
 
          //2. 创建目录（2种方法）
          script {  //方法1
            sh 'mkdir mydir/'
          }
                
          dir('mydir') {
            sh 'ls -alh'
            //进入目录
          }
          //使用 sh 命令创建目录
          sh 'mkdir -p anotherdir'  //方法2
 
          //3. 复制文件
          script {
            sh 'cp testfile.txt mydir/'
          }
 
          //4. 删除文件
          script {
            sh 'rm testfile2.txt'
          }

          //5. 判断文件是否存在
          script {
            def fileExists = fileExists('testfile.txt')
            echo "5. testfile.txt exists: ${fileExists}"
          }
 
          //6. 删除目录及其内容（2种方法）
          dir('anotherdir') {
            deleteDir()  //方法1
          }
          //script {
          //  sh 'rm -rf anotherdir'  //方法2
          //}
                
          //7. 将文件内容写入
          script {
            writeFile file: 'newfile.txt', text: 'This is new content.'
          }
 
          //8. 读取文件内容
          script {
            def content = readFile('newfile.txt')
            echo "8. File content: ${content}"
          }
 
          //9. 使用归档功能（通常是发布阶段使用）
          archiveArtifacts artifacts: 'testfile.txt', allowEmptyArchive: true
        }
      }
    }
 
    //post {
    //  always {
    //    //清理工作空间（需要 Workspace Cleanup Plugin 插件支持）
    //    cleanWs()
    //  }
    //}
  }
  ```

  - 消息或控制：
    - `echo("message")`：打印指定的消息
    - `error("message")`：主动报错，并中止当前 pipeline。
    - `retry(count){}`：重复执行 count 次在 {} 中定义的代码块
    - `sleep`：让 pipeline 休眠一段时间，支持如下参数：
      - `time`：整数值，休眠时长。
      - `unit`：时间单位，支持 NANOSECONDS、MICROSECONDS、MILLISECONDS、SECONDS、MINUTES、HOURS 和 DAYS；可选参数。
    - `timeout`：代码块的超时时长，支持如下参数：
      - `time`：整数值，休眠时长。
      - `unit`：时间单位，支持 NANOSECONDS、MICROSECONDS、MILLISECONDS、SECONDS、MINUTES、HOURS 和 DAYS；可选参数。
      - `activity`：布尔类型，值为 true 时，表示在该代码块不再有日志活动时才算真正超时；可选参数。
    - `waitUntil`：等待指定的条件满足时执行定义的代码块
      - `initialRecurrencePeriod`：初始的重试周期，即测试条件是否满足的重试周期，默认为 250ms；可选参数。
      - `quiet`：是否禁止将每次的条件测试都记入日志，默认为 false，即记入日志；可选参数。
  - 发送通知：
    - `mail`：向指定邮箱发送邮件
      - `subject`：邮件标题
      - `body`：邮件正文
      - `from(optional)`：发件人地址列表，逗号分隔。
      - `cc(optional)`：CC email 地址列表，逗号分隔。
      - `bcc(optional)`：BCC email 地址列表，逗号分隔。
      - `charset(optional)`：编码格式
      - `mimeType(optional)`：Email 正文的 MIME 类型，默认为 text/plain。
      - `replyTo(optional)`：回件地址，默认为 Jenkins 设置的全局配置中的邮箱地址。

## 8. Pipeline 支持的 Directive

pipeline 支持的 Directive（指令）主要包括：

- `environment`：
  - 设定环境变量，可用于 pipeline 或 stage 代码块中。
  - 支持 `credentials()` 函数，用于通过标识符访问预定义的凭证。
- `tools`：
  - 指定需要在 agent 上下载并配置的工具，如 git、maven、jdk 等，这些工具可经由 PATH 环境变量指定的位置访问到。
  - 可用于 pipeline 或 stage 代码块中。
- `parameters`：
  - 用户在触发 pipeline 时应该提供的参数列表
  - 仅可用于 pipeline 级别
- `options`：
  - 仅可用在 `pipeline` 级别来配置 pipeline 自身的选项，支持的参数可由 pipeline 自身提供，也可由其它插件（如 `timestamps`）提供。
  - 如 `retry(2)` 允许在 pipeline 失败时重试两次
- `triggers`：
  - 用于指定负责自动启动 pipeline 的触发器，对于集成了 Github 或 Gitlab 等自带触发机制的系统场景来说，triggers 并非必须的指令。
  - 仅可用于 pipeline 级别
- `libraries`：当前 pipeline 可以导入的共享库，该共享库内部的代码则可被该 pipeline 调用。
- `stage`：负责在 stages 配置段中封装 steps 配置段，以及其它可用于 stage 中的指令。
- `input`：stage 中的专用指令，用于暂停 pipeline 并提示用户输入内容后继续。
- `when`：stage 中的专用指令，用于设定该 stage 的运行条件。

## 9. Pipeline 的常用步骤

Node 和 Process 相关：

- `bat`：Windows 的批处理脚本
- `powershell`：运行指定的 PowerShell 脚本，支持 Microsoft PowerShell 3+。
- `pwsh`：PowerShell Core Script
- `node`：在指定的节点上运行后续的脚本
- 🔥 `sh`：运行 shell 脚本，支持的参数如下：
  - `script{}`：脚本代码块，支持指定脚本解释器，如 “#!/usr/bin/perl”，否则将使用系统默认的解释器，且使用了 -xe 选项。
  - `encoding(optional)`：脚本执行后输出的日志信息的编码格式，未定义时使用系统默认编码格式。
  - `label(optional)`：显示在 Dashboard 中的详细信息
  - `returnStdout(optional)`：布尔型值，true 表示任务的标准输出将作为 step 的返回值，而不是打印到日志中；若有错误，依然会记入日志。
  - `returnStatus(optional)`：正常情况命令执行失败会返回非零状态码，设定该参数值为 true 时，表示将返回该 step 的结果，而非状态码。
- `ws`：分配工作空间

## 10. 参考链接

- [Pipeline Syntax | Jenkins Doc](https://www.jenkins.io/doc/book/pipeline/syntax/)
- [Pipeline Steps Reference | Jenkins Doc](https://www.jenkins.io/doc/pipeline/steps/)
