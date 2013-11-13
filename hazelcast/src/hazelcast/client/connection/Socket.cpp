#include "hazelcast/client/connection/Socket.h"
#include "IOException.h"

#ifdef WIN32
	typedef int socklen_t;
#endif

namespace hazelcast {
    namespace client {
        namespace connection {

            Socket::Socket(const Address &address) : address(address), size(32 * 1024) {
#ifdef WIN32
                    if(WSAStartup(MAKEWORD(2, 0), &wsa_data);
                #endif
                getInfo(address);
                socketId = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
                setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, &size, sizeof(int));
                setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, &size, sizeof(int));
                //setsockopt(socketId, SOL_SOCKET, SO_NOSIGPIPE, &size, sizeof(int));
            };

            Socket::Socket(const Socket &rhs) : address(rhs.address) {
                //private
            };

            Socket::~Socket() {
                close();
            };

            void Socket::connect() {
                if (::connect(socketId, server_info->ai_addr, server_info->ai_addrlen) == -1)
                    throw exception::IOException("Socket::connect", strerror(errno));
            }

            void Socket::send(const void *buffer, int len) const {
                if (::send(socketId, buffer, len, 0) == -1)
                    throw exception::IOException("Socket::send ", "Error socket send" + std::string(strerror(errno)));
            };

            void Socket::receive(void *buffer, int len) const {
                int size = ::recv(socketId, buffer, len, MSG_WAITALL);
                if (size == -1)
                    throw exception::IOException("Socket::receive", "Error socket read");
                else if (size == 0) {
                    throw exception::IOException("Socket::receive", "Connection closed by remote");
                } else if (size != len) {
                    throw exception::IOException("Socket::receive", "incomplete data expected:" + util::to_string(len) + ", actual:" + util::to_string(size));
                }
            };


            std::string Socket::getHost() const {
                return address.getHost();
            }

            int Socket::getPort() const {
                return address.getPort();
            }

            void Socket::close() {
#ifdef WIN32
		        WSACleanup();
		        closesocket(socketId);
	            #else
                ::close(socketId);
#endif
                ::freeaddrinfo(server_info);
            }

            int Socket::getSocketId() const {
                return socketId;
            }

            void Socket::getInfo(const Address &address) {
                struct addrinfo hints;
                std::memset(&hints, 0, sizeof (hints));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                hints.ai_flags = AI_PASSIVE;

                int status;
                if ((status = getaddrinfo(address.getHost().c_str(), hazelcast::util::to_string(address.getPort()).c_str(), &hints, &server_info)) != 0) {
                    throw exception::IOException("Socket::getInfo", address.getHost() + ":" + util::to_string(address.getPort()) + "getaddrinfo error: " + std::string(gai_strerror(status)));
                }

            };

        }
    }
}
