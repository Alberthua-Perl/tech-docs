## Web 相关命令使用汇总

- wget 命令使用：
  
  - wget 命令常用选项：
    
    ```bash
    -c    断点续传（同时与 -r 选项使用时下载速度较慢）。
    -r    递归下载，即下载指定网页某一目录下（包括子目录）的所有文件。 
    -nd   递归下载时不创建一层一层的目录，而是将所有的文件下载到当前目录。
    -np   递归下载时不搜索上层目录。
          若下载 http://<url>/<path>/ 没有加 -np 选项，就会同时下载 
          <path> 的上一级目录中的其它文件。
    -k    将绝对链接转为相对链接，下载整个站点后脱机浏览网页，最好加上该参数。 
    -L    递归时不进入其它主机。
          若下载 www.xxx.org，但网站内有一个这样的链接 www.yyy.org，
          不加 -L 选项，就会递归下载 www.yyy.org 网站。
    -p    下载网页所需的所有文件，如图片等。
    -A    指定要下载的文件样式列表，多个样式用逗号分隔。
    -i    指定文件，文件内指明要下载的 URL 地址。
    -R, -–reject=LIST
          逗号分隔的不被接受的扩展名的列表。
          若不加该参数，结果中将包含大量的 "index.html"、"index.html?C=D;O=A"、
          "index.html?C=D;O=D" 等文件。
    -q    静默模式，不输出 wget 的内容、进度等信息（脚本处理中常用）。
    ```
  
  - wget 命令示例：
    
    ```bash
    $ wget -O <filename> -q http://<url>:<port>
    $ wget -m -c -np -nH \
      --cut-dirs=4 -e robots=off -P <dir_prefix> http://<url>      
    ```
  
  - wget 递归下载目录中的所有文件：
    
    ```bash
    $ wget -c -r -nd -np -k -L -p -A c,h http://<url>/<path>/
    $ wget -c -r -nd -np -R "index.html*" http://<url>/<path>/
    # 两种示例方法
    ```

- curl 命令使用：
  
  - curl 命令常用选项：
    
    ```bash
    -v, --verbose            显示 URL 连接过程中的详细信息，可用于 debug 调试。
    -i, --include            显示 URL 连接的输出，包含 http 头（http-header），属于 -v 选项子集。  
    -I, --head               显示 http 头信息，属于 -v 选项子集。
    -x, --proxy              使用指定的代理与端口，如 <protocol>://<host>:<port>。
    -L, --location           若服务端显示目标页面已更换地址（header 返回码 3XX），可重定向目标页面。
                             若更换过的目标页面需要认证用户名与密码，不可使用 --user 选项。
    --digest                 启用 http digest 认证，密码在传输过程中依然为明文。
                             与 --user 选项提供认证用户名与密码一起使用。
    -u, --user <user:passwd> 提供服务端所需的认证用户名与密码              
    -X, --request            指定 http 请求方式，包括 GET、PUT、POST 与 DELETE（CRUD 操作）等，
                             可用于 restful api 调试。
    -s, --silent             不显示命令的错误信息（静默模式）
    -S, --show-error         显示命令的错误信息
    -f, --fail               连接失败时不显示 http 错误
    -#，--progress-bar       以进度条方式显示下载或输出进度
    -o, --output <filename>  将返回的输出信息写入指定文件中而不是标准输出，可重定向下载的文件。
    -O, --remote-name        将下载文件的文件名与远程源文件文件名设置相同
    -k, --insecure           允许 curl 命令使用 "非安全" 的 SSL 连接与转发，如使用自签名的 CA 证书。
    ```
    
    ![](D:\Linux操作系统与编程语言汇总\Typora文档汇总\Linux\pictures\Web相关命令使用汇总\curl-k-https.jpg)
  
  - curl 命令示例：
    
    ```bash
    $ curl [-s | -S] http://servera.lab.example.com
    # 查看 URL 的 http 返回信息，不包含 http 头信息。
    
    $ curl --progress-bar -o <filename> http://servera.lab.example.com
    # 以进度条方式显示下载进度，并将输出写入指定文件中。 
    
    $ curl -w %{http_code} http://servera.lab.example.com
    # 查看 URL 的 http 返回状态码
    
    $ curl -X GET http://servera.lab.example.com
    # 使用 GET 方法查询 URL
    # 若该 URL 为 RESTful API 接口，可使用该命令进行调试。
    
    $ curl -I -x <http_server_ip>:<port> http://<fqdn> 
    ```
