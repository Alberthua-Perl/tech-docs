# 🔎 find 与 locate 命令常用示例

## 文档目录

- [🔎 find 与 locate 命令常用示例](#-find-与-locate-命令常用示例)
  - [文档目录](#文档目录)
  - [find 命令常用示例](#find-命令常用示例)
  - [locate 命令常用示例](#locate-命令常用示例)

## find 命令常用示例

- find 命令使用注意：find 查找文件或目录时通过扫描磁盘来进行，尽可能不要大范围的查找文件或目录，尤其是在 `/` 目录下搜索，会长时间消耗服务器的 CPU 资源。如果是生产环境的机器，执行前要考虑是否会对业务造成影响！
- find 命令的说明：
  - 通过在文件系统中实时（real time）搜索来查找文件。
  - 比 locate 命令慢，但准确度更高。
  - find 命令还可以根据文件权限、文件类型、文件大小或修改时间、按文件名查找文件。
- 按文件名查找文件或目录（可使用通配符）：
  - `-name` 选项：指定文件名称模式（pattern），实现精确匹配。
  - `-iname` 选项：忽略文件名称模式中的大小写
  - `-inode` 选项：指定文件 `inode` 编号查找
  - 示例：

    ```bash
    $ find / -name sshd_config
    # 查找名为 sshd_config 的文件或目录
    
    $ find /etc -name "*pass*"
    # /etc 目录中查找具有 pass 字段的文件或目录
    
    $ find /etc -name "[A-Z]*.txt"
    # /etc 目录中查找大写字母开头的 txt 文件
    
    $ find / -iname "*messages*"
    # 执行不区分大小写的文件名或目录名查找
    
    $ find /etc -iname "*.conf" -exec cp {} /tmp/conf.bak \;
    # /etc 目录中查找名称中具有 conf 的文件（忽略大小写），并将其拷贝至 /tmp/conf.bak 
    # 目录中。
    ```

    ```bash
    $ find . -name "*.txt" -prune -o -name "*.conf"
    # 当前目录中查找名称中除了 txt 结尾的而以 conf 结尾的文件
    
    $ find /etc/systemd/system/ -name "*conf*" -prune -o -name "*.service"
    # /etc/systemd/system/ 目录中查找除名称中带有 conf 字段的以 .service 结尾的文件
    $ find /etc/systemd/ -path "/etc/systemd/user" -prune -o -name "*.conf"
    # /etc/systemd/ 目录中查找 /etc/systemd/user 目录外的名称中具有 .conf 字段的文件
    # 注意：-prune -o 选项：排除文件（-name 指定）或目录（-path 指定）
    
    ### 示例 ###
    ┌─[root][myrhel8][~]
    └─➞ find /etc/systemd/ -path "/etc/systemd/user" -prune -o -name "*.conf"
    /etc/systemd/bootchart.conf
    /etc/systemd/coredump.conf
    /etc/systemd/journald.conf
    /etc/systemd/system/dhcpd.service.d/training.conf
    /etc/systemd/system/libvirtd.service.d/training.conf
    /etc/systemd/system.conf
    /etc/systemd/user.conf
    /etc/systemd/logind.conf
    ```
  
  - find 命令中 `-print` 选项与 `-print0` 选项的区别：
    - 👉 -print 选项：在标准输出中输出整行，该选项为默认行为，可省略。
    - 👉 -print0 选项：将查找的结果传至管道后的命令，而管道后的命令需使用 `xargs -0` 接收。

    ```bash
    $ man find
    ...
    -print  True; print the full file name on the standard output, followed by a 
            newline. If you are piping the output of find into another program and 
            there is the faintest possibility that the files which you are searching 
            for might contain a newline,  then  you  should  seriously  consider using
            the -print0 option instead of -print.  See the UNUSUAL FILENAMES section 
            for information about how unusual characters in filenames are handled.
    
    -print0  True; print the full file name on the standard output, followed by a null
             character (instead of the  newline  character  that  -print  uses). This 
             allows file names that contain newlines or other types of white space to 
             be correctly interpreted by programs that process the find output. This 
             option corresponds to the -0 option of xargs.
    ...
    ```

- 按文件类型查找文件或目录：
  - find 命令中支持的文件类型（`-type` 选项）：
    - `f`：普通文件
    - `d`：目录
    - `l`：软链接
    - `s`：套接字
    - `p`：管道
    - `b`：块设备（如磁盘）
    - `c`：字符设备（如键盘、鼠标、网卡）
  - 示例：

    ```bash
    $ find /path/to/dir/ -type d
    # 查找指定目录中的所有目录
    
    $ find /dev -type b
    # 查找系统上的所有块设备文件
    
    $ find /path/to/dir/ -type f -links +1
    # 查找指定目录中硬链接数大于 1 的文件
    
    $ find /path/to/dir/ -type f -links -2
    # 查找指定目录中硬链接数小于 2 的文件
    
    $ find . -type [f|d] -empty
    # 当前目录中查找空文件或空目录
    
    $ find . -type [f|d] ! -empty
    # 当前目录中查找非空文件或目录
    ```

- 按大小查找文件或目录：
  - `-size` 选项：按指定大小查找文件
  - 该选项的单位如下所示：
    - `b`：512 字节块（默认）
    - `c`：字节
    - `w`：双字节字
    - `k`：KiB
    - `M`：MiB
    - `G`：GiB
  - `find` 与 `du` 命令中的 `k`、`M`、`G` 都为二进制的容量单位，换算时以 `1024` 为基础。
  - 示例：

    ```bash
    $ find /path/to/dir/ -size 10M
    # 指定目录中查找大小为 10M 的文件
    
    $ find /path/to/dir/ -size +10G
    # 指定目录中查找大小大于 10G 的文件
    
    $ find /path/to/dir/ -size -10k
    # 指定目录中查找大小小于 10k 的文件
    ```
  
  - 查找大文件的方法：

    ```bash
    $ find . -type f -size +800M
    # 当前目录中查找大于 800M 的文件
    
    $ find / -type f -size +800M -print0 | xargs -0 ls -lh
    # 根目录中查找大于 800M 的文件，并打印其长格式的属性。
    
    $ find / -type f -size +800M -print0 | xargs -0 du -h[s]
    # 根目录中查找大于 800M 的文件，并且显示其具体的容量大小。
    
    $ find / -type f -size +800M -print0 | xargs -0 du -h[s] | sort -nr
    # 根目录中查找大于 800M 的文件，并且根据数值大小反向排序显示其具体的容量。
    ```
  
  - 查找大目录的方法：

    ```bash
    $ du -hs /path/to/dir/
    # 查看指定目录的总大小
    # 注意：-s 选项不能与 --max-depth 选项一起使用，两者冲突！
    
    $ du -h /path/to/dir/ --max-depth=N | sort -n
    # 查看指定目录中 N 级目录的大小，结果以正向排序显示。
    
    $ du -h[m] /path/to/dir/ --max-depth=N | sort -n
    # 查看指定目录中 N 级目录的大小，结果以 M 的大小显示。
    
    ### 示例 ###
    $ du -h ~/backup/vm-imgs/ --max-depth=2 | sort -n
    136G    /home/alberthua/backup/vm-imgs/
    $ du -hm ~/backup/vm-imgs/ --max-depth=2 | sort -n
    138869  /home/alberthua/backup/vm-imgs/
    ```

- 按指定时间戳查找文件或目录：
  - `-atime` 选项：访问时间戳，最后一次访问（读取）文件或目录的时间。
  - `-mtime` 选项：修改时间戳，文件或目录内容最后一次被修改的时间。
  - `-ctime` 选项：更改时间戳，文件或目录元数据最后一次被修改的时间（如，所有权、位置、文件类型和权限等）。
  - `-newer` 选项：以修改指定的文件内容为基础进行时间戳比较
  - 时间戳中的 `+` 与 `-`：
    - `+`：表示大于指定的时间（默认为天数 `n*24`）
    - `-`：表示小于指定的时间
    - 若时间戳中不使用任何符号，则表示具体指定的时间。
    - 时间关系如下所示：👉 此处以 `5` 天为例

      ```bash
      Days  ...    9    8    7    6    5    4    3    2    1    now
            |<----------- +5 --------->|<--------- -5 ---------->|
      ```
  
  - 示例：

    ```bash
    $ find . -type f -atime +365 -exec rm -rf {} \;
    # 当前目录中查找 365 天前访问的文件并将其删除
    
    $ find . -type f -mtime 5
    # 当前目录中查找 5 天前修改过内容的文件
    
    $ find . -type f -ctime +5 -ctime -10
    # 当前目录中查找 5~10 天内更改过元数据的文件
    
    $ find /path/to/dir/ -mmin +200
    # 指定目录中查找 200 分钟以前修改过内容的文件或目录
    
    $ find /path/to/dir/ -mmin -150
    # 指定目录中查找 150 分钟以内修改过内容的文件或目录
    
    $ find . -type f -newer "testfile1"
    # 当前目录中查找比 testfile1 文件更新的文件
    $ find . -type f ! -newer "testfile1"
    # 当前目录中查找比 testfile1 文件更旧的文件
    ```

- ✨ 按权限查找文件或目录：
  - `-user` 选项：以用户名查找
  - `-group` 选项：以用户组查找
  - `-uid` 选项：以用户 ID 查找
  - `-gid` 选项：以组 ID 查找
  - `-nouser` 选项：查找没有所有者的文件
  - `-nogroup` 选项：查找没有所属组的文件
  - 🚀 `-perm` 选项：⽤于查找具有特定权限集的文件或目录
  - 权限可以描述为八进制值，包含代表读取、写⼊和执行的 4、2 和 1 的某些组合。
  - 权限前⾯可以加上 `/` 或 `-` 符号。
  - 👉 权限前面有 `/` 符号：匹配该权限集中的至少一位（相当于 `or` 运算）
  - 👉 权限前带有 `-` 符号：匹配该权限集中的所有三个权限（相当于 `and` 运算）
  - 👉 与 `/` 或 `-` 一起使用时， `0` 值类似于通配符，表示无任何内容的权限。
  - 示例：

    ```bash
    $ find /path/to/dir/ -user <username>
    # 查找所有者为指定用户的文件或目录
    
    $ find /path/to/dir/ -group <groupname>
    # 查找所属组为指定组的文件或目录
    
    $ find /path/to/dir/ -uid <uid>
    # 查找指定 UID 的文件或目录
    
    $ find /path/to/dir/ -gid <gid>
    # 查找指定 GID 的文件或目录
    ```

    ```bash
    $ find /path/to/dir/ -perm 764
    # 查找用户具有读取、写入与执行权限、组具有读取、写入权限与其他人具有读取权限
    # 的文件与目录。
    
    $ find /path/to/dir/ -perm -324
    # 查找用户至少具有写入和执行权限、组至少具有写入权限与其他人至少具有读取权限
    # 的文件与目录。
    
    $ find /path/to/dir/ -perm /442
    # 查找用户具有读取权限、或者组具有读取权限、或者其他人具有写入权限的文件与目录。
    
    $ find /path/to/dir/ -perm -002
    # 查找其他人至少具有写入权限的文件与目录
    
    $ find / -type d -perm 777 -exec chmod 755 {} \;
    # 根目录中查找权限为 777 的目录，并将其更改为 755 权限。
    
    $ find /home -user foo -o -user bar -type f -perm /642 -size -5M 
    # /home 目录中查找所有者为 foo 用户或 bar 用户，所有者可读可写、或者所属组可读、
    # 或者其他用户可写的权限，大小小于 5M 的文件。 
    ```

- 多条件的组合：find 命令也可以使用多个条件的组合，支持 `-a`、`-o`、`-not` 或 `!` 选项，在以上的示例中均已列出，此处不再赘述。
- ✨ 处理动作：
  - `-print`：打印，默认动作，可省略。
  - `-ls`：以 ls 长文件格式输出
  - `-delete`：删除查找到的文件
  - `-exec`：查找到的文件传递给任何 Linux 命令
  - `-ok`：与 -exec 功能相同，区别是需要用户确认每次的操作。

## locate 命令常用示例

- `locate` 命令查找预生成索引中的文件名或文件路径，并即时返回结果。
- `find` 命令通过遍历整个文件系统来实时查找文件。
- 根据名称查找文件：
  - locate 命令从 `mlocate` 数据库中查找，速度比较快。
  - 但数据库不会实时更新，必须经常更新才能保持结果准确。
  - locate 数据库每日自动更新，`updatedb` 命令可以手动更新。
  - locate 命令与 updatedb 命令由 mlocate 软件包提供。
  - 示例：

    ```bash
    $ locate <pattern>
    # 查找匹配模式的文件
    $ locate -i <pattern>
    # 忽略匹配模式中的大小写查找文件
    $ locate -n <number> <pattern>
    # 查看指定前几行的文件列表
    ```
