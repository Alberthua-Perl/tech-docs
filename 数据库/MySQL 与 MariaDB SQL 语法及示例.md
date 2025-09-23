## MySQL 与 MariaDB SQL 语法及示例

### 文档目录：

- 数据库全局操作

- 用户与权限操作

- SELECT 语句

- INSERT INTO 语句

- UPDATE 语句

### 数据库全局操作：

```sql
### 操作数据库 ###
SHOW DATABASES;
# 查看所有的数据库
USE <database>;
# 使用指定的数据库
CREATE DATABASE <database>;
# 创建数据库
DROP DATABASE <database>;
# 删除数据库与其中的所有数据

### 操作表 ###
SHOW TABLES;
# 查看数据库中所有的表
DESCRIBE <table>;
# 描述指定表的属性信息（列标题）
DROP TABLE <table>;
# 删除数据库中的表与数据记录

SET NAMES utf8;
# 使用 URF-8 编码

SHOW variables LIKE '%char%';
SHOW columns FROM mysql.user;
SHOW full processlist;
# 查看当前数据库实例登录的数据库用户信息
```

### 用户与权限操作：

```sql
SELECT USER();
# 查看当前的用户

SELECT User,Host,Password FROM mysql.user;
# 查看 mysql 数据库 user 表中的用户信息

SELECT DISTINCT User FROM mysql.user;
# 只显示一次同名的用户信息

CREATE USER <username>@<hostname_or_ip> IDENTIFIED BY '<password>';
# 创建数据库用户

DROP USER <username>@<hostname_or_ip>;
# 删除数据库用户及其相应的权限
# 注意：
#   用户的删除不会影响其之前所创建的表、索引或其他数据库对象，因为 MySQL 并不记录由谁创建
#   这些对象。

GRANT ALL PRIVILEGES ON *.* TO root@<hostname_or_ip>;
# 授权数据库 root 用户对数据库的所有权限（该权限过大，慎重使用!）

GRANT SELECT,UPDATE,INSERT,DELETE ON <database>.<table>
-> TO <username>@<hostname_or_ip>;
# 授权用户对表具有查询、更新、插入与删除的权限（CRUD）

GRANT [SELECT|UPDATE|INSERT|DELETE] ON <database>.<table>
-> TO <username>@<hostname_or_ip>;
# 授权用户对表具有查询、或更新、或插入、或删除的权限

REVOKE SELECT,UPDATE,INSERT,DELETE ON <database>.<table>
-> FROM <username>@<hostname_or_ip>;
# 移除用户对表具有查询、更新、插入与删除的权限（CRUD）

REVOKE [SELECT|UPDATE|INSERT|DELETE] ON <database>.<table>
-> FROM <username>@<hostname_or_ip>;
# 移除用户对表具有查询、或更新、或插入、或删除的权限

SHOW GRANTS FOR <username>@<hostname_or_ip>;
# 查询用户权限

FLUSH PRIVILEGES;
# 刷新用户权限信息（每次用户权限更新后都需执行）
```

### SELECT 语句：

```sql
SELECT * FROM <table>;
# 查询整个表的所有记录

SELECT column1,column2,... FROM <table>;
# 查询表中特定列的记录

### DISTINCT 子句 ###
SELECT DISTINCT column1,column2,... FROM <table>;
# 查询表中列的不同值（唯一值）

### WHERE 子句 ###
SELECT column1,column2,... FROM <table> WHERE <column> operator <value>;
# WHERE 子句用于提取满足指定标准的记录

SELECT * FROM websites WHERE country='CN';
# 查询 websites 表中 country 列为 CN 的记录（字符串使用单引号圈引）

SELECT id,name,url FROM websites WHERE id=1;
# 查询 websites 表中 id 为 1 的记录（数值不用单引号）

SELECT * FROM emp WHERE empno=7900;
# 查询 emp 表中 empno 列等于 7900 的记录

SELECT * FROM emp WHERE ename='SMITH';
# 查询 emp 表中 ename 列等于 SMITH 的数据
```

👉 `AND` 与 `OR` 运算符：

- 若第一个条件和第二个条件都成立, 则 AND 运算符显示一条记录。

- 若第一个条件和第二个条件中只要有一个成立, 则 OR 运算符显示一条记录。

```sql
SELECT * FROM websites WHERE country='CN' AND alexa > 50;
# 查询 websites 表中 country 列为 CN 且 alexa 列大于 50 的所有记录

SELECT * FROM emp WHERE sal > 2000 AND sal < 3000;
# 查询 sal 列大于 2000 且小于 3000 的记录

SELECT * FROM emp WHERE sal > 2000 OR comm > 500;
# 查询 sal 列大于 2000 或 comm 列大于 500 的记录

SELECT * FROM websites WHERE country='USA' OR country='CN';
# 查询 websites 表中 country 列为 USA 或者 CN 的所有记录

SELECT * FROM websites WHERE alexa > 15
-> AND (country='USA' OR country='CN');
# 查询 alexa 列大于 15 且 country 列为 CN 或 USA 的所有记录
```

👉 `BETWEEN ` 运算符：在指定范围之间的值

```sql
SELECT * FROM emp WHERE sal BETWEEN 1500 AND 3000;
# 查询 sal 列中在 1500 与 3000 之间的值
```

👉 `IN` 运算符：其中的值

```sql
SELECT * FROM emp WHERE sal IN (3000,5000,1500);
# 查询 sal 列中符合的值
```

👉 `LIKE` 运算符：模糊匹配

```sql
SELECT * FROM emp WHERE ename LIKE 'M%';
# 查询 ename 列中开头为 M 的字符串
# 注意：
#   %   : 多个字符
#   _   : 下划线，单个字符。
#   M%  : 模糊匹配，匹配以 M 开头的字符串。
#   %M% : 匹配包含 M 的字符串
#   %M_ : 匹配倒数第二个字符为 M 的字符串
```

👉 `NOT` 运算符：

```sql
SELECT * FROM emp WHERE NOT sal > 1500;
# 查询 sal 列小于等于 1500 的记录
```

👉 `is null`：空值

```sql
SELECT * FROM emp WHERE comm is null;
# 查询 comm 列中的空值
```

👉 `ORDER BY` 关键字：

- 用于对结果集进行排序

- 用于对结果集按照一个列或者多个列进行排序

- 默认按照升序对记录进行排序

- 如果需要按照降序对记录进行排序，可以使用 `DESC` 关键字，默认为 `ASC` 升序排列。

```sql
SELECT column1,column2,... FROM <table>
-> OEDER BY column1,column2,... [ASC|DESC];
```

```sql
SELECT * FROM websites ORDER BY alexa;
# 查询 websites 表并按照 alexa 列升序排序

SELECT * FROM websites ORDER BY alexa ASC;
# 查询 websites 表并按照 alexa 列升序排序

SELECT * FROM websites ORDER BY alexa DESC;
# 查询 websites 表并按照 alexa 列降序排序

SELECT * FROM websites ORDER BY country,alexa;
# 根据属性的多列排序
# 注意：
#   ORDER BY 多列的时候，先按照第一个列排序，再按照第二个列排序：
#   1. 先将 country 这列排序，同为 CN 的排前面，同为 USA 的排后面。
#   2. 然后在同为 CN 的这些多行数据中，再根据 alexa 列的值的大小排列。
```

### INSERT INTO 语句：

- `INSERT INTO` 语句用于向表中插入新记录，即插入新行。

- INSERT INTO 语句可以有两种编写形式：
  
  - 1️⃣ 第一种形式无需指定要插入数据的列名，只需提供被插入的值即可：
    
    ```sql
    INSERT INTO <table> VALUES (value1,value2,value3,...);
    ```
  
  - 2️⃣ 第二种形式需要指定列名及被插入的值：
    
    ```sql
    INSERT INTO <table> (column1,column2,column3,...)
    -> VALUES (value1,value2,value3,...);
    ```

- 示例：
  
  ```sql
  INSERT INTO websites (name,url,alexa,country)
  -> VALUES ('百度','https://www.baidu.com','4','CN');
  # 根据表属性（列的名称）以行的方式插入数据记录
  # id 列是自动更新的，表中的每条记录都有一个唯一的数字。
  
  INSERT INTO websites (name,url,country)
  -> VALUES ('stackoverflow','http://stackoverflow.com','CN');
  # 指定列插入指定数据
  # 问题：如何根据列名称在特定位置插入行？
  ```

### UPDATE 语句：

- `UPDATE` 语句用于更新表中的记录：
  
  ```sql
  UPDATE table_name
  -> SET column1=value1,column2=value2,...
  -> WHERE some_column=some_value;
  ```

- 💥 WHERE 子句规定哪条记录或哪些记录需更新，若省略 WHERE 子句，所有的记录都将被更新！

- 示例：
  
  ```sql
  UPDATE product
  -> SET price=89.90, stock=60
  -> WHERE id=5;
  # 更新 product 表中 id 为 5 的行，设置 price 与 stock 的值。
  
  DELETE FROM product  WHERE id=5;
  # 删除表中 id 为 5 的记录
  # 注意：若 DELETE 语句中不使用 WHERE 子句，则表中的所有记录都将被删除！
  ```
