/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/internal/socket/TcpSocket.h"
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
        namespace internal {
            namespace socket {
                TcpSocket::TcpSocket(const client::Address &address) {
                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    int n= WSAStartup(MAKEWORD(2, 0), &wsa_data);
                    if(n == -1) throw exception::IOException("TcpSocket::TcpSocket ", "WSAStartup error");
                    #endif
                    struct addrinfo hints;
                    memset(&hints, 0, sizeof(hints));
                    hints.ai_family = AF_UNSPEC;
                    hints.ai_socktype = SOCK_STREAM;
                    hints.ai_flags = AI_PASSIVE;

                    int status;
                    serverInfo = NULL;
                    if ((status = getaddrinfo(address.getHost().c_str(),
                                              hazelcast::util::IOUtil::to_string(address.getPort()).c_str(), &hints,
                                              &serverInfo)) != 0) {
                        std::string message = util::IOUtil::to_string(address) + " getaddrinfo error: " +
                                              std::string(gai_strerror(status));
                        throw client::exception::IOException("TcpSocket::TcpSocket", message);
                    }

                    socketId = ::socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
                    if (-1 == socketId) {
                        char errorMsg[200];
                        util::strerror_s(errno, errorMsg, 200, "[TcpSocket::TcpSocket] Failed to obtain socket.");
                        throw client::exception::IOException("TcpSocket::TcpSocket", errorMsg);
                    }
                    isOpen = true;
                    int size = 32 * 1024;
                    if (::setsockopt(socketId, SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(size))) {
                        char errorMsg[200];
                        util::strerror_s(errno, errorMsg, 200, "Failed set socket receive buffer size.");
                        throw client::exception::IOException("TcpSocket::TcpSocket", errorMsg);
                    }
                    if (::setsockopt(socketId, SOL_SOCKET, SO_SNDBUF, (char *) &size, sizeof(size))) {
                        char errorMsg[200];
                        util::strerror_s(errno, errorMsg, 200, "Failed set socket send buffer size.");
                        throw client::exception::IOException("TcpSocket::TcpSocket", errorMsg);
                    }
                    #if defined(SO_NOSIGPIPE)
                    int on = 1;
                    if (setsockopt(socketId, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(int))) {
                        char errorMsg[200];
                        util::strerror_s(errno, errorMsg, 200, "Failed set socket option SO_NOSIGPIPE.");
                        throw client::exception::IOException("TcpSocket::TcpSocket", errorMsg);
                    }
                    #endif
                }

                TcpSocket::TcpSocket(int socketId)
                        : serverInfo(NULL), socketId(socketId), isOpen(true) {
                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    int n= WSAStartup(MAKEWORD(2, 0), &wsa_data);
                    if(n == -1) throw exception::IOException("TcpSocket::TcpSocket ", "WSAStartup error");
                    #endif
                }

                TcpSocket::~TcpSocket() {
                    close();
                }

                int TcpSocket::connect(int timeoutInMillis) {
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
                    tv.tv_usec = (timeoutInMillis - (int) tv.tv_sec * 1000) * 1000;
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

                    int error = 0;
                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    error = WSAGetLastError();
                    #else
                    error = errno;
                    #endif
                    char errorMsg[200];
                    util::strerror_s(error, errorMsg, 200, "Failed to connect the socket.");
                    throw client::exception::IOException("Socket::connect", errorMsg);
                }

                void TcpSocket::setBlocking(bool blocking) {
                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    unsigned long iMode;
                    if(blocking){
                        iMode = 0l;
                    } else {
                        iMode = 1l;
                    }
                    if (ioctlsocket(socketId, FIONBIO, &iMode)) {
                        char errorMsg[200];
                        int error = WSAGetLastError();
                        util::strerror_s(error, errorMsg, 200, "Failed to set the blocking mode.");
                        throw client::exception::IOException("TcpSocket::setBlocking", errorMsg);
                    }
                    #else
                    int arg = fcntl(socketId, F_GETFL, NULL);
                    if (-1 == arg) {
                        char errorMsg[200];
                        util::strerror_s(errno, errorMsg, 200, "Could not get the value of socket flags.");
                        throw client::exception::IOException("TcpSocket::setBlocking", errorMsg);
                    }
                    if (blocking) {
                        arg &= (~O_NONBLOCK);
                    } else {
                        arg |= O_NONBLOCK;
                    }
                    if (-1 == fcntl(socketId, F_SETFL, arg)) {
                        char errorMsg[200];
                        util::strerror_s(errno, errorMsg, 200, "Could not set the blocking value of socket flags.");
                        throw client::exception::IOException("TcpSocket::setBlocking", errorMsg);
                    }
                    #endif
                }

                int TcpSocket::send(const void *buffer, int len) const {
                    #if !defined(WIN32) && !defined(_WIN32) && !defined(WIN64) && !defined(_WIN64)
                    errno = 0;
                    #endif

                    int bytesSend = 0;
                    /**
                     * In linux, sometimes SIGBUS may be received during this call when the server closes the connection.
                     * The returned error code is still error when this flag is set. Hence, it is safe to use.
                     * MSG_NOSIGNAL (since Linux 2.2)
                     * Requests not to send SIGPIPE on errors on stream oriented sockets when the other end breaks the connection.
                     * The EPIPE error is still returned.
                     */
                    if ((bytesSend = ::send(socketId, (char *) buffer, (size_t) len, MSG_NOSIGNAL)) == -1) {
                        #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                        int error = WSAGetLastError();
                        if (WSAEWOULDBLOCK == error) {
                        #else
                        int error = errno;
                        if (EAGAIN == error) {
                        #endif
                            return 0;
                        }

                        char errorMsg[200];
                        util::strerror_s(error, errorMsg, 200, "Send failed.");
                        throw client::exception::IOException("TcpSocket::send", errorMsg);
                    }
                    return bytesSend;
                }

                int TcpSocket::receive(void *buffer, int len, int flag) const {
                    #if !defined(WIN32) && !defined(_WIN32) && !defined(WIN64) && !defined(_WIN64)
                    errno = 0;
                    #endif

                    int size = ::recv(socketId, (char *) buffer, (size_t) len, flag);

                    if (size == -1) {
                        #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                        int error = WSAGetLastError();
                        if (WSAEWOULDBLOCK == error) {
                        #else
                        int error = errno;
                        if (EAGAIN == error) {
                        #endif
                            return 0;
                        }

                        char errorMsg[200];
                        util::strerror_s(error, errorMsg, 200, "Receive failed.");
                        throw client::exception::IOException("TcpSocket::receive", errorMsg);
                    } else if (size == 0) {
                        throw client::exception::IOException("TcpSocket::receive", "Connection closed by remote");
                    }
                    return size;
                }

                int TcpSocket::getSocketId() const {
                    return socketId;
                }


                void TcpSocket::setRemoteEndpoint(const client::Address &address) {
                    remoteEndpoint = address;
                }

                const client::Address &TcpSocket::getRemoteEndpoint() const {
                    return remoteEndpoint;
                }

                client::Address TcpSocket::getAddress() const {
                    char host[1024];
                    char service[20];
                    getnameinfo(serverInfo->ai_addr, serverInfo->ai_addrlen, host, sizeof host, service, sizeof service,
                                NI_NUMERICHOST | NI_NUMERICSERV);
                    Address address(host, atoi(service));
                    return address;
                }

                void TcpSocket::close() {
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
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
