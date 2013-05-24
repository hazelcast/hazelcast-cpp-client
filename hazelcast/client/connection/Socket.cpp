#include "Socket.h"
#include "../HazelcastException.h"
#include <errno.h>

#include <vector>
#include <iostream>

namespace hazelcast {
    namespace client {
        namespace connection {

            Socket::Socket(Address& address) : address(address) {
                getInfo();
                socketId = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
            };

            Socket::Socket(const Socket& rhs) : address(rhs.address) {
                //private
            };

            Socket::~Socket() {
                close();
            };

            void Socket::connect() {
                if (::connect(socketId, server_info->ai_addr, server_info->ai_addrlen) == -1)
                    throw hazelcast::client::HazelcastException(strerror(errno));
            }

            void Socket::send(const void *buffer, int len) {
                if (::send(socketId, buffer, len, 0) == -1)
                    throw hazelcast::client::HazelcastException("Socket::send :Error socket send" + std::string(strerror(errno)));
            };

            void Socket::receive(void *buffer, int len) {
                int size = ::recv(socketId, buffer, len, 0);
                if (size == -1)
                    throw hazelcast::client::HazelcastException("Socket::receive :Error socket read");
                else if (size == 0) {
                    throw hazelcast::client::HazelcastException("Socket::receive : Connection closed by remote");
                }
            };

            void Socket::close() {
                ::freeaddrinfo(server_info);
                ::close(socketId);
            }

            int Socket::getSocketId() const {
                return socketId;
            }

            void Socket::getInfo() {
                struct addrinfo hints;
                std::memset(&hints, 0, sizeof (hints));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_flags = AI_PASSIVE;

                int status;
                if ((status = getaddrinfo(address.getAddress().c_str(), address.getPort().c_str(), &hints, &server_info)) != 0) {
                    std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
                }

            };

        }
    }
}
