#include "Socket.h"
#include "HazelcastException.h"

#include <vector>

namespace hazelcast {
    namespace client {
        namespace protocol2 {

            Socket::Socket(Address& address) : address(address) {
                getInfo();
                socketId = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
                if (connect(socketId, server_info->ai_addr, server_info->ai_addrlen) == -1) {
                    std::cout << "connection error" << std::endl;

                }
            };

            Socket::Socket(const Socket& rhs) {
                //private
                address = rhs.address;
                socketId = rhs.socketId;
            };

            Socket::~Socket() {
                ::freeaddrinfo(server_info);
                ::close(socketId);
            };

            void Socket::sendData(const void *buffer, int len) {
                if (send(socketId, buffer, len, 0) == -1)
                    throw hazelcast::client::HazelcastException("Socket::sendData :Error socket send");
            };

            std::string Socket::readLine() {
                std::string line;
                bool lineIsRead = false;
                while (!lineIsRead) {
                    char current;
                    recvData(&current, sizeof (char));
                    if (current == '\n') {
                        lineIsRead = true;
                    } else if (current != '\r') {
                        line.push_back(current);
                    }

                }
                return line;
            };

            void Socket::recvData(void *buffer, int len) {
                int size = recv(socketId, buffer, len, 0);
                if (size == -1)
                    throw hazelcast::client::HazelcastException("Socket::recvData :Error socket read");
                else if (size == 0) {
                    throw hazelcast::client::HazelcastException("Socket::recvData : Connection closed by remote");
                }
            };

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
