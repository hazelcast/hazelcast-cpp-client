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

#include <atomic>
#include <memory>
#include <random>
#include <thread>
#include <future>
#include <vector>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>

#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/ConcurrentSet.h"
#include "hazelcast/client/lifecycle_event.h"
#include "hazelcast/client/address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/internal/socket/SocketFactory.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/connection/ConnectionListenable.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/config/client_connection_strategy_config.h"
#include "hazelcast/client/socket_interceptor.h"
#include "hazelcast/logger.h"
#include "hazelcast/client/connection/wait_strategy.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#pragma warning(disable : 4003) // for  not enough actual parameters for macro
                                // 'min' in asio wait_traits
#endif

namespace hazelcast {
namespace util {
class hz_thread_pool;
}
namespace client {
class Credentials;
class load_balancer;

namespace protocol {
class ClientMessage;
}

namespace spi {
namespace impl {
class ClientExecutionServiceImpl;
}
} // namespace spi

namespace impl {
class hazelcast_client_instance_impl;
}

namespace security {
class credentials;
}

namespace connection {
class Connection;

class ConnectionListener;

class AddressProvider;

/**
 * Responsible for managing {@link Connection} objects.
 */
class HAZELCAST_API ClientConnectionManagerImpl
  : public ConnectionListenable
  , public std::enable_shared_from_this<ClientConnectionManagerImpl>
{
public:
    ClientConnectionManagerImpl(
      spi::ClientContext& client,
      std::unique_ptr<AddressProvider> address_provider);

    virtual ~ClientConnectionManagerImpl();

    bool start();

    void shutdown();

    /**
     * @param m member to be connected
     * @return associated connection if available, creates new connection
     * otherwise
     * @throws io if connection is not established
     */
    std::shared_ptr<Connection> get_or_connect(const member& m);

    std::vector<std::shared_ptr<Connection>> get_active_connections();

    std::shared_ptr<Connection> get_connection(boost::uuids::uuid uuid);

    bool is_alive();

    void on_connection_close(const std::shared_ptr<Connection>& connection);

    void add_connection_listener(
      const std::shared_ptr<ConnectionListener>& connection_listener);

    logger& get_logger();

    std::shared_ptr<Connection> get_random_connection();

    /**
     * Return:<ol>
     *     <li>a random connection to a data member from the larger same-version
     *         group
     *     <li>if there's no such connection, return connection to a random data
     *         member
     *     <li>if there's no such connection, return any random connection
     * </ol>
     */
    std::shared_ptr<connection::Connection> connection_for_sql(
      std::function<boost::optional<member>()>
        member_of_large_same_version_group,
      std::function<boost::optional<member>(boost::uuids::uuid)>
        get_cluster_member);

    boost::uuids::uuid get_client_uuid() const;

    /**
     * Check the connected state and user connection strategy configuration to see if an invocation is allowed at the moment
     * returns without throwing exception only when is the client is Connected to cluster
     *
     * @throws io_exception                    if client is disconnected and ReconnectMode is ON or
     *                                         if client is starting and async start is false
     * @throws hazelcast_client_offline        if client is disconnected and ReconnectMode is ASYNC or
     *                                         if client is starting and async start is true
     */
    void check_invocation_allowed();

    /**
     * Returns {@code true} if the client is initialized on the cluster, by
     * sending its local state, if necessary.
     */
    bool client_initialized_on_cluster() const;

    void connect_to_all_cluster_members();

    void notify_backup(int64_t call_id);

private:
    static constexpr size_t EXECUTOR_CORE_POOL_SIZE = 10;
    static constexpr int32_t CLIENT = 1;
    static const endpoint_qualifier PUBLIC_ENDPOINT_QUALIFIER;
    static constexpr int SQL_CONNECTION_RANDOM_ATTEMPTS = 10;

    struct auth_response
    {
        byte status;
        boost::uuids::uuid member_uuid;
        byte serialization_version;
        int32_t partition_count;
        boost::uuids::uuid cluster_id;
        boost::optional<address> server_address;
        std::string server_version;
    };

    enum class client_state
    {
        /**
         * Clients start with this state. Once a client connects to a cluster,
         * it directly switches to {@link #INITIALIZED_ON_CLUSTER} instead of
         * {@link #CONNECTED_TO_CLUSTER} because on startup a client has no
         * local state to send to the cluster.
         */
        INITIAL,

        /**
         * When a client switches to a new cluster, it moves to this state.
         * It means that the client has connected to a new cluster but not sent
         * its local state to the new cluster yet.
         */
        CONNECTED_TO_CLUSTER,

        /**
         * When a client sends its local state to the cluster it has connected,
         * it switches to this state.
         * <p>
         * Invocations are allowed in this state.
         */
        INITIALIZED_ON_CLUSTER,

        /**
         * When the client closes the last connection to the cluster it
         * currently connected to, it switches to this state.
         * <p>
         */
        DISCONNECTED_FROM_CLUSTER,
    };

    friend std::ostream HAZELCAST_API& operator<<(std::ostream& os,
                                                  client_state);

    auth_response authenticate_on_cluster(
      std::shared_ptr<Connection>& connection);

    void fire_connection_added_event(
      const std::shared_ptr<Connection>& connection);

    void fire_connection_removed_event(
      const std::shared_ptr<Connection>& connection);

    void submit_connect_to_cluster_task();

    void connect_to_cluster();

    void connect_to_all_members();

    static void shutdown_with_external_thread(
      std::weak_ptr<client::impl::hazelcast_client_instance_impl> client_impl);

    void do_connect_to_cluster();

    std::vector<address> get_possible_member_addresses();

    template<typename Container>
    void shuffle(Container& member_addresses) const
    {
        if (member_addresses.empty()) {
            return;
        }

        static thread_local std::random_device rd;
        static thread_local std::mt19937 gen(rd());

        std::shuffle(member_addresses.begin(), member_addresses.end(), gen);
    }

    void check_client_active();

    void initialize_client_on_cluster(boost::uuids::uuid);

    template<typename T>
    std::shared_ptr<Connection> try_connect(const T& target)
    {
        try {
            return get_or_connect(target);
        } catch (std::exception& e) {
            HZ_LOG(
              logger_,
              warning,
              boost::str(boost::format(
                           "Exception during initial connection to %1%: %2%") %
                         target % e.what()));
            return nullptr;
        }
    }

    protocol::ClientMessage encode_authentication_request(
      serialization::pimpl::SerializationService& ss);

    /**
     * The returned connection could be different than the one passed to this
     * method if there is already an existing connection to the given member.
     */
    std::shared_ptr<Connection> on_authenticated(
      const std::shared_ptr<Connection>& connection,
      auth_response& response);

    std::atomic_bool alive_;
    logger& logger_;
    std::chrono::milliseconds connection_timeout_millis_;
    spi::ClientContext& client_;
    std::unique_ptr<boost::asio::io_context> io_context_;
    socket_interceptor socket_interceptor_;
    util::SynchronizedMap<member, bool> connecting_members_;
    // TODO: change with CopyOnWriteArraySet<ConnectionListener> as in Java
    util::ConcurrentSet<std::shared_ptr<ConnectionListener>>
      connection_listeners_;
    std::unique_ptr<hazelcast::util::hz_thread_pool> executor_;
    bool shuffle_member_list_;
    std::unique_ptr<AddressProvider> address_provider_;
    std::atomic<int32_t> connection_id_gen_;
    std::unique_ptr<boost::asio::ip::tcp::resolver> io_resolver_;
    std::unique_ptr<internal::socket::SocketFactory> socket_factory_;
    HeartbeatManager heartbeat_;
    std::thread io_thread_;
    std::unique_ptr<boost::asio::io_context::work> io_guard_;
    const bool async_start_;
    const config::client_connection_strategy_config::reconnect_mode
      reconnect_mode_;
    const bool smart_routing_enabled_;
    boost::optional<boost::asio::steady_timer> connect_to_members_timer_;
    boost::uuids::uuid client_uuid_;
    boost::chrono::milliseconds authentication_timeout_;
    std::vector<std::string> labels_;
    load_balancer& load_balancer_;
    wait_strategy wait_strategy_;

    // following fields are updated inside synchronized(clientStateMutex)
    mutable std::recursive_mutex client_state_mutex_;
    util::SynchronizedMap<boost::uuids::uuid,
                          Connection,
                          boost::hash<boost::uuids::uuid>>
      active_connections_;
    util::SynchronizedMap<int32_t, Connection> active_connection_ids_;
#ifdef __linux__
    // default support for 16 byte atomics is missing for linux
    util::Sync<boost::uuids::uuid> cluster_id_;
#else
    std::atomic<boost::uuids::uuid> cluster_id_;
#endif
    client_state client_state_;
    std::atomic_bool connect_to_cluster_task_submitted_;
    bool established_initial_cluster_connection;

    bool use_public_address_{ false };

    void schedule_connect_to_all_members();

    void fire_life_cycle_event(lifecycle_event::lifecycle_state state);

    void check_partition_count(int32_t new_partition_count);

    void trigger_cluster_reconnection();

    std::shared_ptr<Connection> connect(const address& address);

    address translate(const member& m);
};

} // namespace connection
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
