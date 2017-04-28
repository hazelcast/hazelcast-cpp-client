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
#ifndef HAZELCAST_CLIENT_INTERNAL_SOCKET_SOCKETFACTORY_H_
#define HAZELCAST_CLIENT_INTERNAL_SOCKET_SOCKETFACTORY_H_

#include <memory>

#include "hazelcast/util/HazelcastDll.h"

#ifdef HZ_BUILD_WITH_SSL
#include <asio.hpp>
#include <asio/ssl.hpp>
#endif // HZ_BUILD_WITH_SSL

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class Socket;
        class Address;

        namespace spi {
            class ClientContext;
        }
        namespace internal {
            namespace socket {
                class HAZELCAST_API SocketFactory {
                public:
                    SocketFactory(spi::ClientContext &clientContext);

                    bool start();

                    std::auto_ptr<Socket> create(const Address &address) const;
                private:
                    spi::ClientContext &clientContext;

                    #ifdef HZ_BUILD_WITH_SSL
                    std::auto_ptr<asio::io_service> ioService;
                    std::auto_ptr<asio::ssl::context> sslContext;
                    #endif
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_SOCKET_SOCKETFACTORY_H_ */
