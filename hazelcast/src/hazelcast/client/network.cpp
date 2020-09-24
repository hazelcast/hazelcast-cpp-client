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
        SocketInterceptor::~SocketInterceptor() = default;

        namespace connection {
            constexpr size_t ClientConnectionManagerImpl::EXECUTOR_CORE_POOL_SIZE;
            constexpr int32_t ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
            constexpr int32_t ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC;

            ClientConnectionManagerImpl::ClientConnectionManagerImpl(spi::ClientContext &client,
                                                                     const std::shared_ptr<AddressTranslator> &addressTranslator,
                                                                     const std::vector<std::shared_ptr<AddressProvider> > &addressProviders)
                    : alive_(false), logger_(client.getLogger()), connection_timeout_millis_(std::chrono::milliseconds::max()),
                      client_(client),
                      socket_interceptor_(client.getClientConfig().getSocketInterceptor()),
                      execution_service_(client.getClientExecutionService()),
                      translator_(addressTranslator), current_credentials_(nullptr), connection_id_gen_(0),
                      heartbeat_(client, *this), partition_count_(-1),
                      async_start_(client.getClientConfig().getConnectionStrategyConfig().isAsyncStart()),
                      reconnect_mode_(client.getClientConfig().getConnectionStrategyConfig().getReconnectMode()),
                      smart_routing_enabled_(client.getClientConfig().getNetworkConfig().isSmartRouting()),
                      connect_to_cluster_task_submitted_(false),
                      client_uuid_(client.random_uuid()),
                      authentication_timeout_(heartbeat_.getHeartbeatTimeout().count()),
                      cluster_id_(boost::uuids::nil_uuid()),
                      load_balancer_(client.getClientConfig().getLoadBalancer()) {
                config::ClientNetworkConfig &networkConfig = client.getClientConfig().getNetworkConfig();
                int64_t connTimeout = networkConfig.getConnectionTimeout();
                if (connTimeout > 0) {
                    connection_timeout_millis_ = std::chrono::milliseconds(connTimeout);
                }

                ClientProperties &clientProperties = client.getClientProperties();
                shuffle_member_list_ = clientProperties.getBoolean(clientProperties.getShuffleMemberList());

                ClientConnectionManagerImpl::address_providers_ = addressProviders;

                connection_attempt_period_ = networkConfig.getConnectionAttemptPeriod();

                int connAttemptLimit = networkConfig.getConnectionAttemptLimit();
                bool isAsync = client.getClientConfig().getConnectionStrategyConfig().isAsyncStart();

                if (connAttemptLimit < 0) {
                    this->connection_attempt_limit_ = isAsync ? DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC
                                                           : DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
                } else {
                    this->connection_attempt_limit_ = connAttemptLimit == 0 ? INT32_MAX : connAttemptLimit;
                }

                io_thread_count_ = clientProperties.getInteger(clientProperties.getIOThreadCount());
            }

            bool ClientConnectionManagerImpl::start() {
                bool expected = false;
                if (!alive_.compare_exchange_strong(expected, true)) {
                    return false;
                }

                io_context_.reset(new boost::asio::io_context);
                io_resolver_.reset(new boost::asio::ip::tcp::resolver(io_context_->get_executor()));
                socket_factory_.reset(new internal::socket::SocketFactory(client_, *io_context_, *io_resolver_));
                io_guard_.reset(new boost::asio::io_context::work(*io_context_));

                if (!socket_factory_->start()) {
                    return false;
                }

                socket_interceptor_ = client_.getClientConfig().getSocketInterceptor();

                for (int j = 0; j < io_thread_count_; ++j) {
                    io_threads_.emplace_back([=]() { io_context_->run(); });
                }

                executor_.reset(new hazelcast::util::hz_thread_pool(EXECUTOR_CORE_POOL_SIZE));
                connect_to_members_timer_ = boost::asio::steady_timer(executor_->get_executor());

                heartbeat_.start();

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

                    if (!client_.getLifecycleService().isRunning()) {
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
                for (auto &connection : active_connections_.values()) {
                    // prevent any exceptions
                    util::IOUtil::closeResource(connection.get(), "Hazelcast client is shutting down");
                }

                spi::impl::ClientExecutionServiceImpl::shutdownThreadPool(executor_.get());

                io_guard_.reset();
                io_context_->stop();
                boost::asio::use_service<boost::asio::detail::resolver_service<boost::asio::ip::tcp>>(*io_context_).shutdown();
                std::for_each(io_threads_.begin(), io_threads_.end(), [](std::thread &t) { t.join(); });

                connection_listeners_.clear();
                active_connections_.clear();
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

                auto target = translator_->translate(address);
                connection = std::make_shared<Connection>(target, client_, ++connection_id_gen_,
                                                          *socket_factory_, *this, connection_timeout_millis_);
                connection->connect();

                // call the interceptor from user thread
                if (socket_interceptor_) {
                    socket_interceptor_->onConnect(connection->getSocket());
                }

                authenticate_on_cluster(connection);

                return connection;
            }

            std::vector<std::shared_ptr<Connection> > ClientConnectionManagerImpl::getActiveConnections() {
                return active_connections_.values();
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getConnection(const Address &address) {
                for (const auto &connection : active_connections_.values()) {
                    auto remote_address = connection->getRemoteAddress();
                    if (remote_address && *remote_address == address) {
                        return connection;
                    }
                }
                return nullptr;
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getConnection(boost::uuids::uuid uuid) {
                return active_connections_.get(uuid);
            }

            void
            ClientConnectionManagerImpl::authenticate_on_cluster(std::shared_ptr<Connection> &connection) {
                auto request = encodeAuthenticationRequest(client_.getSerializationService());
                auto clientInvocation = spi::impl::ClientInvocation::create(client_, request, "", connection);
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
                auto cluster_name = client_.getClientConfig().getClusterName();

                auto credential = current_credentials_.load();
                if (!credential) {
                    GroupConfig &groupConfig = client_.getClientConfig().getGroupConfig();
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
                                                                             client_.getName(), labels_);
                    }
                    case security::credentials::type::secret:
                    case security::credentials::token:
                    {
                        auto cr = boost::static_pointer_cast<security::token_credentials>(credential);
                        return protocol::codec::client_authenticationcustom_encode(cluster_name, cr->get_secret(),
                                                                                   client_uuid_, protocol::ClientTypes::CPP,
                                                                                   serializationVersion, HAZELCAST_VERSION,
                                                                                   client_.getName(), labels_);
                    }
                }
                assert(0);
                return protocol::ClientMessage();
            }

            void
            ClientConnectionManagerImpl::fireConnectionAddedEvent(const std::shared_ptr<Connection> &connection) {
                for (const std::shared_ptr<ConnectionListener> &connectionListener : connection_listeners_.toArray()) {
                    connectionListener->connectionAdded(connection);
                }
            }

            void
            ClientConnectionManagerImpl::fireConnectionRemovedEvent(const std::shared_ptr<Connection> &connection) {
                for (const auto &listener : connection_listeners_.toArray()) {
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
                        if (active_connections_.empty()) {
                            if (logger_.isFinestEnabled()) {
                                boost::uuids::uuid cluster_id = cluster_id_;
                                logger_.warning("No connection to cluster: ", cluster_id);
                            }

                            submit_connect_to_cluster_task();
                        }

                    } catch (exception::IException &e) {
                        logger_.warning("Could not connect to any cluster, shutting down the client: ", e);
                        shutdownWithExternalThread(client_.getHazelcastClientImplementation());
                    }
                });
            }

            void ClientConnectionManagerImpl::connect_to_all_members() {
                if (!client_.getLifecycleService().isRunning()) {
                    return;
                }

                auto connecting_addresses = std::make_shared<util::sync_associative_container<std::unordered_set<Address>>>();
                for (const auto &member : client_.getClientClusterService().getMemberList()) {
                    const auto& address = member.getAddress();

                    if (client_.getLifecycleService().isRunning() && !getConnection(address)
                        && connecting_addresses->insert(address).second) {
                        // submit a task for this address only if there is no
                        // another connection attempt for it
                        Address addr = address;
                        boost::asio::post(executor_->get_executor(), [=] () {
                            try {
                                if (!client_.getLifecycleService().isRunning()) {
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

                while (attempt < connection_attempt_limit_) {
                    attempt++;
                    int64_t nextTry = util::currentTimeMillis() + connection_attempt_period_;

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

                    if (attempt < connection_attempt_limit_) {
                        const int64_t remainingTime = nextTry - util::currentTimeMillis();
                        logger_.warning("Unable to get alive cluster connection, try in ",
                                       (remainingTime > 0 ? remainingTime : 0), " ms later, attempt ", attempt,
                                       " of ", connection_attempt_limit_, ".");

                        if (remainingTime > 0) {
                            // TODO use a condition variable here
                            std::this_thread::sleep_for(std::chrono::milliseconds(remainingTime));
                        }
                    } else {
                        logger_.warning("Unable to get alive cluster connection, attempt ", attempt, " of ",
                                       connection_attempt_limit_, ".");
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
                for (auto &&member : client_.getClientClusterService().getMemberList()) {
                    addresses.emplace_back(std::move(member.getAddress()));
                }

                if (shuffle_member_list_) {
                    shuffle(addresses);
                }

                std::vector<Address> provided_addresses;
                for (auto &addressProvider : address_providers_) {
                    auto addrList = addressProvider->loadAddresses();
                    provided_addresses.insert(provided_addresses.end(), addrList.begin(), addrList.end());
                }

                if (shuffle_member_list_) {
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
                return alive_;
            }

            void ClientConnectionManagerImpl::on_connection_close(Connection &connection, std::exception_ptr cause) {
                auto endpoint = connection.getRemoteAddress();
                auto member_uuid = connection.getRemoteUuid();

                auto socket_remote_address = connection.getSocket().getRemoteEndpoint();

                if (!endpoint) {
                    if (logger_.isFinestEnabled()) {
                        logger_.finest("Destroying ", connection, ", but it has end-point set to null ",
                                      "-> not removing it from a connection map");
                    }
                    return;
                }

                auto conn = connection.shared_from_this();
                if (active_connections_.remove(member_uuid, conn)) {
                    logger_.info("Removed connection to endpoint: ", *endpoint, ", connection: ", connection);
                    if (active_connections_.empty()) {
                        fire_life_cycle_event(LifecycleEvent::LifecycleState::CLIENT_DISCONNECTED);

                        trigger_cluster_reconnection();
                    }

                    fireConnectionRemovedEvent(connection.shared_from_this());
                } else {
                    if (logger_.isFinestEnabled()) {
                        logger_.finest("Destroying a connection, but there is no mapping ", endpoint, " -> ",
                                      connection, " in the connection map.");
                    }
                }
            }

            void
            ClientConnectionManagerImpl::addConnectionListener(
                    const std::shared_ptr<ConnectionListener> &connectionListener) {
                connection_listeners_.add(connectionListener);
            }

            ClientConnectionManagerImpl::~ClientConnectionManagerImpl() {
                shutdown();
            }

            util::ILogger &ClientConnectionManagerImpl::getLogger() {
                return client_.getLogger();
            }

            void ClientConnectionManagerImpl::check_client_active() {
                if (!client_.getLifecycleService().isRunning()) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastClientNotActiveException(
                            "ClientConnectionManagerImpl::check_client_active", "Client is shutdown"));
                }
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::connect(const Address &address) {
                try {
                    logger_.info("Trying to connect to ", address);
                    return getOrConnect(address);
                } catch (std::exception &e) {
                    logger_.warning("Exception during initial connection to ", address, ": ", e.what());
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

                if (logger_.isFinestEnabled()) {
                    logger_.finest("Checking the cluster: ", new_cluster_id, ", current cluster: ", current_cluster_id);
                }

                auto initial_connection = active_connections_.empty();
                auto changedCluster = initial_connection && !current_cluster_id.is_nil() && !(new_cluster_id == current_cluster_id);
                if (changedCluster) {
                    logger_.warning("Switching from current cluster: ", current_cluster_id, " to new cluster: ", new_cluster_id);
                    client_.getHazelcastClientImplementation()->on_cluster_restart();
                }

                active_connections_.put(response.member_uuid, connection);

                if (initial_connection) {
                    cluster_id_ = new_cluster_id;
                    fire_life_cycle_event(LifecycleEvent::LifecycleState::CLIENT_CONNECTED);
                }

                auto local_address = connection->getLocalSocketAddress();
                if (local_address) {
                    logger_.info("Authenticated with server ", response.address , ":", response.member_uuid
                            , ", server version: " , response.server_version
                            , ", local address: ", *local_address);
                } else {
                    logger_.info("Authenticated with server ", response.address , ":", response.member_uuid
                            , ", server version: " , response.server_version
                            , ", no local address (connection disconnected ?)");
                }

                fireConnectionAddedEvent(connection);
            }

            void ClientConnectionManagerImpl::fire_life_cycle_event(LifecycleEvent::LifecycleState state) {
                client_.getLifecycleService().fireLifecycleEvent(state);
            }

            void ClientConnectionManagerImpl::check_partition_count(int32_t new_partition_count) {
                auto &partition_service = static_cast<spi::impl::ClientPartitionServiceImpl &>(client_.getPartitionService());
                if (!partition_service.check_and_set_partition_count(new_partition_count)) {
                    BOOST_THROW_EXCEPTION(exception::ClientNotAllowedInClusterException("ClientConnectionManagerImpl::check_partition_count",
                          (boost::format("Client can not work with this cluster because it has a different partition count. "
                                         "Expected partition count: %1%, Member partition count: %2%")
                                         %partition_service.getPartitionCount() %new_partition_count).str()));
                }
            }

            void ClientConnectionManagerImpl::trigger_cluster_reconnection() {
                if (reconnect_mode_ == config::ClientConnectionStrategyConfig::ReconnectMode::OFF) {
                    logger_.info("RECONNECT MODE is off. Shutting down the client.");
                    shutdownWithExternalThread(client_.getHazelcastClientImplementation());
                    return;
                }

                if (client_.getLifecycleService().isRunning()) {
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

                auto connections = active_connections_.values();
                if (connections.empty()) {
                    return nullptr;
                }

                return connections[0];
            }

            boost::uuids::uuid ClientConnectionManagerImpl::getClientUuid() const {
                return client_uuid_;
            }

            void ClientConnectionManagerImpl::check_invocation_allowed() {
                if (active_connections_.size() > 0) {
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

                for (const auto &member : client_.getClientClusterService().getMemberList()) {
                    try {
                        getOrConnect(member.getAddress());
                    } catch (std::exception &) {
                        // ignore
                    }
                }
            }

            ReadHandler::ReadHandler(Connection &connection, size_t bufferSize)
                    : buffer(new char[bufferSize]), byteBuffer(buffer, bufferSize), builder_(connection),
                      last_read_time_duration_(std::chrono::steady_clock::now().time_since_epoch()) {
            }

            ReadHandler::~ReadHandler() {
                delete[] buffer;
            }

            void ReadHandler::handle() {
                last_read_time_duration_ = std::chrono::steady_clock::now().time_since_epoch();

                if (byteBuffer.position() == 0)
                    return;

                byteBuffer.flip();

                // it is important to check the onData return value since there may be left data less than a message
                // header size, and this may cause an infinite loop.
                while (byteBuffer.hasRemaining() && builder_.onData(byteBuffer)) {
                }

                if (byteBuffer.hasRemaining()) {
                    byteBuffer.compact();
                } else {
                    byteBuffer.clear();
                }
            }

            std::chrono::steady_clock::time_point ReadHandler::getLastReadTime() const {
                return std::chrono::steady_clock::time_point(last_read_time_duration_);
            }

            Connection::Connection(const Address &address, spi::ClientContext &clientContext, int connectionId, // NOLINT(cppcoreguidelines-pro-type-member-init)
                                   internal::socket::SocketFactory &socketFactory,
                                   ClientConnectionManagerImpl &clientConnectionManager,
                                   std::chrono::steady_clock::duration &connectTimeoutInMillis)
                    : readHandler(*this, 16 << 10),
                      start_time_(std::chrono::steady_clock::now()),
                      closed_time_duration_(),
                      client_context_(clientContext),
                      invocation_service_(clientContext.getInvocationService()),
                      connection_id_(connectionId),
                      remote_uuid_(boost::uuids::nil_uuid()), logger_(clientContext.getLogger()), alive_(true) {
                socket_ = socketFactory.create(address, connectTimeoutInMillis);
            }

            Connection::~Connection() = default;

            void Connection::connect() {
                socket_->connect(shared_from_this());
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

                closed_time_duration_.store(std::chrono::steady_clock::now().time_since_epoch());

                close_cause_ = cause;
                close_reason_ = reason;

                logClose();

                try {
                    innerClose();
                } catch (exception::IException &e) {
                    client_context_.getLogger().warning("Exception while closing connection", e.getMessage());
                }

                client_context_.getConnectionManager().on_connection_close(*this, close_cause_);

                auto thisConnection = shared_from_this();
                boost::asio::post(socket_->get_executor(), [=]() {
                    for (auto &invocationEntry : invocations) {
                        invocationEntry.second->notifyException(std::make_exception_ptr(boost::enable_current_exception(
                                exception::TargetDisconnectedException("Connection::close",
                                                                       thisConnection->getCloseReason()))));
                    }
                });
            }

            void Connection::write(const std::shared_ptr<spi::impl::ClientInvocation> &clientInvocation) {
                socket_->asyncWrite(shared_from_this(), clientInvocation);
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
                    logger_.warning("No invocation' for callId: ", correlationId, ". Dropping this message: ", *message);
                    return;
                }
                auto invocation = invocationIterator->second;
                if (!invocation->getEventHandler()) {
                    // erase only for non-event messages
                    invocations.erase(invocationIterator);
                }
                if (message->is_flag_set(message->getHeaderFlags(), protocol::ClientMessage::IS_EVENT_FLAG)) {
                    client_context_.getClientListenerService().handleClientMessage(invocation, message);
                } else {
                    invocation_service_.handleClientMessage(invocation, message);
                }
            }

            int Connection::getConnectionId() const {
                return connection_id_;
            }

            bool Connection::isAlive() const {
                return alive_;
            }

            const std::string &Connection::getCloseReason() const {
                return close_reason_;
            }

            void Connection::logClose() {
                std::ostringstream message;
                message << *this << " closed. Reason: ";
                if (!close_reason_.empty()) {
                    message << close_reason_;
                } else if (close_cause_) {
                    try {
                        std::rethrow_exception(close_cause_);
                    } catch (exception::IException &ie) {
                        message << ie.getSource() << "[" + ie.getMessage() << "]";
                    }
                } else {
                    message << "Socket explicitly closed";
                }

                if (client_context_.getLifecycleService().isRunning()) {
                    if (!close_cause_) {
                        logger_.info(message.str());
                    } else {
                        try {
                            std::rethrow_exception(close_cause_);
                        } catch (exception::IException &ie) {
                            logger_.warning(message.str(), ie);
                        }
                    }
                } else {
                    if (logger_.isFinestEnabled()) {
                        if (!close_cause_) {
                            logger_.finest(message.str());
                        } else {
                            try {
                                std::rethrow_exception(close_cause_);
                            } catch (exception::IException &ie) {
                                logger_.finest(message.str(), ie);
                            }
                        }
                    }
                }
            }

            bool Connection::operator==(const Connection &rhs) const {
                return connection_id_ == rhs.connection_id_;
            }

            bool Connection::operator!=(const Connection &rhs) const {
                return !(rhs == *this);
            }

            const std::string &Connection::getConnectedServerVersionString() const {
                return connected_server_version_string_;
            }

            void Connection::setConnectedServerVersion(const std::string &connectedServer) {
                Connection::connected_server_version_string_ = connectedServer;
            }

            boost::optional<Address> Connection::getLocalSocketAddress() const {
                return socket_->localSocketAddress();
            }

            std::chrono::steady_clock::time_point Connection::lastReadTime() const {
                return readHandler.getLastReadTime();
            }

            void Connection::innerClose() {
                if (!socket_) {
                    return;
                }

                auto thisConnection = shared_from_this();
                boost::asio::post(socket_->get_executor(), [=] () { thisConnection->socket_->close(); });
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
                   << ", closedTime=" << util::StringUtil::timeToString(std::chrono::steady_clock::time_point(connection.closed_time_duration_))
                   << ", connected server version=" << connection.connected_server_version_string_
                   << '}';

                return os;
            }

            bool Connection::operator<(const Connection &rhs) const {
                return connection_id_ < rhs.connection_id_;
            }

            std::chrono::steady_clock::time_point Connection::getStartTime() const {
                return start_time_;
            }

            Socket &Connection::getSocket() {
                return *socket_;
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
                    : client_(client), client_connection_manager_(connectionManager), logger_(client.getLogger()) {
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
                spi::impl::ClientExecutionServiceImpl &clientExecutionService = client_.getClientExecutionService();

                timer_ = clientExecutionService.scheduleWithRepetition([=]() {
                    if (!client_connection_manager_.isAlive()) {
                        return;
                    }

                    for (auto &connection : client_connection_manager_.getActiveConnections()) {
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
                        logger_.warning("Heartbeat failed over the connection: ", *connection);
                        onHeartbeatStopped(connection, "Heartbeat timed out");
                    }
                }

                if (now - connection->lastReadTime() > heartbeat_interval_) {
                    auto request = protocol::codec::client_ping_encode();
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client_, request, "", connection);
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
                if (timer_) {
                    boost::system::error_code ignored;
                    timer_->cancel(ignored);
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
                        : client_context_(clientContext), io_(io), io_resolver_(resolver) {
                }

                bool SocketFactory::start() {
#ifdef HZ_BUILD_WITH_SSL
                    const client::config::SSLConfig &sslConfig = client_context_.getClientConfig().getNetworkConfig().getSSLConfig();
                    if (sslConfig.isEnabled()) {
                        ssl_context_ = std::unique_ptr<boost::asio::ssl::context>(new boost::asio::ssl::context(
                                (boost::asio::ssl::context_base::method) sslConfig.getProtocol()));

                        const std::vector<std::string> &verifyFiles = sslConfig.getVerifyFiles();
                        bool success = true;
                        util::ILogger &logger = client_context_.getLogger();
                        for (std::vector<std::string>::const_iterator it = verifyFiles.begin(); it != verifyFiles.end();
                             ++it) {
                            boost::system::error_code ec;
                            ssl_context_->load_verify_file(*it, ec);
                            if (ec) {
                                logger.warning(
                                        std::string("SocketFactory::start: Failed to load CA "
                                                    "verify file at ") + *it + " "
                                        + ec.message());
                                success = false;
                            }
                        }

                        if (!success) {
                            ssl_context_.reset();
                            logger.warning("SocketFactory::start: Failed to load one or more "
                                           "configured CA verify files (PEM files). Please "
                                           "correct the files and retry.");
                            return false;
                        }

                        // set cipher list if the list is set
                        const std::string &cipherList = sslConfig.getCipherList();
                        if (!cipherList.empty()) {
                            if (!SSL_CTX_set_cipher_list(ssl_context_->native_handle(), cipherList.c_str())) {
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
                    if (ssl_context_.get()) {
                        return std::unique_ptr<Socket>(new internal::socket::SSLSocket(io_, *ssl_context_, address,
                                                                                       client_context_.getClientConfig().getNetworkConfig().getSocketOptions(),
                                                                                       connectTimeoutInMillis, io_resolver_));
                    }
#endif

                    return std::unique_ptr<Socket>(new internal::socket::TcpSocket(io_, address,
                                                                                   client_context_.getClientConfig().getNetworkConfig().getSocketOptions(),
                                                                                   connectTimeoutInMillis, io_resolver_));
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


