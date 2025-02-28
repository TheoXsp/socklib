#pragma once

#include "socket/socket.hpp"

class TcpSocket : public Socket {
 public:
  TcpSocket();
  TcpSocket(sock_t sock);
  TcpSocket(TcpSocket&& other) noexcept;
  TcpSocket& operator=(TcpSocket&& other) noexcept;
  ~TcpSocket() = default;

  std::error_code listen();
  std::expected<TcpSocket, std::error_code> accept();

 private:
  bool _listening;
};
