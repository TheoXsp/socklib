#pragma once

#include "socket/socket.hpp"

class UdpSocket : public Socket {
 public:
  UdpSocket();
  UdpSocket(UdpSocket&& other) noexcept;
  UdpSocket& operator=(UdpSocket&& other) noexcept;
  ~UdpSocket() = default;

  std::expected<size_t, std::error_code> sendTo(std::span<std::byte> data,
                                                const Socket::Endpoint& endpoint);
  std::expected<size_t, std::error_code> receiveFrom(std::span<std::byte> buffer, Socket::Endpoint& endpoint);
};
