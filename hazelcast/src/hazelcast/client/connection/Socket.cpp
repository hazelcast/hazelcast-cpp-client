#include "Socket.h"
#include "hazelcast/client/HazelcastException.h"
#include <errno.h>

#include <iostream>

namespace hazelcast {
    namespace client {
        namespace connection {

            Socket::Socket(const Address& address) : address(address), size(32 * 1024) {
                getInfo(address);
                socketId = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
                setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int));
                setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int));
            };

            Socket::Socket(const Socket& rhs) : address(rhs.address) {
                //private
            };

            Socket::~Socket() {
                close();
            };

            void Socket::connect() {
                if (::connect(socketId, server_info->ai_addr, server_info->ai_addrlen) == -1)
                    throw HazelcastException(strerror(errno));
            }

            void Socket::send(const void *buffer, int len) {
                if (::send(socketId, buffer, len, 0) == -1)
                    throw HazelcastException("Socket::send :Error socket send" + std::string(strerror(errno)));
            };

            void Socket::receive(void *buffer, int len) {
                int size = ::recv(socketId, buffer, len, 0);
                if (size == -1)
                    throw HazelcastException("Socket::receive :Error socket read");
                else if (size == 0) {
                    throw HazelcastException("Socket::receive : Connection closed by remote");
                }
            };


            std::string Socket::getHost() const {
                return address.getHost();
            }

            int Socket::getPort() const {
                return address.getPort();
            }

            void Socket::close() {
                ::freeaddrinfo(server_info);
                ::close(socketId);
            }

            int Socket::getSocketId() const {
                return socketId;
            }

            void Socket::getInfo(const Address& address) {
                struct addrinfo hints;
                std::memset(&hints, 0, sizeof (hints));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_flags = AI_PASSIVE;

                int status;
                if ((status = getaddrinfo(address.getHost().c_str(), hazelcast::util::to_string(address.getPort()).c_str(), &hints, &server_info)) != 0) {
                    std::cerr << address << "getaddrinfo error: " << gai_strerror(status) << std::endl;
                }

            };

        }
    }
}
