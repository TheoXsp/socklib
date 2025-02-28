#include <iostream>
#include <memory>

#include <span>
#include "tcpSocket/tcpSocket.hpp"
#include "udpSocket/udpSocket.hpp"

int main(int ac, char **av) {
  std::unique_ptr<UdpSocket> client = std::make_unique<UdpSocket>();
  std::vector<std::byte> buffer(1024);

  std::error_code err = client->connect({"127.0.0.1", 8080});
  if (err) {
    std::cerr << "Error connecting to server: " << err.message() << std::endl;
    return 1;
  }
  std::string msg = "hello world";
  std::copy(msg.begin(), msg.end(), reinterpret_cast<char*>(buffer.data()));
  auto result = client->send(std::span{buffer}, msg.size());
  if (!result) {
    std::cerr << "Error sending data: " << result.error().message() << std::endl;
    return 1;
  }
  std::cout << "Sent " << result.value() << " bytes to server" << std::endl;
  client->close();
  std::cout << "Closed connection" << std::endl;
  return 0;
}
