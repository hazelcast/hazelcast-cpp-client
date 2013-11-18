#include "hazelcast/client/connection/Socket.h"
#include "IOException.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Socket::Socket(const Address &address): address(address), size(32 * 1024) {
                #ifdef WIN32
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
                    throw exception::IOException("Socket::getInfo", address.getHost() + ":" + util::to_string(address.getPort()) + "getaddrinfo error: " + std::string(gai_strerror(status)));
                }
                socketId = ::socket(serverInfo->ai_family,serverInfo->ai_socktype, serverInfo->ai_protocol);

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
                if (::connect(socketId, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1)
                    throw exception::IOException("Socket::connect", strerror(errno));
            }

            void Socket::send(const void *buffer, int len) const {
                if (::send(socketId, (char*)buffer, len, 0) == -1)
                    throw exception::IOException("Socket::send ", "Error socket send" + std::string(strerror(errno)));
            };

            void Socket::receive(void *buffer, int len) const {
                #ifdef WIN32
					int size = ::recv(socketId, (char*)buffer, len, 0 );
				#else
                int size = ::recv(socketId, buffer, len, MSG_WAITALL);
                #endif

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
            }

            int Socket::getSocketId() const {
                return socketId;
            }

        }
    }
}
