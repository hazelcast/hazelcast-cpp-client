/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/Util.h"

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <string.h>

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
            memset(&hints, 0, sizeof (hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;

            int status;
			serverInfo = NULL;
            if ((status = getaddrinfo(address.getHost().c_str(), hazelcast::util::IOUtil::to_string(address.getPort()).c_str(), &hints, &serverInfo)) != 0) {
                std::string message = util::IOUtil::to_string(address) + " getaddrinfo error: " + std::string(gai_strerror(status));
                throw client::exception::IOException("Socket::Socket", message);
            }

            socketId = ::socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
            if (-1 == socketId) {
                char errorMsg[200];
                util::strerror_s(errno, errorMsg, 200, "[Socket::Socket] Failed to obtain socket.");
                throw client::exception::IOException("Socket::Socket", errorMsg);
            }
            isOpen = true;
            int size = 32 * 1024;
            if (::setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(size))) {
                char errorMsg[200];
                util::strerror_s(errno, errorMsg, 200, "Failed set socket receive buffer size.");
                throw client::exception::IOException("Socket::Socket", errorMsg);
            }
            if (::setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, (char *) &size, sizeof(size))) {
                char errorMsg[200];
                util::strerror_s(errno, errorMsg, 200, "Failed set socket send buffer size.");
                throw client::exception::IOException("Socket::Socket", errorMsg);
            }
			#if defined(SO_NOSIGPIPE)
            int on = 1;
            if (setsockopt(socketId, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(int))) {
                char errorMsg[200];
                util::strerror_s(errno, errorMsg, 200, "Failed set socket option SO_NOSIGPIPE.");
                throw client::exception::IOException("Socket::Socket", errorMsg);
            }
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

            if (::connect(socketId, serverInfo->ai_addr, serverInfo->ai_addrlen)) {
                int error = 0;
                #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                error = WSAGetLastError();
			    if (WSAEWOULDBLOCK != error && WSAEINPROGRESS != error && WSAEALREADY != error) {
                #else
                error = errno;
                if (EINPROGRESS != error && EALREADY != error) {
                #endif
                    char errorMsg[200];
                    util::strerror_s(error, errorMsg, 200, "Failed to connect the socket.");
                    throw client::exception::IOException("Socket::connect", errorMsg);
                }
            }

            struct timeval tv;
            tv.tv_sec = timeoutInMillis / 1000;
            tv.tv_usec = (timeoutInMillis - (int)tv.tv_sec * 1000) * 1000;
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

            return 0;
        }

        void Socket::setBlocking(bool blocking) {
            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            unsigned long iMode;
            if(blocking){
                iMode = 0l;
            } else {
                iMode = 1l;
            }
            if (ioctlsocket(socketId, FIONBIO, &iMode)) {
                char errorMsg[200];
                util::strerror_s(errno, errorMsg, 200, "Failed to set the blocking mode.");
                throw client::exception::IOException("Socket::setBlocking", errorMsg);
            }
            #else
            int arg = fcntl(socketId, F_GETFL, NULL);
            if (-1 == arg) {
                char errorMsg[200];
                util::strerror_s(errno, errorMsg, 200, "Could not get the value of socket flags.");
                throw client::exception::IOException("Socket::setBlocking", errorMsg);
            }
            if (blocking) {
                arg &= (~O_NONBLOCK);
            } else {
                arg |= O_NONBLOCK;
            }
            if (-1 == fcntl(socketId, F_SETFL, arg)) {
                char errorMsg[200];
                util::strerror_s(errno, errorMsg, 200, "Could not set the blocking value of socket flags.");
                throw client::exception::IOException("Socket::setBlocking", errorMsg);
            }
            #endif
        }

        int Socket::send(const void *buffer, int len) const {
            errno = 0;
            int bytesSend = 0;
            /**
             * In linux, sometimes SIGBUS may be received during this call when the server closes the connection.
             * The returned error code is still error when this flag is set. Hence, it is safe to use.
             * MSG_NOSIGNAL (since Linux 2.2)
             * Requests not to send SIGPIPE on errors on stream oriented sockets when the other end breaks the connection.
             * The EPIPE error is still returned.
             */
            if ((bytesSend = ::send(socketId, (char *)buffer, (size_t)len, MSG_NOSIGNAL)) == -1) {
                if (errno == EAGAIN) {
                    return 0;
                }
                throw client::exception::IOException("Socket::send ", "Error socket send " + std::string(strerror(errno)));
            }
            return bytesSend;
        }

        int Socket::receive(void *buffer, int len, int flag) const {
            errno = 0;
            int size = ::recv(socketId, (char *)buffer, (size_t)len, flag);

            if (size == -1) {
                if (errno == EAGAIN) {
                    return 0;
                }
                throw client::exception::IOException("Socket::receive", "Error socket read " + std::string(strerror(errno)));
            } else if (size == 0) {
                throw client::exception::IOException("Socket::receive", "Connection closed by remote");
            }
            return size;
        }

        int Socket::getSocketId() const {
            return socketId;
        }


        void Socket::setRemoteEndpoint(const client::Address &address) {
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

        bool socketPtrComp::operator ()(Socket const *const &lhs, Socket const *const &rhs) const {
            return lhs->getSocketId() > rhs->getSocketId();
        }

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
