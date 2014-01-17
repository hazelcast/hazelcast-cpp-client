#include "hazelcast/client/Socket.h"
#include "hazelcast/client/exception/IOException.h"
#include <iostream>
#include <cassert>

namespace hazelcast {
    namespace client {
        Socket::Socket(const client::Address &address){
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
            int size = 32 * 1024;
            ::setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(size));
            ::setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, (char *) &size, sizeof(size));
            #if defined(SO_NOSIGPIPE)
            int on = 1;
            setsockopt(socketId, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(int));
            #endif

        };


        Socket::Socket(int socketId)
        : serverInfo(NULL)
        ,socketId(socketId)
        ,isOpen(true){

        }

        Socket::Socket(const Socket &rhs) {
            //private
        };

        Socket::~Socket() {
            close();
            if (serverInfo != NULL)
                ::freeaddrinfo(serverInfo);
        };

        int Socket::connect() {
            assert(serverInfo != NULL && "Socket is already connected");
            if (::connect(socketId, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
                #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                int error =   WSAGetLastError();
                #else
                int error = errno;
                #endif
                return error;
            }
            return 0;
        }

        int Socket::send(const void *buffer, int len) const {
            int bytesSend = 0;
            if ((bytesSend = ::send(socketId, (char *) buffer, (size_t)len, 0)) == -1)
                throw client::exception::IOException("Socket::send ", "Error socket send" + std::string(strerror(errno)));
            return bytesSend;
        };

        int Socket::receive(void *buffer, int len, int flag) const {
            int size = ::recv(socketId, (char *) buffer, (size_t)len, flag);

            if (size == -1)
                throw client::exception::IOException("Socket::receive", "Error socket read");
            else if (size == 0) {
                throw client::exception::IOException("Socket::receive", "Connection closed by remote");
            }
            return size;
        };

        int Socket::getSocketId() const {
            return socketId;
        }

        client::Address Socket::getAddress() const {
            char host[1024];
            char service[20];
            getnameinfo(serverInfo->ai_addr, serverInfo->ai_addrlen, host, sizeof host, service, sizeof service, 0);
            Address address(host,atoi(service));
            return address;
        }

        void Socket::close() {
            bool expected = true;
            if (isOpen.compare_exchange_strong(expected, false)) {
                ::shutdown(socketId, SHUT_RD);
                char buffer[1];
                ::recv(socketId, buffer, 1, MSG_WAITALL);
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
