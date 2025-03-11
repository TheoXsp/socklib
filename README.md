# Socklib

## Contexte

Library de gestion de sockets POSIX cross-platform, conveniante et performante.

## Sommaire

## Problématique

Jusqu'alors (et jusqu'à ce que [std::net](https://en.cppreference.com/w/cpp/header/experimental/net) ne soit pas totalement implementé), la communication réseau en C++ n'est pas triviale. Dans cette optique, l'objectif est de proposer une encapsulation cross-platform facile d'utilisation et performante pour l'utilisation de sockets.

## Structure globale

La phylosophie est inspirée du module network de [SFML](https://github.com/SFML/SFML), c'est à dire qu'on peut détenir un objet **UdpSocket** ou **TcpSocket**. La différence majeure est qu'on utilise ici au maximum les uses et coutumes du standard 23 (là où SFML est actuellement sur C++17). On profite donc de **std::expected** ou encore **std::error_code**, clarifiant les intentions de chaque méthode tout en ayant un flow de contrôle performant.
Attention, les fonctionnalités pour les serveurs TCP sont incluses dans l'objet **TcpSocket**, et non scindées dans un objet spécifique pour l'écoute comme dans SFML.

## Concepts

Pour imager un **endpoint**, on à l'objet suivant:

```c++
struct Endpoint {
    std::string addr;
    uint16_t port;

    bool operator!() const { return addr.empty() || port == 0; }
    bool operator==(const Endpoint& other) const { return addr == other.addr && port == other.port; }
    bool operator!=(const Endpoint& other) const { return !(*this == other); }
};
```

On peut vérifier leur validité et les comparer, c'est avec cet objet qu'on utilise les méthodes **connect**, **sendTo** ou **receiveFrom** par exemple.

## Exemples minimaux

### Serveur et client TCP, sockets bloquantes

#### Serveur

```c++
#include <iostream>
#include <memory>
#include <vector>

#include "tcpSocket/tcpSocket.hpp"
#include "udpSocket/udpSocket.hpp"

int main(int ac, char **av) {
  std::unique_ptr<TcpSocket> server = std::make_unique<TcpSocket>();
  std::vector<std::byte> buffer(1024);

  server->bind({"0.0.0.0", 8080});
  server->listen();
  auto client = server->accept();

  if (client) {
    std::cout << "Client connected, waiting for data ..." << std::endl;
    auto data = client->receive(buffer.data(), buffer.size());
    if (data) {
      std::cout << "Received " << *data << " bytes" << std::endl;
      std::cout << "Data: " << std::string(reinterpret_cast<char *>(buffer.data()), *data) << std::endl;
    } else {
      std::cout << "Error receiving data" << std::endl;
    }
  }
  client->close();
  server->close();
  return 0;
}
```

#### Client

```c++
#include <iostream>
#include <memory>
#include <vector>

#include "tcpSocket/tcpSocket.hpp"
#include "udpSocket/udpSocket.hpp"

int main(int ac, char **av) {
  std::unique_ptr<TcpSocket> client = std::make_unique<TcpSocket>();
  std::string data = "Hello, World!";

  auto err = client->connect({"127.0.0.1", 8080});
  if (err) {
    std::cerr << "Failed to connect: " << err.message() << std::endl;
    return 1;
  }
  client->send(data.c_str(), data.size());
  client->close();
}
```

### Serveur et client UDP, sockets non bloquantes

#### Serveur

```c++
#include <iostream>
#include <memory>
#include <vector>

#include "tcpSocket/tcpSocket.hpp"
#include "udpSocket/udpSocket.hpp"

int main(int ac, char **av) {
  std::unique_ptr<UdpSocket> server = std::make_unique<UdpSocket>();
  std::vector<std::byte> buffer(1024);

  server->bind({"0.0.0.0", 8080});
  server->setBlocking(false);
  while (true) {
    Socket::Endpoint remote;
    auto data = server->receiveFrom(buffer.data(), buffer.size(), remote);
    if (data) {
      std::cout << "Received " << *data << " bytes from " << remote.addr << ":" << remote.port << std::endl;
      std::cout << "Data: " << std::string(reinterpret_cast<char *>(buffer.data()), *data) << std::endl;
    }
  }
  server->close();
  return 0;
}
```

#### Client

```c++
#include <iostream>
#include <memory>
#include <vector>

#include "tcpSocket/tcpSocket.hpp"
#include "udpSocket/udpSocket.hpp"

int main(int ac, char **av) {
  std::unique_ptr<UdpSocket> client = std::make_unique<UdpSocket>();
  std::string data = "Hello, World!";

  client->setBlocking(false);
  client->sendTo(data.c_str(), data.size(), {"127.0.0.1", 8080});
  client->close();
}
```
