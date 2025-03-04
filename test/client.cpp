#include <iostream>
#include <memory>
#include <span>

#include "tcpSocket/tcpSocket.hpp"
#include "udpSocket/udpSocket.hpp"

int main(int ac, char **av) {
  std::unique_ptr<TcpSocket> client = std::make_unique<TcpSocket>();
  std::vector<uint8_t> buffer(1024);

  if (client->connect({"127.0.0.1", 8080}) != std::error_code{}) {
    std::cerr << "Failed to connect to server" << std::endl;
    return 1;
  }
  client->send("Hello world !", 13);
  client->close();
  return 0;
}
