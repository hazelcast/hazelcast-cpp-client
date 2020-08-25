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
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/connection/ConnectionFuture.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/spi/impl/listener/listener_service_impl.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/client/impl/BuildInfo.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/config/SSLConfig.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/sync_unoredered_set.h"

namespace hazelcast {
    namespace client {
        SocketInterceptor::~SocketInterceptor() = default;

        namespace connection {
            constexpr size_t ClientConnectionManagerImpl::EXECUTOR_CORE_POOL_SIZE;
            constexpr int32_t ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
            constexpr int32_t ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC;

            ClientConnectionManagerImpl::ClientConnectionManagerImpl(spi::ClientContext &client,
                                                                     const std::shared_ptr<AddressTranslator> &addressTranslator,
                                                                     const std::vector<std::shared_ptr<AddressProvider> > &addressProviders)
                    : alive(false), logger(client.getLogger()), connectionTimeoutMillis(std::chrono::milliseconds::max()),
                      client(client),
                      socketInterceptor(client.getClientConfig().getSocketInterceptor()),
                      executionService(client.getClientExecutionService()),
                      translator(addressTranslator), current_credentials_(nullptr), connectionIdGen(0),
                      heartbeat(client, *this), partition_count_(-1),
                      async_start_(client.getClientConfig().getConnectionStrategyConfig().isAsyncStart()),
                      reconnect_mode_(client.getClientConfig().getConnectionStrategyConfig().getReconnectMode()),
                      smart_routing_enabled_(client.getClientConfig().getNetworkConfig().isSmartRouting()),
                      connect_to_cluster_task_submitted_(false),
                      client_uuid_(boost::uuids::random_generator()()),
                      authentication_timeout_(heartbeat.getHeartbeatTimeout().count()),
                      load_balancer_(client.getClientConfig().getLoadBalancer()) {
                boost::uuids::uuid u;
                std::memset(&u, 0, sizeof(boost::uuids::uuid));
                cluster_id_ = u;
                config::ClientNetworkConfig &networkConfig = client.getClientConfig().getNetworkConfig();
                int64_t connTimeout = networkConfig.getConnectionTimeout();
                if (connTimeout > 0) {
                    connectionTimeoutMillis = std::chrono::milliseconds(connTimeout);
                }

                current_credentials_ = client.getClientConfig().credentials_;

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

                return true;
            }

            void ClientConnectionManagerImpl::schedule_connect_to_all_members() {
                connect_to_members_timer_->expires_from_now(boost::asio::chrono::seconds(1));
                connect_to_members_timer_->async_wait([=](boost::system::error_code ec) {
                    if (ec == boost::asio::error::operation_aborted) {
                        return;
                    }
                    connect_to_all_members();

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

                // let the waiting authentication futures not block anymore
                for (auto &f : connectionsInProgress.values()) {
                    f->onFailure(std::make_exception_ptr(
                            exception::IllegalStateException("ClientConnectionManagerImpl::shutdown",
                                                             "Client is shutting down")));
                }

                // close connections
                for (auto &connection : activeConnections.values()) {
                    // prevent any exceptions
                    util::IOUtil::closeResource(connection.get(), "Hazelcast client is shutting down");
                }

                spi::impl::ClientExecutionServiceImpl::shutdownThreadPool(executor_.get());

                ioGuard.reset();
                ioResolver.reset();
                ioContext->stop();
                boost::asio::use_service<boost::asio::detail::resolver_service<boost::asio::ip::tcp>>(*ioContext).shutdown();
                std::for_each(ioThreads.begin(), ioThreads.end(), [](std::thread &t) { t.join(); });
                ioContext.reset();

                connectionListeners.clear();
                activeConnections.clear();
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getOrConnect(const Address &address) {
                auto connection = getConnection(address);
                if (connection) {
                    return connection;
                }

                auto future = std::make_shared<ConnectionFuture>(address, connection, connectionsInProgress);
                auto f = connectionsInProgress.putIfAbsent(address, future);
                bool outstanding_connection_exists = static_cast<bool>(f);
                if (outstanding_connection_exists) {
                    future = f;
                }
                std::lock_guard<std::mutex> g(future->getLock());
                // double check here
                connection = getConnection(address);
                if (connection) {
                    return connection;
                }
                if (outstanding_connection_exists) {
                    // Outstanding connection seems to be failed. Try connecting only if we could actually insert into
                    // the outstanding connections map
                    return getOrConnect(address);
                }

                auto target = translator->translate(address);
                connection = std::make_shared<Connection>(target, client, ++connectionIdGen, future,
                                                          *socketFactory, *this, connectionTimeoutMillis);
                connection->asyncStart();

                try {
                    connection = future->get();
                } catch (exception::IException &e) {
                    connectionsInProgress.remove(address);
                    throw;
                }

                // call the interceptor from user thread
                if (socketInterceptor) {
                    socketInterceptor->onConnect(connection->getSocket());
                }

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
                                "not received for %1% msecs for %2%") %(boost::chrono::duration_cast<boost::chrono::milliseconds>(
                                authentication_timeout_).count()) %*clientInvocation).str()));
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
                auto cluster_name = client.getClientConfig().getClusterName();

                auto credential = current_credentials_.load();
                if (!credential) {
                    GroupConfig &groupConfig = client.getClientConfig().getGroupConfig();
                    credential = boost::make_shared<security::username_password_credentials>(groupConfig.getName(),
                                                                                             groupConfig.getPassword());
                    current_credentials_.store(
                            boost::static_pointer_cast<security::credentials>(credential));
                }

                switch(credential->get_type()) {
                    case security::credentials::type::username_password:
                    {
                        auto cr = boost::static_pointer_cast<security::username_password_credentials>(credential);
                        const std::string *username = cr->get_name().empty() ? nullptr : &cr->get_name();
                        const std::string *password = cr->get_password().empty() ? nullptr : &cr->get_password();
                        return protocol::codec::client_authentication_encode(cluster_name, username, password,
                                                                             client_uuid_, protocol::ClientTypes::CPP,
                                                                             serializationVersion, HAZELCAST_VERSION,
                                                                             client.getName(), labels_);
                    }
                    case security::credentials::type::secret:
                    case security::credentials::token:
                    {
                        auto cr = boost::static_pointer_cast<security::token_credentials>(credential);
                        return protocol::codec::client_authenticationcustom_encode(cluster_name, cr->get_secret(),
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
                    } catch (exception::IException &exception) {
                        clientInstance->getLogger()->severe("Exception during client shutdown ",
                                                            clientInstance->getName() + ".clientShutdown-", ":",
                                                            exception);
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
                            if (logger.isFinestEnabled()) {
                                boost::uuids::uuid cluster_id = cluster_id_;
                                logger.warning("No connection to cluster: ", cluster_id);
                            }

                            submit_connect_to_cluster_task();
                        }

                    } catch (exception::IException &e) {
                        logger.warning("Could not connect to any cluster, shutting down the client: ", e);
                        shutdownWithExternalThread(client.getHazelcastClientImplementation());
                    }
                });
            }

            void ClientConnectionManagerImpl::connect_to_all_members() {
                if (!client.getLifecycleService().isRunning()) {
                    return;
                }

                auto connecting_addresses = std::make_shared<util::sync_unordered_set<Address>>();
                for (const auto &member : client.getClientClusterService().getMemberList()) {
                    const auto& address = member.getAddress();

                    if (client.getLifecycleService().isRunning() && !getConnection(address)
                        && connecting_addresses->insert(address).second) {
                        // submit a task for this address only if there is no
                        // another connection attempt for it
                        boost::asio::post([=] () {
                            try {
                                if (!client.getLifecycleService().isRunning()) {
                                    return;
                                }
                                if (!getConnection(member.getUuid())) {
                                    getOrConnect(address);
                                }
                                connecting_addresses->erase(address);
                            } catch (exception::IException &) {
                                connecting_addresses->erase(address);
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
                    int64_t nextTry = util::currentTimeMillis() + connectionAttemptPeriod;

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
                        const int64_t remainingTime = nextTry - util::currentTimeMillis();
                        logger.warning("Unable to get alive cluster connection, try in ",
                                       (remainingTime > 0 ? remainingTime : 0), " ms later, attempt ", attempt,
                                       " of ", connectionAttemptLimit, ".");

                        if (remainingTime > 0) {
                            // TODO use a condition variable here
                            std::this_thread::sleep_for(std::chrono::milliseconds(remainingTime));
                        }
                    } else {
                        logger.warning("Unable to get alive cluster connection, attempt ", attempt, " of ",
                                       connectionAttemptLimit, ".");
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

            std::unordered_set<Address> ClientConnectionManagerImpl::getPossibleMemberAddresses() {
                std::vector<Address> addresses;
                for (auto &&member : client.getClientClusterService().getMemberList()) {
                    addresses.emplace_back(member.getAddress());
                }

                if (shuffleMemberList) {
                    shuffle(addresses);
                }

                for (auto &addressProvider : addressProviders) {
                    auto addrList = addressProvider->loadAddresses();
                    addresses.insert(addresses.end(), addrList.begin(), addrList.end());
                }

                if (shuffleMemberList) {
                    shuffle(addresses);
                }

                std::unordered_set<Address> result;
                for (auto &&a : std::move(addresses)) {
                    result.insert(a);
                }
                return result;
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
                auto conn_future = connectionsInProgress.remove(socket_remote_address);
                if (conn_future) {
                    conn_future->onFailure(cause);
                }

                if (!endpoint) {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Destroying ", connection, ", but it has end-point set to null ",
                                      "-> not removing it from a connection map");
                    }
                    return;
                }

                auto conn = connection.shared_from_this();
                if (activeConnections.remove(member_uuid, conn)) {
                    logger.info("Removed connection to endpoint: ", *endpoint, ", connection: ", connection);
                    if (activeConnections.empty()) {
                        fire_life_cycle_event(LifecycleEvent::LifecycleState::CLIENT_DISCONNECTED);

                        trigger_cluster_reconnection();
                    }

                    fireConnectionRemovedEvent(connection.shared_from_this());
                } else {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Destroying a connection, but there is no mapping ", endpoint, " -> ",
                                      connection, " in the connection map.");
                    }
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

            util::ILogger &ClientConnectionManagerImpl::getLogger() {
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
                    logger.info("Trying to connect to ", address);
                    return getOrConnect(address);
                } catch (exception::IException &e) {
                    logger.warning("Exception during initial connection to ", address, ": ", e);
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

                if (logger.isFinestEnabled()) {
                    logger.finest("Checking the cluster: ", new_cluster_id, ", current cluster: ", current_cluster_id);
                }

                auto initial_connection = activeConnections.empty();
                auto changedCluster = initial_connection && !current_cluster_id.is_nil() && !(new_cluster_id == current_cluster_id);
                if (changedCluster) {
                    logger.warning("Switching from current cluster: ", current_cluster_id, " to new cluster: ", new_cluster_id);
                    client.getHazelcastClientImplementation()->on_cluster_restart();
                }

                activeConnections.put(response.member_uuid, connection);

                if (initial_connection) {
                    cluster_id_ = new_cluster_id;
                    fire_life_cycle_event(LifecycleEvent::LifecycleState::CLIENT_CONNECTED);
                }

                auto local_address = connection->getLocalSocketAddress();
                if (local_address) {
                    logger.info("Authenticated with server ", response.address , ":", response.member_uuid
                            , ", server version: " , response.server_version
                            , ", local address: ", *local_address);
                } else {
                    logger.info("Authenticated with server ", response.address , ":", response.member_uuid
                            , ", server version: " , response.server_version
                            , ", no local address (connection disconnected ?)");
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
                    logger.info("RECONNECT MODE is off. Shutting down the client.");
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

            boost::shared_ptr<security::credentials> ClientConnectionManagerImpl::getCurrentCredentials() const {
                return current_credentials_.load();
            }

            void ClientConnectionManagerImpl::connect_to_all_cluster_members() {
                if (!smart_routing_enabled_) {
                    return;
                }

                for (const auto &member : client.getClientClusterService().getMemberList()) {
                    try {
                        getOrConnect(member.getAddress());
                    } catch (exception::IException &) {
                        // ignore
                    }
                }
            }

            ConnectionFuture::ConnectionFuture(Address address,
                                               std::shared_ptr<Connection> connectionInProgress,
                                               util::SynchronizedMap<Address, ConnectionFuture> &connectionsInProgress)
                    : address_(std::move(address)), connection_in_progress_(std::move(connectionInProgress)),
                      connections_in_progress_(connectionsInProgress) {}

            const std::shared_ptr<Connection> &ConnectionFuture::getConnectionInProgress() const {
                return connection_in_progress_;
            }

            std::mutex &ConnectionFuture::getLock() {
                return lock_;
            }

            void ConnectionFuture::onSuccess(const std::shared_ptr<Connection> &conn) {
                if (connections_in_progress_.remove(address_)) {
                    promise_.set_value(conn);
                }
            }

            void ConnectionFuture::onFailure(std::exception_ptr t) {
                util::IOUtil::closeResource(connection_in_progress_.get(),
                                            (boost::format("Connection to address %1% failed.") %
                                             address_).str().c_str());
                if (connections_in_progress_.remove(address_)) {
                    promise_.set_exception(t);
                }
            }

            std::shared_ptr<Connection> ConnectionFuture::get() {
                return promise_.get_future().get();
            }

            ReadHandler::ReadHandler(Connection &connection, size_t bufferSize)
                    : buffer(new char[bufferSize]), byteBuffer(buffer, bufferSize), builder(connection),
                      lastReadTimeDuration(std::chrono::steady_clock::now().time_since_epoch()) {
            }

            ReadHandler::~ReadHandler() {
                delete[] buffer;
            }

            void ReadHandler::handle() {
                lastReadTimeDuration = std::chrono::steady_clock::now().time_since_epoch();

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
                return std::chrono::steady_clock::time_point(lastReadTimeDuration);
            }

            Connection::Connection(const Address &address, spi::ClientContext &clientContext, int connectionId, // NOLINT(cppcoreguidelines-pro-type-member-init)
                                   const std::shared_ptr<ConnectionFuture> &authFuture,
                                   internal::socket::SocketFactory &socketFactory,
                                   ClientConnectionManagerImpl &clientConnectionManager,
                                   std::chrono::steady_clock::duration &connectTimeoutInMillis)
                    : readHandler(*this, 16 << 10),
                      startTime(std::chrono::steady_clock::now()),
                      closedTimeDuration(),
                      clientContext(clientContext),
                      invocationService(clientContext.getInvocationService()),
                      connectionId(connectionId),
                      connectedServerVersion(impl::BuildInfo::UNKNOWN_HAZELCAST_VERSION),
                      logger(clientContext.getLogger()), alive(true), authFuture(authFuture) {
                socket = socketFactory.create(address, connectTimeoutInMillis);
                std::memset(&remote_uuid_, 0, sizeof(boost::uuids::uuid));
            }

            Connection::~Connection() = default;

            void Connection::asyncStart() {
                socket->asyncStart(shared_from_this(), authFuture);
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

                closedTimeDuration.store(std::chrono::steady_clock::now().time_since_epoch());

                closeCause = cause;
                closeReason = reason;

                logClose();

                try {
                    innerClose();
                } catch (exception::IException &e) {
                    clientContext.getLogger().warning("Exception while closing connection", e.getMessage());
                }

                clientContext.getConnectionManager().on_connection_close(*this, closeCause);

                auto thisConnection = shared_from_this();
                boost::asio::post(socket->get_executor(), [=]() {
                    for (auto &invocationEntry : invocations) {
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
                    logger.warning("No invocation' for callId: ", correlationId, ". Dropping this message: ", *message);
                    return;
                }
                auto invocation = invocationIterator->second;
                if (!invocation->getEventHandler()) {
                    // erase only for non-event messages
                    invocations.erase(invocationIterator);
                }
                if (message->is_flag_set(message->getHeaderFlags(), protocol::ClientMessage::IS_EVENT_FLAG)) {
                    clientContext.getClientListenerService().handleClientMessage(invocation, message);
                } else {
                    invocationService.handleClientMessage(invocation, message);
                }
            }

            int Connection::getConnectionId() const {
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
                        logger.info(message.str());
                    } else {
                        try {
                            std::rethrow_exception(closeCause);
                        } catch (exception::IException &ie) {
                            logger.warning(message.str(), ie);
                        }
                    }
                } else {
                    if (logger.isFinestEnabled()) {
                        if (!closeCause) {
                            logger.finest(message.str());
                        } else {
                            try {
                                std::rethrow_exception(closeCause);
                            } catch (exception::IException &ie) {
                                logger.finest(message.str(), ie);
                            }
                        }
                    }
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
                connectedServerVersion = impl::BuildInfo::calculateVersion(connectedServer);
            }

            int Connection::getConnectedServerVersion() const {
                return connectedServerVersion;
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
                   << ", closedTime=" << util::StringUtil::timeToString(std::chrono::steady_clock::time_point(connection.closedTimeDuration))
                   << ", connected server version=" << connection.connectedServerVersionString
                   << '}';

                return os;
            }

            bool Connection::operator<(const Connection &rhs) const {
                return connectionId < rhs.connectionId;
            }

            std::chrono::steady_clock::time_point Connection::getStartTime() const {
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
                    : client(client), clientConnectionManager(connectionManager), logger(client.getLogger()) {
                ClientProperties &clientProperties = client.getClientProperties();
                auto timeout_millis = clientProperties.getLong(clientProperties.getHeartbeatTimeout());
                heartbeat_timeout_ = std::chrono::milliseconds(
                        timeout_millis > 0 ? timeout_millis : util::IOUtil::to_value<int64_t>(
                                ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT));

                int interval_millis = clientProperties.getLong(clientProperties.getHeartbeatInterval());
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
                    if (connection->isAlive()) {
                        logger.warning("Heartbeat failed over the connection: ", *connection);
                        onHeartbeatStopped(connection, "Heartbeat timed out");
                    }
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

            std::chrono::steady_clock::duration HeartbeatManager::getHeartbeatTimeout() const {
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
                        util::ILogger &logger = clientContext.getLogger();
                        for (std::vector<std::string>::const_iterator it = verifyFiles.begin(); it != verifyFiles.end();
                             ++it) {
                            boost::system::error_code ec;
                            sslContext->load_verify_file(*it, ec);
                            if (ec) {
                                logger.warning(
                                        std::string("SocketFactory::start: Failed to load CA "
                                                    "verify file at ") + *it + " "
                                        + ec.message());
                                success = false;
                            }
                        }

                        if (!success) {
                            sslContext.reset();
                            logger.warning("SocketFactory::start: Failed to load one or more "
                                           "configured CA verify files (PEM files). Please "
                                           "correct the files and retry.");
                            return false;
                        }

                        // set cipher list if the list is set
                        const std::string &cipherList = sslConfig.getCipherList();
                        if (!cipherList.empty()) {
                            if (!SSL_CTX_set_cipher_list(sslContext->native_handle(), cipherList.c_str())) {
                                logger.warning(
                                        std::string("SocketFactory::start: Could not load any "
                                                    "of the ciphers in the config provided "
                                                    "ciphers:") + cipherList);
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
                                                              std::chrono::steady_clock::duration &connectTimeoutInMillis) {
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
                                     std::chrono::steady_clock::duration &connectTimeoutInMillis,
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

                void SSLSocket::async_handle_connect(const std::shared_ptr<connection::Connection> connection,
                                                     const std::shared_ptr<connection::ConnectionFuture> authFuture) {
                    socket_.async_handshake(boost::asio::ssl::stream_base::client,
                                             [=](const boost::system::error_code &ec) {
                                                 if (ec) {
                                                     authFuture->onFailure(
                                                             std::make_exception_ptr(exception::IOException(
                                                                     "Connection::do_connect", (boost::format(
                                                                             "Handshake with server %1% failed. %2%") %
                                                                                                remoteEndpoint %
                                                                                                ec).str())));
                                                     return;
                                                 }

                                                 BaseSocket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>::async_handle_connect(
                                                         connection, authFuture);
                                             });
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
                                     std::chrono::steady_clock::duration &connectTimeoutInMillis,
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


