#include "udpSocket.hpp"

UdpSocket::UdpSocket() : Socket(Socket::SocketType::UDP) {}

UdpSocket::UdpSocket(UdpSocket &&other) noexcept : Socket(std::move(other)) {}

UdpSocket &UdpSocket::operator=(UdpSocket &&other) noexcept {
  Socket::operator=(std::move(other));
  return *this;
}

std::expected<size_t, std::error_code> UdpSocket::sendTo(const void *data, size_t size,
                                                         const Socket::Endpoint &endpoint) {
  sockaddr_in addr = createAddr(endpoint);
  ssize_t sent =
      ::sendto(_sock, reinterpret_cast<const char *>(data), size, 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

  if (sent == -1) {
#ifdef _WIN32
    int err = WSAGetLastError();

    if (err == WSAEWOULDBLOCK) return std::unexpected(std::make_error_code(std::errc::resource_unavailable_try_again));
    return std::unexpected(std::error_code(err, std::system_category()));
#else
    if (errno == EWOULDBLOCK || errno == EAGAIN)
      return std::unexpected(std::make_error_code(std::errc::resource_unavailable_try_again));
    return std::unexpected(std::error_code(errno, std::system_category()));
#endif
  }
  return sent;
}

std::expected<size_t, std::error_code> UdpSocket::receiveFrom(void *data, size_t size, Socket::Endpoint &endpoint) {
  sockaddr_in src_addr{};
  socklen_t addr_len = sizeof(src_addr);
  ssize_t recvd =
      ::recvfrom(_sock, reinterpret_cast<char *>(data), size, 0, reinterpret_cast<sockaddr *>(&src_addr), &addr_len);

  if (recvd == -1) {
#ifdef _WIN32
    int err = WSAGetLastError();

    if (err == WSAEWOULDBLOCK) return std::unexpected(std::make_error_code(std::errc::resource_unavailable_try_again));
    return std::unexpected(std::error_code(err, std::system_category()));
#else
    if (errno == EWOULDBLOCK || errno == EAGAIN)
      return std::unexpected(std::make_error_code(std::errc::resource_unavailable_try_again));
    return std::unexpected(std::error_code(errno, std::system_category()));
#endif
  }
  endpoint.addr = inet_ntoa(src_addr.sin_addr);
  endpoint.port = ntohs(src_addr.sin_port);
  return recvd;
}
