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
#pragma once

#include <atomic>
#include <memory>
#include <random>
#include <thread>
#include <future>
#include <vector>
#include <boost/asio.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>

#include <hazelcast/util/sync_associative_container.h>
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/ConcurrentSet.h"
#include "hazelcast/client/lifecycle_event.h"
#include "hazelcast/client/address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/internal/socket/SocketFactory.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/connection/AddressTranslator.h"
#include "hazelcast/client/connection/ConnectionListenable.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/config/client_connection_strategy_config.h"
#include "hazelcast/client/socket_interceptor.h"
#include "hazelcast/logger.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#pragma warning(disable: 4003) //for  not enough actual parameters for macro 'min' in asio wait_traits
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
        }

        namespace impl {
            class hazelcast_client_instance_impl;
        }

        namespace security {
            class credentials;
        }

        namespace connection {
            class Connection;

            class ConnectionListener;

            class ConnectionFuture;

            class AddressProvider;

            /**
            * Responsible for managing {@link Connection} objects.
            */
            class HAZELCAST_API ClientConnectionManagerImpl
                    : public ConnectionListenable, public std::enable_shared_from_this<ClientConnectionManagerImpl> {
            public:
                ClientConnectionManagerImpl(spi::ClientContext &client,
                                            const std::shared_ptr<AddressTranslator> &address_translator,
                                            const std::vector<std::shared_ptr<AddressProvider> > &address_providers);

                virtual ~ClientConnectionManagerImpl();

                bool start();

                void shutdown();

                /**
                 * @param address to be connected
                 * @return associated connection if available, creates new connection otherwise
                 * @throws io if connection is not established
                 */
                std::shared_ptr<Connection> get_or_connect(const address &address);

                std::vector<std::shared_ptr<Connection>> get_active_connections();

                std::shared_ptr<Connection> get_connection(boost::uuids::uuid uuid);

                bool is_alive();

                void on_connection_close(Connection &connection, std::exception_ptr ptr);

                void add_connection_listener(const std::shared_ptr<ConnectionListener> &connection_listener) ;

                logger &get_logger();

                std::shared_ptr<Connection> get_random_connection();

                boost::uuids::uuid get_client_uuid() const;

                void check_invocation_allowed();

                void connect_to_all_cluster_members();

                void notify_backup(int64_t call_id);

            private:
                static constexpr size_t EXECUTOR_CORE_POOL_SIZE = 10;
                static constexpr int32_t DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC = 2;
                static constexpr int32_t DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC = 20;

                struct auth_response {
                    byte status;
                    boost::uuids::uuid member_uuid;
                    byte serialization_version;
                    int32_t partition_count;
                    boost::uuids::uuid cluster_id;
                    boost::optional<address> server_address;
                    std::string server_version;
                };

                std::shared_ptr<Connection> get_connection(const address &address);

                void authenticate_on_cluster(std::shared_ptr<Connection> &connection);

                void fire_connection_added_event(const std::shared_ptr<Connection> &connection);

                void fire_connection_removed_event(const std::shared_ptr<Connection> &connection);

                void submit_connect_to_cluster_task();

                void connect_to_cluster();

                void connect_to_all_members();

                static void
                shutdown_with_external_thread(const std::weak_ptr<client::impl::hazelcast_client_instance_impl>& client_impl);

                bool do_connect_to_cluster();

                std::vector<address> get_possible_member_addresses();

                template<typename Container>
                void shuffle(Container &member_addresses) const {
                    if (member_addresses.empty()) {
                        return;
                    }
                    std::random_shuffle(member_addresses.begin(), member_addresses.end());
                }

                void check_client_active();

                std::shared_ptr<Connection> connect(const address &address);

                protocol::ClientMessage
                encode_authentication_request(serialization::pimpl::SerializationService &ss);

                void handle_successful_auth(const std::shared_ptr<Connection> &connection, auth_response response);

                std::atomic_bool alive_;
                logger &logger_;
                std::chrono::milliseconds connection_timeout_millis_;
                spi::ClientContext &client_;
                std::unique_ptr<boost::asio::io_context> io_context_;
                socket_interceptor socket_interceptor_;
                spi::impl::ClientExecutionServiceImpl &execution_service_;
                std::shared_ptr<AddressTranslator> translator_;
                util::SynchronizedMap<boost::uuids::uuid, Connection, boost::hash<boost::uuids::uuid>> active_connections_;
                util::SynchronizedMap<int32_t, Connection> active_connection_ids_;
                util::sync_associative_container<std::unordered_map<address, std::unique_ptr<std::mutex>>> conn_locks_;
                // TODO: change with CopyOnWriteArraySet<ConnectionListener> as in Java
                util::ConcurrentSet<std::shared_ptr<ConnectionListener> > connection_listeners_;
                std::unique_ptr<hazelcast::util::hz_thread_pool> executor_;
                std::chrono::milliseconds connection_attempt_period_;
                int32_t connection_attempt_limit_;
                int32_t io_thread_count_;
                bool shuffle_member_list_;
                std::vector<std::shared_ptr<AddressProvider> > address_providers_;
                std::atomic<int32_t> connection_id_gen_;
                std::unique_ptr<boost::asio::ip::tcp::resolver> io_resolver_;
                std::unique_ptr<internal::socket::SocketFactory> socket_factory_;
                HeartbeatManager heartbeat_;
                std::vector<std::thread> io_threads_;
                std::unique_ptr<boost::asio::io_context::work> io_guard_;
                std::atomic<int32_t> partition_count_;
                const bool async_start_;
                const config::client_connection_strategy_config::reconnect_mode reconnect_mode_;
                const bool smart_routing_enabled_;
                boost::optional<boost::asio::steady_timer> connect_to_members_timer_;
                std::atomic_bool connect_to_cluster_task_submitted_;
                boost::uuids::uuid client_uuid_;
                boost::chrono::milliseconds authentication_timeout_;
#ifdef __linux__
                // default support for 16 byte atomics is missing for linux
                util::Sync<boost::uuids::uuid> cluster_id_;
#else
                std::atomic<boost::uuids::uuid> cluster_id_;
#endif
                std::vector<std::string> labels_;
                load_balancer *load_balancer_;

                void schedule_connect_to_all_members();

                void fire_life_cycle_event(lifecycle_event::lifecycle_state state);

                void check_partition_count(int32_t new_partition_count);

                void trigger_cluster_reconnection();
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



