#include "hazelcast/client/Socket.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/IOUtil.h"
#include <iostream>
#include <cassert>
#include <cstdlib>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        Socket::Socket(const client::Address &address) {
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
			serverInfo = NULL;
            if ((status = getaddrinfo(address.getHost().c_str(), hazelcast::util::IOUtil::to_string(address.getPort()).c_str(), &hints, &serverInfo)) != 0) {
                std::string message = util::IOUtil::to_string(address) + " getaddrinfo error: " + std::string(gai_strerror(status));
                throw client::exception::IOException("Socket::getInfo", message);
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

        }


        Socket::Socket(int socketId)
        : serverInfo(NULL)
        , socketId(socketId)
        , isOpen(true) {
			#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            int n= WSAStartup(MAKEWORD(2, 0), &wsa_data);
            if(n == -1) throw exception::IOException("Socket::Socket ", "WSAStartup error");
			#endif
        }

        Socket::~Socket() {
            close();
        }

        int Socket::connect(int timeoutInMillis) {
            assert(serverInfo != NULL && "Socket is already connected");
            setBlocking(false);
            ::connect(socketId, serverInfo->ai_addr, serverInfo->ai_addrlen);

            struct timeval tv;
            tv.tv_sec = timeoutInMillis / 1000;
            tv.tv_usec = (timeoutInMillis - tv.tv_sec * 1000) * 1000;
            fd_set mySet, err;
            FD_ZERO(&mySet);
            FD_ZERO(&err);
            FD_SET(socketId, &mySet);
            FD_SET(socketId, &err);
            errno = 0;
            if (select(socketId + 1, NULL, &mySet, &err, &tv) > 0) {
                setBlocking(true);
                return 0;
            }
            setBlocking(true);

            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            int error =   WSAGetLastError();
            #else
            int error = errno;
            #endif
            return error;

        }

        void Socket::setBlocking(bool blocking) {
            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            unsigned long iMode;
            if(blocking){
                iMode = 0l;
            } else {
                iMode = 1l;
            }
            ioctlsocket(socketId, FIONBIO, &iMode);
            #else
            long arg = fcntl(socketId, F_GETFL, NULL);
            if (blocking) {
                arg &= (~O_NONBLOCK);
            } else {
                arg |= O_NONBLOCK;
            }
            fcntl(socketId, F_SETFL, arg);
            #endif

        }

        int Socket::send(const void *buffer, int len) const {
            errno = 0;
            int bytesSend = 0;
            if ((bytesSend = ::send(socketId, (char *) buffer, (size_t) len, 0)) == -1)
                throw client::exception::IOException("Socket::send ", "Error socket send " + std::string(strerror(errno)));
            return bytesSend;
        }

        int Socket::receive(void *buffer, int len, int flag) const {
            errno = 0;
            int size = ::recv(socketId, (char *) buffer, (size_t) len, flag);

            if (size == -1)
                throw client::exception::IOException("Socket::receive", "Error socket read " + std::string(strerror(errno)));
            else if (size == 0) {
                throw client::exception::IOException("Socket::receive", "Connection closed by remote");
            }
            return size;
        }

        int Socket::getSocketId() const {
            return socketId;
        }


        void Socket::setRemoteEndpoint(client::Address &address) {
            remoteEndpoint = address;
        }

        const client::Address &Socket::getRemoteEndpoint() const {
            return remoteEndpoint;
        }

        client::Address Socket::getAddress() const {
            char host[1024];
            char service[20];
            getnameinfo(serverInfo->ai_addr, serverInfo->ai_addrlen, host, sizeof host, service, sizeof service, 0);
            Address address(host, atoi(service));
            return address;
        }

        void Socket::close() {
            if (isOpen.compareAndSet(true, false)) {
				if (serverInfo != NULL)
					::freeaddrinfo(serverInfo);

				#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
				::shutdown(socketId, SD_RECEIVE);
				char buffer[1];
                ::recv(socketId, buffer, 1, MSG_WAITALL);
				WSACleanup();
                closesocket(socketId);
                #else
				::shutdown(socketId, SHUT_RD);
				char buffer[1];
                ::recv(socketId, buffer, 1, MSG_WAITALL);
                ::close(socketId);
                #endif

                socketId = -1; // set it to invalid descriptor to avoid misuse of the socket for this connection
            }
        }


    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
