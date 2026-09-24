# Linux 服务与配置

- 此目录用于 Linux 中各服务部署实施与配置使用
- 目前关于网站架构一般比较合理流行的架构方案：
  - Web 前端负载均衡器：Nginx/HAProxy + Keepalived
  - 后端负载均衡器：LVS + Keepalived + MySQL 数据库一主多从 + 读写分离