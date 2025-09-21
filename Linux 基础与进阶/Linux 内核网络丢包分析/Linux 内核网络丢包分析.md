
# Linux 内核网络丢包分析示例

## SETUP IPTABLES TO DROP ICMP PACKETS

```bash
### kiosk@foundation0 ###
$ sudo iptables -t filter -A INPUT -s 172.25.250.10 --protocol icmp -j DROP
$ sudo iptables -t filter -L INPUT --line-numbers
Chain INPUT (policy ACCEPT)
num  target     prot opt source               destination
1    LIBVIRT_INP  all  --  anywhere             anywhere
2    DROP       icmp --  servera.lab.example.com  anywhere
```  
  
## IPV4 PING FROM SERVERA

```bash
### student@servera ###
$ ping 172.25.250.250  #NO reponse
```

## DEBUG PACKETS DROP FROM FUNCTION STACK

```bash
### kiosk@foundation0 ###

$ sudo dnf install -y dropwatch
$ sudo dropwatch -l kas
  Initalizing kallsyms db
  dropwatch> start  #start to catch packets
  Enabling monitoring...
  Kernel monitoring activated.
  Issue Ctrl-C to stop monitoring
  1 drops at nf_hook_slow+a7 (0xffffffffb981ab57)  #drop occurs periodically, so trace this function
  1 drops at unix_stream_connect+4ff (0xffffffffb98aeeef)
  1 drops at __udp4_lib_rcv+ab0 (0xffffffffb9860dc0)
  2 drops at nf_hook_slow+a7 (0xffffffffb981ab57)
  2 drops at unix_stream_connect+4ff (0xffffffffb98aeeef)
  2 drops at nf_hook_slow+a7 (0xffffffffb981ab57)
  2 drops at unix_stream_connect+4ff (0xffffffffb98aeeef)
  2 drops at nf_hook_slow+a7 (0xffffffffb981ab57)
  1 drops at unix_stream_connect+4ff (0xffffffffb98aeeef)
  2 drops at unix_stream_connect+4ff (0xffffffffb98aeeef)
  ^CGot a stop message
  dropwatch> exit
  
$ sudo bpftrace -l | grep <probes>
$ sudo bpftrace -e '
BEGIN { @cnt = 0; }
tracepoint:napi:napi_poll 
{
    printf("%s\n", kstack);
	  @cnt;
	  if (@cnt >= 5) { exit(); }
}'
#Just display 5 times for tracing results
  
$ sudo bpftrace -e 'BEGIN { @cnt = 0; } kprobe:nf_hook_slow { printf("%s\n", kstack); @cnt++; if (@cnt >= 5) { exit(); }}'
Attaching 2 probes...

      nf_hook_slow+1  <-- trace kernerl function
      nf_hook+134
      ip6_xmit+717
      inet6_csk_xmit+151
      __tcp_transmit_skb+1338
      tcp_connect+2876
      tcp_v6_connect+1470
      __inet_stream_connect+209
      inet_stream_connect+54
      __sys_connect+154
      __x64_sys_connect+22
      do_syscall_64+91
      entry_SYSCALL_64_after_hwframe+101
		
	    ...
		
$ VMLINUX=/usr/lib/debug/lib/modules/$(uname -r)/vmlinux
$ FUNC=nf_hook_slow
$ OFFSET=0x1
$ ADDR=$(nm $VMLINUX | awk "\$3==\"$FUNC\" {print \$1}")
$ ABS_ADDR=$(printf "%x\n" $((0x$ADDR + OFFSET)))
$ echo "0x$ABS_ADDR"
$ addr2line -e $VMLINUX -f $ABS_ADDR
```