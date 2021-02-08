#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <list>

namespace jc {

sockaddr_in SockAddress(int port, const std::string& ip) {
  sockaddr_in res;
  res.sin_family = AF_INET;
  res.sin_port = htons(port);
  inet_pton(AF_INET, ip.c_str(),
            &res.sin_addr);  // res.sin_addr.s_addr = htonl(INADDR_ANY);
  bzero(&(res.sin_zero), 8);
  return res;
}

int ListenFd(int port, const std::string& ip, int backlog = 5) {
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
  if (bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
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
  int accept_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client),
                         &client_address_length);
  if (accept_fd == -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  std::cout << "receive connect request from " << inet_ntoa(client.sin_addr)
            << ':' << ntohs(client.sin_port) << '\n';
  return accept_fd;
}

void Send(int accept_fd, const std::string& s) {
  send(accept_fd, s.c_str(), strlen(s.c_str()), 0);
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

}  // namespace jc

int main() {
  int listen_fd = jc::ListenFd(12345, "0.0.0.0");
  assert(listen_fd != -1);
  fd_set read_fds;
  std::list<int> accept_fds;
  while (true) {
    FD_ZERO(&read_fds);
    FD_SET(listen_fd, &read_fds);
    int max_fd = listen_fd;
    for (auto& x : accept_fds) {
      FD_SET(x, &read_fds);
      max_fd = std::max(max_fd, x);
    }
    timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    int ret = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);
    if (ret == -1) {
      std::cout << "errno: " << strerror(errno) << '\n';
      break;
    }
    if (ret == 0) {
      std::cout << "timeout" << '\n';
      continue;
    }
    if (FD_ISSET(listen_fd, &read_fds)) {
      int fd = jc::AcceptFd(listen_fd);
      accept_fds.emplace_back(fd);
    }
    for (auto& x : accept_fds) {
      if (FD_ISSET(x, &read_fds)) {
        if (jc::PrintReceiveMessage(x)) {
          jc::Send(x, "welcome to join");
        } else {
          FD_CLR(x, &read_fds);
          close(x);
          x = -1;
        }
      }
    }
    accept_fds.remove(-1);
  }
  close(listen_fd);
}
