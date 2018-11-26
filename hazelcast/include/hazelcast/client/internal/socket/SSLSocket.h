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
#ifndef HAZELCAST_CLIENT_INTERNAL_SOCKET_SSLSOCKET_H_
#define HAZELCAST_CLIENT_INTERNAL_SOCKET_SSLSOCKET_H_

#ifdef HZ_BUILD_WITH_SSL

#include <asio.hpp>
#include <asio/ssl.hpp>

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/config/SocketOptions.h"
#include "hazelcast/util/AtomicBoolean.h"

#if !defined(MSG_NOSIGNAL)
#  define MSG_NOSIGNAL 0
#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#pragma warning(disable: 4003) //for  not enough actual parameters for macro 'min' in asio wait_traits
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                /**
                 * SSL Socket using asio library
                 */
                class HAZELCAST_API SSLSocket : public Socket {
                public:
                    struct CipherInfo {
                        std::string name;
                        int numberOfBits;
                        std::string version;
                        std::string description;
                    };

                    /**
                     * Constructor
                     */
                    SSLSocket(const client::Address &address, asio::ssl::context &sslContext,
                            client::config::SocketOptions &socketOptions);

                    /**
                     * Destructor
                     */
                    virtual ~SSLSocket();

                    /**
                     * connects to given address in constructor.
                     * @param timeoutInMillis if not connected within timeout, it will return errorCode
                     * @return zero if error. -1 otherwise.
                     */
                    int connect(int timeoutInMillis);

                    /**
                     * @param buffer
                     * @param len length of the buffer
                     * @param flag bsd sockets options flag. Only MSG_WAITALL is supported when SSL is enabled.
                     * @return number of bytes send
                     * @throw IOException in failure.
                     */
                    int send(const void *buffer, int len, int flag = 0);

                    /**
                     * @param buffer
                     * @param len  length of the buffer to be received.
                     * @param flag bsd sockets options flag. Only MSG_WAITALL is supported when SSL is enabled.
                     * @return number of bytes received.
                     * @throw IOException in failure.
                     */
                    int receive(void *buffer, int len, int flag = 0);

                    /**
                     * return socketId
                     */
                    int getSocketId() const;

                    /**
                     * closes the socket. Automatically called in destructor.
                     * Second call to this function is no op.
                     */
                    void close();

                    client::Address getAddress() const;

                    void setBlocking(bool blocking);

                    /**
                     * @return Returns the supported ciphers. Uses SSL_get_ciphers.
                     */
                    std::vector<SSLSocket::CipherInfo> getCiphers() const;

                    std::auto_ptr<Address> localSocketAddress() const;

                private:
                    SSLSocket(const Socket &rhs);

                    SSLSocket &operator=(const Socket &rhs);

                    class ReadHandler {
                    public:
                        ReadHandler(size_t &numRead, asio::error_code &ec);

                        void operator()(const asio::error_code &err, std::size_t bytes_transferred);

                        size_t &getNumRead() const;

                        asio::error_code &getErrorCode() const;

                    private:
                        size_t &numRead;
                        asio::error_code &errorCode;
                    };

                    /**
                     * @return numBytes if the no error or error is try_again or would_block
                     * @throws IOException if the error exists and different from try_again and would_block
                     */
                    int handleError(const std::string &source, size_t numBytes, const asio::error_code &error) const;

                    void handleConnect(const asio::error_code &error);

                    void checkDeadline(const asio::error_code &ec);

                    void setSocketOptions();

                    client::Address remoteEndpoint;

                    asio::io_service ioService;
                    asio::ssl::context &sslContext;
                    std::auto_ptr<asio::ssl::stream<asio::ip::tcp::socket> > socket;
                    asio::deadline_timer deadline;
                    asio::error_code errorCode;
                    int socketId;
                    const client::config::SocketOptions &socketOptions;
                };

                std::ostream &operator<<(std::ostream &out, const SSLSocket::CipherInfo &info);
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HZ_BUILD_WITH_SSL */

#endif /* HAZELCAST_CLIENT_INTERNAL_SOCKET_SSLSOCKET_H_ */
