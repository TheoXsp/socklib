#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <span>
#include <vector>

#include "tcpSocket/tcpSocket.hpp"
#include "udpSocket/udpSocket.hpp"

int main(int ac, char **av) {
  std::unique_ptr<UdpSocket> server = std::make_unique<UdpSocket>();
  std::vector<std::byte> buffer(1024);

  std::error_code err = server->bind({"0.0.0.0", 8080});
  if (err) {
    std::cerr << "Error binding socket: " << err.message() << std::endl;
    return 1;
  }
  auto result = server->receive(std::span{buffer});
  if (!result) {
    std::cerr << "Error receiving data: " << static_cast<int>(result.error()) << std::endl;
    return 1;
  }
  std::cout << "Received " << result.value() << " bytes from client" << std::endl;
  std::string_view msg(reinterpret_cast<const char *>(buffer.data()), result.value());
  std::cout << "Received message: " << msg << std::endl;

  server->close();
  std::cout << "Closed connection" << std::endl;
  return 0;
}
