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

#include <cstdlib>
#include <unordered_set>

#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/connection/AddressProvider.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/protocol/AuthenticationStatus.h"
#include "hazelcast/client/exception/AuthenticationException.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/spi/impl/listener/listener_service_impl.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/config/SSLConfig.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/sync_associative_container.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            constexpr size_t ClientConnectionManagerImpl::EXECUTOR_CORE_POOL_SIZE;
            constexpr int32_t ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
            constexpr int32_t ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC;

            ClientConnectionManagerImpl::ClientConnectionManagerImpl(spi::ClientContext &client,
                                                                     const std::shared_ptr<AddressTranslator> &addressTranslator,
                                                                     const std::vector<std::shared_ptr<AddressProvider> > &addressProviders)
                    : alive_(false), logger_(client.get_logger()), connectionTimeoutMillis_(std::chrono::milliseconds::max()),
                      client_(client),
                      socketInterceptor_(client.get_client_config().get_socket_interceptor()),
                      executionService_(client.get_client_execution_service()),
                      translator_(addressTranslator), connectionIdGen_(0),
                      heartbeat_(client, *this), partition_count_(-1),
                      async_start_(client.get_client_config().get_connection_strategy_config().is_async_start()),
                      reconnect_mode_(client.get_client_config().get_connection_strategy_config().get_reconnect_mode()),
                      smart_routing_enabled_(client.get_client_config().get_network_config().is_smart_routing()),
                      connect_to_cluster_task_submitted_(false),
                      client_uuid_(client.random_uuid()),
                      authentication_timeout_(boost::chrono::milliseconds(heartbeat_.get_heartbeat_timeout().count())),
                      cluster_id_(boost::uuids::nil_uuid()),
                      load_balancer_(client.get_client_config().get_load_balancer()) {
                config::ClientNetworkConfig &networkConfig = client.get_client_config().get_network_config();
                auto connTimeout = networkConfig.get_connection_timeout();
                if (connTimeout.count() > 0) {
                    connectionTimeoutMillis_ = std::chrono::milliseconds(connTimeout);
                }

                ClientProperties &clientProperties = client.get_client_properties();
                shuffleMemberList_ = clientProperties.get_boolean(clientProperties.get_shuffle_member_list());

                ClientConnectionManagerImpl::addressProviders_ = addressProviders;

                connectionAttemptPeriod_ = networkConfig.get_connection_attempt_period();

                int connAttemptLimit = networkConfig.get_connection_attempt_limit();
                bool isAsync = client.get_client_config().get_connection_strategy_config().is_async_start();

                if (connAttemptLimit < 0) {
                    this->connectionAttemptLimit_ = isAsync ? DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC
                                                           : DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
                } else {
                    this->connectionAttemptLimit_ = connAttemptLimit == 0 ? INT32_MAX : connAttemptLimit;
                }

                ioThreadCount_ = clientProperties.get_integer(clientProperties.get_io_thread_count());
            }

            bool ClientConnectionManagerImpl::start() {
                bool expected = false;
                if (!alive_.compare_exchange_strong(expected, true)) {
                    return false;
                }

                ioContext_.reset(new boost::asio::io_context);
                ioResolver_.reset(new boost::asio::ip::tcp::resolver(ioContext_->get_executor()));
                socketFactory_.reset(new internal::socket::SocketFactory(client_, *ioContext_, *ioResolver_));
                ioGuard_.reset(new boost::asio::io_context::work(*ioContext_));

                if (!socketFactory_->start()) {
                    return false;
                }

                socketInterceptor_ = client_.get_client_config().get_socket_interceptor();

                for (int j = 0; j < ioThreadCount_; ++j) {
                    ioThreads_.emplace_back([=]() { ioContext_->run(); });
                }

                executor_.reset(new hazelcast::util::hz_thread_pool(EXECUTOR_CORE_POOL_SIZE));
                connect_to_members_timer_ = boost::asio::steady_timer(executor_->get_executor());

                heartbeat_.start();

                connect_to_cluster();
                if (smart_routing_enabled_) {
                    schedule_connect_to_all_members();
                }

                return true;
            }

            void ClientConnectionManagerImpl::schedule_connect_to_all_members() {
                connect_to_members_timer_->expires_from_now(boost::asio::chrono::seconds(1));
                connect_to_members_timer_->async_wait([=](boost::system::error_code ec) {
                    if (ec == boost::asio::error::operation_aborted) {
                        return;
                    }
                    connect_to_all_members();

                    if (!client_.get_lifecycle_service().is_running()) {
                        return;
                    }

                    schedule_connect_to_all_members();
                });
            }

            void ClientConnectionManagerImpl::shutdown() {
                bool expected = true;
                if (!alive_.compare_exchange_strong(expected, false)) {
                    return;
                }

                if (connect_to_members_timer_) {
                    connect_to_members_timer_->cancel();
                }

                heartbeat_.shutdown();

                // close connections
                for (auto &connection : activeConnections_.values()) {
                    // prevent any exceptions
                    util::IOUtil::close_resource(connection.get(), "Hazelcast client is shutting down");
                }

                spi::impl::ClientExecutionServiceImpl::shutdown_thread_pool(executor_.get());

                ioGuard_.reset();
                ioContext_->stop();
                boost::asio::use_service<boost::asio::detail::resolver_service<boost::asio::ip::tcp>>(*ioContext_).shutdown();
                std::for_each(ioThreads_.begin(), ioThreads_.end(), [](std::thread &t) { t.join(); });

                connectionListeners_.clear();
                activeConnections_.clear();
                active_connection_ids_.clear();
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::get_or_connect(const Address &address) {
                auto connection = get_connection(address);
                if (connection) {
                    return connection;
                }

                auto f = conn_locks_.emplace(address, std::unique_ptr<std::mutex>(new std::mutex()));
                std::lock_guard<std::mutex> g(*f.first->second);
                // double check here
                connection = get_connection(address);
                if (connection) {
                    return connection;
                }

                auto target = translator_->translate(address);
                connection = std::make_shared<Connection>(target, client_, ++connectionIdGen_,
                                                          *socketFactory_, *this, connectionTimeoutMillis_);
                connection->connect();

                // call the interceptor from user thread
                socketInterceptor_.connect_(connection->get_socket());

                authenticate_on_cluster(connection);

                return connection;
            }

            std::vector<std::shared_ptr<Connection> > ClientConnectionManagerImpl::get_active_connections() {
                return activeConnections_.values();
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::get_connection(const Address &address) {
                for (const auto &connection : activeConnections_.values()) {
                    auto remote_address = connection->get_remote_address();
                    if (remote_address && *remote_address == address) {
                        return connection;
                    }
                }
                return nullptr;
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::get_connection(boost::uuids::uuid uuid) {
                return activeConnections_.get(uuid);
            }

            void
            ClientConnectionManagerImpl::authenticate_on_cluster(std::shared_ptr<Connection> &connection) {
                auto request = encode_authentication_request(client_.get_serialization_service());
                auto clientInvocation = spi::impl::ClientInvocation::create(client_, request, "", connection);
                auto f = clientInvocation->invoke_urgent();

                struct auth_response result;
                try {
                    if (f.wait_for(authentication_timeout_) != boost::future_status::ready) {
                        BOOST_THROW_EXCEPTION(exception::TimeoutException(
                                "ClientConnectionManagerImpl::authenticate", (boost::format("Authentication response is "
                                "not received for %1% msecs for %2%") %authentication_timeout_.count() %*clientInvocation).str()));
                    }
                    auto response = f.get();
                    auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(response.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                    result = {
                            response.get<byte>(), response.get<boost::uuids::uuid>(),
                            response.get<byte>(), response.get<int32_t>(),
                            response.get<boost::uuids::uuid>()
                    };
                    // skip first frame
                    response.rd_ptr(static_cast<int32_t>(initial_frame->frame_len) - ClientMessage::RESPONSE_HEADER_LEN - 2 * ClientMessage::UINT8_SIZE -
                                    2 * (sizeof(boost::uuids::uuid) + ClientMessage::UINT8_SIZE) - ClientMessage::INT32_SIZE);

                    result.address = response.get_nullable<Address>();
                    result.server_version = response.get<std::string>();
                } catch (exception::IException &) {
                    connection->close("Failed to authenticate connection", std::current_exception());
                    throw;
                }

                auto authentication_status = (protocol::AuthenticationStatus) result.status;
                switch (authentication_status) {
                    case protocol::AUTHENTICATED: {
                        handle_successful_auth(connection, std::move(result));
                        break;
                    }
                    case protocol::CREDENTIALS_FAILED: {
                        auto e = std::make_exception_ptr(exception::AuthenticationException("AuthCallback::onResponse",
                                                                                            "Authentication failed. The configured cluster name on the client (see ClientConfig::setClusterName()) does not match the one configured in the cluster or the credentials set in the Client security config could not be authenticated"));
                        connection->close("Failed to authenticate connection", e);
                        std::rethrow_exception(e);
                    }
                    default: {
                        auto e = std::make_exception_ptr(exception::AuthenticationException(
                                "AuthCallback::onResponse",
                                (boost::format("Authentication status code not supported. status: %1%") %authentication_status).str()));
                        connection->close("Failed to authenticate connection", e);
                        std::rethrow_exception(e);
                    }
                }
            }

            protocol::ClientMessage
            ClientConnectionManagerImpl::encode_authentication_request(serialization::pimpl::SerializationService &ss) {
                byte serializationVersion = ss.get_version();
                ClientConfig &clientConfig = client_.get_client_config();
                auto cluster_name = clientConfig.get_cluster_name();

                auto credential = clientConfig.get_credentials();
                if (!credential) {
                    return protocol::codec::client_authentication_encode(cluster_name, nullptr, nullptr,
                                                                         client_uuid_, protocol::ClientTypes::CPP,
                                                                         serializationVersion, HAZELCAST_VERSION,
                                                                         client_.get_name(), labels_);
                }

                switch(credential->type()) {
                    case security::credentials::credential_type::username_password:
                    {
                        auto cr = std::static_pointer_cast<security::username_password_credentials>(credential);
                        return protocol::codec::client_authentication_encode(cluster_name, &cr->name(), &cr->password(),
                                                                             client_uuid_, protocol::ClientTypes::CPP,
                                                                             serializationVersion, HAZELCAST_VERSION,
                                                                             client_.get_name(), labels_);
                    }
                    case security::credentials::credential_type::token:
                    {
                        auto cr = std::static_pointer_cast<security::token_credentials>(credential);
                        return protocol::codec::client_authenticationcustom_encode(cluster_name, cr->token(),
                                                                                   client_uuid_, protocol::ClientTypes::CPP,
                                                                                   serializationVersion, HAZELCAST_VERSION,
                                                                                   client_.get_name(), labels_);
                    }
                }
                assert(0);
                return protocol::ClientMessage();
            }

            void
            ClientConnectionManagerImpl::fire_connection_added_event(const std::shared_ptr<Connection> &connection) {
                for (const std::shared_ptr<ConnectionListener> &connectionListener : connectionListeners_.to_array()) {
                    connectionListener->connection_added(connection);
                }
            }

            void
            ClientConnectionManagerImpl::fire_connection_removed_event(const std::shared_ptr<Connection> &connection) {
                for (const auto &listener : connectionListeners_.to_array()) {
                    listener->connection_removed(connection);
                }
            }

            void
            ClientConnectionManagerImpl::shutdown_with_external_thread(
                    const std::weak_ptr<client::impl::HazelcastClientInstanceImpl>& clientImpl) {

                std::thread([=] {
                    std::shared_ptr<client::impl::HazelcastClientInstanceImpl> clientInstance = clientImpl.lock();
                    if (!clientInstance || !clientInstance->get_lifecycle_service().is_running()) {
                        return;
                    }

                    try {
                        clientInstance->get_lifecycle_service().shutdown();
                    } catch (exception::IException &e) {
                        HZ_LOG(*clientInstance->get_logger(), severe, 
                            boost::str(boost::format("Exception during client shutdown "
                                                     "%1%.clientShutdown-:%2%")
                                                     % clientInstance->get_name()
                                                     % e)
                        );
                    }
                }).detach();
            }

            void ClientConnectionManagerImpl::submit_connect_to_cluster_task() {
                bool expected = false;
                if (!connect_to_cluster_task_submitted_.compare_exchange_strong(expected, true)) {
                    return;
                }

                boost::asio::post(executor_->get_executor(), [=] () {
                    try {
                        do_connect_to_cluster();
                        connect_to_cluster_task_submitted_ = false;
                        if (activeConnections_.empty()) {
                            HZ_LOG(logger_, finest, 
                                boost::str(boost::format("No connection to cluster: %1%")
                                                         % cluster_id_)
                            );

                            submit_connect_to_cluster_task();
                        }

                    } catch (exception::IException &e) {
                        HZ_LOG(logger_, warning,
                            boost::str(boost::format("Could not connect to any cluster, "
                                                     "shutting down the client: %1%")
                                                     % e)
                        );

                        shutdown_with_external_thread(client_.get_hazelcast_client_implementation());
                    }
                });
            }

            void ClientConnectionManagerImpl::connect_to_all_members() {
                if (!client_.get_lifecycle_service().is_running()) {
                    return;
                }

                auto connecting_addresses = std::make_shared<util::sync_associative_container<std::unordered_set<Address>>>();
                for (const auto &member : client_.get_client_cluster_service().get_member_list()) {
                    const auto& address = member.get_address();

                    if (client_.get_lifecycle_service().is_running() && !get_connection(address)
                        && connecting_addresses->insert(address).second) {
                        // submit a task for this address only if there is no
                        // another connection attempt for it
                        Address addr = address;
                        boost::asio::post(executor_->get_executor(), [=] () {
                            try {
                                if (!client_.get_lifecycle_service().is_running()) {
                                    return;
                                }
                                if (!get_connection(member.get_uuid())) {
                                    get_or_connect(addr);
                                }
                                connecting_addresses->erase(addr);
                            } catch (std::exception &) {
                                connecting_addresses->erase(addr);
                            }
                        });
                    }
                }
            }

            bool ClientConnectionManagerImpl::do_connect_to_cluster() {
                int attempt = 0;
                std::unordered_set<Address> triedAddresses;

                while (attempt < connectionAttemptLimit_) {
                    attempt++;
                    auto nextTryTime = std::chrono::steady_clock::now() + connectionAttemptPeriod_;

                    for (const Address &address : get_possible_member_addresses()) {
                        check_client_active();
                        triedAddresses.insert(address);
                        auto connection = connect(address);
                        if (connection) {
                            return true;
                        }
                    }

                    // If the address providers load no addresses (which seems to be possible), then the above loop is not entered
                    // and the lifecycle check is missing, hence we need to repeat the same check at this point.
                    check_client_active();

                    if (attempt < connectionAttemptLimit_) {
                        auto remainingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                                nextTryTime - std::chrono::steady_clock::now()).count();
                        HZ_LOG(logger_, warning,
                            boost::str(boost::format("Unable to get alive cluster connection, try in "
                                                     "%1% ms later, attempt %2% of %3%.")
                                                     % (remainingTime > 0 ? remainingTime : 0)
                                                     % attempt % connectionAttemptLimit_)
                        );

                        if (remainingTime > 0) {
                            // TODO use a condition variable here
                            std::this_thread::sleep_for(std::chrono::milliseconds(remainingTime));
                        }
                    } else {
                        HZ_LOG(logger_, warning, 
                            boost::str(boost::format("Unable to get alive cluster connection, "
                                                     "attempt %1% of %2%.")
                                                     % attempt % connectionAttemptLimit_)
                        );
                    }
                }
                std::ostringstream out;
                out << "Unable to connect to any address! The following addresses were tried: { ";
                for (const auto &address : triedAddresses) {
                    out << address << " , ";
                }
                out << "}";
                BOOST_THROW_EXCEPTION(
                        exception::IllegalStateException("ConnectionManager::do_connect_to_cluster", out.str()));
            }

            std::vector<Address> ClientConnectionManagerImpl::get_possible_member_addresses() {
                std::vector<Address> addresses;
                for (auto &&member : client_.get_client_cluster_service().get_member_list()) {
                    addresses.emplace_back(std::move(member.get_address()));
                }

                if (shuffleMemberList_) {
                    shuffle(addresses);
                }

                std::vector<Address> provided_addresses;
                for (auto &addressProvider : addressProviders_) {
                    auto addrList = addressProvider->load_addresses();
                    provided_addresses.insert(provided_addresses.end(), addrList.begin(), addrList.end());
                }

                if (shuffleMemberList_) {
                    shuffle(provided_addresses);
                }

                addresses.insert(addresses.end(), provided_addresses.begin(), provided_addresses.end());

                return addresses;
            }

            void ClientConnectionManagerImpl::connect_to_cluster() {
                if (async_start_) {
                    submit_connect_to_cluster_task();
                } else {
                    do_connect_to_cluster();
                }
            }

            bool ClientConnectionManagerImpl::is_alive() {
                return alive_;
            }

            void ClientConnectionManagerImpl::on_connection_close(Connection &connection, std::exception_ptr cause) {
                auto endpoint = connection.get_remote_address();
                auto member_uuid = connection.get_remote_uuid();

                auto socket_remote_address = connection.get_socket().get_remote_endpoint();

                if (!endpoint) {
                    HZ_LOG(logger_, finest,
                        boost::str(boost::format("Destroying %1% , but it has end-point set to null "
                                                 "-> not removing it from a connection map")
                                                 % connection)
                    );
                    return;
                }

                auto conn = connection.shared_from_this();
                if (activeConnections_.remove(member_uuid, conn)) {
                    active_connection_ids_.remove(conn->get_connection_id());

                    HZ_LOG(logger_, info, 
                        boost::str(boost::format("Removed connection to endpoint: %1%, connection: %2%")
                                                 % *endpoint % connection)
                    );

                    if (activeConnections_.empty()) {
                        fire_life_cycle_event(LifecycleEvent::LifecycleState::CLIENT_DISCONNECTED);

                        trigger_cluster_reconnection();
                    }

                    fire_connection_removed_event(connection.shared_from_this());
                } else {
                    HZ_LOG(logger_, finest,
                        boost::str(boost::format("Destroying a connection, but there is no mapping "
                                                 "%1% -> %2% in the connection map.")
                                                 % endpoint % connection)
                    );
                }
            }

            void
            ClientConnectionManagerImpl::add_connection_listener(
                    const std::shared_ptr<ConnectionListener> &connectionListener) {
                connectionListeners_.add(connectionListener);
            }

            ClientConnectionManagerImpl::~ClientConnectionManagerImpl() {
                shutdown();
            }

            logger &ClientConnectionManagerImpl::get_logger() {
                return client_.get_logger();
            }

            void ClientConnectionManagerImpl::check_client_active() {
                if (!client_.get_lifecycle_service().is_running()) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastClientNotActiveException(
                            "ClientConnectionManagerImpl::check_client_active", "Client is shutdown"));
                }
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::connect(const Address &address) {
                try {
                    HZ_LOG(logger_, info, 
                        boost::str(boost::format("Trying to connect to %1%") % address));
                    return get_or_connect(address);
                } catch (std::exception &e) {
                    HZ_LOG(logger_, warning,
                        boost::str(boost::format("Exception during initial connection to %1%: %2%")
                                                 % address % e.what()));
                    return nullptr;
                }
            }

            void ClientConnectionManagerImpl::handle_successful_auth(const std::shared_ptr<Connection> &connection,
                                                                   auth_response response) {
                check_partition_count(response.partition_count);
                connection->set_connected_server_version(response.server_version);
                connection->set_remote_address(std::move(response.address));
                connection->set_remote_uuid(response.member_uuid);

                auto new_cluster_id = response.cluster_id;
                boost::uuids::uuid current_cluster_id = cluster_id_;

                HZ_LOG(logger_, finest, 
                    boost::str(boost::format("Checking the cluster: %1%, current cluster: %2%") 
                                             % new_cluster_id % current_cluster_id)    
                );

                auto initial_connection = activeConnections_.empty();
                auto changedCluster = initial_connection && !current_cluster_id.is_nil() && !(new_cluster_id == current_cluster_id);
                if (changedCluster) {
                    HZ_LOG(logger_, warning,
                        boost::str(boost::format("Switching from current cluster: %1%  to new cluster: %2%")
                                                 % current_cluster_id % new_cluster_id)
                    );
                    client_.get_hazelcast_client_implementation()->on_cluster_restart();
                }

                active_connection_ids_.put(connection->get_connection_id(), connection);
                activeConnections_.put(response.member_uuid, connection);

                if (initial_connection) {
                    cluster_id_ = new_cluster_id;
                    fire_life_cycle_event(LifecycleEvent::LifecycleState::CLIENT_CONNECTED);
                }

                auto local_address = connection->get_local_socket_address();
                if (local_address) {
                    HZ_LOG(logger_, info,
                        boost::str(boost::format("Authenticated with server %1%:%2%, server version: %3%, "
                                                 "local address: %4%")
                                                 % response.address % response.member_uuid
                                                 % response.server_version % *local_address)
                    );
                } else {
                    HZ_LOG(logger_, info,
                        boost::str(boost::format("Authenticated with server %1%:%2%, server version: %3%, "
                                                 "no local address: (connection disconnected ?)")
                                                 % response.address % response.member_uuid
                                                 % response.server_version)
                    );
                }

                fire_connection_added_event(connection);
            }

            void ClientConnectionManagerImpl::fire_life_cycle_event(LifecycleEvent::LifecycleState state) {
                client_.get_lifecycle_service().fire_lifecycle_event(state);
            }

            void ClientConnectionManagerImpl::check_partition_count(int32_t new_partition_count) {
                auto &partition_service = static_cast<spi::impl::ClientPartitionServiceImpl &>(client_.get_partition_service());
                if (!partition_service.check_and_set_partition_count(new_partition_count)) {
                    BOOST_THROW_EXCEPTION(exception::ClientNotAllowedInClusterException("ClientConnectionManagerImpl::check_partition_count",
                          (boost::format("Client can not work with this cluster because it has a different partition count. "
                                         "Expected partition count: %1%, Member partition count: %2%")
                                         %partition_service.get_partition_count() %new_partition_count).str()));
                }
            }

            void ClientConnectionManagerImpl::trigger_cluster_reconnection() {
                if (reconnect_mode_ == config::ClientConnectionStrategyConfig::ReconnectMode::OFF) {
                    HZ_LOG(logger_, info, "RECONNECT MODE is off. Shutting down the client.");
                    shutdown_with_external_thread(client_.get_hazelcast_client_implementation());
                    return;
                }

                if (client_.get_lifecycle_service().is_running()) {
                    submit_connect_to_cluster_task();
                }
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::get_random_connection() {
                if (smart_routing_enabled_) {
                    auto member = load_balancer_->next();
                    if (!member) {
                        return nullptr;
                    }
                    auto connection = get_connection(member->get_uuid());
                    if (connection) {
                        return connection;
                    }
                }

                auto connections = activeConnections_.values();
                if (connections.empty()) {
                    return nullptr;
                }

                return connections[0];
            }

            boost::uuids::uuid ClientConnectionManagerImpl::get_client_uuid() const {
                return client_uuid_;
            }

            void ClientConnectionManagerImpl::check_invocation_allowed() {
                if (activeConnections_.size() > 0) {
                    return;
                }

                if (async_start_) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastClientOfflineException(
                            "ClientConnectionManagerImpl::check_invocation_allowed",
                            "No connection found to cluster and async start is configured."));
                } else if (reconnect_mode_ == config::ClientConnectionStrategyConfig::ReconnectMode::ASYNC) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastClientOfflineException(
                            "ClientConnectionManagerImpl::check_invocation_allowed",
                            "No connection found to cluster and reconnect mode is async."));
                } else {
                    BOOST_THROW_EXCEPTION(
                            exception::IOException("ClientConnectionManagerImpl::check_invocation_allowed",
                                                   "No connection found to cluster."));
                }
            }

            void ClientConnectionManagerImpl::connect_to_all_cluster_members() {
                if (!smart_routing_enabled_) {
                    return;
                }

                for (const auto &member : client_.get_client_cluster_service().get_member_list()) {
                    try {
                        get_or_connect(member.get_address());
                    } catch (std::exception &) {
                        // ignore
                    }
                }
            }

            void ClientConnectionManagerImpl::notify_backup(int64_t call_id) {
                struct correlation_id {
                    int32_t connnection_id;
                    int32_t call_id;
                };
                union {
                    int64_t id;
                    correlation_id composed_id;
                } c_id_union;
                c_id_union.id = call_id;
                auto connection_id = c_id_union.composed_id.connnection_id;
                auto connection = active_connection_ids_.get(connection_id);
                if (!connection) {
                    return;
                }
                boost::asio::post(connection->get_socket().get_executor(), [=] () {
                    auto invocation_it = connection->invocations.find(call_id);
                    if (invocation_it != connection->invocations.end()) {
                        invocation_it->second->notify_backup();
                    }
                });
            }

            ReadHandler::ReadHandler(Connection &connection, size_t bufferSize)
                    : buffer(new char[bufferSize]), byte_buffer(buffer, bufferSize), builder_(connection),
                      lastReadTimeDuration_(std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::steady_clock::now().time_since_epoch())) {
            }

            ReadHandler::~ReadHandler() {
                delete[] buffer;
            }

            void ReadHandler::handle() {
                lastReadTimeDuration_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch());

                if (byte_buffer.position() == 0)
                    return;

                byte_buffer.flip();

                // it is important to check the onData return value since there may be left data less than a message
                // header size, and this may cause an infinite loop.
                while (byte_buffer.has_remaining() && builder_.on_data(byte_buffer)) {
                }

                if (byte_buffer.has_remaining()) {
                    byte_buffer.compact();
                } else {
                    byte_buffer.clear();
                }
            }

            std::chrono::steady_clock::time_point ReadHandler::get_last_read_time() const {
                return std::chrono::steady_clock::time_point(
                        std::chrono::duration_cast<std::chrono::steady_clock::duration>(lastReadTimeDuration_.load()));
            }

            Connection::Connection(const Address &address, spi::ClientContext &clientContext, int connectionId, // NOLINT(cppcoreguidelines-pro-type-member-init)
                                   internal::socket::SocketFactory &socketFactory,
                                   ClientConnectionManagerImpl &clientConnectionManager,
                                   std::chrono::milliseconds &connectTimeoutInMillis)
                    : read_handler(*this, 16 << 10),
                      startTime_(std::chrono::system_clock::now()),
                      closedTimeDuration_(),
                      clientContext_(clientContext),
                      invocationService_(clientContext.get_invocation_service()),
                      connectionId_(connectionId),
                      remote_uuid_(boost::uuids::nil_uuid()), logger_(clientContext.get_logger()), alive_(true) {
                socket_ = socketFactory.create(address, connectTimeoutInMillis);
            }

            Connection::~Connection() = default;

            void Connection::connect() {
                socket_->connect(shared_from_this());
                backup_timer_.reset(new boost::asio::steady_timer(socket_->get_executor()));
                auto backupTimeout = static_cast<spi::impl::ClientInvocationServiceImpl &>(invocationService_).get_backup_timeout();
                auto this_connection = shared_from_this();
                schedule_periodic_backup_cleanup(backupTimeout, this_connection);
            }

            void Connection::schedule_periodic_backup_cleanup(std::chrono::milliseconds backupTimeout,
                                                              std::shared_ptr<Connection> this_connection) {
                backup_timer_->expires_from_now(backupTimeout);
                backup_timer_->async_wait([=] (boost::system::error_code ec) {
                    if (ec) {
                        return;
                    }
                    for (const auto &it : this_connection->invocations) {
                        it.second->detect_and_handle_backup_timeout(backupTimeout);
                    }

                    schedule_periodic_backup_cleanup(backupTimeout, this_connection);
                });
            }

            void Connection::close() {
                close("");
            }

            void Connection::close(const std::string &reason) {
                close(reason, nullptr);
            }

            void Connection::close(const std::string &reason, std::exception_ptr cause) {
                bool expected = true;
                if (!alive_.compare_exchange_strong(expected, false)) {
                    return;
                }

                closedTimeDuration_.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()));

                if (backup_timer_) {
                    boost::system::error_code ignored;
                    backup_timer_->cancel(ignored);
                }

                closeCause_ = cause;
                closeReason_ = reason;

                log_close();

                try {
                    inner_close();
                } catch (exception::IException &e) {
                    HZ_LOG(clientContext_.get_logger(), warning,
                        boost::str(boost::format("Exception while closing connection %1%")
                                                 % e.get_message())
                    );
                }

                clientContext_.get_connection_manager().on_connection_close(*this, closeCause_);

                auto thisConnection = shared_from_this();
                boost::asio::post(socket_->get_executor(), [=]() {
                    for (auto &invocationEntry : thisConnection->invocations) {
                        invocationEntry.second->notify_exception(std::make_exception_ptr(boost::enable_current_exception(
                                exception::TargetDisconnectedException("Connection::close",
                                                                       thisConnection->get_close_reason()))));
                    }
                });
            }

            void Connection::write(const std::shared_ptr<spi::impl::ClientInvocation> &clientInvocation) {
                socket_->async_write(shared_from_this(), clientInvocation);
            }

            const boost::optional<Address> &Connection::get_remote_address() const {
                return remote_address_;
            }

            void Connection::set_remote_address(boost::optional<Address> endpoint) {
                this->remote_address_ = std::move(endpoint);
            }

            void Connection::handle_client_message(const std::shared_ptr<protocol::ClientMessage> &message) {
                auto correlationId = message->get_correlation_id();
                auto invocationIterator = invocations.find(correlationId);
                if (invocationIterator == invocations.end()) {
                    HZ_LOG(logger_, warning, 
                        boost::str(boost::format("No invocation for callId:  %1%. "
                                                 "Dropping this message: %2%")
                                                 % correlationId % *message)
                    );
                    return;
                }
                auto invocation = invocationIterator->second;
                auto flags = message->get_header_flags();
                if (message->is_flag_set(flags, protocol::ClientMessage::BACKUP_EVENT_FLAG)) {
                    message->rd_ptr(protocol::ClientMessage::EVENT_HEADER_LEN);
                    correlationId = message->get<int64_t>();
                    clientContext_.get_connection_manager().notify_backup(correlationId);
                } else if (message->is_flag_set(flags, protocol::ClientMessage::IS_EVENT_FLAG)) {
                    clientContext_.get_client_listener_service().handle_client_message(invocation, message);
                } else {
                    invocationService_.handle_client_message(invocation, message);
                }
            }

            int32_t Connection::get_connection_id() const {
                return connectionId_;
            }

            bool Connection::is_alive() const {
                return alive_;
            }

            const std::string &Connection::get_close_reason() const {
                return closeReason_;
            }

            void Connection::log_close() {
                std::ostringstream message;
                message << *this << " closed. Reason: ";
                if (!closeReason_.empty()) {
                    message << closeReason_;
                } else if (closeCause_) {
                    try {
                        std::rethrow_exception(closeCause_);
                    } catch (exception::IException &ie) {
                        message << ie.get_source() << "[" + ie.get_message() << "]";
                    }
                } else {
                    message << "Socket explicitly closed";
                }

                if (clientContext_.get_lifecycle_service().is_running()) {
                    if (!closeCause_) {
                        HZ_LOG(logger_, info, message.str());
                    } else {
                        try {
                            std::rethrow_exception(closeCause_);
                        } catch (exception::IException &ie) {
                            HZ_LOG(logger_, warning, 
                                boost::str(boost::format("%1%%2%") % message.str() % ie)
                            );
                        }
                    }
                } else {
                    HZ_LOG(logger_, finest,
                        message.str() +
                        [this]() -> std::string {
                            if (closeCause_) {
                                try {
                                    std::rethrow_exception(closeCause_);
                                } catch (exception::IException &ie) {
                                    return ie.what();
                                }
                            }
                            return "";
                        }()
                    );
                }
            }

            bool Connection::operator==(const Connection &rhs) const {
                return connectionId_ == rhs.connectionId_;
            }

            bool Connection::operator!=(const Connection &rhs) const {
                return !(rhs == *this);
            }

            const std::string &Connection::get_connected_server_version_string() const {
                return connectedServerVersionString_;
            }

            void Connection::set_connected_server_version(const std::string &connectedServer) {
                Connection::connectedServerVersionString_ = connectedServer;
            }

            boost::optional<Address> Connection::get_local_socket_address() const {
                return socket_->local_socket_address();
            }

            std::chrono::steady_clock::time_point Connection::last_read_time() const {
                return read_handler.get_last_read_time();
            }

            void Connection::inner_close() {
                if (!socket_) {
                    return;
                }

                auto thisConnection = shared_from_this();
                boost::asio::post(socket_->get_executor(), [=] () { thisConnection->socket_->close(); });
            }

            std::ostream &operator<<(std::ostream &os, const Connection &connection) {
                os << "ClientConnection{"
                   << "alive=" << connection.is_alive()
                   << ", connectionId=" << connection.get_connection_id()
                   << ", remoteEndpoint=";
                if (connection.get_remote_address()) {
                    os << *connection.get_remote_address();
                } else {
                    os << "null";
                }
                os << ", lastReadTime=" << util::StringUtil::time_to_string(connection.last_read_time())
                   << ", closedTime=" << util::StringUtil::time_to_string(std::chrono::steady_clock::time_point(
                        std::chrono::duration_cast<std::chrono::steady_clock::duration>(connection.closedTimeDuration_.load())))
                   << ", connected server version=" << connection.connectedServerVersionString_
                   << '}';

                return os;
            }

            bool Connection::operator<(const Connection &rhs) const {
                return connectionId_ < rhs.connectionId_;
            }

            std::chrono::system_clock::time_point Connection::get_start_time() const {
                return startTime_;
            }

            Socket &Connection::get_socket() {
                return *socket_;
            }

            void Connection::deregister_invocation(int64_t callId) {
                invocations.erase(callId);
            }

            boost::uuids::uuid Connection::get_remote_uuid() const {
                return remote_uuid_;
            }

            void Connection::set_remote_uuid(boost::uuids::uuid remoteUuid) {
                remote_uuid_ = remoteUuid;
            }

            HeartbeatManager::HeartbeatManager(spi::ClientContext &client,
                                               ClientConnectionManagerImpl &connectionManager)
                    : client_(client), clientConnectionManager_(connectionManager), logger_(client.get_logger()) {
                ClientProperties &clientProperties = client.get_client_properties();
                auto timeout_millis = clientProperties.get_long(clientProperties.get_heartbeat_timeout());
                heartbeat_timeout_ = std::chrono::milliseconds(
                        timeout_millis > 0 ? timeout_millis : util::IOUtil::to_value<int64_t>(
                                ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT));

                auto interval_millis = clientProperties.get_long(clientProperties.get_heartbeat_interval());
                heartbeat_interval_ = std::chrono::milliseconds(interval_millis > 0 ? interval_millis
                                                                                    : util::IOUtil::to_value<int64_t>(ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT));
            }

            void HeartbeatManager::start() {
                spi::impl::ClientExecutionServiceImpl &clientExecutionService = client_.get_client_execution_service();

                timer_ = clientExecutionService.schedule_with_repetition([=]() {
                    if (!clientConnectionManager_.is_alive()) {
                        return;
                    }

                    for (auto &connection : clientConnectionManager_.get_active_connections()) {
                        check_connection(connection);
                    }
                }, heartbeat_interval_, heartbeat_interval_);
            }

            void HeartbeatManager::check_connection(const std::shared_ptr<Connection> &connection) {
                if (!connection->is_alive()) {
                    return;
                }

                auto now = std::chrono::steady_clock::now();
                if (now - connection->last_read_time() > heartbeat_timeout_) {
                    HZ_LOG(logger_, warning, 
                        boost::str(boost::format("Heartbeat failed over the connection: %1%") % *connection)
                    );
                    on_heartbeat_stopped(connection, "Heartbeat timed out");
                    return;
                }

                if (now - connection->last_read_time() > heartbeat_interval_) {
                    auto request = protocol::codec::client_ping_encode();
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client_, request, "", connection);
                    clientInvocation->invoke_urgent();
                }
            }

            void
            HeartbeatManager::on_heartbeat_stopped(const std::shared_ptr<Connection> &connection,
                                                 const std::string &reason) {
                connection->close(reason, std::make_exception_ptr(
                        (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                                "HeartbeatManager::onHeartbeatStopped") << "Heartbeat timed out to connection "
                                                                        << *connection).build()));
            }

            void HeartbeatManager::shutdown() {
                if (timer_) {
                    boost::system::error_code ignored;
                    timer_->cancel(ignored);
                }
            }

            std::chrono::milliseconds HeartbeatManager::get_heartbeat_timeout() const {
                return heartbeat_timeout_;
            }

        }

        namespace internal {
            namespace socket {
                SocketFactory::SocketFactory(spi::ClientContext &clientContext, boost::asio::io_context &io,
                                             boost::asio::ip::tcp::resolver &resolver)
                        : clientContext_(clientContext), io_(io), ioResolver_(resolver) {
                }

                bool SocketFactory::start() {
#ifdef HZ_BUILD_WITH_SSL
                    const client::config::SSLConfig &sslConfig = clientContext_.get_client_config().get_network_config().get_ssl_config();
                    if (sslConfig.is_enabled()) {
                        sslContext_ = std::unique_ptr<boost::asio::ssl::context>(new boost::asio::ssl::context(
                                (boost::asio::ssl::context_base::method) sslConfig.get_protocol()));

                        const std::vector<std::string> &verifyFiles = sslConfig.get_verify_files();
                        bool success = true;
                        logger &lg = clientContext_.get_logger();
                        for (std::vector<std::string>::const_iterator it = verifyFiles.begin(); it != verifyFiles.end();
                             ++it) {
                            boost::system::error_code ec;
                            sslContext_->load_verify_file(*it, ec);
                            if (ec) {
                                HZ_LOG(lg, warning, 
                                    boost::str(boost::format("SocketFactory::start: Failed to load CA "
                                                             "verify file at %1% %2%")
                                                             % *it % ec.message())
                                );
                                success = false;
                            }
                        }

                        if (!success) {
                            sslContext_.reset();
                            HZ_LOG(lg, warning,
                                "SocketFactory::start: Failed to load one or more "
                                "configured CA verify files (PEM files). Please "
                                "correct the files and retry."
                            );
                            return false;
                        }

                        // set cipher list if the list is set
                        const std::string &cipherList = sslConfig.get_cipher_list();
                        if (!cipherList.empty()) {
                            if (!SSL_CTX_set_cipher_list(sslContext_->native_handle(), cipherList.c_str())) {
                                HZ_LOG(lg, warning, 
                                    std::string("SocketFactory::start: Could not load any "
                                                "of the ciphers in the config provided "
                                                "ciphers:") + cipherList
                                );
                                return false;
                            }
                        }

                    }
#else
                    (void) clientContext;
#endif
                    return true;
                }

                std::unique_ptr<Socket> SocketFactory::create(const Address &address,
                                                              std::chrono::milliseconds &connectTimeoutInMillis) {
#ifdef HZ_BUILD_WITH_SSL
                    if (sslContext_.get()) {
                        return std::unique_ptr<Socket>(new internal::socket::SSLSocket(io_, *sslContext_, address,
                                                                                       clientContext_.get_client_config().get_network_config().get_socket_options(),
                                                                                       connectTimeoutInMillis, ioResolver_));
                    }
#endif

                    return std::unique_ptr<Socket>(new internal::socket::TcpSocket(io_, address,
                                                                                   clientContext_.get_client_config().get_network_config().get_socket_options(),
                                                                                   connectTimeoutInMillis, ioResolver_));
                }

#ifdef HZ_BUILD_WITH_SSL

                SSLSocket::SSLSocket(boost::asio::io_context &ioService, boost::asio::ssl::context &sslContext,
                                     const client::Address &address, client::config::SocketOptions &socketOptions,
                                     std::chrono::milliseconds &connectTimeoutInMillis,
                                     boost::asio::ip::tcp::resolver &resolver)
                        : BaseSocket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(resolver, address,
                                socketOptions, ioService,connectTimeoutInMillis, sslContext) {
                }

                std::vector<SSLSocket::CipherInfo> SSLSocket::get_ciphers() {
                    STACK_OF(SSL_CIPHER) *ciphers = SSL_get_ciphers(socket_.native_handle());
                    std::vector<CipherInfo> supportedCiphers;
                    for (int i = 0; i < sk_SSL_CIPHER_num(ciphers); ++i) {
                        struct SSLSocket::CipherInfo info;
                        const SSL_CIPHER *cipher = sk_SSL_CIPHER_value(ciphers, i);
                        info.name = SSL_CIPHER_get_name(cipher);
                        info.number_of_bits = SSL_CIPHER_get_bits(cipher, 0);
                        info.version = SSL_CIPHER_get_version(cipher);
                        char descBuf[256];
                        info.description = SSL_CIPHER_description(cipher, descBuf, 256);
                        supportedCiphers.push_back(info);
                    }
                    return supportedCiphers;
                }

                void SSLSocket::post_connect() {
                    socket_.handshake(boost::asio::ssl::stream_base::client);
                }

                std::ostream &operator<<(std::ostream &out, const SSLSocket::CipherInfo &info) {
                    out << "Cipher{"
                           "Name: " << info.name <<
                        ", Bits:" << info.number_of_bits <<
                        ", Version:" << info.version <<
                        ", Description:" << info.description << "}";

                    return out;
                }

#endif // HZ_BUILD_WITH_SSL

                TcpSocket::TcpSocket(boost::asio::io_context &io, const Address &address,
                                     client::config::SocketOptions &socketOptions,
                                     std::chrono::milliseconds &connectTimeoutInMillis,
                                     boost::asio::ip::tcp::resolver &resolver)
                        : BaseSocket<boost::asio::ip::tcp::socket>(resolver, address, socketOptions, io,
                                                                   connectTimeoutInMillis) {
                }

            }
        }
    }
}

namespace std {
    std::size_t hash<std::shared_ptr<hazelcast::client::connection::Connection>>::operator()(
            const std::shared_ptr<hazelcast::client::connection::Connection> &conn) const noexcept {
        if (!conn) {
            return 0;
        }
        return std::abs(conn->get_connection_id());
    }
}


