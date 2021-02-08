#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace jc {

sockaddr_in SockAddress(int port, const std::string &ip) {
  sockaddr_in res;
  res.sin_family = AF_INET;
  res.sin_port = htons(port);
  inet_pton(AF_INET, ip.c_str(),
            &res.sin_addr);  // res.sin_addr.s_addr = htonl(INADDR_ANY);
  bzero(&(res.sin_zero), 8);
  return res;
}

int ListenFd(int port, const std::string &ip, int backlog = 5) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  int reuse = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  sockaddr_in address = SockAddress(port, ip);
  if (bind(fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  if (listen(fd, backlog) == -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  return fd;
}

int AcceptFd(int listen_fd) {
  sockaddr_in client;
  socklen_t client_address_length = sizeof(client);
  int accept_fd = accept(listen_fd, reinterpret_cast<sockaddr *>(&client),
                         &client_address_length);
  if (accept_fd == -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  std::cout << "receive connect request from " << inet_ntoa(client.sin_addr)
            << ':' << ntohs(client.sin_port) << '\n';
  return accept_fd;
}

void Send(int accept_fd, const std::string &s) {
  send(accept_fd, s.c_str(), strlen(s.c_str()), 0);
}

bool PrintReceiveMessage(int fd, std::string &s) {
  char buf[1024] = {};
  int n = recv(fd, buf, sizeof(buf) - 1, 0);
  if (n <= 0) {
    return false;
  }
  std::cout << n << " bytes message: " << buf << '\n';
  s = std::string{buf};
  return true;
}

int SetNonBlocking(int fd) {  // evutil_make_socket_nonblocking
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

void DelFd(int epfd, int fd) {
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLIN;
  epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
}

void ModFd(int epfd, int fd, int op) {
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = op;
  epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

}  // namespace jc

int main() {
  int listen_fd = jc::ListenFd(12345, "0.0.0.0");
  assert(listen_fd != -1);
  int epfd = epoll_create1(0);
  jc::AddFd(epfd, listen_fd);
  std::vector<epoll_event> v(1000);
  std::unordered_map<int, std::string> m;
  while (true) {
    constexpr int timeout = -1;
    int ret = epoll_wait(epfd, v.data(), v.size(), timeout);
    if (ret == -1) {
      std::cout << "errno: " << strerror(errno) << '\n';
      break;
    }
    for (int i = 0; i < ret; ++i) {
      int fd = v[i].data.fd;
      if (fd == listen_fd) {
        jc::AddFd(epfd, jc::AcceptFd(listen_fd));
      } else if (v[i].events & EPOLLIN) {
        if (jc::PrintReceiveMessage(fd, m[fd])) {
          jc::ModFd(epfd, fd, EPOLLOUT);
        } else {
          jc::DelFd(epfd, fd);
          close(fd);
        }
      } else if (v[i].events & EPOLLOUT) {
        jc::Send(fd, m[fd]);
        jc::ModFd(epfd, fd, EPOLLIN);
      }
    }
  }
  close(epfd);
  close(listen_fd);
}
