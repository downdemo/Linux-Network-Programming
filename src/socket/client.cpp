#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>

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

}  // namespace jc

int main() {
  int fd = jc::Connect(12345, "0.0.0.0");
  assert(fd != -1);
  jc::PrintReceiveMessage(fd);
  close(fd);
}
