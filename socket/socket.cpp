#include "socket.hpp"

#include <chrono>
#include <iostream>

Socket::Socket(SocketType type)
    : _sock(INVALID_SOCK), _connected(false), _blocking(true), _localEndpoint({"", 0}), _remoteEndpoint({"", 0}) {
  _type = (type == SocketType::TCP) ? SOCK_STREAM : SOCK_DGRAM;

  _sock = ::socket(AF_INET, _type, 0);
}

Socket::Socket(sock_t sock) : _sock(sock), _connected(true), _localEndpoint({"", 0}), _remoteEndpoint({"", 0}) {}

Socket::Socket(Socket&& other) noexcept
    : _sock(other._sock),
      _connected(other._connected),
      _localEndpoint(other._localEndpoint),
      _remoteEndpoint(other._remoteEndpoint) {
  other._sock = INVALID_SOCK;
}

Socket& Socket::operator=(Socket&& other) {
  if (this != &other) {
    close();
    _sock = other._sock;
    _connected = other._connected;
    _localEndpoint = other._localEndpoint;
    _remoteEndpoint = other._remoteEndpoint;
    other._sock = INVALID_SOCK;
  }
  return *this;
}

Socket::~Socket() { close(); }

void Socket::close() {
  if (_sock != INVALID_SOCK) {
#ifdef _WIN32
    closesocket(_sock);
#else
    ::close(_sock);
#endif
  }
  _sock = INVALID_SOCK;
  _connected = false;
  _localEndpoint = {"", 0};
  _remoteEndpoint = {"", 0};
}

auto Socket::getNativeHandle() const { return _sock; }

std::error_code Socket::bind(const Endpoint& endpoint) {
  sockaddr_in addr = createAddr(endpoint);

  if (::bind(_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
    return std::make_error_code(std::errc::address_not_available);
  _connected = true;
  _localEndpoint = endpoint;
  return {};
}

std::error_code Socket::connect(const Endpoint& endpoint, std::chrono::duration<double> timeout) {
  close();
  _sock = ::socket(AF_INET, _type, 0);
  sockaddr_in addr = createAddr(endpoint);

  if (timeout == std::chrono::seconds::zero()) {
    if (::connect(_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1)
      return std::make_error_code(std::errc::connection_refused);
    _connected = true;
    _remoteEndpoint = endpoint;
    return {};
  }

  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timeout);
  timeval tv{.tv_sec = static_cast<decltype(tv.tv_sec)>(milliseconds.count() / 1000),
             .tv_usec = static_cast<decltype(tv.tv_usec)>((milliseconds.count() % 1000) * 1000)};
  fd_set writeSet;
  bool blocking = _blocking;

  setBlocking(false);
  FD_ZERO(&writeSet);
  FD_SET(_sock, &writeSet);
  if (::connect(_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) {
    setBlocking(blocking);
    _connected = true;
    _remoteEndpoint = endpoint;
    return {};
  }
#ifdef _WIN32
  if (WSAGetLastError() != WSAEWOULDBLOCK) return std::make_error_code(std::errc::connection_refused);
#else
  if (errno != EINPROGRESS) return std::make_error_code(std::errc::connection_refused);
#endif
  if (::select(static_cast<int>(_sock + 1), nullptr, &writeSet, nullptr, &tv) <= 0) {
    close();
    return std::make_error_code(std::errc::timed_out);
  }

  int sockErr = 0;
  socklen_t len = sizeof(sockErr);
  if (getsockopt(_sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&sockErr), &len) < 0) {
    close();
    return std::make_error_code(std::errc::connection_refused);
  }
  if (sockErr != 0) {
    close();
    return std::make_error_code(std::errc::connection_aborted);
  }
  setBlocking(blocking);
  _connected = true;
  _remoteEndpoint = endpoint;
  return {};
}

std::error_code Socket::setBlocking(bool enabled) {
  if (_blocking == enabled) {
    std::cout << "AAA" << std::endl;
    return {};
  }
#ifdef _WIN32
  u_long mode = enabled ? 0 : 1;

  if (ioctlsocket(_sock, FIONBIO, &mode) == SOCKET_ERROR)
    return std::error_code(WSAGetLastError(), std::system_category());
#else
  int flags = fcntl(_sock, F_GETFL, 0);

  if (flags == -1) return std::error_code(errno, std::system_category());
  if (enabled)
    flags &= ~O_NONBLOCK;
  else
    flags |= O_NONBLOCK;
  if (fcntl(_sock, F_SETFL, flags) == -1) return std::error_code(errno, std::system_category());
#endif
  _blocking = enabled;
  return {};
}

std::error_code Socket::setReuse(bool enabled) {
  int opt = enabled ? 1 : 0;

  if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) == -1)
    return std::make_error_code(std::errc::operation_not_permitted);
  return {};
}

bool Socket::isBlocking() { return _blocking; }

sockaddr_in Socket::createAddr(const Endpoint& endpoint) {
  sockaddr_in addr{};

  addr.sin_family = AF_INET;
  addr.sin_port = htons(endpoint.port);
  inet_pton(AF_INET, endpoint.addr.c_str(), &addr.sin_addr);
  return addr;
}

std::expected<size_t, std::error_code> Socket::send(const void* data, size_t size) {
  if (!_connected) return std::unexpected(make_error_code(std::errc::not_connected));
  ssize_t sent = ::send(_sock, reinterpret_cast<const char*>(data), size, 0);

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

std::expected<size_t, Socket::SocketStatus> Socket::receive(void* data, size_t size) {
  if (!_connected) return std::unexpected(SocketStatus::Disconnected);
  ssize_t received = ::recv(_sock, reinterpret_cast<char*>(data), size, 0);

  if (received == -1) {
#ifdef _WIN32
    int err = WSAGetLastError();

    if (err == WSAEWOULDBLOCK) return std::unexpected(SocketStatus::NotReady);
    if (err == WSAECONNRESET || err == WSAECONNABORTED) {
      close();
      return std::unexpected(SocketStatus::Disconnected);
    }
    return std::unexpected(SocketStatus::Error);
#else
    if (errno == EWOULDBLOCK || errno == EAGAIN) return std::unexpected(SocketStatus::NotReady);
    if (errno == ECONNRESET) {
      close();
      return std::unexpected(SocketStatus::Disconnected);
    }
    return std::unexpected(SocketStatus::Error);
#endif
  }
  if (received == 0) {
    close();
    return std::unexpected(SocketStatus::Disconnected);
  }
  return received;
}
