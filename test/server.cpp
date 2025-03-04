#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <span>
#include <vector>

#include "tcpSocket/tcpSocket.hpp"
#include "udpSocket/udpSocket.hpp"

int main(int ac, char **av) {
  std::unique_ptr<TcpSocket> server = std::make_unique<TcpSocket>();
  std::vector<uint8_t> buffer(1024);

  server->bind({"0.0.0.0", 8080});
  server->listen();
  if (auto client = server->accept(); client) {
    while (true) {
      auto res = client->receive(buffer.data(), buffer.size());
      if (!res) {
        if (res.error() == Socket::SocketStatus::Disconnected) {
          std::cout << "Connection closed" << std::endl;
          break;
        }
      }
      if (res.value() > 0)
        std::cout << "Received: " << buffer.data() << std::endl;
    }
  } else {
    std::cerr << "Failed to accept client" << std::endl;
    return 1;
  }
  return 0;
}
