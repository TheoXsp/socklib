#include "tcpSocket.hpp"

TcpSocket::TcpSocket() : Socket(Socket::SocketType::TCP) {}

TcpSocket::TcpSocket(sock_t sock) : Socket(sock) {}

TcpSocket::TcpSocket(TcpSocket&& other) noexcept : Socket(std::move(other)) {}

TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept {
  Socket::operator=(std::move(other));
  return *this;
}

std::error_code TcpSocket::listen() {
  if (::listen(_sock, SOMAXCONN) == -1) return std::make_error_code(std::errc::operation_not_permitted);
  return {};
}

std::expected<TcpSocket, std::error_code> TcpSocket::accept() {
  sockaddr_in client_addr{};
  socklen_t addr_len = sizeof(client_addr);
  sock_t clientId = ::accept(_sock, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);

  if (clientId == INVALID_SOCK) return std::unexpected(std::make_error_code(std::errc::operation_not_permitted));
  return TcpSocket{std::move(clientId)};
}
