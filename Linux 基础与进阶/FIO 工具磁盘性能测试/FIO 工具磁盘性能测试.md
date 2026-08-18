# FIO 工具磁盘性能测试

## 文档目录

- [FIO 工具磁盘性能测试](#fio-工具磁盘性能测试)
  - [文档目录](#文档目录)
  - [fio 命令测试示例与解读](#fio-命令测试示例与解读)
  - [fio 测试脚本示例](#fio-测试脚本示例)
  - [Linux 内核故障注入框架（fail\_io）：启用前提](#linux-内核故障注入框架fail_io启用前提)
  - [Linux 内核故障注入框架（fail\_io）：示例命令](#linux-内核故障注入框架fail_io示例命令)
  - [参考链接](#参考链接)

## fio 命令测试示例与解读

`fio` 常规测试命令：

```bash
$ time fio \
  --name=128K_write_4.result \
  --filename=/dev/nvme0n1 \
  --diect=1 \
  --iodepth=4096 \
  --thread \
  --ioengine=libaio \
  --runtime=120 \
  --size=1T \
  --numjobs=4 \
  --group_reporting \
  --bs=128K \
  --rw=write \
  --output=./128K_write_4_6.result
```

💥 注意：`timebase + runtime` 选项精确控制 fio 运行时间

## fio 测试脚本示例

fio_qps_test.sh 脚本如下：

```bash
#!/bin/bash
#
# FIO QPS 压测脚本

TEST_FILE="/data/fio_testfile"   # 测试文件路径 (不要放在数据库数据目录!!!)
SIZE="1G"                        # 测试文件大小
BS="4k"                          # 块大小
RUNTIME="30"                     # 每个测试持续时间(秒)
IODEPTH="64"                     # 队列深度

# 检查是否安装 fio
if ! command -v fio &>/dev/null; then
    echo "未检测到 fio，请先安装，即 yum install -y fio 或 apt install -y fio"
    exit 1
fi

echo "============================================="
echo " FIO 磁盘 QPS 压测开始 "
echo " 文件: $TEST_FILE, 大小: $SIZE, 块大小: $BS, 时长: ${RUNTIME}s, 队列深度: $IODEPTH"
echo "============================================="

# 结果保存变量
RESULTS=()

function fio_test() {
    NAME=$1
    RW=$2
    MIX=$3

    if [[ "$MIX" != "" ]]; then
        MIX_OPT="--rwmixread=$MIX"
    else
        MIX_OPT=""
    fi

    echo "测试: $NAME ..."
    OUT=$(fio --name=$NAME --filename=$TEST_FILE --size=$SIZE --bs=$BS --rw=$RW $MIX_OPT \
          --ioengine=libaio --iodepth=$IODEPTH --runtime=$RUNTIME --direct=1 --group_reporting 2>/dev/null)

    IOPS=$(echo "$OUT" | grep -E 'iops=' | head -n1 | sed -E 's/.*iops=([0-9\.kK]+).*/\1/')
    BW=$(echo "$OUT" | grep -E 'bw=' | head -n1 | sed -E 's/.*bw=([0-9\.A-Za-z\/]+).*/\1/')

    RESULTS+=("$NAME | $RW | $IOPS | $BW")
}

# 随机读
fio_test "randread_test" "randread" ""

# 随机写
fio_test "randwrite_test" "randwrite" ""

# 混合读写 (70%读 30%写)
fio_test "randrw_test" "randrw" "70"

echo
echo "================= 压测结果 ================="
printf "%-12s | %-12s | %-10s | %-10s\n" "测试场景" "模式" "IOPS(QPS)" "带宽"
echo "-------------------------------------------------------------"
for FIELD in "${RESULTS[@]}"; do
    echo "$FIELD"
done

echo "============================================="
echo "注意: 压测可能影响线上服务，请勿在生产库数据盘直接运行！"
```

运行与结果：

```bash
./fio_qps_test.sh
================= 压测结果 =================
测试场景        | 模式          | IOPS(QPS)  | 带宽
-----------------------------------------------------
randread       | randread      | 1520       | 6.2MB/s
randwrite      | randwrite     | 980        | 3.9MB/s
randrw         | randrw        | 1100       | 4.5MB/s
=============================================
```

## Linux 内核故障注入框架（fail_io）：启用前提

## Linux 内核故障注入框架（fail_io）：示例命令

Linux 内核故障注入框架（**fail_io**）的配置，用于 **人为制造块设备 I/O 错误**，通常用于测试文件系统容错、多路径切换、应用降级逻辑等场景。

| 命令 | 功能 |
| :----- | :----- |
| `echo "8:16" > /sys/kernel/debug/fail_io/blkdevs` | **指定目标设备**：`8:16` = 主设备号 8、次设备号 16，即 `/dev/sdb`（`ls -l /dev/sdb` 可验证） |
| `echo 2 > /sys/kernel/debug/fail_io/probability` | **故障概率 2%**：每 100 个 I/O 请求中，约 2 个会被注入失败 |
| `echo 0 > /sys/kernel/debug/fail_io/ignore_reads` | **不忽略读请求**：读 I/O 也参与故障注入（0=不忽略，1=忽略） |
| `echo 0 > /sys/kernel/debug/fail_io/ignore_writes` | **不忽略写请求**：写 I/O 也参与故障注入 |
| `echo 1 > /sys/kernel/debug/fail_io/should_fail` | **总开关打开**：开始生效 |

## 参考链接
