# TASKS

Hey, i'm developping a C++23 POSIX/Winsock2 sockets managements library. The main concern is to provide modern and simple, crossplatform way to send and receive packets via casual sockets.

The way i see things is pretty simplen here is the overall structure:

- Socket -> bind - connect - send - receive
- Udp -> SendTo/ReceiveFrom
- Tcp -> Listen / Accept

Additional Socket methods:

- setOpt (TCP_NODELAY, block/nonBlock, SO_REUSEADDR/SO_REUSEPORT)
- Timeout

The library must be IPV4 only, RAII complient, std::error_code based for error handling (or some enum class or something similar for non-blocing sockets status gathering).
Based on that descriptions and needs, go ahead and provide an implementation for those.

## Reflexions

return pour send: std::expected size_t | std::error_code
return pour recv: std::expected status | std::error_code

## Reworks

- usage de span
- size de retour et d'envois (gerer les envois partiels)

taille de send au choix
preciser type ?