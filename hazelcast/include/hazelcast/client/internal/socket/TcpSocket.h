/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/asio.hpp>

#include "hazelcast/client/internal/socket/BaseSocket.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace socket {
                class HAZELCAST_API TcpSocket : public BaseSocket<boost::asio::ip::tcp::socket> {
                public:
                    TcpSocket(boost::asio::io_context &io, const Address &address,
                              client::config::SocketOptions &socketOptions, int64_t connectTimeoutInMillis);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_SOCKET_TCPSOCKET_H_ */
