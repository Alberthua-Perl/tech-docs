# 🐂 GCC 常用方法与调试技巧

## 文档目录
- [🐂 GCC 常用方法与调试技巧](#-gcc-常用方法与调试技巧)
  - [文档目录](#文档目录)
  - [Linux 中查看与指定支持的 C 语言标准](#linux-中查看与指定支持的-c-语言标准)
  - [指定 C 语言标准编译](#指定-c-语言标准编译)
  - [参考链接](#参考链接)

## Linux 中查看与指定支持的 C 语言标准

```bash
$ gcc -E -dM - < /dev/null | grep "STDC_VERSION"
  #define __STDC_VERSION__ 201710L
# 宏定义（macros）显示 GCC 默认支持的 C 语言标准（C17）
```

输出结果和 C 语言标准的对应关系如下：

- 如果是 `#define __STDC_VERSION__ 199901L`，则默认支持的是 `C99` 标准。
- 如果是 `#define __STDC_VERSION__ 201112L`，则默认支持的是 `C11` 标准。
- 如果是 `#define __STDC_VERSION__ 201710L`，则默认支持的是 `C17` 标准。
- 如果没查到，则默认支持的是 `C89` 标准。

## 指定 C 语言标准编译

当查询到当前 GCC 编译器支持的 C 语言标准后，如果想在编译时指定 C 语言标准，可以使用 `-std` 选项进行指定，常用的（非全部）选项如下：

```bash
-std=c17    # Conform to the ISO 2017 C standard
-std=c11    # Conform to the ISO 2011 C standard
-std=c99    # Conform to the ISO 1999 C standard
-std=c90    # Conform to the ISO 1990 C standard
-std=c89    # Conform to the ISO 1990 C standard

-std=gnu17  # Conform to the ISO 2017 C standard with GNU extensions
-std=gnu11  # Conform to the ISO 2011 C standard with GNU extensions
-std=gnu99  # Conform to the ISO 1999 C standard with GNU extensions
-std=gnu90  # Conform to the ISO 1990 C standard with GNU extensions
-std=gnu89  # Conform to the ISO 1990 C standard with GNU extensions
```

在 Linux 系统中，默认情况下如果不指明 `-std` 选项，GCC 会使用 `-std=gnu11` 作为默认支持的 C 语言版本，也就是 C11 标准加上 `GCC extension` 的组合。

## 参考链接
