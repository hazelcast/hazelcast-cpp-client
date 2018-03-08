/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_SOCKET_TCPSOCKET_H_
#define HAZELCAST_CLIENT_INTERNAL_SOCKET_TCPSOCKET_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

typedef int socklen_t;

#else

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <fcntl.h>

#endif

#include "hazelcast/client/Socket.h"

#include "hazelcast/client/Address.h"
#include "hazelcast/util/AtomicBoolean.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

#if !defined(MSG_NOSIGNAL)
#  define MSG_NOSIGNAL 0
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                /**
                 * c Sockets wrapper class.
                 */
                class HAZELCAST_API TcpSocket : public Socket {
                public:
                    /**
                     * Constructor
                     */
                    TcpSocket(int socketId);

                    /**
                     * Constructor
                     */
                    TcpSocket(const client::Address &address);

                    /**
                     * Destructor
                     */
                    virtual ~TcpSocket();

                    /**
                     * connects to given address in constructor.
                     * @param timeoutInMillis if not connected within timeout, it will return errorCode
                     * @return zero if error. -1 otherwise.
                     */
                    int connect(int timeoutInMillis);

                    /**
                     * @param buffer
                     * @param len length of the buffer
                     * @return number of bytes send
                     * @throw IOException in failure.
                     */
                    int send(const void *buffer, int len) const;

                    /**
                     * @param buffer
                     * @param len  length of the buffer to be received.
                     * @param flag bsd sockets options flag.
                     * @return number of bytes received.
                     * @throw IOException in failure.
                     */
                    int receive(void *buffer, int len, int flag = 0) const;

                    /**
                     * return socketId
                     */
                    int getSocketId() const;

                    /**
                     * @param address remote endpoint address.
                     */
                    void setRemoteEndpoint(const client::Address &address);

                    /**
                     * @return remoteEndpoint
                     */
                    const client::Address &getRemoteEndpoint() const;

                    /**
                     * closes the socket. Automatically called in destructor.
                     * Second call to this function is no op.
                     */
                    void close();

                    client::Address getAddress() const;

                    void setBlocking(bool blocking);

                private:
                    TcpSocket(const Socket &rhs);

                    TcpSocket &operator=(const Socket &rhs);

                    void throwIOException(const char *methodName, const char *prefix) const;

                    void throwIOException(int error, const char *methodName, const char *prefix) const;

                    client::Address remoteEndpoint;

                    const client::Address configAddress;

                    struct addrinfo *serverInfo;
                    int socketId;
                    util::AtomicBoolean isOpen;

                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    WSADATA wsa_data;
                    #endif
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_SOCKET_TCPSOCKET_H_ */
