/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include <memory>

#include "hazelcast/util/export.h"

#ifdef HZ_BUILD_WITH_SSL

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#endif // HZ_BUILD_WITH_SSL

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class socket;

        class address;

        namespace spi {
            class ClientContext;
        }
        namespace internal {
            namespace socket {
                class HAZELCAST_API SocketFactory {
                public:
                    SocketFactory(spi::ClientContext &client_context, boost::asio::io_context &io,
                            boost::asio::ip::tcp::resolver &resolver);

                    bool start();

                    std::unique_ptr<hazelcast::client::socket>
                    create(const address &address, std::chrono::milliseconds &connect_timeout_in_millis);

                private:
                    spi::ClientContext &client_context_;
                    boost::asio::io_context &io_;
                    boost::asio::ip::tcp::resolver &io_resolver_;
                    std::shared_ptr<boost::asio::ssl::context> ssl_context_;
#ifdef HZ_BUILD_WITH_SSL
#endif
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
