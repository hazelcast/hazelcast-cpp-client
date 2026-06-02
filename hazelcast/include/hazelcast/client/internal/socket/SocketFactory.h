/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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
// See BaseSocket.h: <boost/asio/ssl.hpp> in Boost <= 1.83 transitively
// pulls in rfc2818_verification, which fails to compile against
// OpenSSL 3.x (opaque ASN1_STRING). Use specific subheaders instead.
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/asio/ssl/stream.hpp>

#endif // HZ_BUILD_WITH_SSL

namespace hazelcast {
namespace client {
class socket;

class address;

namespace spi {
class ClientContext;
}
namespace internal {
namespace socket {
class HAZELCAST_API SocketFactory
{
public:
    SocketFactory(spi::ClientContext& client_context);

    bool start();

    std::unique_ptr<hazelcast::client::socket> create(
      const address& address,
      std::chrono::milliseconds& connect_timeout_in_millis,
      boost::asio::io_context& io,
      boost::asio::ip::tcp::resolver& resolver);

private:
    spi::ClientContext& client_context_;
#ifdef HZ_BUILD_WITH_SSL
    std::shared_ptr<boost::asio::ssl::context> ssl_context_;
#endif
};
} // namespace socket
} // namespace internal
} // namespace client
} // namespace hazelcast
