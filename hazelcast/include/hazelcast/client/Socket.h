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
#ifndef HAZELCAST_CLIENT_SOCKET_H_
#define HAZELCAST_CLIENT_SOCKET_H_

#include <boost/asio.hpp>

#include "hazelcast/client/config/SocketOptions.h"
#include "hazelcast/client/Address.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;

            class AuthenticationFuture;
        }

        namespace spi {
            namespace impl {
                class ClientInvocation;
            }
        }

        class HAZELCAST_API Socket {
        public:
            virtual ~Socket() {
            }

            virtual void
            asyncStart(const std::shared_ptr<connection::Connection> connection,
                       const std::shared_ptr<connection::AuthenticationFuture> authFuture) = 0;

            virtual void
            asyncWrite(const std::shared_ptr<connection::Connection> connection,
                       const std::shared_ptr<spi::impl::ClientInvocation> invocation) = 0;

            virtual void close() = 0;

            virtual Address getAddress() const = 0;

            /**
             *
             * This function is used to obtain the locally bound endpoint of the socket.
             *
             * @returns An address that represents the local endpoint of the socket.
             */
            virtual std::unique_ptr<Address> localSocketAddress() const = 0;

            virtual const Address &getRemoteEndpoint() const = 0;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_SOCKET_H_ */
