#pragma once

#if defined(_WIN32)
#  define SOCK_WINDOWS 1
#  define SOCK_POSIX 0
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#  define SOCK_WINDOWS 0
#  define SOCK_POSIX 1
#else
#  error "Platform not supported"
#endif

#include <cstdint>

#if SOCK_WINDOWS
#  include <WinSock2.h>
#  include <WS2tcpip.h>
#  pragma comment(lib, "Ws2_32.lib") // Need to link with Ws2_32.lib
#elif SOCK_POSIX
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <netinet/tcp.h>
#  include <poll.h>
#  include <sys/errno.h>
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

namespace Kontroller
{
   namespace Sock
   {
      constexpr int kSocketError = -1;

      enum class Result
      {
         Success,
         Error,
         Timeout
      };

#if SOCK_WINDOWS
      using Command = long;
      using Length = int;
      using NumFileDescriptors = ULONG;
      using SignedResult = int;
      using Socket = SOCKET;

      enum Errors
      {
         NoError = 0,
         WouldBlock = WSAEWOULDBLOCK,
         InProgress = WSAEINPROGRESS
      };

      enum class ShutdownMethod
      {
         Read = SD_RECEIVE,
         Write = SD_SEND,
         ReadWrite = SD_BOTH
      };

      constexpr Socket kInvalidSocket = INVALID_SOCKET;
#elif SOCK_POSIX
      using Command = unsigned long;
      using Length = size_t;
      using NumFileDescriptors = nfds_t;
      using SignedResult = ssize_t;
      using Socket = int;

      enum Errors
      {
         NoError = 0,
         WouldBlock = EWOULDBLOCK,
         InProgress = EINPROGRESS
      };

      enum class ShutdownMethod
      {
         Read = SHUT_RD,
         Write = SHUT_WR,
         ReadWrite = SHUT_RDWR
      };

      constexpr Socket kInvalidSocket = -1;
#endif

      namespace System
      {
         inline int getLastError()
         {
#if SOCK_WINDOWS
            return WSAGetLastError();
#elif SOCK_POSIX
            return errno;
#endif
         }

         inline int initialize()
         {
#if SOCK_WINDOWS
            WSADATA wsaData;
            return WSAStartup(MAKEWORD(2, 2), &wsaData);
#elif SOCK_POSIX
            return 0;
#endif
         }

         inline bool terminate()
         {
#if SOCK_WINDOWS
            return WSACleanup() == 0;
#elif SOCK_POSIX
            return true;
#endif
         }
      }

      namespace Endian
      {
         inline uint32_t hostToNetworkLong(uint32_t hostLong)
         {
            return htonl(hostLong);
         }

         inline uint16_t hostToNetworkShort(uint16_t hostShort)
         {
            return htons(hostShort);
         }

         inline uint32_t networkToHostLong(uint32_t netLong)
         {
            return ntohl(netLong);
         }

         inline uint16_t networkToHostShort(uint16_t netShort)
         {
            return ntohs(netShort);
         }
      }

      inline Socket accept(Socket socket, sockaddr* addr, socklen_t* addrlen)
      {
         return ::accept(socket, addr, addrlen);
      }

      inline int bind(Socket socket, const sockaddr* addr, socklen_t addrlen)
      {
         return ::bind(socket, addr, addrlen);
      }

      inline int close(Socket socket)
      {
#if SOCK_WINDOWS
         return ::closesocket(socket);
#elif SOCK_POSIX
         return ::close(socket);
#endif
      }

      inline int connect(Socket socket, const sockaddr* addr, socklen_t addrlen)
      {
         return ::connect(socket, addr, addrlen);
      }

      inline void freeaddrinfo(addrinfo* res)
      {
         return ::freeaddrinfo(res);
      }

      inline int getaddrinfo(const char* node, const char* service, const addrinfo* hints, addrinfo** result)
      {
         return ::getaddrinfo(node, service, hints, result);
      }

      inline int getpeername(Socket socket, sockaddr* address, socklen_t* addressLen)
      {
         return ::getpeername(socket, address, addressLen);
      }

      inline int getsockname(Socket socket, sockaddr* address, socklen_t* addressLen)
      {
         return ::getsockname(socket, address, addressLen);
      }

      inline int getsockopt(Socket socket, int level, int optionName, void* optionValue, socklen_t* optionLen)
      {
#if SOCK_WINDOWS
         return ::getsockopt(socket, level, optionName, static_cast<char*>(optionValue), optionLen);
#elif SOCK_POSIX
         return ::getsockopt(socket, level, optionName, optionValue, optionLen);
#endif
      }

      inline int ioctl(Socket socket, Command command, unsigned long* arg)
      {
#if SOCK_WINDOWS
         return ::ioctlsocket(socket, command, arg);
#elif SOCK_POSIX
         return ::ioctl(socket, command, arg);
#endif
      }

      inline int listen(Socket socket, int backlog)
      {
         return ::listen(socket, backlog);
      }

      inline int poll(pollfd* fds, NumFileDescriptors nfds, int timeout)
      {
#if SOCK_WINDOWS
         return WSAPoll(fds, nfds, timeout);
#elif SOCK_POSIX
         return ::poll(fds, nfds, timeout);
#endif
      }

      inline SignedResult recv(Socket socket, void* buf, Length len, int flags)
      {
#if SOCK_WINDOWS
         return ::recv(socket, static_cast<char*>(buf), len, flags);
#elif SOCK_POSIX
         return ::recv(socket, buf, len, flags);
#endif
      }

      inline SignedResult recvfrom(Socket socket, void* buf, Length len, int flags, sockaddr* srcAddr, socklen_t* addrlen)
      {
#if SOCK_WINDOWS
         return ::recvfrom(socket, static_cast<char*>(buf), len, flags, srcAddr, addrlen);
#elif SOCK_POSIX
         return ::recvfrom(socket, buf, len, flags, srcAddr, addrlen);
#endif
      }

      inline int select(Socket nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout)
      {
#if SOCK_WINDOWS
         return ::select(0, readfds, writefds, exceptfds, timeout);
#elif SOCK_POSIX
         return ::select(nfds, readfds, writefds, exceptfds, timeout);
#endif
      }

      inline SignedResult send(Socket socket, const void* buf, Length len, int flags)
      {
#if SOCK_WINDOWS
         return ::send(socket, static_cast<const char*>(buf), len, flags);
#elif SOCK_POSIX
         return ::send(socket, buf, len, flags);
#endif
      }

      inline SignedResult sendto(Socket socket, const void* buf, Length len, int flags, const sockaddr* destAddr, socklen_t addrlen)
      {
#if SOCK_WINDOWS
         return ::sendto(socket, static_cast<const char*>(buf), len, flags, destAddr, addrlen);
#elif SOCK_POSIX
         return ::sendto(socket, buf, len, flags, destAddr, addrlen);
#endif
      }

      inline int setsockopt(Socket socket, int level, int optionName, const void* optionValue, socklen_t optionLen)
      {
#if SOCK_WINDOWS
         return ::setsockopt(socket, level, optionName, static_cast<const char*>(optionValue), optionLen);
#elif SOCK_POSIX
         return ::setsockopt(socket, level, optionName, optionValue, optionLen);
#endif
      }

      inline int shutdown(Socket socket, ShutdownMethod how)
      {
         return ::shutdown(socket, static_cast<int>(how));
      }

      inline Socket socket(int domain, int type, int protocol)
      {
         return ::socket(domain, type, protocol);
      }

      namespace Helpers
      {
         Result poll(Socket socket, short events, int timeoutMS, const char* caller = nullptr, bool printErrors = false);
      }
   }
}
