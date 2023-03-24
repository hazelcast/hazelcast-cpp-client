/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/asio.hpp>

#include "hazelcast/client/config/socket_options.h"
#include "hazelcast/client/address.h"
#include "hazelcast/client/connection/ReadHandler.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace connection {
class Connection;
}

namespace spi {
namespace impl {
class ClientInvocation;
}
} // namespace spi

class HAZELCAST_API socket
{
public:
    virtual ~socket() = default;

    virtual void connect(
      const std::shared_ptr<connection::Connection> connection) = 0;

    virtual bool write(std::vector<byte> buffer) = 0;

    virtual void async_write(
      const std::shared_ptr<connection::Connection> connection,
      const std::shared_ptr<spi::impl::ClientInvocation> invocation) = 0;

    virtual void close() = 0;

    virtual address get_address() const = 0;

    virtual std::chrono::steady_clock::time_point last_read_time() const = 0;

    virtual std::chrono::steady_clock::time_point last_write_time() const = 0;

    virtual bool is_closed() const = 0;

    /**
     *
     * This function is used to obtain the locally bound endpoint of the socket.
     *
     * @returns An address that represents the local endpoint of the socket.
     */
    virtual boost::optional<address> local_socket_address() const = 0;

    virtual const address& get_remote_endpoint() const = 0;
};

std::ostream HAZELCAST_API&
operator<<(std::ostream& os, const socket&);
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
