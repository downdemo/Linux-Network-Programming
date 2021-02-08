## TCP 连接的建立与断开

* 下面将用 GDB 断点运行 [socket sample](https://github.com/downdemo/Linux-Network-Programming/tree/master/src/socket)，观测真实的 TCP 连接建立与断开的过程
* 首先用 tcpdump 监听 TCP 端口 12345

```
sudo tcpdump -i any tcp port 12345
```

* 进入 Makefile 所在目录，编译

```
make
```

* 新起一个 Shell，用 GDB 打开 server

```
gdb debug/socket_server_debug
```

* 按下 `Ctrl + x + a` 进入 TUI 模式，以便观察代码运行到的位置。执行 `start` 进入 main 函数的起始位置，此时可以看到 TUI 中 `int main()` 所在行高亮
* 输入 `next` 或 `n`，GDB 将执行当前行的代码，TUI 将高亮到下一行代码。之后如果执行相同的指令，直接回车即可。此时高亮行停留在如下代码，回车执行此行代码，以调用 `bind` 和 `listen`

```cpp
int listen_fd = jc::ListenFd(12345, "0.0.0.0");
```

* 此时 12345 端口将进入 `LISTEN` 状态

```
netstat -antp | grep 12345
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
tcp        0      0 0.0.0.0:12345           0.0.0.0:*               LISTEN      65639/socket_server 
```


* 新起一个 Shell，用 GDB 打开 client

```
gdb debug/socket_client_debug
```

* `Ctrl + x + a` 并 `start`，`n` 如下代码进行 `connect` 调用

```cpp
int fd = jc::Connect(12345, "0.0.0.0");
```

* tcpdump 将打印如下信息

```
11:58:17.059466 IP localhost.56234 > localhost.12345: Flags [S], seq 2271595046, win 65495, options [mss 65495,sackOK,TS val 988640603 ecr 0,nop,wscale 7], length 0
11:58:17.068493 IP localhost.12345 > localhost.56234: Flags [S.], seq 2153659986, ack 2271595047, win 65483, options [mss 65495,sackOK,TS val 988640612 ecr 988640603,nop,wscale 7], length 0
11:58:17.068515 IP localhost.56234 > localhost.12345: Flags [.], ack 1, win 512, options [nop,nop,TS val 988640613 ecr 988640612], length 0
```

* 其中第一列是时间，精确到微秒，`>` 左侧为发送方，右侧为接收方，`win` 表示窗口大小（16 位，最大 65535）。为了便于分析，提取出关键部分

```
localhost.56234 > localhost.12345: Flags [S], seq 2271595046
localhost.12345 > localhost.56234: Flags [S.], seq 2153659986, ack 2271595047
localhost.56234 > localhost.12345: Flags [.], ack 1
```

* 一对套接字唯一标识了网络上的一个 TCP 连接，它是一个四元组，包括本地 IP 地址、本地 TCP 端口号、目的 IP 地址、目的 TCP 端口号
* 端口号为 16 位，取值为 0-65535，可划分为三段
  * 0-1023：well-known port，也是 Unix 保留端口，只能赋予特权用户进程的套接字，由 IANA（The Internet Assigned Numbers Authority，互联网数字分配机构） 分配和控制，相同端口号将分配给同一服务，如 SSH 的 22 端口、HTTP 的 80 端口
  * 1024-49151：registered port，不受 IANA 控制，由 IANA 登记并提供它们的使用情况清单
  * 49152-65535：dynamic 或 private，临时端口，49152 = 65536 \* 0.75
* 抓到的 TCP 包体现了建立 TCP 连接的三次握手，`S` 表示 `SYN`，`.` 表示 `ACK`，过程如下
  * 第一次握手：server 调用 `bind` 和 `listen` 后，处于 `LISTEN` 状态。client 调用 `connect` 向 server 发送一个 `SYN`，进入 `SYN_SENT` 状态，`SYN` 序列号为 `2271595046`，这个序列号是一个随机生成的 ISN（Initial Sequence Number），随时间变化
  * 第二次握手：server 收到 `SYN` 后，向 client 发送一个 `SYN + ACK`，状态由 `LISTEN` 转为 `SYN_RCVD`，这个 `SYN` 的序列号也是随机的，而 `ACK` 序列号为 client 传过来的 SYN 的序列号加 1
  * 第三次握手：client 收到 server 的 `SYN + ACK` 后，向 server 发送一个 `ACK`，状态由 `SYN_SENT` 转为 `ESTABLISHED`，同理，`ACK` 的序列号为 server 传过来的 `SYN` 的序列号加 1，这里显示 1 是因为 tcpdump 默认使用相对序号，如果没有 `SYN` 则显示相对于它的 `SYN` 的增量
  * 最后 server 收到 `ACK`，状态由 `SYN_RCVD` 转为 `ESTABLISHED`，双方进入 `ESTABLISHED` 状态，TCP 连接建立完毕
  * 除了三次握手的第一次主动发起连接发送的是 `SYN`，`ACK` 置 0，其他的所有 TCP 包的 `ACK` 都置为 1，因为 32 位确认号本身就是 TCP 头部的一部分，`ACK` 设为 1 只是顺带利用了这部分，并没有额外的代价
  * 每个 `SYN` 可以含有多个 TCP 选项，常用 TCP 选项如下（后两者称为 [RFC 1323](https://www.ietf.org/rfc/rfc1323.txt) 选项，也叫长胖管道选项，因为高带宽或长延迟的网络称为长胖管道）
    * MSS（maximum segment size）：16 位，最大值为 65535，发送 `SYN` 的一端通知对端它的最大分节大小，即在本连接的 TCP 分节中愿意接受的最大数据量，发送端使用接收端的 MSS 值作为所发送分节的最大大小。网络通常有一个硬件规定的 MTU（Maximum Transmission Unit，最大传输单元），以太网的 MTU 为 1500 字节，IPv4 要求的最小链路 MTU 为 68 字节，IPv6 要求为 1280 字节，IPv6 可以运行在 MTU 小于此值的链路上，但需要特定于链路的分片和重组功能，以使这些链路看起来具有至少 1280 字节的 MTU。两个主机之间的路径中的最小 MTU 称为路径 MTU，相反方向的路径 MTU 可以不同，最常见的路径 MTU 是以太网 MTU，为 1500 字节。如果传递的 IP 数据包比 MTU 大，则执行 IP 分片，分片在到达目的地前不会重组。MSS 的目的是避免分片，通常设置为 MTU 减去 IP 和 TCP 首部的固定长度，以太网中使用 IPv4 的 MSS 为 1500 - 20（TCP 首部）- 20（IPv4 首部） = 1460，IPv6 首部是 40 字节，对应 MSS 为 1440
    * 窗口规模选项：16 位，TCP 连接任何一端能告知对端的最大窗口大小是 65535。如今为了获得更大的吞吐量，要求更大的窗口，此选项指定左移位数（0-14），所提供的最大窗口接近 1GB（65536 \* 2 ^ 14）。使用此选项的前提是两端系统都要支持此选项，该选项受 `SO_RVCBUF` 套接字选项影响
    * 时间戳选项：防止由时而复现的分组可能造成的数据损坏，作为网络编程者无需考虑此选项

* 可以看到 server 和 client 已建立连接，双方进入 `ESTABLISHED` 状态

```
netstat -antp | grep 12345
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
tcp        0      0 0.0.0.0:12345           0.0.0.0:*               LISTEN      65639/socket_server 
tcp        0      0 127.0.0.1:56234         127.0.0.1:12345         ESTABLISHED 65633/socket_client 
tcp        0      0 127.0.0.1:12345         127.0.0.1:56234         ESTABLISHED 65639/socket_server
```

* 在 server 中继续 `n` 如下代码，`accept` 用于从 `ESTABLISHED` 队列取出第一个已完成连接，并返回新连接的文件描述符。如果队列为空则进程阻塞

```cpp
int accept_fd = jc::AcceptFd(listen_fd);
```

* 由于 client 和 server 已建立连接，`accept` 将直接返回而不会阻塞。继续 `n` 如下代码，调用 `send`

```cpp
jc::Send(accept_fd, "welcome to join");
```

* tcpdump 打印如下信息

```
11:59:19.715745 IP localhost.12345 > localhost.56234: Flags [P.], seq 1:16, ack 1, win 512, options [nop,nop,TS val 988703260 ecr 988640613], length 15
11:59:19.715757 IP localhost.56234 > localhost.12345: Flags [.], ack 16, win 512, options [nop,nop,TS val 988703260 ecr 988703260], length 0
```

* 其中 `P` 表示 `PSH`，置 1 表示有数据发送，16 是数据长度加 1（`welcome to join` 长度为 15），ack 是偏移，如果下次再发送就是 16（上一次的 ack + length）。client 回复的 ack 就是序列号冒号后的值。注意这些 TCP 包仅仅是 server 调用 `send` 产生的，此时还没有任何接收方来接收消息，可见 `send` 不是真正地把数据发给了对方
* 每一个 TCP 套接字有一个发送缓冲区，可用 `SO_SNDBUF` 套接字选项更改大小，当某个应用进程调用 `send` 时，内核从该应用进程的缓冲区中复制所有数据到所写套接字的发送缓冲区，如果发送缓冲区容不下数据，应用进程将阻塞，直到应用进程缓冲区中的所有数据都复制到发送缓冲区。`send` 的返回值就是放到缓冲区的数据长度，如果与参数中指定的长度不一致，就是没有完全放进去，这会导致数据丢失。`send` 返回只表示可以重新使用原来的应用进程缓冲区，并不表明对端已接收到数据
* `Ubuntu 18.04.5 LTS` 的内核参数中设置的缓冲区大小如下

```
sysctl -a | grep "net.ipv4.tcp_.*mem"
net.ipv4.tcp_mem = 22062	29417	44124
net.ipv4.tcp_rmem = 4096	131072	6291456 # 接受消息缓冲区大小为 6.29 MB
net.ipv4.tcp_wmem = 4096	16384	4194304 # 发送消息缓冲区大小为 4.19 MB
```

* 在 client 中 `n` 如下代码，调用 `recv` 从缓冲区取数据。实际过程中通常有多次收发，由于 TCP 是面向流的协议，因此并不保证每次 `send` 与 `recv` 的数据一一对应。发送方为了减少发送包的数量，可以使用 Nagel 算法，等待小包累积一段时间后才发送，而接收方也不一定能及时从缓冲区取出所有数据。实际网络编程中，需要双方规定应用层协议才能解析出所需要的数据

```cpp
jc::PrintReceiveMessage(fd);
```

* 在 client 中 `n` 如下代码主动申请关闭连接

```cpp
close(fd);
```

* tcpdump 打印如下信息

```
12:00:42.379997 IP localhost.56234 > localhost.12345: Flags [F.], seq 1, ack 16, win 512, options [nop,nop,TS val 988785924 ecr 988703260], length 0
12:00:42.383661 IP localhost.12345 > localhost.56234: Flags [.], ack 2, win 512, options [nop,nop,TS val 988785928 ecr 988785924], length 0
```

* 这是断开 TCP 连接的四次挥手的前两次挥手，其中 `F` 表示 `FIN`
  * 第一次挥手：client 调用 `close` 向 server 发送 `FIN + ACK`，进入 `FIN_WAIT_1` 状态
  * 第二次挥手：server 收到 `FIN + ACK` 后，发送一个 `ACK`，进入 `CLOSE_WAIT` 状态，这个状态是为了让 server 继续发送没传完的数据，正因为有可能存在没传完的数据，所以断开连接的过程要比建立连接多一次。client 收到 `ACK` 后，状态由 `FIN_WAIT_1` 转为 `FIN_WAIT_2`

* 可以看到 client 进入 `FIN_WAIT2` 状态，server 进入 `CLOSE_WAIT` 状态

```
netstat -antp | grep 12345
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
tcp        0      0 0.0.0.0:12345           0.0.0.0:*               LISTEN      65639/socket_server 
tcp        0      0 127.0.0.1:56234         127.0.0.1:12345         FIN_WAIT2   -                   
tcp        1      0 127.0.0.1:12345         127.0.0.1:56234         CLOSE_WAIT  65639/socket_server
```

* 在 server 中 `n` 如下代码，关闭连接

```cpp
close(accept_fd);
```

* tcpdump 打印如下信息

```
12:01:24.794975 IP localhost.12345 > localhost.56234: Flags [F.], seq 16, ack 2, win 512, options [nop,nop,TS val 988828339 ecr 988785924], length 0
12:01:24.794993 IP localhost.56234 > localhost.12345: Flags [.], ack 17, win 512, options [nop,nop,TS val 988828339 ecr 988828339], length 0
```

* 这是断开 TCP 连接的四次挥手的后两次挥手
  * 第三次挥手：server 调用 `close` 向 client 发送 `FIN + ACK`，状态由 `CLOSE_WAIT` 转为 `LAST_ACK`
  * 第四次挥手：client 收到 `FIN + ACK` 后，发送一个 `ACK`，状态由 `FIN_WAIT_2` 转为 `TIME_WAIT`。server 收到 `ACK` 后，状态由 `LAST_ACK` 转为 `CLOSED`，`2MSL`（Maximum Segment Lifetime，最大段生存期）后，client 状态由 `TIME_WAIT` 转为 `CLOSED`
  * `TIME_WAIT` 存在的理由
    * 可靠地实现 TCP 全双工连接的终止，如果 server 没有收到 client 最后发送的 `ACK`，就会重发 `FIN + ACK`，client 再重发 `ACK`，并重启 `2MSL` 计时器
    * 允许老的重复分节在网络中消逝，比如关闭了一个连接，不久又在相同的 IP 和端口间建立另一个连接，`TIME_WAIT` 可以防止此情况发生。对于处于 `TIME_WAIT` 状态的端口，调用 `bind` 将失败，因此一般会在创建套接字时设置 `SO_REUSEADDR` 选项，这样即使端口处于 `TIME_WAIT` 状态也能被 `bind`
* 任何 `TCP` 实现都要为 MSL 选择一个值，[RFC 793](https://www.ietf.org/rfc/rfc1122.txt) 规定 MSL 为 2分钟（`For this specification the MSL is taken to be 2 minutes`）。`Ubuntu 18.04.5 LTS` 的 `TIME_WAIT` 持续时间为 60 秒，`uname -r` 为 `4.15.0-135-generic`，`/usr/src/linux-headers-4.15.0-135-generic/include/net/tcp.h` 中可以看到如下代码

```cpp
#define TCP_TIMEWAIT_LEN (60*HZ) /* how long to wait to destroy TIME-WAIT
                                  * state, about 60 seconds     */
#define TCP_FIN_TIMEOUT TCP_TIMEWAIT_LEN
                                 /* BSD style FIN_WAIT2 deadlock breaker.
                                  * It used to be 3min, new value is 60sec,
                                  * to combine FIN-WAIT-2 timeout with
                                  * TIME-WAIT timer.
                                  */
```

* 也可以直接看内核参数

```
sysctl -a | grep net.ipv4.tcp_fin_timeout
net.ipv4.tcp_fin_timeout = 60
```

* 此时双方连接已关闭。在 server 中 `n` 如下代码，关闭监听端口的文件描述符

```cpp
close(listen_fd);
```

* 以上是正常的连接与断开流程，可以参考此过程进行不同顺序的操作或强制结束进程，可以了解各种情况下的 TCP 状态，比如不运行 server 时让 client 调用 `connect`，抓到的包如下

```
12:50:57.975036 IP localhost.56262 > localhost.12345: Flags [S], seq 1183472988, win 65495, options [mss 65495,sackOK,TS val 991801517 ecr 0,nop,wscale 7], length 0
12:50:57.975051 IP localhost.12345 > localhost.56262: Flags [R.], seq 0, ack 1183472989, win 0, length 0
```

* 其中 `R` 表示 `RST`，client 调用 `connect` 发起连接，由于端口未开启，返回一个 `RST`，client 收到 `RST` 后无需返回 `ACK`，直接释放连接，状态转为 `CLOSED`，`connect` 返回 -1 表示调用失败。如果 client 和 server 正常连接，一段时间后 server 挂掉，此时 client 向 server 发送请求，也会收到一个 `RST` 来重置连接
* tcpdump 抓的包可以用如下命令写到文件，用 [Wireshark](https://www.wireshark.org/) 打开文件可以更直观地看包的内容

```
sudo tcpdump -i any tcp port 12345 -w socket_debug.cap
```

## UDP

* UDP 无需建立连接即可直接发送消息。先监听 UDP 端口

```
sudo tcpdump -iany udp port 12345
```

* 运行 UDP 例子

```
build/udp_server&build/udp_client
```

* 抓到的包如下

```
09:19:53.027405 IP localhost.39094 > localhost.12345: UDP, length 12
09:19:53.029951 IP localhost.12345 > localhost.39094: UDP, length 15
```
