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

bool PrintReceiveMessage(int fd) {
  char buf[1024] = {};
  int n = recvfrom(fd, buf, sizeof(buf) - 1, 0, nullptr, nullptr);
  if (n <= 0) {
    return false;
  }
  std::cout << n << " bytes message: " << buf << std::endl;
  return true;
}

void Send(int fd, const std::string& s, sockaddr_in* server_address) {
  sendto(fd, s.c_str(), strlen(s.c_str()), 0,
         reinterpret_cast<sockaddr*>(server_address), sizeof(*server_address));
}

}  // namespace jc

int main() {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd != -1);
  sockaddr_in server_address = jc::SockAddress(12345, "0.0.0.0");
  jc::Send(fd, "hello world!", &server_address);
  jc::PrintReceiveMessage(fd);
  close(fd);
}
