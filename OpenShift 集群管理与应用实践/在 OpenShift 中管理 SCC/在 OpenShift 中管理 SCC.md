# ⭕ 在 OpenShift 中管理 SCC

## 文档说明

此文档是 [Managing SCCs in OpenShift](https://www.redhat.com/en/blog/managing-sccs-in-openshift) 的中文翻译与更改版。

## 文档目录

- 引出
- SCC 的内部究竟是如何工作的？

## 引出

- 在第一篇文章中，我们研究了 Linux 和进程权限，并了解了容器如何具有完全相同的行为。我们看到这些权限特定于容器进程在其下运行的 Linux 用户。
- 此外，安全上下文（security context）是一种特殊配置，可以在您的 pod 或容器清单上设置，以从 OpenShift API 请求其中一些权限。
- 最后，我们得出结论，SCC 是 OpenShift 提供的工具，用于控制平台上允许为每个 pod 请求哪种权限。
- SCC 限制 OpenShift 中运行的 pod 访问主机环境（host resources）。
- SCC 控制以下主机资源：
  - 运行提权的容器
  - 为容器请求额外的能力（capabilities）
  - 使用主机目录作为卷
  - 更改容器的 SELinux 上下文
  - 更改用户 ID

## SCC 的内部究竟是如何工作的？

进一步了解 SCC 的工作原理，我们可能会问：OpenShift 内部的什么负责检查 Pod 是否符合要求？API 服务器中的准入流程（admission process）将负责这项工作。OpenShift 配备了 8 个预定义的 SCC，您可以使用 `oc get scc` 命令列出这些约束。对于那些不熟悉这些约束的人，​​这里尝试按限制性从最严格到最不严格列出（但请查看注释）：

- 1️⃣ `restricted`：
  - 描述：
    - 拒绝访问所有主机功能，并要求 pod 使用分配给命名空间的 UID 和 SELinux 上下文运行。
    - 这是最严格的 SCC，默认情况下用于经过身份验证的用户。
  - 评论：换句话说，这是最安全的。我们将在这篇博文中进一步详细解释这一点。
- 2️⃣ `nonroot`：
  - 描述：
    - 提供受限 SCC 的所有功能，但允许用户使用任何非 root UID 运行。
    - 用户必须指定 UID，或者必须在容器运行时的清单上指定。
  - 评论：需要可预测的非 root UID 且具有相同其他受限 SCC 安全功能的应用程序可以使用此 SCC，只要它们在其清单中告知 UID 即可。
- 3️⃣ `anyuid`：
  - 描述：提供受限 SCC 的所有功能，但允许用户以任何 UID 和任何 GID 运行。
  - 评论：
    - 在 Kubernetes 和 OpenShift 等平台中，这相当于允许 UID 0 或 root 用户在容器内部和外部使用。
    - 这将在后续博客文章中讨论。
    - SELinux 在这里发挥着重要作用，增加了一层保护，使用 `seccomp` 过滤不需要的系统调用也是一个好主意。
- 4️⃣ `hostmount-anyuid`：
  - 描述：
    - 提供受限 SCC 的所有功能，但允许 pod 进行主机挂载和任何 UID。
    - 这主要由持久卷回收器（persistent volume recycler）使用。
    - 警告：此 SCC 允许以任何 UID（包括 UID 0）访问主机文件系统。请谨慎授予。
  - 评论：
    - 与 anyuid 相同的警告，但这里更进一步，还允许安装主机卷。
    - 请注意，描述中提到的卷回收器是受信任的工作负载和必不可少的基础设施部分。
- 5️⃣ `hostnetwork`：
  - 描述：允许使用主机网络和主机端口，但仍需要使用分配给命名空间的 UID 和 SELinux 上下文来运行 pod
  - 评论：
    - 在这里，pod/容器将能够直接 "查看和使用" 主机网络堆栈。
    - 非零 UID 和预分配的 SELinux 上下文将有助于增加另一层安全性。
- 6️⃣ `node-exporter`：
  - 描述：node-exporter scc 用于 Prometheus 节点导出器。
  - 评论：
    - 节点导出器专为 Prometheus 设计，用于从集群检索指标。
    - 它允许访问主机网络、主机 PIDs 和主机卷，但不允许访问主机 IPC。
    - 还允许 anyuid。
    - 不得由其他应用程序使用。
- 7️⃣ `hostaccess`：
  - 描述：hostaccess 允许访问所有主机命名空间，但仍要求 pod 使用分配给命名空间的 UID 和 SELinux 上下文运行。警告：此 SCC 允许主机访问命名空间、文件系统和 PID。它只能由受信任的 pod 使用。请谨慎授予。
  - 评论：描述中的主机命名空间是指 pod 或容器命名空间之外，或者，我们可以将其称为节点或根 Linux 命名空间。限制 UID 和使用 SELinux 确实会增加一层安全性来保护节点。但这仍然是一个非常宽松的 SC​​C，应该只由绝对必要的受信任工作负载使用。
- 8️⃣ `privileged`：  
  - 描述：特权允许访问所有特权和主机功能，并能够以任何用户、任何组、任何 fsGroup 和任何 SELinux 上下文运行。警告：这是最宽松的 SC​​C，应仅用于集群管理。请谨慎授予。
  - 评论：此 scc 允许 pod/容器控制主机/工作节点甚至其他容器中的所有内容。这是最特权和最宽松的 SC​​C 策略。只有受信任的工作负载才应使用此策略，并且讨论是否应在生产中使用它是有效的。特权 pod 可以完全控制主机。
