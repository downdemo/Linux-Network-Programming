![Supported Platforms](https://img.shields.io/badge/platform-Linux-red.svg)
[![Build Status](https://travis-ci.org/downdemo/Linux-Network-Programming.svg?branch=master)](https://travis-ci.org/downdemo/Linux-Network-Programming)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/downdemo/Linux-Network-Programming/blob/master/LICENSE)

## Documentation

* [通过抓包理解 TCP 与 UDP](通过抓包理解TCP与UDP.html)
* [基本 TCP 套接字 API](基本TCP套接字API.html)
* [网络编程 I/O 模型](IO模型.html)
* [Linux I/O 多路复用 API：select、poll、epoll](IO多路复用API.html)

## Build and Run

* build

```
git clone git@github.com:downdemo/Linux-Network-Programming.git
cd Linux-Network-Programming
make
```

* Service would be generated in build directory

```
cd build
```

* Run server before client
* One running server at most because samples use the same port 12345
* `Ctrl + c` to interrupt blocking server and client process

### Run [UDP sample](https://github.com/downdemo/Linux-Network-Programming/tree/master/src/udp)

* Run server

```
./udp_server
```

* Run client in another shell

```
./udp_client
```

### Run [socket sample](https://github.com/downdemo/Linux-Network-Programming/tree/master/src/socket)

* Run server

```
./socket_server
```

* Run client

```
./socket_client
```

### Run [fork sample](https://github.com/downdemo/Linux-Network-Programming/tree/master/src/fork)

* Run server

```
./fork_server
```

* Run client multiple times

```
./fork_client
./fork_client
./fork_client
./fork_client
./fork_client
```

### Run [select sample](https://github.com/downdemo/Linux-Network-Programming/tree/master/src/select)、[poll sample](https://github.com/downdemo/Linux-Network-Programming/tree/master/src/poll)、[epoll sample](https://github.com/downdemo/Linux-Network-Programming/tree/master/src/epoll)

* Run any I/O multiplexing server, such as epoll_server

```
./epoll_server
```

* Run multiple I/O multiplexing clients

```
./select_client
./select_client
./poll_client
./poll_client
./epoll_client
./epoll_client
```

## [RFC](https://www.ietf.org/rfc/)

* Ethernet：[RFC 894, A Standard for the Transmission of IP Datagrams over Ethernet Networks](https://www.ietf.org/rfc/rfc0894.txt)
* ARP：[RFC 826, An Ethernet Address Resolution Protocol](https://www.ietf.org/rfc/rfc0826.txt)
* PPP：[RFC 1661, The Point-to-Point Protocol (PPP)](https://www.ietf.org/rfc/rfc1661.txt)
* PPPoE：[RFC 2516, A Method for Transmitting PPP Over Ethernet (PPPoE)](https://www.ietf.org/rfc/rfc2516.txt)
* ICMPv4：[RFC 792, INTERNET CONTROL MESSAGE PROTOCOL](https://www.ietf.org/rfc/rfc0792.txt)
* ICMPv6：[RFC 2463, Internet Control Message Protocol (ICMPv6)](https://www.ietf.org/rfc/rfc2463.txt)
* IPv4：[RFC 791, INTERNET PROTOCOL](https://www.ietf.org/rfc/rfc0791.txt)
* IPv6：[RFC 8200, Internet Protocol, Version 6 (IPv6) Specification](https://www.ietf.org/rfc/rfc8200.txt)
* IGMPv3：[RFC 3376, Internet Group Management Protocol, Version 3](https://www.ietf.org/rfc/rfc3376.txt)
* IPSec：[RFC 2401, Security Architecture for the Internet Protocol](https://www.ietf.org/rfc/rfc2401.txt)
* PPTP：[RFC 2637, Point-to-Point Tunneling Protocol (PPTP)](https://www.ietf.org/rfc/rfc2637.txt)
* L2TP：[RFC 2661, Layer Two Tunneling Protocol "L2TP"](https://www.ietf.org/rfc/rfc2661.txt)
* TCP：[RFC 793, TRANSMISSION CONTROL PROTOCOL](https://www.ietf.org/rfc/rfc0793.txt)
* UDP：[RFC 768, User Datagram Protocol](https://www.ietf.org/rfc/rfc0768.txt)
* SOCKS5: [RFC 1928, SOCKS Protocol Version 5](https://www.ietf.org/rfc/rfc1928.txt)
* PNG：[RFC 2083,  PNG (Portable Network Graphics) Specification Version 1.0](https://www.ietf.org/rfc/rfc2083.txt)
* JSON：[RFC 7159, The JavaScript Object Notation (JSON) Data Interchange Format](https://www.ietf.org/rfc/rfc7159.txt)
* SIP：[RFC 3261, SIP: Session Initiation Protocol](https://www.ietf.org/rfc/rfc3261.txt)
* RTP：[RFC 3550, RTP: A Transport Protocol for Real-Time Applications](https://www.ietf.org/rfc/rfc3550.txt)
* TLS 1.3：[RFC 8446, The Transport Layer Security (TLS) Protocol Version 1.3](https://www.ietf.org/rfc/rfc8446.txt)
* HTTP/1.1：[RFC 7230, Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing](https://www.ietf.org/rfc/rfc7230.txt)
* HTTP/2：[RFC 7540, Hypertext Transfer Protocol Version 2 (HTTP/2)](https://www.ietf.org/rfc/rfc7540.txt)
* HTTP/3：[RFC 9114, HTTP/3](https://www.ietf.org/rfc/rfc9114.txt)
* QUIC：[RFC 9000, QUIC: A UDP-Based Multiplexed and Secure Transport](https://www.ietf.org/rfc/rfc9000.txt)
* HTTPS：[RFC 2818, HTTP Over TLS](https://www.ietf.org/rfc/rfc2818.txt)
* WebSocket：[RFC 6455, The WebSocket Protocol](https://www.ietf.org/rfc/rfc6455.txt)
* TELNET：[RFC 854, TELNET PROTOCOL SPECIFICATION](https://www.ietf.org/rfc/rfc0854.txt)
* SSH：[RFC 4254, The Secure Shell (SSH) Connection Protocol](https://www.ietf.org/rfc/rfc4254.txt)
* DNS：[RFC 1034, DOMAIN NAMES](https://www.ietf.org/rfc/rfc1034.txt)
* DHCP：[RFC 2131, Dynamic Host Configuration Protocol](https://www.ietf.org/rfc/rfc2131.txt)
* NTP：[RFC 1305, Network Time Protocol (Version 3)](https://www.ietf.org/rfc/rfc1305.txt)
* FTP：[RFC 959, FILE TRANSFER PROTOCOL (FTP)](https://www.ietf.org/rfc/rfc0959.txt)
* NFSv4.2：[RFC 7862, Network File System (NFS) Version 4 Minor Version 2 Protocol](https://www.ietf.org/rfc/rfc7862.txt)
* NNTP：[RFC 977, Network News Transfer Protocol](https://www.ietf.org/rfc/rfc0977.txt)
* POP3：[RFC 1939, Post Office Protocol - Version 3](https://www.ietf.org/rfc/rfc1939.txt)
* IMAP4：[RFC 1730, INTERNET MESSAGE ACCESS PROTOCOL - VERSION 4](https://www.ietf.org/rfc/rfc1730.txt)
* SMTP：[RFC 2821, Simple Mail Transfer Protocol](https://www.ietf.org/rfc/rfc2821.txt)
