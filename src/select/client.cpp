#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>

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

}  // namespace jc

int main() {
  int fd = jc::Connect(12345, "0.0.0.0");
  assert(fd != -1);
  jc::Send(fd, "hello");
  fd_set read_fds;
  while (true) {
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int ret = select(fd + 1, &read_fds, nullptr, nullptr, &timeout);
    if (ret == -1) {
      std::cout << "errno: " << strerror(errno) << '\n';
      break;
    }
    if (ret == 0) {
      std::cout << "timeout" << '\n';
      continue;
    }
    if (FD_ISSET(fd, &read_fds)) {
      if (jc::PrintReceiveMessage(fd)) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        jc::Send(fd, "got your message");
      } else {
        FD_CLR(fd, &read_fds);
        break;
      }
    }
  }
  close(fd);
}
