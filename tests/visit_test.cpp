#include "extensions/visit_protocol.hpp"
#include <cassert>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
int main() {
  visit::Packet p{};
  p.code = 123456;
  p.request = 123;
  p.individual = 987;
  p.kind = 2;
  p.decor = 11;
  auto bytes = visit::encode(p);
  visit::Packet decoded{};
  assert(visit::decode(bytes.data(), bytes.size(), 123456, decoded));
  assert(!visit::decode(bytes.data(), bytes.size(), 654321, decoded));
  for (size_t n = 0; n < bytes.size(); n++)
    assert(!visit::decode(bytes.data(), n, 123456, decoded));
  for (size_t n = 0; n < bytes.size(); n++) {
    auto corrupt = bytes;
    corrupt[n] ^= 1;
    assert(!visit::decode(corrupt.data(), corrupt.size(), 123456, decoded));
  }
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd >= 0);
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = 0;
  assert(bind(fd, (sockaddr *)&address, sizeof address) == 0);
  socklen_t size = sizeof address;
  getsockname(fd, (sockaddr *)&address, &size);
  timeval timeout{2, 0};
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
  pid_t child = fork();
  assert(child >= 0);
  if (child == 0) {
    int peer = socket(AF_INET, SOCK_DGRAM, 0);
    for (int i = 0; i < 2; i++)
      sendto(peer, bytes.data(), bytes.size(), 0, (sockaddr *)&address, size);
    close(peer);
    _exit(0);
  }
  uint64_t last = 0;
  unsigned applied = 0;
  for (int i = 0; i < 2; i++) {
    uint8_t packet[256];
    auto n = recv(fd, packet, sizeof packet, 0);
    assert(n == 36 && visit::decode(packet, n, 123456, decoded));
    if (decoded.request != last) {
      last = decoded.request;
      applied++;
    }
  }
  assert(applied == 1);
  int status = 0;
  waitpid(child, &status, 0);
  assert(status == 0);
  close(fd);
  std::cout << "PASS: visita LAN em dois processos UDP, validação de bytes e "
               "duplicação\n";
}
