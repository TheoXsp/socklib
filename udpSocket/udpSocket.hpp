#pragma once

#include "socket/socket.hpp"

class UdpSocket : public Socket {
 public:
  UdpSocket();
  UdpSocket(UdpSocket &&other) noexcept;
  UdpSocket &operator=(UdpSocket &&other) noexcept;
  ~UdpSocket() = default;

  std::expected<size_t, std::error_code> sendTo(void *data, size_t size, const Socket::Endpoint &endpoint);
  std::expected<size_t, std::error_code> receiveFrom(void *data, size_t size, Socket::Endpoint &endpoint);
};
