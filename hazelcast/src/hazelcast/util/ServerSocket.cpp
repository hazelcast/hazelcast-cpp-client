//
// Created by sancar koyunlu on 30/12/13.
//

#include "hazelcast/util/ServerSocket.h"
#include "hazelcast/client/exception/IOException.h"

namespace hazelcast {
    namespace util {

        ServerSocket::ServerSocket(int port) {
            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                int n= WSAStartup(MAKEWORD(2, 0), &wsa_data);
                if(n == -1) throw exception::IOException("Socket::Socket ", "WSAStartup error");
            #endif
            struct addrinfo hints;
            struct addrinfo *serverInfo;

            std::memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
            ::getaddrinfo(NULL, hazelcast::util::to_string(port).c_str(), &hints, &serverInfo);
            socketId = ::socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
            isOpen = true;
            ::bind(socketId, serverInfo->ai_addr, serverInfo->ai_addrlen);
            ::listen(socketId, 10);
            ::freeaddrinfo(serverInfo);

        }


        ServerSocket::~ServerSocket() {
            close();
        }

        void ServerSocket::close() {
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

        int ServerSocket::getPort() const {
            struct sockaddr_in sin;
            socklen_t len = sizeof(sin);
            if (getsockname(socketId, (struct sockaddr *)&sin, &len) == -1)
                throw client::exception::IOException("ServerSocket::getPort()", "getsockname");
            return ntohs(sin.sin_port);
        }

        client::Socket *ServerSocket::accept() {
            struct sockaddr_storage their_address;
            socklen_t address_size = sizeof their_address;
            int sId = ::accept(socketId, (struct sockaddr *) &their_address, &address_size);

            if (sId == -1) {
                #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                int error =   WSAGetLastError();
                #else
                int error = errno;
                #endif
                throw client::exception::IOException("Socket::accept", strerror(error));
            }
            return new client::Socket(sId);
        }
    }
}