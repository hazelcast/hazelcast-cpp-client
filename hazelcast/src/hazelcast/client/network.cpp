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
                    : alive(false), logger_(client.getLogger()), connectionTimeoutMillis(std::chrono::milliseconds::max()),
                      client(client),
                      socketInterceptor(client.getClientConfig().getSocketInterceptor()),
                      executionService(client.getClientExecutionService()),
                      translator(addressTranslator), connectionIdGen(0),
                      heartbeat(client, *this), partition_count_(-1),
                      async_start_(client.getClientConfig().getConnectionStrategyConfig().isAsyncStart()),
                      reconnect_mode_(client.getClientConfig().getConnectionStrategyConfig().getReconnectMode()),
                      smart_routing_enabled_(client.getClientConfig().getNetworkConfig().isSmartRouting()),
                      connect_to_cluster_task_submitted_(false),
                      client_uuid_(client.random_uuid()),
                      authentication_timeout_(boost::chrono::milliseconds(heartbeat.getHeartbeatTimeout().count())),
                      cluster_id_(boost::uuids::nil_uuid()) {

                auto configured_load_balancer = client.getClientConfig().getLoadBalancer();
                if (configured_load_balancer) {
                    load_balancer_= std::move(configured_load_balancer);
                }
                else {
                    load_balancer_.reset(new impl::RoundRobinLB{});
                }

                config::ClientNetworkConfig &networkConfig = client.getClientConfig().getNetworkConfig();
                auto connTimeout = networkConfig.getConnectionTimeout();
                if (connTimeout.count() > 0) {
                    connectionTimeoutMillis = std::chrono::milliseconds(connTimeout);
                }

                ClientProperties &clientProperties = client.getClientProperties();
                shuffleMemberList = clientProperties.getBoolean(clientProperties.getShuffleMemberList());

                ClientConnectionManagerImpl::addressProviders = addressProviders;

                connectionAttemptPeriod = networkConfig.getConnectionAttemptPeriod();

                int connAttemptLimit = networkConfig.getConnectionAttemptLimit();
                bool isAsync = client.getClientConfig().getConnectionStrategyConfig().isAsyncStart();

                if (connAttemptLimit < 0) {
                    this->connectionAttemptLimit = isAsync ? DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC
                                                           : DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
                } else {
                    this->connectionAttemptLimit = connAttemptLimit == 0 ? INT32_MAX : connAttemptLimit;
                }

                ioThreadCount = clientProperties.getInteger(clientProperties.getIOThreadCount());
            }

            bool ClientConnectionManagerImpl::start() {
                bool expected = false;
                if (!alive.compare_exchange_strong(expected, true)) {
                    return false;
                }

                ioContext.reset(new boost::asio::io_context);
                ioResolver.reset(new boost::asio::ip::tcp::resolver(ioContext->get_executor()));
                socketFactory.reset(new internal::socket::SocketFactory(client, *ioContext, *ioResolver));
                ioGuard.reset(new boost::asio::io_context::work(*ioContext));

                if (!socketFactory->start()) {
                    return false;
                }

                socketInterceptor = client.getClientConfig().getSocketInterceptor();

                for (int j = 0; j < ioThreadCount; ++j) {
                    ioThreads.emplace_back([=]() { ioContext->run(); });
                }

                executor_.reset(new hazelcast::util::hz_thread_pool(EXECUTOR_CORE_POOL_SIZE));
                connect_to_members_timer_ = boost::asio::steady_timer(executor_->get_executor());

                heartbeat.start();

                connectToCluster();
                if (smart_routing_enabled_) {
                    schedule_connect_to_all_members();
                }

                load_balancer_->init(client.getCluster());

                return true;
            }

            void ClientConnectionManagerImpl::schedule_connect_to_all_members() {
                connect_to_members_timer_->expires_from_now(boost::asio::chrono::seconds(1));
                connect_to_members_timer_->async_wait([=](boost::system::error_code ec) {
                    if (ec == boost::asio::error::operation_aborted) {
                        return;
                    }
                    connect_to_all_members();

                    if (!client.getLifecycleService().isRunning()) {
                        return;
                    }

                    schedule_connect_to_all_members();
                });
            }

            void ClientConnectionManagerImpl::shutdown() {
                bool expected = true;
                if (!alive.compare_exchange_strong(expected, false)) {
                    return;
                }

                if (connect_to_members_timer_) {
                    connect_to_members_timer_->cancel();
                }

                heartbeat.shutdown();

                // close connections
                for (auto &connection : activeConnections.values()) {
                    // prevent any exceptions
                    util::IOUtil::closeResource(connection.get(), "Hazelcast client is shutting down");
                }

                spi::impl::ClientExecutionServiceImpl::shutdownThreadPool(executor_.get());

                ioGuard.reset();
                ioContext->stop();
                boost::asio::use_service<boost::asio::detail::resolver_service<boost::asio::ip::tcp>>(*ioContext).shutdown();
                std::for_each(ioThreads.begin(), ioThreads.end(), [](std::thread &t) { t.join(); });

                connectionListeners.clear();
                activeConnections.clear();
                active_connection_ids_.clear();
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getOrConnect(const Address &address) {
                auto connection = getConnection(address);
                if (connection) {
                    return connection;
                }

                auto f = conn_locks_.emplace(address, std::unique_ptr<std::mutex>(new std::mutex()));
                std::lock_guard<std::mutex> g(*f.first->second);
                // double check here
                connection = getConnection(address);
                if (connection) {
                    return connection;
                }

                auto target = translator->translate(address);
                connection = std::make_shared<Connection>(target, client, ++connectionIdGen,
                                                          *socketFactory, *this, connectionTimeoutMillis);
                connection->connect();

                // call the interceptor from user thread
                socketInterceptor.connect(connection->getSocket());

                authenticate_on_cluster(connection);

                return connection;
            }

            std::vector<std::shared_ptr<Connection> > ClientConnectionManagerImpl::getActiveConnections() {
                return activeConnections.values();
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getConnection(const Address &address) {
                for (const auto &connection : activeConnections.values()) {
                    auto remote_address = connection->getRemoteAddress();
                    if (remote_address && *remote_address == address) {
                        return connection;
                    }
                }
                return nullptr;
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getConnection(boost::uuids::uuid uuid) {
                return activeConnections.get(uuid);
            }

            void
            ClientConnectionManagerImpl::authenticate_on_cluster(std::shared_ptr<Connection> &connection) {
                auto request = encodeAuthenticationRequest(client.getSerializationService());
                auto clientInvocation = spi::impl::ClientInvocation::create(client, request, "", connection);
                auto f = clientInvocation->invokeUrgent();

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

                    result.address = response.getNullable<Address>();
                    result.server_version = response.get<std::string>();
                } catch (exception::IException &) {
                    connection->close("Failed to authenticate connection", std::current_exception());
                    throw;
                }

                auto authentication_status = (protocol::AuthenticationStatus) result.status;
                switch (authentication_status) {
                    case protocol::AUTHENTICATED: {
                        handleSuccessfulAuth(connection, std::move(result));
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
            ClientConnectionManagerImpl::encodeAuthenticationRequest(serialization::pimpl::SerializationService &ss) {
                byte serializationVersion = ss.getVersion();
                ClientConfig &clientConfig = client.getClientConfig();
                auto cluster_name = clientConfig.getClusterName();

                auto credential = clientConfig.getCredentials();
                if (!credential) {
                    return protocol::codec::client_authentication_encode(cluster_name, nullptr, nullptr,
                                                                         client_uuid_, protocol::ClientTypes::CPP,
                                                                         serializationVersion, HAZELCAST_VERSION,
                                                                         client.getName(), labels_);
                }

                switch(credential->type()) {
                    case security::credentials::credential_type::username_password:
                    {
                        auto cr = std::static_pointer_cast<security::username_password_credentials>(credential);
                        return protocol::codec::client_authentication_encode(cluster_name, &cr->name(), &cr->password(),
                                                                             client_uuid_, protocol::ClientTypes::CPP,
                                                                             serializationVersion, HAZELCAST_VERSION,
                                                                             client.getName(), labels_);
                    }
                    case security::credentials::credential_type::token:
                    {
                        auto cr = std::static_pointer_cast<security::token_credentials>(credential);
                        return protocol::codec::client_authenticationcustom_encode(cluster_name, cr->token(),
                                                                                   client_uuid_, protocol::ClientTypes::CPP,
                                                                                   serializationVersion, HAZELCAST_VERSION,
                                                                                   client.getName(), labels_);
                    }
                }
                assert(0);
                return protocol::ClientMessage();
            }

            void
            ClientConnectionManagerImpl::fireConnectionAddedEvent(const std::shared_ptr<Connection> &connection) {
                for (const std::shared_ptr<ConnectionListener> &connectionListener : connectionListeners.toArray()) {
                    connectionListener->connectionAdded(connection);
                }
            }

            void
            ClientConnectionManagerImpl::fireConnectionRemovedEvent(const std::shared_ptr<Connection> &connection) {
                for (const auto &listener : connectionListeners.toArray()) {
                    listener->connectionRemoved(connection);
                }
            }

            void
            ClientConnectionManagerImpl::shutdownWithExternalThread(
                    const std::weak_ptr<client::impl::HazelcastClientInstanceImpl>& clientImpl) {

                std::thread([=] {
                    std::shared_ptr<client::impl::HazelcastClientInstanceImpl> clientInstance = clientImpl.lock();
                    if (!clientInstance || !clientInstance->getLifecycleService().isRunning()) {
                        return;
                    }

                    try {
                        clientInstance->getLifecycleService().shutdown();
                    } catch (exception::IException &e) {
                        HZ_LOG(*clientInstance->getLogger(), severe, 
                            boost::str(boost::format("Exception during client shutdown "
                                                     "%1%.clientShutdown-:%2%")
                                                     % clientInstance->getName()
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
                        if (activeConnections.empty()) {
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

                        shutdownWithExternalThread(client.getHazelcastClientImplementation());
                    }
                });
            }

            void ClientConnectionManagerImpl::connect_to_all_members() {
                if (!client.getLifecycleService().isRunning()) {
                    return;
                }

                auto connecting_addresses = std::make_shared<util::sync_associative_container<std::unordered_set<Address>>>();
                for (const auto &member : client.getClientClusterService().getMemberList()) {
                    const auto& address = member.getAddress();

                    if (client.getLifecycleService().isRunning() && !getConnection(address)
                        && connecting_addresses->insert(address).second) {
                        // submit a task for this address only if there is no
                        // another connection attempt for it
                        Address addr = address;
                        boost::asio::post(executor_->get_executor(), [=] () {
                            try {
                                if (!client.getLifecycleService().isRunning()) {
                                    return;
                                }
                                if (!getConnection(member.getUuid())) {
                                    getOrConnect(addr);
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

                while (attempt < connectionAttemptLimit) {
                    attempt++;
                    auto nextTryTime = std::chrono::steady_clock::now() + connectionAttemptPeriod;

                    for (const Address &address : getPossibleMemberAddresses()) {
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

                    if (attempt < connectionAttemptLimit) {
                        auto remainingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                                nextTryTime - std::chrono::steady_clock::now()).count();
                        HZ_LOG(logger_, warning,
                            boost::str(boost::format("Unable to get alive cluster connection, try in "
                                                     "%1% ms later, attempt %2% of %3%.")
                                                     % (remainingTime > 0 ? remainingTime : 0)
                                                     % attempt % connectionAttemptLimit)
                        );

                        if (remainingTime > 0) {
                            // TODO use a condition variable here
                            std::this_thread::sleep_for(std::chrono::milliseconds(remainingTime));
                        }
                    } else {
                        HZ_LOG(logger_, warning, 
                            boost::str(boost::format("Unable to get alive cluster connection, "
                                                     "attempt %1% of %2%.")
                                                     % attempt % connectionAttemptLimit)
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

            std::vector<Address> ClientConnectionManagerImpl::getPossibleMemberAddresses() {
                std::vector<Address> addresses;
                for (auto &&member : client.getClientClusterService().getMemberList()) {
                    addresses.emplace_back(std::move(member.getAddress()));
                }

                if (shuffleMemberList) {
                    shuffle(addresses);
                }

                std::vector<Address> provided_addresses;
                for (auto &addressProvider : addressProviders) {
                    auto addrList = addressProvider->loadAddresses();
                    provided_addresses.insert(provided_addresses.end(), addrList.begin(), addrList.end());
                }

                if (shuffleMemberList) {
                    shuffle(provided_addresses);
                }

                addresses.insert(addresses.end(), provided_addresses.begin(), provided_addresses.end());

                return addresses;
            }

            void ClientConnectionManagerImpl::connectToCluster() {
                if (async_start_) {
                    submit_connect_to_cluster_task();
                } else {
                    do_connect_to_cluster();
                }
            }

            bool ClientConnectionManagerImpl::isAlive() {
                return alive;
            }

            void ClientConnectionManagerImpl::on_connection_close(Connection &connection, std::exception_ptr cause) {
                auto endpoint = connection.getRemoteAddress();
                auto member_uuid = connection.getRemoteUuid();

                auto socket_remote_address = connection.getSocket().getRemoteEndpoint();

                if (!endpoint) {
                    HZ_LOG(logger_, finest,
                        boost::str(boost::format("Destroying %1% , but it has end-point set to null "
                                                 "-> not removing it from a connection map")
                                                 % connection)
                    );
                    return;
                }

                auto conn = connection.shared_from_this();
                if (activeConnections.remove(member_uuid, conn)) {
                    active_connection_ids_.remove(conn->getConnectionId());

                    HZ_LOG(logger_, info, 
                        boost::str(boost::format("Removed connection to endpoint: %1%, connection: %2%")
                                                 % *endpoint % connection)
                    );

                    if (activeConnections.empty()) {
                        fire_life_cycle_event(LifecycleEvent::LifecycleState::CLIENT_DISCONNECTED);

                        trigger_cluster_reconnection();
                    }

                    fireConnectionRemovedEvent(connection.shared_from_this());
                } else {
                    HZ_LOG(logger_, finest,
                        boost::str(boost::format("Destroying a connection, but there is no mapping "
                                                 "%1% -> %2% in the connection map.")
                                                 % endpoint % connection)
                    );
                }
            }

            void
            ClientConnectionManagerImpl::addConnectionListener(
                    const std::shared_ptr<ConnectionListener> &connectionListener) {
                connectionListeners.add(connectionListener);
            }

            ClientConnectionManagerImpl::~ClientConnectionManagerImpl() {
                shutdown();
            }

            logger &ClientConnectionManagerImpl::getLogger() {
                return client.getLogger();
            }

            void ClientConnectionManagerImpl::check_client_active() {
                if (!client.getLifecycleService().isRunning()) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastClientNotActiveException(
                            "ClientConnectionManagerImpl::check_client_active", "Client is shutdown"));
                }
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::connect(const Address &address) {
                try {
                    HZ_LOG(logger_, info, 
                        boost::str(boost::format("Trying to connect to %1%") % address));
                    return getOrConnect(address);
                } catch (std::exception &e) {
                    HZ_LOG(logger_, warning,
                        boost::str(boost::format("Exception during initial connection to %1%: %2%")
                                                 % address % e.what()));
                    return nullptr;
                }
            }

            void ClientConnectionManagerImpl::handleSuccessfulAuth(const std::shared_ptr<Connection> &connection,
                                                                   auth_response response) {
                check_partition_count(response.partition_count);
                connection->setConnectedServerVersion(response.server_version);
                connection->setRemoteAddress(std::move(response.address));
                connection->setRemoteUuid(response.member_uuid);

                auto new_cluster_id = response.cluster_id;
                boost::uuids::uuid current_cluster_id = cluster_id_;

                HZ_LOG(logger_, finest, 
                    boost::str(boost::format("Checking the cluster: %1%, current cluster: %2%") 
                                             % new_cluster_id % current_cluster_id)    
                );

                auto initial_connection = activeConnections.empty();
                auto changedCluster = initial_connection && !current_cluster_id.is_nil() && !(new_cluster_id == current_cluster_id);
                if (changedCluster) {
                    HZ_LOG(logger_, warning,
                        boost::str(boost::format("Switching from current cluster: %1%  to new cluster: %2%")
                                                 % current_cluster_id % new_cluster_id)
                    );
                    client.getHazelcastClientImplementation()->on_cluster_restart();
                }

                active_connection_ids_.put(connection->getConnectionId(), connection);
                activeConnections.put(response.member_uuid, connection);

                if (initial_connection) {
                    cluster_id_ = new_cluster_id;
                    fire_life_cycle_event(LifecycleEvent::LifecycleState::CLIENT_CONNECTED);
                }

                auto local_address = connection->getLocalSocketAddress();
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

                fireConnectionAddedEvent(connection);
            }

            void ClientConnectionManagerImpl::fire_life_cycle_event(LifecycleEvent::LifecycleState state) {
                client.getLifecycleService().fireLifecycleEvent(state);
            }

            void ClientConnectionManagerImpl::check_partition_count(int32_t new_partition_count) {
                auto &partition_service = static_cast<spi::impl::ClientPartitionServiceImpl &>(client.getPartitionService());
                if (!partition_service.check_and_set_partition_count(new_partition_count)) {
                    BOOST_THROW_EXCEPTION(exception::ClientNotAllowedInClusterException("ClientConnectionManagerImpl::check_partition_count",
                          (boost::format("Client can not work with this cluster because it has a different partition count. "
                                         "Expected partition count: %1%, Member partition count: %2%")
                                         %partition_service.getPartitionCount() %new_partition_count).str()));
                }
            }

            void ClientConnectionManagerImpl::trigger_cluster_reconnection() {
                if (reconnect_mode_ == config::ClientConnectionStrategyConfig::ReconnectMode::OFF) {
                    HZ_LOG(logger_, info, "RECONNECT MODE is off. Shutting down the client.");
                    shutdownWithExternalThread(client.getHazelcastClientImplementation());
                    return;
                }

                if (client.getLifecycleService().isRunning()) {
                    submit_connect_to_cluster_task();
                }
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::get_random_connection() {
                if (smart_routing_enabled_) {
                    auto member = load_balancer_->next();
                    if (!member) {
                        return nullptr;
                    }
                    auto connection = getConnection(member->getUuid());
                    if (connection) {
                        return connection;
                    }
                }

                auto connections = activeConnections.values();
                if (connections.empty()) {
                    return nullptr;
                }

                return connections[0];
            }

            boost::uuids::uuid ClientConnectionManagerImpl::getClientUuid() const {
                return client_uuid_;
            }

            void ClientConnectionManagerImpl::check_invocation_allowed() {
                if (activeConnections.size() > 0) {
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

                for (const auto &member : client.getClientClusterService().getMemberList()) {
                    try {
                        getOrConnect(member.getAddress());
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
                boost::asio::post(connection->getSocket().get_executor(), [=] () {
                    auto invocation_it = connection->invocations.find(call_id);
                    if (invocation_it != connection->invocations.end()) {
                        invocation_it->second->notify_backup();
                    }
                });
            }

            ReadHandler::ReadHandler(Connection &connection, size_t bufferSize)
                    : buffer(new char[bufferSize]), byteBuffer(buffer, bufferSize), builder(connection),
                      lastReadTimeDuration(std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::steady_clock::now().time_since_epoch())) {
            }

            ReadHandler::~ReadHandler() {
                delete[] buffer;
            }

            void ReadHandler::handle() {
                lastReadTimeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch());

                if (byteBuffer.position() == 0)
                    return;

                byteBuffer.flip();

                // it is important to check the onData return value since there may be left data less than a message
                // header size, and this may cause an infinite loop.
                while (byteBuffer.hasRemaining() && builder.onData(byteBuffer)) {
                }

                if (byteBuffer.hasRemaining()) {
                    byteBuffer.compact();
                } else {
                    byteBuffer.clear();
                }
            }

            std::chrono::steady_clock::time_point ReadHandler::getLastReadTime() const {
                return std::chrono::steady_clock::time_point(
                        std::chrono::duration_cast<std::chrono::steady_clock::duration>(lastReadTimeDuration.load()));
            }

            Connection::Connection(const Address &address, spi::ClientContext &clientContext, int connectionId, // NOLINT(cppcoreguidelines-pro-type-member-init)
                                   internal::socket::SocketFactory &socketFactory,
                                   ClientConnectionManagerImpl &clientConnectionManager,
                                   std::chrono::milliseconds &connectTimeoutInMillis)
                    : readHandler(*this, 16 << 10),
                      startTime(std::chrono::system_clock::now()),
                      closedTimeDuration(),
                      clientContext(clientContext),
                      invocationService(clientContext.getInvocationService()),
                      connectionId(connectionId),
                      remote_uuid_(boost::uuids::nil_uuid()), logger_(clientContext.getLogger()), alive(true) {
                socket = socketFactory.create(address, connectTimeoutInMillis);
            }

            Connection::~Connection() = default;

            void Connection::connect() {
                socket->connect(shared_from_this());
                backup_timer_.reset(new boost::asio::steady_timer(socket->get_executor()));
                auto backupTimeout = static_cast<spi::impl::ClientInvocationServiceImpl &>(invocationService).getBackupTimeout();
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
                if (!alive.compare_exchange_strong(expected, false)) {
                    return;
                }

                closedTimeDuration.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()));

                if (backup_timer_) {
                    boost::system::error_code ignored;
                    backup_timer_->cancel(ignored);
                }

                closeCause = cause;
                closeReason = reason;

                logClose();

                try {
                    innerClose();
                } catch (exception::IException &e) {
                    HZ_LOG(clientContext.getLogger(), warning,
                        boost::str(boost::format("Exception while closing connection %1%")
                                                 % e.getMessage())
                    );
                }

                clientContext.getConnectionManager().on_connection_close(*this, closeCause);

                auto thisConnection = shared_from_this();
                boost::asio::post(socket->get_executor(), [=]() {
                    for (auto &invocationEntry : thisConnection->invocations) {
                        invocationEntry.second->notifyException(std::make_exception_ptr(boost::enable_current_exception(
                                exception::TargetDisconnectedException("Connection::close",
                                                                       thisConnection->getCloseReason()))));
                    }
                });
            }

            void Connection::write(const std::shared_ptr<spi::impl::ClientInvocation> &clientInvocation) {
                socket->asyncWrite(shared_from_this(), clientInvocation);
            }

            const boost::optional<Address> &Connection::getRemoteAddress() const {
                return remote_address_;
            }

            void Connection::setRemoteAddress(boost::optional<Address> endpoint) {
                this->remote_address_ = std::move(endpoint);
            }

            void Connection::handleClientMessage(const std::shared_ptr<protocol::ClientMessage> &message) {
                auto correlationId = message->getCorrelationId();
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
                auto flags = message->getHeaderFlags();
                if (message->is_flag_set(flags, protocol::ClientMessage::BACKUP_EVENT_FLAG)) {
                    message->rd_ptr(protocol::ClientMessage::EVENT_HEADER_LEN);
                    correlationId = message->get<int64_t>();
                    clientContext.getConnectionManager().notify_backup(correlationId);
                } else if (message->is_flag_set(flags, protocol::ClientMessage::IS_EVENT_FLAG)) {
                    clientContext.getClientListenerService().handleClientMessage(invocation, message);
                } else {
                    invocationService.handleClientMessage(invocation, message);
                }
            }

            int32_t Connection::getConnectionId() const {
                return connectionId;
            }

            bool Connection::isAlive() const {
                return alive;
            }

            const std::string &Connection::getCloseReason() const {
                return closeReason;
            }

            void Connection::logClose() {
                std::ostringstream message;
                message << *this << " closed. Reason: ";
                if (!closeReason.empty()) {
                    message << closeReason;
                } else if (closeCause) {
                    try {
                        std::rethrow_exception(closeCause);
                    } catch (exception::IException &ie) {
                        message << ie.getSource() << "[" + ie.getMessage() << "]";
                    }
                } else {
                    message << "Socket explicitly closed";
                }

                if (clientContext.getLifecycleService().isRunning()) {
                    if (!closeCause) {
                        HZ_LOG(logger_, info, message.str());
                    } else {
                        try {
                            std::rethrow_exception(closeCause);
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
                            if (closeCause) {
                                try {
                                    std::rethrow_exception(closeCause);
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
                return connectionId == rhs.connectionId;
            }

            bool Connection::operator!=(const Connection &rhs) const {
                return !(rhs == *this);
            }

            const std::string &Connection::getConnectedServerVersionString() const {
                return connectedServerVersionString;
            }

            void Connection::setConnectedServerVersion(const std::string &connectedServer) {
                Connection::connectedServerVersionString = connectedServer;
            }

            boost::optional<Address> Connection::getLocalSocketAddress() const {
                return socket->localSocketAddress();
            }

            std::chrono::steady_clock::time_point Connection::lastReadTime() const {
                return readHandler.getLastReadTime();
            }

            void Connection::innerClose() {
                if (!socket) {
                    return;
                }

                auto thisConnection = shared_from_this();
                boost::asio::post(socket->get_executor(), [=] () { thisConnection->socket->close(); });
            }

            std::ostream &operator<<(std::ostream &os, const Connection &connection) {
                os << "ClientConnection{"
                   << "alive=" << connection.isAlive()
                   << ", connectionId=" << connection.getConnectionId()
                   << ", remoteEndpoint=";
                if (connection.getRemoteAddress()) {
                    os << *connection.getRemoteAddress();
                } else {
                    os << "null";
                }
                os << ", lastReadTime=" << util::StringUtil::timeToString(connection.lastReadTime())
                   << ", closedTime=" << util::StringUtil::timeToString(std::chrono::steady_clock::time_point(
                        std::chrono::duration_cast<std::chrono::steady_clock::duration>(connection.closedTimeDuration.load())))
                   << ", connected server version=" << connection.connectedServerVersionString
                   << '}';

                return os;
            }

            bool Connection::operator<(const Connection &rhs) const {
                return connectionId < rhs.connectionId;
            }

            std::chrono::system_clock::time_point Connection::getStartTime() const {
                return startTime;
            }

            Socket &Connection::getSocket() {
                return *socket;
            }

            void Connection::deregisterInvocation(int64_t callId) {
                invocations.erase(callId);
            }

            boost::uuids::uuid Connection::getRemoteUuid() const {
                return remote_uuid_;
            }

            void Connection::setRemoteUuid(boost::uuids::uuid remoteUuid) {
                remote_uuid_ = remoteUuid;
            }

            HeartbeatManager::HeartbeatManager(spi::ClientContext &client,
                                               ClientConnectionManagerImpl &connectionManager)
                    : client(client), clientConnectionManager(connectionManager), logger_(client.getLogger()) {
                ClientProperties &clientProperties = client.getClientProperties();
                auto timeout_millis = clientProperties.getLong(clientProperties.getHeartbeatTimeout());
                heartbeat_timeout_ = std::chrono::milliseconds(
                        timeout_millis > 0 ? timeout_millis : util::IOUtil::to_value<int64_t>(
                                ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT));

                auto interval_millis = clientProperties.getLong(clientProperties.getHeartbeatInterval());
                heartbeat_interval_ = std::chrono::milliseconds(interval_millis > 0 ? interval_millis
                                                                                    : util::IOUtil::to_value<int64_t>(ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT));
            }

            void HeartbeatManager::start() {
                spi::impl::ClientExecutionServiceImpl &clientExecutionService = client.getClientExecutionService();

                timer = clientExecutionService.scheduleWithRepetition([=]() {
                    if (!clientConnectionManager.isAlive()) {
                        return;
                    }

                    for (auto &connection : clientConnectionManager.getActiveConnections()) {
                        checkConnection(connection);
                    }
                }, heartbeat_interval_, heartbeat_interval_);
            }

            void HeartbeatManager::checkConnection(const std::shared_ptr<Connection> &connection) {
                if (!connection->isAlive()) {
                    return;
                }

                auto now = std::chrono::steady_clock::now();
                if (now - connection->lastReadTime() > heartbeat_timeout_) {
                    HZ_LOG(logger_, warning, 
                        boost::str(boost::format("Heartbeat failed over the connection: %1%") % *connection)
                    );
                    onHeartbeatStopped(connection, "Heartbeat timed out");
                    return;
                }

                if (now - connection->lastReadTime() > heartbeat_interval_) {
                    auto request = protocol::codec::client_ping_encode();
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, "", connection);
                    clientInvocation->invokeUrgent();
                }
            }

            void
            HeartbeatManager::onHeartbeatStopped(const std::shared_ptr<Connection> &connection,
                                                 const std::string &reason) {
                connection->close(reason, std::make_exception_ptr(
                        (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                                "HeartbeatManager::onHeartbeatStopped") << "Heartbeat timed out to connection "
                                                                        << *connection).build()));
            }

            void HeartbeatManager::shutdown() {
                if (timer) {
                    boost::system::error_code ignored;
                    timer->cancel(ignored);
                }
            }

            std::chrono::milliseconds HeartbeatManager::getHeartbeatTimeout() const {
                return heartbeat_timeout_;
            }

        }

        namespace internal {
            namespace socket {
                SocketFactory::SocketFactory(spi::ClientContext &clientContext, boost::asio::io_context &io,
                                             boost::asio::ip::tcp::resolver &resolver)
                        : clientContext(clientContext), io(io), ioResolver(resolver) {
                }

                bool SocketFactory::start() {
#ifdef HZ_BUILD_WITH_SSL
                    const client::config::SSLConfig &sslConfig = clientContext.getClientConfig().getNetworkConfig().getSSLConfig();
                    if (sslConfig.isEnabled()) {
                        sslContext = std::unique_ptr<boost::asio::ssl::context>(new boost::asio::ssl::context(
                                (boost::asio::ssl::context_base::method) sslConfig.getProtocol()));

                        const std::vector<std::string> &verifyFiles = sslConfig.getVerifyFiles();
                        bool success = true;
                        logger &lg = clientContext.getLogger();
                        for (std::vector<std::string>::const_iterator it = verifyFiles.begin(); it != verifyFiles.end();
                             ++it) {
                            boost::system::error_code ec;
                            sslContext->load_verify_file(*it, ec);
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
                            sslContext.reset();
                            HZ_LOG(lg, warning,
                                "SocketFactory::start: Failed to load one or more "
                                "configured CA verify files (PEM files). Please "
                                "correct the files and retry."
                            );
                            return false;
                        }

                        // set cipher list if the list is set
                        const std::string &cipherList = sslConfig.getCipherList();
                        if (!cipherList.empty()) {
                            if (!SSL_CTX_set_cipher_list(sslContext->native_handle(), cipherList.c_str())) {
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
                    if (sslContext.get()) {
                        return std::unique_ptr<Socket>(new internal::socket::SSLSocket(io, *sslContext, address,
                                                                                       clientContext.getClientConfig().getNetworkConfig().getSocketOptions(),
                                                                                       connectTimeoutInMillis, ioResolver));
                    }
#endif

                    return std::unique_ptr<Socket>(new internal::socket::TcpSocket(io, address,
                                                                                   clientContext.getClientConfig().getNetworkConfig().getSocketOptions(),
                                                                                   connectTimeoutInMillis, ioResolver));
                }

#ifdef HZ_BUILD_WITH_SSL

                SSLSocket::SSLSocket(boost::asio::io_context &ioService, boost::asio::ssl::context &sslContext,
                                     const client::Address &address, client::config::SocketOptions &socketOptions,
                                     std::chrono::milliseconds &connectTimeoutInMillis,
                                     boost::asio::ip::tcp::resolver &resolver)
                        : BaseSocket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(resolver, address,
                                socketOptions, ioService,connectTimeoutInMillis, sslContext) {
                }

                std::vector<SSLSocket::CipherInfo> SSLSocket::getCiphers() {
                    STACK_OF(SSL_CIPHER) *ciphers = SSL_get_ciphers(socket_.native_handle());
                    std::vector<CipherInfo> supportedCiphers;
                    for (int i = 0; i < sk_SSL_CIPHER_num(ciphers); ++i) {
                        struct SSLSocket::CipherInfo info;
                        const SSL_CIPHER *cipher = sk_SSL_CIPHER_value(ciphers, i);
                        info.name = SSL_CIPHER_get_name(cipher);
                        info.numberOfBits = SSL_CIPHER_get_bits(cipher, 0);
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
                        ", Bits:" << info.numberOfBits <<
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
        return std::abs(conn->getConnectionId());
    }
}


