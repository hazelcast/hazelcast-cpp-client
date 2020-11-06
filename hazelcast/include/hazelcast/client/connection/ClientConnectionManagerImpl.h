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
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/internal/socket/SocketFactory.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/connection/AddressTranslator.h"
#include "hazelcast/client/connection/ConnectionListenable.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"
#include "hazelcast/client/SocketInterceptor.h"
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
        class LoadBalancer;

        namespace protocol {
            class ClientMessage;
        }

        namespace spi {
            namespace impl {
                class ClientExecutionServiceImpl;
            }
        }

        namespace impl {
            class HazelcastClientInstanceImpl;
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
                                            const std::shared_ptr<AddressTranslator> &addressTranslator,
                                            const std::vector<std::shared_ptr<AddressProvider> > &addressProviders);

                virtual ~ClientConnectionManagerImpl();

                bool start();

                void shutdown();

                /**
                 * @param address to be connected
                 * @return associated connection if available, creates new connection otherwise
                 * @throws IOException if connection is not established
                 */
                std::shared_ptr<Connection> getOrConnect(const Address &address);

                std::vector<std::shared_ptr<Connection>> getActiveConnections();

                std::shared_ptr<Connection> getConnection(boost::uuids::uuid uuid);

                bool isAlive();

                void on_connection_close(Connection &connection, std::exception_ptr ptr);

                void addConnectionListener(const std::shared_ptr<ConnectionListener> &connectionListener) override;

                logger &getLogger();

                std::shared_ptr<Connection> get_random_connection();

                boost::uuids::uuid getClientUuid() const;

                void check_invocation_allowed();

                void connect_to_all_cluster_members();

                void notify_backup(int64_t call_id);

                std::shared_ptr<LoadBalancer> getLoadBalancer();

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
                    boost::optional<Address> address;
                    std::string server_version;
                };

                std::shared_ptr<Connection> getConnection(const Address &address);

                void authenticate_on_cluster(std::shared_ptr<Connection> &connection);

                void fireConnectionAddedEvent(const std::shared_ptr<Connection> &connection);

                void fireConnectionRemovedEvent(const std::shared_ptr<Connection> &connection);

                void submit_connect_to_cluster_task();

                void connectToCluster();

                void connect_to_all_members();

                static void
                shutdownWithExternalThread(const std::weak_ptr<client::impl::HazelcastClientInstanceImpl>& clientImpl);

                bool do_connect_to_cluster();

                std::vector<Address> getPossibleMemberAddresses();

                template<typename Container>
                void shuffle(Container &memberAddresses) const {
                    if (memberAddresses.empty()) {
                        return;
                    }
                    std::random_shuffle(memberAddresses.begin(), memberAddresses.end());
                }

                void check_client_active();

                std::shared_ptr<Connection> connect(const Address &address);

                protocol::ClientMessage
                encodeAuthenticationRequest(serialization::pimpl::SerializationService &ss);

                void handleSuccessfulAuth(const std::shared_ptr<Connection> &connection, auth_response response);

                std::atomic_bool alive;
                logger &logger_;
                std::chrono::milliseconds connectionTimeoutMillis;
                spi::ClientContext &client;
                std::unique_ptr<boost::asio::io_context> ioContext;
                SocketInterceptor socketInterceptor;
                spi::impl::ClientExecutionServiceImpl &executionService;
                std::shared_ptr<AddressTranslator> translator;
                util::SynchronizedMap<boost::uuids::uuid, Connection, boost::hash<boost::uuids::uuid>> activeConnections;
                util::SynchronizedMap<int32_t, Connection> active_connection_ids_;
                util::sync_associative_container<std::unordered_map<Address, std::unique_ptr<std::mutex>>> conn_locks_;
                // TODO: change with CopyOnWriteArraySet<ConnectionListener> as in Java
                util::ConcurrentSet<std::shared_ptr<ConnectionListener> > connectionListeners;
                std::unique_ptr<hazelcast::util::hz_thread_pool> executor_;
                std::chrono::milliseconds connectionAttemptPeriod;
                int32_t connectionAttemptLimit;
                int32_t ioThreadCount;
                bool shuffleMemberList;
                std::vector<std::shared_ptr<AddressProvider> > addressProviders;
                std::atomic<int32_t> connectionIdGen;
                std::unique_ptr<boost::asio::ip::tcp::resolver> ioResolver;
                std::unique_ptr<internal::socket::SocketFactory> socketFactory;
                HeartbeatManager heartbeat;
                std::vector<std::thread> ioThreads;
                std::unique_ptr<boost::asio::io_context::work> ioGuard;
                std::atomic<int32_t> partition_count_;
                const bool async_start_;
                const config::ClientConnectionStrategyConfig::ReconnectMode reconnect_mode_;
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
                std::shared_ptr<LoadBalancer> load_balancer_;

                void schedule_connect_to_all_members();

                void fire_life_cycle_event(LifecycleEvent::LifecycleState state);

                void check_partition_count(int32_t new_partition_count);

                void trigger_cluster_reconnection();
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



