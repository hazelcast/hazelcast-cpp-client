/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 8/21/13.

#include <boost/foreach.hpp>

#include "hazelcast/client/impl/ExecutionCallback.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/connection/DefaultClientConnectionStrategy.h"
#include "hazelcast/client/connection/AddressProvider.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/protocol/AuthenticationStatus.h"
#include "hazelcast/client/exception/AuthenticationException.h"
#include "hazelcast/client/exception/AuthenticationException.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/client/protocol/codec/ClientAuthenticationCodec.h"
#include "hazelcast/client/protocol/codec/ClientAuthenticationCustomCodec.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/Executor.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/connection/AuthenticationFuture.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/connection/HeartbeatManager.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            int ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC = 2;
            int ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC = 20;

            ClientConnectionManagerImpl::ClientConnectionManagerImpl(spi::ClientContext &client,
                                                                     const boost::shared_ptr<AddressTranslator> &addressTranslator,
                                                                     const std::vector<boost::shared_ptr<AddressProvider> > &addressProviders)
                    : logger(util::ILogger::getLogger()), client(client),
                      socketInterceptor(client.getClientConfig().getSocketInterceptor()), inSelector(*this),
                      outSelector(*this),
                      inSelectorThread(boost::shared_ptr<util::Runnable>(new util::RunnableDelegator(inSelector))),
                      outSelectorThread(boost::shared_ptr<util::Runnable>(new util::RunnableDelegator(outSelector))),
                      executionService(client.getClientExecutionService()),
                      translator(addressTranslator), connectionIdGen(0), socketFactory(client) {
                config::ClientNetworkConfig &networkConfig = client.getClientConfig().getNetworkConfig();

                int64_t connTimeout = networkConfig.getConnectionTimeout();
                connectionTimeoutMillis = connTimeout == 0 ? INT64_MAX : connTimeout;

                credentials = client.getClientConfig().getCredentials();

                connectionStrategy = initializeStrategy(client);

                clusterConnectionExecutor = createSingleThreadExecutorService(client);

                shuffleMemberList = client.getClientProperties().getShuffleMemberList().getBoolean();

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

            }

            bool ClientConnectionManagerImpl::start() {
                util::LockGuard guard(lock);
                if (alive) {
                    return true;
                }
                alive = true;

                bool result = socketFactory.start();

                socketInterceptor = client.getClientConfig().getSocketInterceptor();

                if (!inSelector.start()) {
                    return false;
                }
                if (!outSelector.start()) {
                    return false;
                }

                startEventLoopGroup();
                heartbeat.reset(new HeartbeatManager(client));
                heartbeat->start();
                addConnectionHeartbeatListener(boost::shared_ptr<spi::impl::ConnectionHeartbeatListener>(
                        new spi::impl::ConnectionHeartbeatListenerDelegator(*this)));
                connectionStrategy->start();

                return result;
            }

            void ClientConnectionManagerImpl::shutdown() {
                util::LockGuard guard(lock);
                if (!alive) {
                    return;
                }
                alive = false;

                // close connections
                BOOST_FOREACH(boost::shared_ptr<Connection> connection, activeConnections.values()) {
                                // prevent any exceptions
                                util::IOUtil::closeResource(connection.get(), "Hazelcast client is shutting down");
                            }

                spi::impl::ClientExecutionServiceImpl::shutdownExecutor("cluster", *clusterConnectionExecutor, logger);
                stopEventLoopGroup();
                connectionListeners.clear();
                heartbeat->shutdown();

                connectionStrategy->shutdown();

                activeConnectionsFileDescriptors.clear();
                activeConnections.clear();
                socketConnections.clear();
            }

            boost::shared_ptr<Connection>
            ClientConnectionManagerImpl::getOrConnect(const Address &address) {
                return getOrConnect(address, false);
            }

            boost::shared_ptr<Connection> ClientConnectionManagerImpl::getOwnerConnection() {
                boost::shared_ptr<Address> address = ownerConnectionAddress;
                if (address.get() == NULL) {
                    return boost::shared_ptr<Connection>();
                }
                boost::shared_ptr<Connection> connection = getActiveConnection(*address);
                return connection;

            }

            boost::shared_ptr<Connection> ClientConnectionManagerImpl::connectAsOwner(const Address &address) {
                boost::shared_ptr<Connection> connection;
                try {
                    logger.info() << "Trying to connect to " << address << " as owner member";
                    connection = getOrConnect(address, true);
                    client.onClusterConnect(connection);
                    fireConnectionEvent(LifecycleEvent::CLIENT_CONNECTED);
                    connectionStrategy->onConnectToCluster();
                } catch (exception::IException &e) {
                    logger.warning() << "Exception during initial connection to " << address << ", exception " << e;
                    if (NULL != connection.get()) {
                        std::ostringstream reason;
                        reason << "Could not connect to " << address << " as owner";
                        connection->close(reason.str().c_str(), boost::shared_ptr<exception::IException>(e.clone()));
                    }
                    return boost::shared_ptr<Connection>();
                }
                return connection;
            }

            boost::shared_ptr<Connection>
            ClientConnectionManagerImpl::createSocketConnection(const Address &address) {
                boost::shared_ptr<Connection> conn(
                        new Connection(address, client, ++connectionIdGen, inSelector, outSelector, socketFactory));

                conn->connect(client.getClientConfig().getConnectionTimeout());
                if (socketInterceptor != NULL) {
                    socketInterceptor->onConnect(conn->getSocket());
                }

                return conn;
            }


            std::vector<boost::shared_ptr<Connection> > ClientConnectionManagerImpl::getActiveConnections() {
                return activeConnections.values();
            }

            boost::shared_ptr<Connection> ClientConnectionManagerImpl::getOrTriggerConnect(const Address &target) {
                boost::shared_ptr<Connection> connection = getConnection(target, false);
                if (connection.get() != NULL) {
                    return connection;
                }
                triggerConnect(target, false);
                return boost::shared_ptr<Connection>();
            }

            boost::shared_ptr<Connection>
            ClientConnectionManagerImpl::getConnection(const Address &target, bool asOwner) {
                if (!asOwner) {
                    connectionStrategy->beforeGetConnection(target);
                }
                if (!asOwner && getOwnerConnection().get() == NULL) {
                    throw exception::IOException("ConnectionManager::getConnection",
                                                 "Owner connection is not available!");
                }

                boost::shared_ptr<Connection> connection = activeConnections.get(target);

                if (connection.get() != NULL) {
                    if (!asOwner) {
                        return connection;
                    }
                    if (connection->isAuthenticatedAsOwner()) {
                        return connection;
                    }
                }
                return boost::shared_ptr<Connection>();
            }

            boost::shared_ptr<Connection> ClientConnectionManagerImpl::getActiveConnection(const Address &target) {
                return activeConnections.get(target);
            }

            boost::shared_ptr<Address> ClientConnectionManagerImpl::getOwnerConnectionAddress() {
                return ownerConnectionAddress;
            }

            boost::shared_ptr<AuthenticationFuture>
            ClientConnectionManagerImpl::triggerConnect(const Address &target, bool asOwner) {
                if (!asOwner) {
                    connectionStrategy->beforeOpenConnection(target);
                }
                if (!alive) {
                    throw exception::HazelcastException("ConnectionManager::triggerConnect",
                                                        "ConnectionManager is not active!");
                }

                boost::shared_ptr<AuthenticationFuture> future(new AuthenticationFuture());
                boost::shared_ptr<AuthenticationFuture> oldFuture = connectionsInProgress.putIfAbsent(target, future);
                if (oldFuture.get() == NULL) {
                    executionService.execute(
                            boost::shared_ptr<util::Runnable>(new InitConnectionTask(target, asOwner, future, *this)));
                    return future;
                }
                return oldFuture;
            }

            boost::shared_ptr<Connection>
            ClientConnectionManagerImpl::getOrConnect(const Address &address, bool asOwner) {
                while (true) {
                    boost::shared_ptr<Connection> connection = getConnection(address, asOwner);
                    if (connection.get() != NULL) {
                        return connection;
                    }
                    boost::shared_ptr<AuthenticationFuture> firstCallback = triggerConnect(address, asOwner);
                    connection = firstCallback->get();

                    if (!asOwner) {
                        return connection;
                    }
                    if (connection->isAuthenticatedAsOwner()) {
                        return connection;
                    }
                }
            }

            void
            ClientConnectionManagerImpl::authenticate(const Address &target, boost::shared_ptr<Connection> &connection,
                                                      bool asOwner, boost::shared_ptr<AuthenticationFuture> &future) {
                boost::shared_ptr<protocol::Principal> principal = getPrincipal();
                std::auto_ptr<protocol::ClientMessage> clientMessage = encodeAuthenticationRequest(asOwner,
                                                                                                   client.getSerializationService(),
                                                                                                   principal.get());
                boost::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        client, clientMessage, "", connection);
                boost::shared_ptr<spi::impl::ClientInvocationFuture> invocationFuture = clientInvocation->invokeUrgent();
                // TODO: let this return a future and pass it to AuthCallback as in Java
                executionService.schedule(
                        boost::shared_ptr<util::Runnable>(new TimeoutAuthenticationTask(invocationFuture, *this)),
                        connectionTimeoutMillis);
                invocationFuture->andThen(
                        boost::shared_ptr<impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > >(
                                new AuthCallback(connection, asOwner, target, future, *this)));
            }

            const boost::shared_ptr<protocol::Principal> ClientConnectionManagerImpl::getPrincipal() {
                return principal;
            }

            std::auto_ptr<protocol::ClientMessage>
            ClientConnectionManagerImpl::encodeAuthenticationRequest(bool asOwner,
                                                                     serialization::pimpl::SerializationService &ss,
                                                                     const protocol::Principal *principal) {
                byte serializationVersion = ss.getVersion();
                const std::string *uuid = NULL;
                const std::string *ownerUuid = NULL;
                if (principal != NULL) {
                    uuid = principal->getUuid();
                    ownerUuid = principal->getOwnerUuid();
                }
                std::auto_ptr<protocol::ClientMessage> clientMessage;
                if (credentials == NULL) {
                    // TODO: Change UsernamePasswordCredentials to implement Credentials interface so that we can just 
                    // upcast the credentials as done at Java
                    GroupConfig &groupConfig = client.getClientConfig().getGroupConfig();
                    const protocol::UsernamePasswordCredentials cr(groupConfig.getName(), groupConfig.getPassword());
                    clientMessage = protocol::codec::ClientAuthenticationCodec::encodeRequest(
                            cr.getPrincipal(), cr.getPassword(), uuid, ownerUuid, asOwner, protocol::ClientTypes::CPP,
                            serializationVersion, HAZELCAST_VERSION);
                } else {
                    serialization::pimpl::Data data = ss.toData<Credentials>(credentials);
                    clientMessage = protocol::codec::ClientAuthenticationCustomCodec::encodeRequest(data,
                                                                                                                uuid,
                                                                                                                ownerUuid,
                                                                                                                asOwner,
                                                                                                                protocol::ClientTypes::CPP,
                                                                                                                serializationVersion,
                                                                                                                HAZELCAST_VERSION);
                }
                return clientMessage;
            }

            void ClientConnectionManagerImpl::setPrincipal(const boost::shared_ptr<protocol::Principal> &principal) {
                ClientConnectionManagerImpl::principal = principal;
            }

            void ClientConnectionManagerImpl::onAuthenticated(const Address &target,
                                                              const boost::shared_ptr<Connection> &connection) {
                boost::shared_ptr<Connection> oldConnection = activeConnections.put(*connection->getRemoteEndpoint(),
                                                                                    connection);
                int socketId = connection->getSocket().getSocketId();
                activeConnectionsFileDescriptors.put(socketId, connection);
                pendingSocketIdToConnection.remove(socketId);

                if (oldConnection.get() == NULL) {
                    if (logger.isFinestEnabled()) {
                        logger.finest() << "Authentication succeeded for " << *connection
                                        << " and there was no old connection to this end-point";
                    }
                    fireConnectionAddedEvent(connection);
                } else {
                    if (logger.isFinestEnabled()) {
                        logger.finest() << "Re-authentication succeeded for " << *connection;
                    }
                    assert(*connection == *oldConnection);
                }

                connectionsInProgress.remove(target);
                std::ostringstream out;
                if (connection->getRemoteEndpoint().get()) {
                    out << *connection->getRemoteEndpoint();
                } else {
                    out << "null";
                }
                logger.info() << "Authenticated with server " << out.str() << ", server version:"
                              << connection->getConnectedServerVersionString() << " Local address: "
                              << (connection->getLocalSocketAddress().get() != NULL
                                  ? connection->getLocalSocketAddress()->toString() : "null");

                /* check if connection is closed by remote before authentication complete, if that is the case
                we need to remove it back from active connections.
                Race description from https://github.com/hazelcast/hazelcast/pull/8832.(A little bit changed)
                - open a connection client -> member
                - send auth message
                - receive auth reply -> reply processing is offloaded to an executor. Did not start to run yet.
                - member closes the connection -> the connection is trying to removed from map
                                                                     but it was not there to begin with
                - the executor start processing the auth reply -> it put the connection to the connection map.
                - we end up with a closed connection in activeConnections map */
                if (!connection->isAlive()) {
                    removeFromActiveConnections(connection);
                }
            }

            void
            ClientConnectionManagerImpl::fireConnectionAddedEvent(const boost::shared_ptr<Connection> &connection) {
                BOOST_FOREACH(const boost::shared_ptr<ConnectionListener> &connectionListener,
                              connectionListeners.toArray()) {
                                connectionListener->connectionAdded(connection);
                            }
                connectionStrategy->onConnect(connection);
            }

            void
            ClientConnectionManagerImpl::removeFromActiveConnections(const boost::shared_ptr<Connection> &connection) {
                boost::shared_ptr<Address> endpoint = connection->getRemoteEndpoint();

                if (endpoint.get() == NULL) {
                    if (logger.isFinestEnabled()) {
                        logger.finest() << "Destroying " << *connection << ", but it has end-point set to null "
                                        << "-> not removing it from a connection map";
                    }
                    return;
                }

                if (activeConnections.remove(*endpoint, connection)) {
                    logger.info() << "Removed connection to endpoint: " << *endpoint << ", connection: " << *connection;
                    activeConnectionsFileDescriptors.remove(connection->getSocket().getSocketId());
                    fireConnectionRemovedEvent(connection);
                } else {
                    if (logger.isFinestEnabled()) {
                        logger.finest() << "Destroying a connection, but there is no mapping " << endpoint << " -> "
                                        << *connection
                                        << " in the connection map.";
                    }
                }
            }

            void
            ClientConnectionManagerImpl::fireConnectionRemovedEvent(const boost::shared_ptr<Connection> &connection) {
                if (connection->isAuthenticatedAsOwner()) {
                    disconnectFromCluster(connection);
                }

                BOOST_FOREACH (const boost::shared_ptr<ConnectionListener> &listener,
                               connectionListeners.toArray()) {
                                listener->connectionRemoved(connection);
                            }
                connectionStrategy->onDisconnect(connection);
            }

            void ClientConnectionManagerImpl::disconnectFromCluster(const boost::shared_ptr<Connection> &connection) {
                clusterConnectionExecutor->execute(
                        boost::shared_ptr<util::Runnable>(
                                new DisconnecFromClusterTask(connection, *this, *connectionStrategy)));
            }

            boost::shared_ptr<util::impl::SimpleExecutorService>
            ClientConnectionManagerImpl::createSingleThreadExecutorService(spi::ClientContext &client) {
                return boost::static_pointer_cast<util::impl::SimpleExecutorService>(
                        util::Executors::newSingleThreadExecutor(client.getName() + ".cluster-"));
            }

            void
            ClientConnectionManagerImpl::setOwnerConnectionAddress(
                    const boost::shared_ptr<Address> &ownerConnectionAddress) {
                previousOwnerConnectionAddress = this->ownerConnectionAddress;
                ClientConnectionManagerImpl::ownerConnectionAddress = ownerConnectionAddress;
            }

            void
            ClientConnectionManagerImpl::fireConnectionEvent(
                    const hazelcast::client::LifecycleEvent::LifeCycleState &state) {
                spi::LifecycleService &lifecycleService = client.getLifecycleService();
                lifecycleService.fireLifecycleEvent(state);
            }

            boost::shared_ptr<util::Future<bool> > ClientConnectionManagerImpl::connectToClusterAsync() {
                boost::shared_ptr<util::Callable<bool> > task(new ConnectToClusterTask(*this));
                return clusterConnectionExecutor->submit<bool>(task);
            }

            void ClientConnectionManagerImpl::connectToClusterInternal() {
                int attempt = 0;
                std::set<Address> triedAddresses;

                while (attempt < connectionAttemptLimit) {
                    attempt++;
                    int64_t nextTry = util::currentTimeMillis() + connectionAttemptPeriod;

                    std::set<Address> addresses = getPossibleMemberAddresses();
                    BOOST_FOREACH (const Address &address, addresses) {
                                    if (!client.getLifecycleService().isRunning()) {
                                        throw exception::IllegalStateException(
                                                "ConnectionManager::connectToClusterInternal",
                                                "Giving up on retrying to connect to cluster since client is shutdown.");
                                    }
                                    triedAddresses.insert(address);
                                    if (connectAsOwner(address).get() != NULL) {
                                        return;
                                    }
                                }

                    // If the address providers load no addresses (which seems to be possible), then the above loop is not entered
                    // and the lifecycle check is missing, hence we need to repeat the same check at this point.
                    if (!client.getLifecycleService().isRunning()) {
                        throw exception::IllegalStateException("Client is being shutdown.");
                    }

                    if (attempt < connectionAttemptLimit) {
                        const int64_t remainingTime = nextTry - util::currentTimeMillis();
                        logger.warning() << "Unable to get alive cluster connection, try in "
                                         << (remainingTime > 0 ? remainingTime : 0) << " ms later, attempt " << attempt
                                         << " of " << connectionAttemptLimit << ".";

                        if (remainingTime > 0) {
                            util::Thread::sleep(remainingTime);
                        }
                    } else {
                        logger.warning() << "Unable to get alive cluster connection, attempt " << attempt << " of "
                                         << connectionAttemptLimit << ".";
                    }
                }
                std::ostringstream out;
                out << "Unable to connect to any address! The following addresses were tried: { ";
                BOOST_FOREACH(const std::set<Address>::value_type &address, triedAddresses) {
                                out << address << " , ";
                            }
                            out << "}";
                throw exception::IllegalStateException("ConnectionManager::connectToClusterInternal", out.str());
            }

            std::set<Address> ClientConnectionManagerImpl::getPossibleMemberAddresses() {
                std::set<Address> addresses;

                std::vector<Member> memberList = client.getClientClusterService().getMemberList();
                std::vector<Address> memberAddresses;
                BOOST_FOREACH (const Member &member, memberList) {
                                memberAddresses.push_back(member.getAddress());
                            }

                if (shuffleMemberList) {
                    std::random_shuffle(memberAddresses.begin(), memberAddresses.end());
                }

                addresses.insert(memberAddresses.begin(), memberAddresses.end());

                std::set<Address> providerAddressesSet;
                BOOST_FOREACH (boost::shared_ptr<AddressProvider> &addressProvider, addressProviders) {
                                std::vector<Address> addrList = addressProvider->loadAddresses();
                                providerAddressesSet.insert(addrList.begin(), addrList.end());
                            }

                std::vector<Address> providerAddresses(providerAddressesSet.begin(), providerAddressesSet.end());

                if (shuffleMemberList) {
                    std::random_shuffle(providerAddresses.begin(), providerAddresses.end());
                }

                addresses.insert(providerAddresses.begin(), providerAddresses.end());

                boost::shared_ptr<Address> previousAddress = previousOwnerConnectionAddress.get();
                if (previousAddress.get() != NULL) {
                    /*
                     * Previous owner address is moved to last item in set so that client will not try to connect to same one immediately.
                     * It could be the case that address is removed because it is healthy(it not responding to heartbeat/pings)
                     * In that case, trying other addresses first to upgrade make more sense.
                     */
                    addresses.erase(*previousAddress);
                    addresses.insert(*previousAddress);
                }
                return addresses;
            }

            std::auto_ptr<ClientConnectionStrategy>
            ClientConnectionManagerImpl::initializeStrategy(spi::ClientContext &client) {
                // TODO: Add a way so that this strategy can be configurable as in Java
                return std::auto_ptr<ClientConnectionStrategy>(new DefaultClientConnectionStrategy(client, logger,
                                                                                                   client.getClientConfig().getConnectionStrategyConfig()));
            }

            void ClientConnectionManagerImpl::connectToCluster() {
                connectToClusterAsync()->get();
            }

            void ClientConnectionManagerImpl::startEventLoopGroup() {
                inSelectorThread.start();
                outSelectorThread.start();
            }

            bool ClientConnectionManagerImpl::isAlive() {
                return alive;
            }

            void ClientConnectionManagerImpl::addConnectionHeartbeatListener(
                    const boost::shared_ptr<spi::impl::ConnectionHeartbeatListener> &listener) {
                heartbeat->addConnectionHeartbeatListener(listener);
            }

            void ClientConnectionManagerImpl::stopEventLoopGroup() {
                inSelector.shutdown();
                outSelector.shutdown();
            }

            void ClientConnectionManagerImpl::onClose(Connection &connection) {
                removeFromActiveConnections(connection.shared_from_this());
            }

            void
            ClientConnectionManagerImpl::heartbeatResumed(const boost::shared_ptr<Connection> &connection) {
                connectionStrategy->onHeartbeatResumed(connection);
            }

            void
            ClientConnectionManagerImpl::heartbeatStopped(const boost::shared_ptr<Connection> &connection) {
                connectionStrategy->onHeartbeatStopped(connection);
            }

            boost::shared_ptr<Connection> ClientConnectionManagerImpl::getActiveConnection(int fileDescriptor) {
                boost::shared_ptr<Connection> connection = activeConnectionsFileDescriptors.get(fileDescriptor);
                if (connection.get()) {
                    return connection;
                }

                return pendingSocketIdToConnection.get(fileDescriptor);
            }

            void
            ClientConnectionManagerImpl::addConnectionListener(
                    const boost::shared_ptr<ConnectionListener> &connectionListener) {
                connectionListeners.add(connectionListener);
            }

            ClientConnectionManagerImpl::~ClientConnectionManagerImpl() {
            }

            ClientConnectionManagerImpl::InitConnectionTask::InitConnectionTask(const Address &target,
                                                                                const bool asOwner,
                                                                                const boost::shared_ptr<AuthenticationFuture> &future,
                                                                                ClientConnectionManagerImpl &connectionManager)
                    : target(
                    target), asOwner(asOwner), future(future), connectionManager(connectionManager) {}

            void ClientConnectionManagerImpl::InitConnectionTask::run() {
                boost::shared_ptr<Connection> connection;
                try {
                    connection = getConnection(target);
                } catch (exception::IException &e) {
                    util::ILogger::getLogger().finest() << e;
                    future->onFailure(boost::shared_ptr<exception::IException>(e.clone()));
                    connectionManager.connectionsInProgress.remove(target);
                    return;
                }

                try {
                    connectionManager.pendingSocketIdToConnection.put(connection->getSocket().getSocketId(),
                                                                      connection);

                    connection->getReadHandler().registerSocket();

                    connectionManager.authenticate(target, connection, asOwner, future);
                } catch (exception::IException &e) {
                    const boost::shared_ptr<exception::IException> throwable(e.clone());
                    future->onFailure(throwable);
                    connection->close("Failed to authenticate connection", throwable);
                    connectionManager.connectionsInProgress.remove(target);
                }
            }

            const std::string ClientConnectionManagerImpl::InitConnectionTask::getName() const {
                return "ConnectionManager::InitConnectionTask";
            }

            boost::shared_ptr<Connection>
            ClientConnectionManagerImpl::InitConnectionTask::getConnection(const Address &target) {
                boost::shared_ptr<Connection> connection = connectionManager.activeConnections.get(target);
                if (connection.get() != NULL) {
                    return connection;
                }
                Address address = connectionManager.translator->translate(target);
                return connectionManager.createSocketConnection(address);
            }

            ClientConnectionManagerImpl::AuthCallback::AuthCallback(const boost::shared_ptr<Connection> &connection,
                                                                    bool asOwner,
                                                                    const Address &target,
                                                                    boost::shared_ptr<AuthenticationFuture> &future,
                                                                    ClientConnectionManagerImpl &connectionManager)
                    : connection(connection), asOwner(asOwner), target(target), future(future),
                      connectionManager(connectionManager) {
            }

            void ClientConnectionManagerImpl::AuthCallback::onResponse(
                    const boost::shared_ptr<protocol::ClientMessage> &response) {
/*              TODO
                timeoutTaskFuture.cancel(true);
*/

                std::auto_ptr<protocol::codec::ClientAuthenticationCodec::ResponseParameters> result;
                try {
                    result.reset(new protocol::codec::ClientAuthenticationCodec::ResponseParameters(
                            protocol::codec::ClientAuthenticationCodec::ResponseParameters::decode(*response)));
                } catch (exception::IException &e) {
                    onFailure(boost::shared_ptr<exception::IException>(e.clone()));
                    return;
                }
                protocol::AuthenticationStatus authenticationStatus = (protocol::AuthenticationStatus) result->status;
                switch (authenticationStatus) {
                    case protocol::AUTHENTICATED: {
                        connection->setConnectedServerVersion(result->serverHazelcastVersion);
                        connection->setRemoteEndpoint(boost::shared_ptr<Address>(result->address));
                        if (asOwner) {
                            connection->setIsAuthenticatedAsOwner();
                            boost::shared_ptr<protocol::Principal> principal(
                                    new protocol::Principal(result->uuid, result->ownerUuid));
                            connectionManager.setPrincipal(principal);
                            //setting owner connection is moved to here(before onAuthenticated/before connected event)
                            //so that invocations that requires owner connection on this connection go through
                            connectionManager.setOwnerConnectionAddress(connection->getRemoteEndpoint());
                            connectionManager.logger.info() << "Setting " << *connection << " as owner with principal "
                                                            << *principal;
                        }
                        connectionManager.onAuthenticated(target, connection);
                        future->onSuccess(connection);
                        break;
                    }
                    case protocol::CREDENTIALS_FAILED: {
                        boost::shared_ptr<protocol::Principal> p = connectionManager.principal;
                        onFailure((exception::ExceptionBuilder<exception::AuthenticationException>(
                                "ConnectionManager::AuthCallback::onResponse") << "Invalid credentials! Principal: "
                                                                               << *p).buildShared());
                        break;
                    }
                    default: {
                        onFailure((exception::ExceptionBuilder<exception::AuthenticationException>(
                                "ConnectionManager::AuthCallback::onResponse")
                                << "Authentication status code not supported. status: "
                                << authenticationStatus).buildShared());
                    }
                }
            }

            void
            ClientConnectionManagerImpl::AuthCallback::onFailure(const boost::shared_ptr<exception::IException> &e) {
/*
                timeoutTaskFuture.cancel(true);
*/
                onAuthenticationFailed(target, connection, e);
                future->onFailure(e);
            }

            void ClientConnectionManagerImpl::AuthCallback::onAuthenticationFailed(const Address &target,
                                                                                   const boost::shared_ptr<Connection> &connection,
                                                                                   const boost::shared_ptr<exception::IException> &cause) {
                if (connectionManager.logger.isFinestEnabled()) {
                    connectionManager.logger.finest() << "Authentication of " << connection << " failed." << cause;
                }
                connection->close("", cause);
                connectionManager.pendingSocketIdToConnection.remove(connection->getSocket().getSocketId());
                connectionManager.connectionsInProgress.remove(target);
            }

            ClientConnectionManagerImpl::DisconnecFromClusterTask::DisconnecFromClusterTask(
                    const boost::shared_ptr<Connection> &connection, ClientConnectionManagerImpl &connectionManager,
                    ClientConnectionStrategy &connectionStrategy)
                    : connection(
                    connection), connectionManager(connectionManager), connectionStrategy(connectionStrategy) {
            }

            void ClientConnectionManagerImpl::DisconnecFromClusterTask::run() {
                boost::shared_ptr<Address> endpoint = connection->getRemoteEndpoint();
                // it may be possible that while waiting on executor queue, the client got connected (another connection),
                // then we do not need to do anything for cluster disconnect.
                boost::shared_ptr<Address> ownerAddress = connectionManager.ownerConnectionAddress;
                if (ownerAddress.get() && (endpoint.get() && *endpoint != *ownerAddress)) {
                    return;
                }

                connectionManager.setOwnerConnectionAddress(boost::shared_ptr<Address>());
                connectionStrategy.onDisconnectFromCluster();

                if (connectionManager.client.getLifecycleService().isRunning()) {
                    connectionManager.fireConnectionEvent(LifecycleEvent::CLIENT_DISCONNECTED);
                }
            }

            const std::string ClientConnectionManagerImpl::DisconnecFromClusterTask::getName() const {
                return "DisconnecFromClusterTask";
            }

            ClientConnectionManagerImpl::ConnectToClusterTask::ConnectToClusterTask(
                    ClientConnectionManagerImpl &connectionManager)
                    : connectionManager(connectionManager) {
            }

            bool ClientConnectionManagerImpl::ConnectToClusterTask::call() {
                try {
                    connectionManager.connectToClusterInternal();
                    return true;
                } catch (exception::IException &e) {
                    connectionManager.logger.warning() << "Could not connect to cluster, shutting down the client. "
                                                       << e.getMessage();
                    boost::shared_ptr<util::Thread> shutdownThread(new util::Thread(
                            boost::shared_ptr<util::Runnable>(new ShutdownTask(connectionManager.client))));
                    shutdownThread->start();
                    connectionManager.shutdownThreads.offer(shutdownThread);

                    throw;
                }
            }

            const std::string ClientConnectionManagerImpl::ConnectToClusterTask::getName() const {
                return "ClientConnectionManagerImpl::ConnectToClusterTask";
            }

            ClientConnectionManagerImpl::ShutdownTask::ShutdownTask(spi::ClientContext &client)
                    : client(client) {}

            void ClientConnectionManagerImpl::ShutdownTask::run() {
                try {
                    client.getLifecycleService().shutdown();
                } catch (exception::IException &exception) {
                    util::ILogger::getLogger().severe() << "Exception during client shutdown: " << exception;
                }
            }

            const std::string ClientConnectionManagerImpl::ShutdownTask::getName() const {
                return client.getName() + ".clientShutdown-";
            }

            void ClientConnectionManagerImpl::TimeoutAuthenticationTask::run() {
                if (future->isDone()) {
                    return;
                }
                future->complete((exception::ExceptionBuilder<exception::TimeoutException>(
                        "TimeoutAuthenticationTask::TimeoutAuthenticationTask")
                        << "Authentication response did not come back in "
                        << clientConnectionManager.connectionTimeoutMillis << " millis").build());
            }

            const std::string ClientConnectionManagerImpl::TimeoutAuthenticationTask::getName() const {
                return "ClientConnectionManagerImpl::TimeoutAuthenticationTask";
            }

            ClientConnectionManagerImpl::TimeoutAuthenticationTask::TimeoutAuthenticationTask(
                    const boost::shared_ptr<spi::impl::ClientInvocationFuture> &future,
                    ClientConnectionManagerImpl &clientConnectionManager) : future(future), clientConnectionManager(
                    clientConnectionManager) {}
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

