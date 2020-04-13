/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "hazelcast/client/internal/socket/BaseSocket.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#pragma warning(disable: 4003) //for  not enough actual parameters for macro 'min' in asio wait_traits
#endif

using namespace boost;

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                class HAZELCAST_API SSLSocket
                        : public BaseSocket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> {
                public:
                    struct CipherInfo {
                        std::string name;
                        int numberOfBits;
                        std::string version;
                        std::string description;
                    };

                    SSLSocket(boost::asio::io_context &ioService, boost::asio::ssl::context &context,
                              const client::Address &address, client::config::SocketOptions &socketOptions,
                              int64_t connectTimeoutInMillis);

                    /**
                     * @return Returns the supported ciphers. Uses SSL_get_ciphers.
                     */
                    std::vector<SSLSocket::CipherInfo> getCiphers() const;

                    void async_handle_connect(const std::shared_ptr<connection::Connection> connection,
                                              const std::shared_ptr<connection::AuthenticationFuture> authFuture) override;
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
