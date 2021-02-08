#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
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
  std::vector<pollfd> pollfds(1);
  pollfds[0].fd = listen_fd;
  pollfds[0].events = POLLIN;
  while (true) {
    constexpr int timeout = 3000;
    int ret = poll(pollfds.data(), pollfds.size(), timeout);
    if (ret == -1) {
      std::cout << "errno: " << strerror(errno) << '\n';
      break;
    }
    if (ret == 0) {
      std::cout << "timeout" << '\n';
      continue;
    }
    if (pollfds.front().revents & POLLIN) {
      pollfd tmp;
      tmp.fd = jc::AcceptFd(listen_fd);
      tmp.events = POLLIN;
      pollfds.emplace_back(std::move(tmp));
    }
    for (unsigned i = 1; i < pollfds.size(); ++i) {
      if (pollfds[i].revents & POLLIN) {
        if (jc::PrintReceiveMessage(pollfds[i].fd)) {
          jc::Send(pollfds[i].fd, "welcome to join");
        } else {
          close(pollfds[i].fd);
          pollfds[i].fd = -1;
        }
      }
    }
    pollfds.erase(std::remove_if(std::begin(pollfds), std::end(pollfds),
                                 [&](pollfd &x) { return x.fd == -1; }),
                  std::end(pollfds));
  }
  close(listen_fd);
}
