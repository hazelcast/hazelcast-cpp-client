/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <ostream>
#include <stdint.h>
#include <atomic>
#include <unordered_map>
#include <boost/asio.hpp>

#include "hazelcast/client/socket.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/Closeable.h"
#include "hazelcast/client/protocol/ClientMessageBuilder.h"
#include "hazelcast/client/protocol/IMessageHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/logger.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#pragma warning(disable : 4003) // for  not enough actual parameters for macro
                                // 'min' in asio wait_traits
#endif

namespace hazelcast {
namespace client {
namespace exception {
class iexception;
}
namespace spi {
class ClientContext;
}

namespace internal {
namespace socket {
class SocketFactory;
}
} // namespace internal

class address;

class socket_interceptor;

namespace connection {
class ClientConnectionManagerImpl;

class HAZELCAST_API Connection
  : public util::Closeable
  , public std::enable_shared_from_this<Connection>
{
public:
    Connection(const address& address,
               spi::ClientContext& client_context,
               int32_t connection_id,
               internal::socket::SocketFactory& socket_factory,
               ClientConnectionManagerImpl& client_connection_manager,
               std::chrono::milliseconds& connect_timeout_in_millis);

    ~Connection() override;

    void connect();

    void close();

    void close(const std::string& reason) override;

    void close(const std::string& reason, std::exception_ptr cause);

    void write(
      const std::shared_ptr<spi::impl::ClientInvocation>& client_invocation);

    const util::optional<address>& get_remote_address() const;

    void set_remote_address(util::optional<address> endpoint);

    boost::uuids::uuid get_remote_uuid() const;

    void set_remote_uuid(boost::uuids::uuid remote_uuid);

    void handle_client_message(
      const std::shared_ptr<protocol::ClientMessage>& message);

    int get_connection_id() const;

    bool is_alive() const;

    std::chrono::steady_clock::time_point last_read_time() const;

    const std::string& get_close_reason() const;

    bool operator==(const Connection& rhs) const;

    bool operator!=(const Connection& rhs) const;

    bool operator<(const Connection& rhs) const;

    const std::string& get_connected_server_version_string() const;

    void set_connected_server_version(const std::string& connected_server);

    util::optional<address> get_local_socket_address() const;

    std::chrono::system_clock::time_point get_start_time() const;

    socket& get_socket();

    void deregister_invocation(int64_t call_id);

    void last_write_time(std::chrono::steady_clock::time_point tp);

    std::chrono::steady_clock::time_point last_write_time() const;

    friend std::ostream& operator<<(std::ostream& os,
                                    const Connection& connection);

    ReadHandler read_handler;
    std::unordered_map<int64_t, std::shared_ptr<spi::impl::ClientInvocation>>
      invocations;

private:
    void log_close();

    void inner_close();

    std::chrono::system_clock::time_point start_time_;
    std::atomic<std::chrono::milliseconds> closed_time_duration_;
    spi::ClientContext& client_context_;
    protocol::IMessageHandler& invocation_service_;
    std::unique_ptr<socket> socket_;
    int32_t connection_id_;
    std::string close_reason_;
    std::exception_ptr close_cause_;
    std::string connected_server_version_string_;
    // TODO: check if they need to be atomic
    util::optional<address> remote_address_;
    boost::uuids::uuid remote_uuid_;
    logger& logger_;
    std::atomic_bool alive_;
    std::unique_ptr<boost::asio::steady_timer> backup_timer_;
    std::atomic<std::chrono::steady_clock::duration> last_write_time_;

    void schedule_periodic_backup_cleanup(
      std::chrono::milliseconds backup_timeout,
      std::shared_ptr<Connection> this_connection);
};
} // namespace connection
} // namespace client
} // namespace hazelcast

namespace std {
template<>
struct hash<std::shared_ptr<hazelcast::client::connection::Connection>>
{
    std::size_t operator()(
      const std::shared_ptr<hazelcast::client::connection::Connection>& conn)
      const noexcept;
};
} // namespace std

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
