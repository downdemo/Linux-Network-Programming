#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

namespace jc {

sockaddr_in SockAddress(int port, const std::string& ip) {
  sockaddr_in res;
  bzero(&res, sizeof(res));
  res.sin_family = AF_INET;
  res.sin_port = htons(port);
  inet_pton(AF_INET, ip.c_str(), &res.sin_addr);
  return res;
}

int Connect(int port, const std::string& ip) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  sockaddr_in address = SockAddress(port, ip);
  if (connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) ==
      -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  return fd;
}

bool PrintReceiveMessage(int fd) {
  char buf[1024] = {};
  int n = recv(fd, buf, sizeof(buf) - 1, 0);
  if (n <= 0) {
    return false;
  }
  std::cout << n << " bytes message: " << buf << '\n';
  return true;
}

void Send(int fd, const std::string& s) {
  send(fd, s.c_str(), strlen(s.c_str()), 0);
}

int SetNonBlocking(int fd) {
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
  return old_option;
}

void AddFd(int epfd, int fd) {
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLIN | EPOLLET;
  epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
  SetNonBlocking(fd);
}

void ModFd(int epfd, int fd, int op) {
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = op;
  epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

}  // namespace jc

int main() {
  int fd = jc::Connect(12345, "0.0.0.0");
  assert(fd != -1);
  int epfd = epoll_create1(0);
  jc::AddFd(epfd, fd);
  std::vector<epoll_event> v(1);
  int cnt = 0;
  jc::Send(fd, "hello\n");
  while (true) {
    constexpr int timeout = -1;
    int ret = epoll_wait(epfd, v.data(), v.size(), timeout);
    if (ret == -1) {
      std::cout << "errno: " << strerror(errno) << '\n';
      break;
    }
    for (int i = 0; i < ret; ++i) {
      if (v[i].events & EPOLLIN) {
        if (jc::PrintReceiveMessage(v[i].data.fd)) {
          jc::ModFd(epfd, v[i].data.fd, EPOLLOUT);
        } else {
          break;
        }
      } else if (v[i].events & EPOLLOUT) {
        jc::Send(v[i].data.fd, std::to_string(cnt));
        ++cnt;
        jc::ModFd(epfd, v[i].data.fd, EPOLLIN);
      }
    }
  }
  close(epfd);
  close(fd);
}
