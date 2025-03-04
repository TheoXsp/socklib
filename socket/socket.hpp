#pragma once

#include <chrono>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <system_error>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using sock_t = SOCKET;
using socklen_t = int;
constexpr sock_t INVALID_SOCK = INVALID_SOCKET;

class WinSockInitializer {
 public:
  WinSockInitializer() {
    WSADATA wsa_data{};

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) throw std::runtime_error("WSAStartup failed");
  }

  ~WinSockInitializer() { WSACleanup(); }
};

static WinSockInitializer winsockInitializer;
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using sock_t = int;
constexpr sock_t INVALID_SOCK = -1;
#endif

class Socket {
 public:
  struct Endpoint {
    std::string addr;
    uint16_t port;
  };

  enum class SocketType { TCP, UDP };

  enum class SocketStatus { NotReady, Disconnected, Error };

  void close();
  auto getNativeHandle() const;

  std::error_code bind(const Endpoint& endpoint);
  std::error_code connect(const Endpoint& endpoint);
  std::error_code setBlocking(bool enabled);
  std::error_code setReuse(bool enabled);

  std::expected<size_t, std::error_code> send(const void* data, size_t size);
  std::expected<size_t, SocketStatus> receive(void* buffer, size_t size);

 protected:
  Socket(SocketType type);
  Socket(sock_t sock);
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket(Socket&& other) noexcept;
  Socket& operator=(Socket&& other);
  virtual ~Socket();

  sockaddr_in createAddr(const Endpoint& endpoint);

  sock_t _sock;
  bool _connected;
};
