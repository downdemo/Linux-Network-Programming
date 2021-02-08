#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

namespace jc {

sockaddr_in SockAddress(int port, const std::string& ip) {
  sockaddr_in res;
  res.sin_family = AF_INET;
  res.sin_port = htons(port);
  inet_pton(AF_INET, ip.c_str(), &res.sin_addr);
  bzero(&(res.sin_zero), 8);
  return res;
}

int UDPFd(int port, const std::string& ip) {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd == -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  sockaddr_in address = SockAddress(port, ip);
  if (bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
    std::cout << "errno: " << strerror(errno) << '\n';
    return -1;
  }
  return fd;
}

bool PrintReceiveMessage(int fd, sockaddr_in* client_address) {
  char buf[1024] = {};
  socklen_t len = sizeof(client_address);
  int n = recvfrom(fd, buf, sizeof(buf) - 1, 0,
                   reinterpret_cast<sockaddr*>(client_address), &len);
  if (n <= 0) {
    return false;
  }
  std::cout << n << " bytes message: " << buf << '\n';
  return true;
}

void Send(int fd, const std::string& s, sockaddr_in* client_address) {
  sendto(fd, s.c_str(), strlen(s.c_str()), 0,
         reinterpret_cast<sockaddr*>(client_address), sizeof(*client_address));
}

}  // namespace jc

int main() {
  int fd = jc::UDPFd(12345, "0.0.0.0");
  assert(fd != -1);
  sockaddr_in client_address;
  jc::PrintReceiveMessage(fd, &client_address);
  jc::Send(fd, "welcome to join", &client_address);
  close(fd);
}
