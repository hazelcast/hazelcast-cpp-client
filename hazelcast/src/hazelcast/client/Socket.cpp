#include "hazelcast/client/Socket.h"
#include "hazelcast/client/exception/IOException.h"

namespace hazelcast {
    namespace client {
        Socket::Socket(const client::Address &address): address(address), size(32 * 1024) {
            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                int n= WSAStartup(MAKEWORD(2, 0), &wsa_data);
                if(n == -1) throw exception::IOException("Socket::Socket ", "WSAStartup error");
            #endif
            struct addrinfo hints;
            std::memset(&hints, 0, sizeof (hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;

            int status;
            if ((status = getaddrinfo(address.getHost().c_str(), hazelcast::util::to_string(address.getPort()).c_str(), &hints, &serverInfo)) != 0) {
                throw client::exception::IOException("Socket::getInfo", address.getHost() + ":" + util::to_string(address.getPort()) + "getaddrinfo error: " + std::string(gai_strerror(status)));
            }
            socketId = ::socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
            isOpen = true;
            ::setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(size));
            ::setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, (char *) &size, sizeof(size));
            #if defined(SO_NOSIGPIPE)
            setsockopt(socketId, SOL_SOCKET, SO_NOSIGPIPE, &size, sizeof(int));
            #endif

        };

        Socket::Socket(const Socket &rhs) : address(rhs.address) {
            //private
        };

        Socket::~Socket() {
            close();
        };

        void Socket::connect() {
            if (::connect(socketId, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
                #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                int error =   WSAGetLastError();
                #else
                int error = errno;
                #endif
                throw client::exception::IOException("Socket::connect", strerror(error));

            }
        }

        void Socket::send(const void *buffer, int len) const {
            if (::send(socketId, (char *) buffer, len, 0) == -1)
                throw client::exception::IOException("Socket::send ", "Error socket send" + std::string(strerror(errno)));
        };

        int Socket::receive(void *buffer, int len, int flag) const {
            int size = ::recv(socketId, (char *) buffer, len, flag);

            if (size == -1)
                throw client::exception::IOException("Socket::receive", "Error socket read");
            else if (size == 0) {
                throw client::exception::IOException("Socket::receive", "Connection closed by remote");
            }
            return size;
        };


        std::string Socket::getHost() const {
            return address.getHost();
        }

        int Socket::getPort() const {
            return address.getPort();
        }

        int Socket::getSocketId() const {
            return socketId;
        }


        const client::Address& Socket::getAddress() const {
            return address;
        }

        void Socket::close() {
            bool expected = true;
            if (isOpen.compare_exchange_strong(expected, false)) {
                ::shutdown(socketId, SHUT_RD);
                char buffer[1];
                ::recv(socketId, (char *) buffer, 1, MSG_WAITALL);
                #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                WSACleanup();
                closesocket(socketId);
                #else
                ::close(socketId);
                #endif
            }
        }


    }
}
