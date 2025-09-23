# Linux 内核网络收包函数调用栈

以下 Linux 内核函数调用栈从网卡硬中断到 IP 协议栈入口 的完整收包轨迹，包从网线进来后到底怎么走。

```plaintext
secondary_startup_64_no_verify+194   arch/x86/kernel/head_64.S
│ 作用：CPU 从 16 位实模式→64 位长模式后的公共汇编入口，负责跳转到 C 的 start_secondary()
└─ start_secondary+409               arch/x86/kernel/smpboot.c
   作用：初始化当前 CPU（栈、GDT、IDT、TSS），随后进入 idle 循环
   └─ cpu_startup_entry+111          kernel/cpu/idle.c
      作用：选择最合适的 cpuidle  governor，把 CPU 交给空闲调度器
      └─ do_idle+564                 kernel/cpu/idle.c
         作用：主 idle 循环；统计空闲时间、选择 C-state、直到中断到来
         └─ cpuidle_enter+44         drivers/cpuidle/cpuidle.c
            ├─ 选择最优 C-state，准备停机
            └─ 调用体系相关函数让 CPU 进入低功耗
            └─ cpuidle_enter_state+135
               ├─ 计算预测停留时间，更新使用率统计
               └─ 调用具体驱动例程进入睡眠
               ├─ acpi_idle_enter+155 drivers/acpi/processor_idle.c
               └─ 根据 ACPI _CST 表选择 C1/C2/C3
                  └─ 写 IO-port 或 MWAIT 指令关闭核心时钟
                  └─ acpi_idle_do_entry+70
                     └─ 真正执行 HLT / MWAIT；CPU 停指令流，等待中断
                        └─ native_safe_halt+14   arch/x86/include/asm/apic.h
                           └─ 汇编 hlt；触发 C-state，中断到来后从这里继续
                           └─ ret_from_intr+0           arch/x86/entry/entry_64.S
                              └─ 中断返回桩；恢复寄存器并判断是否需要软中断
                              └─ do_IRQ+127             arch/x86/kernel/irq.c
                                 ├─ 根据向量号找到 irq_desc，调用 handle_irq()
                                 └─ 退出时调用 irq_exit() 以触发软中断
                                 └─ irq_exit+247        kernel/softirq.c
                                    ├─ 若中断嵌套深度=0 且 pending softirq → __do_softirq()
                                    └─ 唤醒 ksoftirqd 线程（若线程化）
                                    └─ __softirqentry_text_start+215
                                       └─ 软中断总入口；循环执行 NET_RX / NET_TX / TIMER ...
                                       └─ net_rx_action+329   net/core/dev.c
                                          ├─ 遍历 per-cpu 的 poll_list
                                          └─ 对权重预算内调用 **napi->poll()**
                                          └─ vmxnet3_poll_rx_only+49   drivers/net/vmxnet3    ⬅ 网卡驱动
                                             ├─ 关闭中断，纯收包路径
                                             └─ 预算内循环收帧
                                             └─ vmxnet3_rq_rx_complete+2171
                                                ├─ 读描述符状态位（USED）
                                                ├─ 取对应 rx_buffer[head] 的 DMA 页
                                                ├─ 构造 skb（零拷贝 attach 页）
                                                └─ 清描述符，挂新空页 → ring buffer 槽位重用
                                                └─ **napi_gro_receive+186**   net/core/dev.c
                                                   ├─ GRO 合并分片（若可能）
                                                   └─ 交到上层协议
                                                   └─ netif_receive_skb_internal+61
                                                      └─ 加锁、更新统计
                                                      └─ **__netif_receive_skb_core+635**
                                                         ├─ 复制一份给 ptype_all（tcpdump 用）
                                                         └─ 按 eth->h_proto 选 ptype 链表
                                                         └─ br_handle_frame+340   net/bridge/br_input.c
                                                            ├─ 学习源 MAC，更新 FDB
                                                            └─ 根据目的 MAC 决定转发或上送
                                                            └─ br_handle_frame_finish+356
                                                               └─ 转发分支 / 本地接收分支
                                                               └─ br_pass_frame_up+196
                                                                  └─ 把帧重新注入协议栈（本地接收）
                                                                  └─ __netif_receive_skb_core+1393
                                                                     └─ 第二次分发；此时已出桥
                                                                     └─ **ip_rcv+635**   net/ipv4/ip_input.c
                                                                        ├─  sanity：版本、头长、校验和
                                                                        └─ 通过则继续
                                                                        └─ **ip_rcv_finish+1**   net/ipv4/ip_input.c
                                                                           ├─ 路由 lookup，决定本地/转发
                                                                           └─ 本地：dst_input() → tcp_v4_rcv()/udp_rcv()
```

```plaintext
secondary_startup_64_no_verify+194   arch/x86/kernel/head_64.S         │ 作用：64 位入口，跳 start_secondary
└─ start_secondary+409               arch/x86/kernel/smpboot.c         │ 作用：初始化 CPU，进 idle
   └─ cpu_startup_entry+111          kernel/cpu/idle.c                 │ 作用：选 cpuidle governor，交 idle 调度器
      └─ do_idle+564                 kernel/cpu/idle.c                 │ 作用：主 idle 循环，等中断
         └─ cpuidle_enter+44         drivers/cpuidle/cpuidle.c         │ 作用：选最优 C-state，准备停机
            └─ acpi_idle_enter+155   drivers/acpi/processor_idle.c     │ 作用：按 ACPI _CST 选 C1/C2/C3，关时钟
               └─ acpi_idle_do_entry+70                                │ 作用：执行 HLT/MWAIT，CPU 停指令流
                  └─ native_safe_halt+14   arch/x86/include/asm/apic.h │ 作用：汇编 hlt，等中断
                     └─ ret_from_intr+0   arch/x86/entry/entry_64.S    │ 作用：中断返回桩，判软中断
                        └─ do_IRQ+127   arch/x86/kernel/irq.c          │ 作用：硬中断总入口，调 handle_irq()
                           └─ irq_exit+247   kernel/softirq.c          │ 作用：尾阶段触发软中断
                              └─ __softirqentry_text_start+215         │ 作用：软中断总入口，轮询各 vec
                                 └─ net_rx_action+329   net/core/dev.c │ 作用：NET_RX_SOFTIRQ，NAPI 轮询
                                    └─ vmxnet3_poll_rx_only+49   drivers/net/vmxnet3
                                       │ 作用：VMXNET3 驱动 poll，纯收包
                                       └─ vmxnet3_rq_rx_complete+2171
                                          │ 作用：拆环，DMA 页→skb，ring buffer 重用
                                          └─ napi_gro_receive+186   net/core/dev.c
                                             │ 作用：GRO 入口，可合并分片
                                             └─ netif_receive_skb_internal+61
                                                │ 作用：驱动→协议栈移交，加锁统计
                                                └─ __netif_receive_skb_core+635
                                                   │ 作用：按 eth->type 分发，给 ptype_all
                                                   └─ br_handle_frame+340   net/bridge/br_input.c
                                                      │ 作用：桥处理，学 MAC，决定转发/上送
                                                      └─ br_handle_frame_finish+356
                                                         │ 作用：桥转发/本地接收决策
                                                         └─ br_pass_frame_up+196
                                                            │ 作用：把帧重新注入协议栈（本地接收）
                                                            └─ __netif_receive_skb_core+1393
                                                               │ 作用：二次分发；已出桥
                                                               └─ ip_rcv+635   net/ipv4/ip_input.c
                                                                  │ 作用：IPv4 总入口，sanity + 校验和
                                                                  └─ ip_rcv_finish+1   net/ipv4/ip_input.c
                                                                     │ 作用：路由 lookup，本地/转发决策
                                                                     └─ dst_input()   net/core/dst.c
                                                                        │ 作用：根据路由结果继续 Local Deliver
                                                                        └─ tcp_v4_rcv+offset   net/ipv4/tcp_ipv4.c
                                                                           │ 作用：TCP 接收总入口，合法性检查
                                                                           └─ tcp_v4_do_rcv()   net/ipv4/tcp_ipv4.c
                                                                              │ 作用：状态机分发
                                                                              └─ tcp_rcv_established()   net/ipv4/tcp_input.c
                                                                                 │ 作用：ESTABLISHED 态最后函数
                                                                                 ├─ tcp_queue_rcv()         ← 挂接收队列
                                                                                 └─ sk->sk_data_ready()     ← 唤醒用户进程

```
