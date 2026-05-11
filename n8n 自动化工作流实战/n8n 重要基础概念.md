# n8n 重要基础概念

## 文档目录

- [n8n 重要基础概念](#n8n-重要基础概念)
  - [文档目录](#文档目录)
  - [1. n8n 中的节点类型](#1-n8n-中的节点类型)
  - [2. n8n 中数据存储方式](#2-n8n-中数据存储方式)
  - [参考链接](#参考链接)

## 1. n8n 中的节点类型

- 1️⃣ 触发节点（Trigger Node）：启动工作流的节点，就像 "开关" 一样。
  - Webhook 节点：接收外部系统的 HTTP 请求
  - 定时器节点（Schedule Trigger）：按时间计划自动执行（如每天上午9点）
  - 邮件触发器（Email Trigger）：收到新邮件时自动触发
  - 文件触发器（Local File Trigger）：检测到新文件时触发
- 2️⃣ 操作节点（Action Node）：执行具体操作的节点
  - HTTP Request 节点：调用任何 API 接口
  - 数据库节点：连接 MySQL、PostgreSQL 等数据库
  - 代码节点（Code）：执行自定义 JavaScript 代码或 Python 代码
  - AI 节点：集成 GPT、Claude 等大语言模型
  - 条件判断节点（If）：根据条件分支执行不同流程

## 2. n8n 中数据存储方式

n8n 中各类数据存储于用户家目录的 `$HOME/.n8n/database.sqlite` 数据库中，因此，可通过以下方式安装 SQLite 命令行与可视化工具。

```bash
(llm-aiops) [kernel-aiops] godev › ~ $ cd ~/.n8n
# 切换进入用户家目录中的 n8n 根目录
(llm-aiops) [kernel-aiops] godev › .n8n $ ls -lh
total 5.2M
-rw------- 1 godev godev  170 May  4 21:34 config
-rw-r--r-- 1 godev godev 1.1M May  4 21:39 database.sqlite
-rw-r--r-- 1 godev godev  32K May  7 19:48 database.sqlite-shm
-rw-r--r-- 1 godev godev 4.0M May  7 19:48 database.sqlite-wal
-rw-rw-r-- 1 godev godev    0 May  5 08:55 n8nEventLog-1.log
-rw-rw-r-- 1 godev godev  55K May  4 23:18 n8nEventLog-2.log
-rw-rw-r-- 1 godev godev 4.2K May  4 21:37 n8nEventLog-3.log
-rw-rw-r-- 1 godev godev 7.8K May  7 19:27 n8nEventLog.log
drwxrwxr-x 2 godev godev   26 May  4 15:46 nodes
drwxrwxr-x 3 godev godev   23 May  4 20:43 storage
(llm-aiops) [kernel-aiops] godev › .n8n $ sudo apt install -y sqlite3 sqlitebrowser
# Debian 系列使用此命令安装 SQLite
(llm-aiops) [kernel-aiops] godev › .n8n $ sqlite3 database.sqlite    # 打开并查询 SQLite 数据库
SQLite version 3.45.1 2024-01-30 16:01:20
Enter ".help" for usage hints.
sqlite> SELECT email FROM user;    # 查询 n8n 中用户与 email 的关系
hualongfeiyyy@163.com
sqlite> .quit    # 退出数据库
```

## 参考链接

- [Jenkins 的 CI/CD 之旅 —— Node.js 之 npm 常用方法 | GitHub](https://github.com/Alberthua-Perl/tech-docs/blob/master/DevOps%20%E6%8A%80%E6%9C%AF%E6%A0%88/Jenkins%20%E7%9A%84%20CICD%20%E4%B9%8B%E6%97%85/Node.js%20%E4%B9%8B%20npm%20%E5%B8%B8%E7%94%A8%E6%96%B9%E6%B3%95/Node.js%20%E4%B9%8B%20npm%20%E5%B8%B8%E7%94%A8%E6%96%B9%E6%B3%95.md#jenkins-%E7%9A%84-cicd-%E4%B9%8B%E6%97%85--nodejs-%E4%B9%8B-npm-%E5%B8%B8%E7%94%A8%E6%96%B9%E6%B3%95)
- [SQLite 教程 | RUNOOM.COM ](https://www.runoob.com/sqlite/sqlite-tutorial.html)
