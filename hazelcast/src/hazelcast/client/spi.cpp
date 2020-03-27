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

#include "hazelcast/util/RuntimeAvailableProcessors.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/MembershipEvent.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/spi/impl/SmartClientInvocationService.h"
#include "hazelcast/client/spi/impl/NonSmartClientInvocationService.h"
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/impl/DefaultAddressProvider.h"
#include "hazelcast/client/spi/impl/ClientMembershipListener.h"
#include "hazelcast/client/spi/impl/listener/SmartClientListenerService.h"
#include "hazelcast/client/spi/impl/listener/NonSmartClientListenerService.h"
#include "hazelcast/client/spi/impl/AwsAddressProvider.h"
#include "hazelcast/client/spi/impl/DefaultAddressProvider.h"
#include "hazelcast/client/spi/impl/DefaultAddressTranslator.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h"
#include "hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h"
#include "hazelcast/client/spi/impl/sequence/CallIdFactory.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/impl/BuildInfo.h"
#include "hazelcast/util/AddressHelper.h"
#include "hazelcast/util/HashUtil.h"
#include "hazelcast/util/concurrent/BackoffIdleStrategy.h"

namespace hazelcast {
    namespace client {
        const std::vector<Member> &InitialMembershipEvent::getMembers() const {
            return members;
        }

        Cluster &InitialMembershipEvent::getCluster() {
            return cluster;
        }

        InitialMembershipEvent::InitialMembershipEvent(Cluster &cluster, const std::set<Member> &members) : cluster(
                cluster) {
            for (const Member &member : members) {
                this->members.push_back(Member(member));
            }

        }

        LifecycleEvent::LifecycleEvent(LifeCycleState state)
                : state(state) {

        }

        LifecycleEvent::LifeCycleState LifecycleEvent::getState() const {
            return state;
        }

        namespace spi {
            ProxyManager::ProxyManager(ClientContext &context) : client(context) {
            }

            void ProxyManager::init() {
                spi::impl::AbstractClientInvocationService &invocationService =
                        (spi::impl::AbstractClientInvocationService &) client.getInvocationService();

                invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                invocationRetryPauseMillis = invocationService.getInvocationRetryPauseMillis();
            }

            void ProxyManager::destroy() {
                for (const std::shared_ptr<util::Future<ClientProxy> > &future : proxies.values()) {
                    future->get()->onShutdown();
                }
                proxies.clear();
            }

            std::shared_ptr<ClientProxy> ProxyManager::getOrCreateProxy(
                    const std::string &service, const std::string &id, ClientProxyFactory &factory) {
                DefaultObjectNamespace ns(service, id);
                std::shared_ptr<util::Future<ClientProxy> > proxyFuture = proxies.get(ns);
                if (proxyFuture.get() != NULL) {
                    return proxyFuture->get();
                }

                proxyFuture.reset(new util::Future<ClientProxy>(client.getLogger()));
                std::shared_ptr<util::Future<ClientProxy> > current = proxies.putIfAbsent(ns, proxyFuture);
                if (current.get()) {
                    return current->get();
                }

                try {
                    std::shared_ptr<ClientProxy> clientProxy = factory.create(id);
                    initializeWithRetry(clientProxy);
                    proxyFuture->set_value(clientProxy);
                    return clientProxy;
                } catch (exception::IException &e) {
                    proxies.remove(ns);
                    proxyFuture->complete(e);
                    throw;
                }
            }

            void ProxyManager::initializeWithRetry(const std::shared_ptr<ClientProxy> &clientProxy) {
                int64_t startMillis = util::currentTimeMillis();
                while (util::currentTimeMillis() < startMillis + invocationTimeoutMillis) {
                    try {
                        initialize(clientProxy);
                        return;
                    } catch (exception::IException &e) {
                        bool retryable = isRetryable(e);

                        if (!retryable) {
                            try {
                                e.raise();
                            } catch (exception::ExecutionException &) {
                                retryable = isRetryable(*e.getCause());
                            }
                        }

                        if (retryable) {
                            sleepForProxyInitRetry();
                        } else {
                            e.raise();
                        }
                    }
                }
                int64_t elapsedTime = util::currentTimeMillis() - startMillis;
                throw (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                        "ProxyManager::initializeWithRetry") << "Initializing  " << clientProxy->getServiceName() << ":"
                                                             << clientProxy->getName() << " is timed out after "
                                                             << elapsedTime << " ms. Configured invocation timeout is "
                                                             << invocationTimeoutMillis << " ms").build();
            }

            void ProxyManager::initialize(const std::shared_ptr<ClientProxy> &clientProxy) {
                std::shared_ptr<Address> initializationTarget = findNextAddressToSendCreateRequest();
                if (initializationTarget.get() == NULL) {
                    throw exception::IOException("ProxyManager::initialize",
                                                 "Not able to find a member to create proxy on!");
                }
                std::unique_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientCreateProxyCodec::encodeRequest(
                        clientProxy->getName(),
                        clientProxy->getServiceName(), *initializationTarget);
                spi::impl::ClientInvocation::create(client, clientMessage, clientProxy->getServiceName(),
                                                    *initializationTarget)->invoke()->get();
                clientProxy->onInitialize();
            }

            std::shared_ptr<Address> ProxyManager::findNextAddressToSendCreateRequest() {
                int clusterSize = client.getClientClusterService().getSize();
                if (clusterSize == 0) {
                    throw exception::HazelcastClientOfflineException("ProxyManager::findNextAddressToSendCreateRequest",
                                                                     "Client connecting to cluster");
                }
                std::shared_ptr<Member> liteMember;

                LoadBalancer *loadBalancer = client.getClientConfig().getLoadBalancer();
                for (int i = 0; i < clusterSize; i++) {
                    std::shared_ptr<Member> member;
                    try {
                        member = std::shared_ptr<Member>(new Member(loadBalancer->next()));
                    } catch (exception::IllegalStateException &) {
                        // skip
                    }
                    if (member.get() != NULL && !member->isLiteMember()) {
                        return std::shared_ptr<Address>(new Address(member->getAddress()));
                    } else if (liteMember.get() == NULL) {
                        liteMember = member;
                    }
                }

                return liteMember.get() != NULL ? std::shared_ptr<Address>(new Address(liteMember->getAddress()))
                                                : std::shared_ptr<Address>();
            }

            bool ProxyManager::isRetryable(exception::IException &exception) {
                return spi::impl::ClientInvocation::isRetrySafeException(exception);
            }

            void ProxyManager::sleepForProxyInitRetry() {
                // TODO: change to interruptible sleep
                util::sleepmillis(invocationRetryPauseMillis);
            }

            void ProxyManager::destroyProxy(ClientProxy &proxy) {
                DefaultObjectNamespace objectNamespace(proxy.getServiceName(), proxy.getName());
                std::shared_ptr<util::Future<ClientProxy> > registeredProxyFuture = proxies.remove(
                        objectNamespace);
                std::shared_ptr<ClientProxy> registeredProxy;
                if (registeredProxyFuture.get()) {
                    registeredProxy = registeredProxyFuture->get();
                }

                try {
                    if (registeredProxy.get() != NULL) {
                        try {
                            registeredProxy->destroyLocally();
                            registeredProxy->destroyRemotely();
                        } catch (exception::IException &e) {
                            registeredProxy->destroyRemotely();
                            e.raise();
                        }
                    }
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroyLocally();
                    }
                } catch (exception::IException &e) {
                    if (&proxy != registeredProxy.get()) {
                        // The given proxy is stale and was already destroyed, but the caller
                        // may have allocated local resources in the context of this stale proxy
                        // instance after it was destroyed, so we have to cleanup it locally one
                        // more time to make sure there are no leaking local resources.
                        proxy.destroyLocally();
                    }
                    e.raise();
                }

            }

            ClientContext::ClientContext(const client::HazelcastClient &hazelcastClient) : hazelcastClient(
                    *hazelcastClient.clientImpl) {
            }

            ClientContext::ClientContext(client::impl::HazelcastClientInstanceImpl &hazelcastClient)
                    : hazelcastClient(hazelcastClient) {
            }

            serialization::pimpl::SerializationService &ClientContext::getSerializationService() {
                return hazelcastClient.serializationService;
            }

            ClientClusterService &ClientContext::getClientClusterService() {
                return hazelcastClient.clusterService;
            }

            ClientInvocationService &ClientContext::getInvocationService() {
                return *hazelcastClient.invocationService;
            }

            ClientConfig &ClientContext::getClientConfig() {
                return hazelcastClient.clientConfig;
            }

            ClientPartitionService &ClientContext::getPartitionService() {
                return *hazelcastClient.partitionService;
            }

            LifecycleService &ClientContext::getLifecycleService() {
                return hazelcastClient.lifecycleService;
            }

            ClientListenerService &ClientContext::getClientListenerService() {
                return *hazelcastClient.listenerService;
            }

            connection::ClientConnectionManagerImpl &ClientContext::getConnectionManager() {
                return *hazelcastClient.connectionManager;
            }

            internal::nearcache::NearCacheManager &ClientContext::getNearCacheManager() {
                return *hazelcastClient.nearCacheManager;
            }

            ClientProperties &ClientContext::getClientProperties() {
                return hazelcastClient.clientProperties;
            }

            Cluster &ClientContext::getCluster() {
                return hazelcastClient.cluster;
            }

            std::shared_ptr<impl::sequence::CallIdSequence> &ClientContext::getCallIdSequence() const {
                return hazelcastClient.callIdSequence;
            }

            const protocol::ClientExceptionFactory &ClientContext::getClientExceptionFactory() const {
                return hazelcastClient.getExceptionFactory();
            }

            const std::string &ClientContext::getName() const {
                return hazelcastClient.getName();
            }

            impl::ClientExecutionServiceImpl &ClientContext::getClientExecutionService() const {
                return *hazelcastClient.executionService;
            }

            void ClientContext::onClusterConnect(const std::shared_ptr<connection::Connection> &ownerConnection) {
                hazelcastClient.onClusterConnect(ownerConnection);
            }

            const std::shared_ptr<client::impl::ClientLockReferenceIdGenerator> &
            ClientContext::getLockReferenceIdGenerator() {
                return hazelcastClient.getLockReferenceIdGenerator();
            }

            std::shared_ptr<client::impl::HazelcastClientInstanceImpl>
            ClientContext::getHazelcastClientImplementation() {
                return hazelcastClient.shared_from_this();
            }

            spi::ProxyManager &ClientContext::getProxyManager() {
                return hazelcastClient.getProxyManager();
            }

            util::ILogger &ClientContext::getLogger() {
                return *hazelcastClient.logger;
            }

            client::impl::statistics::Statistics &ClientContext::getClientstatistics() {
                return *hazelcastClient.statistics;
            }

            LifecycleService::LifecycleService(ClientContext &clientContext,
                                               const std::set<LifecycleListener *> &lifecycleListeners,
                                               LoadBalancer *const loadBalancer, Cluster &cluster) : clientContext(
                    clientContext), loadBalancer(loadBalancer), cluster(cluster), shutdownCompletedLatch(1) {
                listeners.insert(lifecycleListeners.begin(), lifecycleListeners.end());
            }

            bool LifecycleService::start() {
                bool expected = false;
                if (!active.compare_exchange_strong(expected, true)) {
                    return false;
                }

                fireLifecycleEvent(LifecycleEvent::STARTED);

                clientContext.getClientExecutionService().start();

                ((spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService()).start();

                if (!((impl::AbstractClientInvocationService &) clientContext.getInvocationService()).start()) {
                    return false;
                }

                ((spi::impl::ClientClusterServiceImpl &) clientContext.getClientClusterService()).start();

                if (!clientContext.getConnectionManager().start()) {
                    return false;
                }

                loadBalancer->init(cluster);

                ((spi::impl::ClientPartitionServiceImpl &) clientContext.getPartitionService()).start();

                clientContext.getClientstatistics().start();

                return true;
            }

            void LifecycleService::shutdown() {
                bool expected = true;
                if (!active.compare_exchange_strong(expected, false)) {
                    shutdownCompletedLatch.await();
                    return;
                }
                fireLifecycleEvent(LifecycleEvent::SHUTTING_DOWN);
                clientContext.getProxyManager().destroy();
                clientContext.getConnectionManager().shutdown();
                ((spi::impl::ClientClusterServiceImpl &) clientContext.getClientClusterService()).shutdown();
                ((spi::impl::AbstractClientInvocationService &) clientContext.getInvocationService()).shutdown();
                ((spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService()).shutdown();
                clientContext.getNearCacheManager().destroyAllNearCaches();
                ((spi::impl::ClientPartitionServiceImpl &) clientContext.getPartitionService()).stop();
                clientContext.getClientExecutionService().shutdown();
                fireLifecycleEvent(LifecycleEvent::SHUTDOWN);
                clientContext.getSerializationService().dispose();
                shutdownCompletedLatch.countDown();
            }

            void LifecycleService::addLifecycleListener(LifecycleListener *lifecycleListener) {
                util::LockGuard lg(listenerLock);
                listeners.insert(lifecycleListener);
            }

            bool LifecycleService::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                util::LockGuard lg(listenerLock);
                return listeners.erase(lifecycleListener) == 1;
            }

            void LifecycleService::fireLifecycleEvent(const LifecycleEvent &lifecycleEvent) {
                util::LockGuard lg(listenerLock);
                util::ILogger &logger = clientContext.getLogger();
                switch (lifecycleEvent.getState()) {
                    case LifecycleEvent::STARTING : {
                        // convert the date string from "2016-04-20" to 20160420
                        std::string date(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_DATE));
                        util::gitDateToHazelcastLogDate(date);
                        std::string commitId(HAZELCAST_STRINGIZE(HAZELCAST_GIT_COMMIT_ID));
                        commitId.erase(std::remove(commitId.begin(), commitId.end(), '"'), commitId.end());
                        char msg[100];
                        util::hz_snprintf(msg, 100, "(%s:%s) LifecycleService::LifecycleEvent STARTING", date.c_str(),
                                          commitId.c_str());
                        logger.info(msg);
                        break;
                    }
                    case LifecycleEvent::STARTED :
                        logger.info("LifecycleService::LifecycleEvent STARTED");
                        break;
                    case LifecycleEvent::SHUTTING_DOWN :
                        logger.info("LifecycleService::LifecycleEvent SHUTTING_DOWN");
                        break;
                    case LifecycleEvent::SHUTDOWN :
                        logger.info("LifecycleService::LifecycleEvent SHUTDOWN");
                        break;
                    case LifecycleEvent::CLIENT_CONNECTED :
                        logger.info("LifecycleService::LifecycleEvent CLIENT_CONNECTED");
                        break;
                    case LifecycleEvent::CLIENT_DISCONNECTED :
                        logger.info("LifecycleService::LifecycleEvent CLIENT_DISCONNECTED");
                        break;
                }

                for (std::set<LifecycleListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    (*it)->stateChanged(lifecycleEvent);
                }

            }

            bool LifecycleService::isRunning() {
                return active;
            }

            LifecycleService::~LifecycleService() {
                if (active) {
                    shutdown();
                }
            }

            DefaultObjectNamespace::DefaultObjectNamespace(const std::string &service, const std::string &object)
                    : serviceName(service), objectName(object) {

            }

            const std::string &DefaultObjectNamespace::getServiceName() const {
                return serviceName;
            }

            const std::string &DefaultObjectNamespace::getObjectName() const {
                return objectName;
            }

            ClientProxy::ClientProxy(const std::string &name, const std::string &serviceName, ClientContext &context)
                    : name(name), serviceName(serviceName), context(context) {}

            ClientProxy::~ClientProxy() {}

            const std::string &ClientProxy::getName() const {
                return name;
            }

            const std::string &ClientProxy::getServiceName() const {
                return serviceName;
            }

            ClientContext &ClientProxy::getContext() {
                return context;
            }

            void ClientProxy::onDestroy() {
            }

            void ClientProxy::destroy() {
                getContext().getProxyManager().destroyProxy(*this);
            }

            void ClientProxy::destroyLocally() {
                if (preDestroy()) {
                    try {
                        onDestroy();
                        postDestroy();
                    } catch (exception::IException &e) {
                        postDestroy();
                        e.raise();
                    }
                }
            }

            bool ClientProxy::preDestroy() {
                return true;
            }

            void ClientProxy::postDestroy() {
            }

            void ClientProxy::onInitialize() {
            }

            void ClientProxy::onShutdown() {
            }

            serialization::pimpl::SerializationService &ClientProxy::getSerializationService() {
                return context.getSerializationService();
            }

            void ClientProxy::destroyRemotely() {
                std::unique_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientDestroyProxyCodec::encodeRequest(
                        getName(), getServiceName());
                spi::impl::ClientInvocation::create(getContext(), clientMessage, getName())->invoke()->get();
            }

            ClientProxy::EventHandlerDelegator::EventHandlerDelegator(client::impl::BaseEventHandler *handler)
                    : handler(
                    handler) {}

            void ClientProxy::EventHandlerDelegator::handle(const std::shared_ptr<protocol::ClientMessage> &event) {
                handler->handle(event);
            }

            void ClientProxy::EventHandlerDelegator::beforeListenerRegister() {
                handler->beforeListenerRegister();
            }

            void ClientProxy::EventHandlerDelegator::onListenerRegister() {
                handler->onListenerRegister();
            }

            std::string ClientProxy::registerListener(const std::shared_ptr<spi::impl::ListenerMessageCodec> &codec,
                                                      client::impl::BaseEventHandler *handler) {
                handler->setLogger(&getContext().getLogger());
                return getContext().getClientListenerService().registerListener(codec,
                                                                                std::shared_ptr<spi::EventHandler<protocol::ClientMessage> >(
                                                                                        new EventHandlerDelegator(
                                                                                                handler)));
            }

            std::string
            ClientProxy::registerListener(const std::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                                          const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                std::static_pointer_cast<client::impl::BaseEventHandler>(handler)->setLogger(&getContext().getLogger());
                return getContext().getClientListenerService().registerListener(listenerMessageCodec, handler);
            }

            bool ClientProxy::deregisterListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }


            namespace impl {
                AbstractClientInvocationService::AbstractClientInvocationService(ClientContext &client)
                        : CLEAN_RESOURCES_MILLIS(client.getClientProperties().getCleanResourcesPeriodMillis()),
                          client(client), invocationLogger(client.getLogger()),
                          connectionManager(NULL),
                          partitionService(client.getPartitionService()),
                          clientListenerService(NULL),
                          invocationTimeoutMillis(client.getClientProperties().getInteger(
                                  client.getClientProperties().getInvocationTimeoutSeconds()) * 1000),
                          invocationRetryPauseMillis(client.getClientProperties().getLong(
                                  client.getClientProperties().getInvocationRetryPauseMillis())),
                          responseThread(client.getName() + ".response-", invocationLogger, *this, client) {
                }

                bool AbstractClientInvocationService::start() {
                    connectionManager = &client.getConnectionManager();
                    clientListenerService = static_cast<listener::AbstractClientListenerService *>(&client.getClientListenerService());

                    responseThread.start();

                    int64_t cleanResourcesMillis = client.getClientProperties().getLong(CLEAN_RESOURCES_MILLIS);
                    if (cleanResourcesMillis <= 0) {
                        cleanResourcesMillis = util::IOUtil::to_value<int64_t>(
                                CLEAN_RESOURCES_MILLIS.getDefaultValue());
                    }

                    client.getClientExecutionService().scheduleWithRepetition(std::shared_ptr<util::Runnable>(
                            new CleanResourcesTask(invocations)), cleanResourcesMillis, cleanResourcesMillis);

                    return true;
                }

                void AbstractClientInvocationService::shutdown() {
                    isShutdown.store(true);

                    responseThread.shutdown();

                    typedef std::vector<std::pair<int64_t, std::shared_ptr<ClientInvocation> > > InvocationEntriesVector;
                    InvocationEntriesVector allEntries = invocations.clear();
                    std::shared_ptr<exception::HazelcastClientNotActiveException> notActiveException(
                            new exception::HazelcastClientNotActiveException(
                                    "AbstractClientInvocationService::shutdown",
                                    "Client is shutting down"));
                    for (InvocationEntriesVector::value_type &entry : allEntries) {
                        entry.second->notifyException(notActiveException);
                    }
                }

                int64_t AbstractClientInvocationService::getInvocationTimeoutMillis() const {
                    return invocationTimeoutMillis;
                }

                int64_t AbstractClientInvocationService::getInvocationRetryPauseMillis() const {
                    return invocationRetryPauseMillis;
                }

                bool AbstractClientInvocationService::isRedoOperation() {
                    return client.getClientConfig().isRedoOperation();
                }

                void AbstractClientInvocationService::handleClientMessage(
                        const std::shared_ptr<connection::Connection> &connection,
                        const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    responseThread.responseQueue.push(clientMessage);
                }

                std::shared_ptr<ClientInvocation> AbstractClientInvocationService::deRegisterCallId(int64_t callId) {
                    return invocations.remove(callId);
                }

                void AbstractClientInvocationService::send(std::shared_ptr<impl::ClientInvocation> invocation,
                                                           std::shared_ptr<connection::Connection> connection) {
                    if (isShutdown) {
                        throw exception::HazelcastClientNotActiveException("AbstractClientInvocationService::send",
                                                                           "Client is shut down");
                    }
                    registerInvocation(invocation);

                    const std::shared_ptr<protocol::ClientMessage> &clientMessage = invocation->getClientMessage();
                    if (!writeToConnection(*connection, clientMessage)) {
                        int64_t callId = clientMessage->getCorrelationId();
                        std::shared_ptr<ClientInvocation> clientInvocation = deRegisterCallId(callId);
                        if (clientInvocation.get() != NULL) {
                            std::ostringstream out;
                            out << "Packet not sent to ";
                            if (connection->getRemoteEndpoint().get()) {
                                out << *connection->getRemoteEndpoint();
                            } else {
                                out << "null";
                            }
                            throw exception::IOException("AbstractClientInvocationService::send", out.str());
                        } else {
                            if (invocationLogger.isFinestEnabled()) {
                                invocationLogger.finest("Invocation not found to deregister for call ID ", callId);
                            }
                            return;
                        }
                    }

                    invocation->setSendConnection(connection);
                }

                void AbstractClientInvocationService::registerInvocation(
                        const std::shared_ptr<ClientInvocation> &clientInvocation) {
                    const std::shared_ptr<protocol::ClientMessage> &clientMessage = clientInvocation->getClientMessage();
                    int64_t correlationId = clientMessage->getCorrelationId();
                    invocations.put(correlationId, clientInvocation);
                    const std::shared_ptr<
                            EventHandler<protocol::ClientMessage> > handler = clientInvocation->getEventHandler();
                    if (handler.get() != NULL) {
                        clientListenerService->addEventHandler(correlationId, handler);
                    }
                }

                bool AbstractClientInvocationService::writeToConnection(connection::Connection &connection,
                                                                        const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    clientMessage->addFlag(protocol::ClientMessage::BEGIN_AND_END_FLAGS);
                    return connection.write(clientMessage);
                }

                void AbstractClientInvocationService::CleanResourcesTask::run() {
                    std::vector<int64_t> invocationsToBeRemoved;
                    typedef std::vector<std::pair<int64_t, std::shared_ptr<ClientInvocation> > > INVOCATION_ENTRIES;
                    for (const INVOCATION_ENTRIES::value_type &entry : invocations.entrySet()) {
                        int64_t key = entry.first;
                        const std::shared_ptr<ClientInvocation> &invocation = entry.second;
                        std::shared_ptr<connection::Connection> connection = invocation->getSendConnection();
                        if (!connection.get()) {
                            continue;
                        }

                        if (connection->isAlive()) {
                            continue;
                        }

                        invocationsToBeRemoved.push_back(key);

                        notifyException(*invocation, connection);
                    }

                    for (int64_t invocationId : invocationsToBeRemoved) {
                        invocations.remove(invocationId);
                    }
                }

                void AbstractClientInvocationService::CleanResourcesTask::notifyException(ClientInvocation &invocation,
                                                                                          std::shared_ptr<connection::Connection> &connection) {
                    std::shared_ptr<exception::IException> ex(
                            new exception::TargetDisconnectedException("CleanResourcesTask::notifyException",
                                                                       connection->getCloseReason()));
                    invocation.notifyException(ex);
                }

                AbstractClientInvocationService::CleanResourcesTask::CleanResourcesTask(
                        util::SynchronizedMap<int64_t, ClientInvocation> &invocations) : invocations(invocations) {}

                const std::string AbstractClientInvocationService::CleanResourcesTask::getName() const {
                    return "AbstractClientInvocationService::CleanResourcesTask";
                }

                AbstractClientInvocationService::~AbstractClientInvocationService() {
                }

                AbstractClientInvocationService::ResponseThread::ResponseThread(const std::string &name,
                                                                                util::ILogger &invocationLogger,
                                                                                AbstractClientInvocationService &invocationService,
                                                                                ClientContext &clientContext)
                        : responseQueue(100000), invocationLogger(invocationLogger),
                          invocationService(invocationService), client(clientContext),
                          worker(std::shared_ptr<util::Runnable>(new util::RunnableDelegator(*this)),
                                 invocationLogger) {
                }

                void AbstractClientInvocationService::ResponseThread::run() {
                    try {
                        doRun();
                    } catch (exception::IException &t) {
                        invocationLogger.severe(t);
                    }
                }

                void AbstractClientInvocationService::ResponseThread::doRun() {
                    while (!invocationService.isShutdown) {
                        std::shared_ptr<protocol::ClientMessage> task;
                        try {
                            task = responseQueue.pop();
                        } catch (exception::InterruptedException &) {
                            continue;
                        }
                        process(task);
                    }
                }

                void AbstractClientInvocationService::ResponseThread::process(
                        const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    try {
                        handleClientMessage(clientMessage);
                    } catch (exception::IException &e) {
                        invocationLogger.severe("Failed to process task: ", clientMessage, " on responseThread: ",
                                                getName(), e);
                    }
                }

                void AbstractClientInvocationService::ResponseThread::handleClientMessage(
                        const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    int64_t correlationId = clientMessage->getCorrelationId();

                    std::shared_ptr<ClientInvocation> future = invocationService.deRegisterCallId(correlationId);
                    if (future.get() == NULL) {
                        invocationLogger.warning("No call for callId: ", correlationId, ", response: ", *clientMessage);
                        return;
                    }
                    if (protocol::codec::ErrorCodec::TYPE == clientMessage->getMessageType()) {
                        std::shared_ptr<exception::IException> exception(
                                client.getClientExceptionFactory().createException(
                                        "AbstractClientInvocationService::ResponseThread::handleClientMessage",
                                        *clientMessage));
                        future->notifyException(exception);
                    } else {
                        future->notify(clientMessage);
                    }
                }

                void AbstractClientInvocationService::ResponseThread::shutdown() {
                    do {
                        responseQueue.interrupt();
                    } while (!worker.waitMilliseconds(100));

                    worker.join();
                }

                void AbstractClientInvocationService::ResponseThread::start() {
                    worker.start();
                }

                const std::string AbstractClientInvocationService::ResponseThread::getName() const {
                    return "AbstractClientInvocationService::ResponseThread";
                }

                AbstractClientInvocationService::ResponseThread::~ResponseThread() {
                }

                NonSmartClientInvocationService::NonSmartClientInvocationService(ClientContext &client)
                        : AbstractClientInvocationService(client) {

                }

                void NonSmartClientInvocationService::invokeOnConnection(
                        std::shared_ptr<impl::ClientInvocation> invocation,
                        std::shared_ptr<connection::Connection> connection) {
                    assert(connection.get() != NULL);
                    send(invocation, connection);
                }

                void NonSmartClientInvocationService::invokeOnPartitionOwner(
                        std::shared_ptr<impl::ClientInvocation> invocation, int partitionId) {
                    invocation->getClientMessage()->setPartitionId(partitionId);
                    send(invocation, getOwnerConnection());
                }

                void NonSmartClientInvocationService::invokeOnRandomTarget(
                        std::shared_ptr<impl::ClientInvocation> invocation) {
                    send(invocation, getOwnerConnection());
                }

                void
                NonSmartClientInvocationService::invokeOnTarget(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                const std::shared_ptr<Address> &target) {
                    send(invocation, getOwnerConnection());
                }

                std::shared_ptr<connection::Connection> NonSmartClientInvocationService::getOwnerConnection() {
                    std::shared_ptr<Address> ownerConnectionAddress = connectionManager->getOwnerConnectionAddress();
                    if (ownerConnectionAddress.get() == NULL) {
                        throw exception::IOException("NonSmartClientInvocationService::getOwnerConnection",
                                                     "Owner connection address is not available.");
                    }
                    std::shared_ptr<connection::Connection> ownerConnection = connectionManager->getActiveConnection(
                            *ownerConnectionAddress);
                    if (ownerConnection.get() == NULL) {
                        throw exception::IOException("NonSmartClientInvocationService::getOwnerConnection",
                                                     "Owner connection is not available.");
                    }
                    return ownerConnection;
                }

                DefaultAddressProvider::DefaultAddressProvider(config::ClientNetworkConfig &networkConfig,
                                                               bool noOtherAddressProviderExist) : networkConfig(
                        networkConfig), noOtherAddressProviderExist(noOtherAddressProviderExist) {
                }

                std::vector<Address> DefaultAddressProvider::loadAddresses() {
                    std::vector<Address> addresses = networkConfig.getAddresses();
                    if (addresses.empty() && noOtherAddressProviderExist) {
                        addresses.push_back(Address("127.0.0.1", 5701));
                    }

                    // TODO Implement AddressHelper to add alternative ports for the same address

                    return addresses;
                }

                impl::ClientClusterServiceImpl::ClientClusterServiceImpl(hazelcast::client::spi::ClientContext &client)
                        : client(client) {
                }

                std::string ClientClusterServiceImpl::addMembershipListenerWithoutInit(
                        const std::shared_ptr<MembershipListener> &listener) {
                    std::string id = util::UuidUtil::newUnsecureUuidString();
                    listeners.put(id, listener);
                    listener->setRegistrationId(id);
                    return id;
                }

                std::shared_ptr<Member> ClientClusterServiceImpl::getMember(const Address &address) {
                    std::map<Address, std::shared_ptr<Member> > currentMembers = members.get();
                    const std::map<hazelcast::client::Address, std::shared_ptr<hazelcast::client::Member> >::iterator &it = currentMembers.find(
                            address);
                    if (it == currentMembers.end()) {
                        return std::shared_ptr<Member>();
                    }
                    return it->second;
                }

                std::shared_ptr<Member> ClientClusterServiceImpl::getMember(const std::string &uuid) {
                    std::vector<Member> memberList = getMemberList();
                    for (const Member &member : memberList) {
                        if (uuid == member.getUuid()) {
                            return std::shared_ptr<Member>(new Member(member));
                        }
                    }
                    return std::shared_ptr<Member>();
                }

                std::vector<Member> ClientClusterServiceImpl::getMemberList() {
                    typedef std::map<Address, std::shared_ptr<Member> > MemberMap;
                    MemberMap memberMap = members.get();
                    std::vector<Member> memberList;
                    for (const MemberMap::value_type &entry : memberMap) {
                        memberList.push_back(*entry.second);
                    }
                    return memberList;
                }

                void ClientClusterServiceImpl::initMembershipListener(MembershipListener &listener) {
                    if (listener.shouldRequestInitialMembers()) {
                        Cluster &cluster = client.getCluster();
                        std::vector<Member> memberCollection = getMemberList();
                        InitialMembershipEvent event(cluster, std::set<Member>(memberCollection.begin(),
                                                                               memberCollection.end()));
                        ((InitialMembershipListener &) listener).init(event);
                    }
                }

                void ClientClusterServiceImpl::start() {
                    clientMembershipListener.reset(new ClientMembershipListener(client));

                    ClientConfig &config = client.getClientConfig();
                    const std::set<std::shared_ptr<MembershipListener> > &membershipListeners = config.getManagedMembershipListeners();

                    for (const std::shared_ptr<MembershipListener> &listener : membershipListeners) {
                        addMembershipListenerWithoutInit(listener);
                    }
                }

                void ClientClusterServiceImpl::handleMembershipEvent(const MembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const Member &member = event.getMember();
                    std::map<Address, std::shared_ptr<Member> > newMap = members.get();
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        newMap[member.getAddress()] = std::shared_ptr<Member>(new Member(member));
                    } else {
                        newMap.erase(member.getAddress());
                    }
                    members = newMap;
                    fireMembershipEvent(event);
                }

                void ClientClusterServiceImpl::fireMembershipEvent(const MembershipEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners.values()) {
                        if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                            listener->memberAdded(event);
                        } else {
                            listener->memberRemoved(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::fireMemberAttributeEvent(const MemberAttributeEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners.values()) {
                        listener->memberAttributeChanged(event);
                    }
                }

                void ClientClusterServiceImpl::handleInitialMembershipEvent(const InitialMembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const std::vector<Member> &initialMembers = event.getMembers();
                    std::map<Address, std::shared_ptr<Member> > newMap;
                    for (const Member &initialMember : initialMembers) {
                        newMap[initialMember.getAddress()] = std::shared_ptr<Member>(
                                new Member(initialMember));
                    }
                    members.set(newMap);
                    fireInitialMembershipEvent(event);

                }

                void ClientClusterServiceImpl::fireInitialMembershipEvent(const InitialMembershipEvent &event) {
                    for (const std::shared_ptr<MembershipListener> &listener : listeners.values()) {
                        if (listener->shouldRequestInitialMembers()) {
                            ((InitialMembershipListener *) listener.get())->init(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::shutdown() {
                }

                void ClientClusterServiceImpl::listenMembershipEvents(
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    clientMembershipListener->listenMembershipEvents(clientMembershipListener, ownerConnection);
                }

                std::string
                ClientClusterServiceImpl::addMembershipListener(const std::shared_ptr<MembershipListener> &listener) {
                    if (listener.get() == NULL) {
                        throw exception::NullPointerException("ClientClusterServiceImpl::addMembershipListener",
                                                              "listener can't be null");
                    }

                    util::LockGuard guard(initialMembershipListenerMutex);
                    std::string id = addMembershipListenerWithoutInit(listener);
                    initMembershipListener(*listener);
                    return id;
                }

                bool ClientClusterServiceImpl::removeMembershipListener(const std::string &registrationId) {
                    return listeners.remove(registrationId).get() != NULL;
                }

                std::vector<Member>
                ClientClusterServiceImpl::getMembers(const cluster::memberselector::MemberSelector &selector) {
                    std::vector<Member> result;
                    for (const Member &member : getMemberList()) {
                        if (selector.select(member)) {
                            result.push_back(member);
                        }
                    }

                    return result;
                }

                int ClientClusterServiceImpl::getSize() {
                    return (int) getMemberList().size();
                }

                Client ClientClusterServiceImpl::getLocalClient() const {
                    connection::ClientConnectionManagerImpl &cm = client.getConnectionManager();
                    std::shared_ptr<connection::Connection> connection = cm.getOwnerConnection();
                    std::shared_ptr<Address> inetSocketAddress =
                            connection.get() != NULL ? std::shared_ptr<Address>(connection->getLocalSocketAddress())
                                                     : std::shared_ptr<Address>();
                    const std::shared_ptr<protocol::Principal> principal = cm.getPrincipal();
                    std::shared_ptr<std::string> uuid =
                            principal.get() != NULL ? std::make_shared<std::string>(*principal->getUuid())
                                                    : std::shared_ptr<std::string>();
                    return Client(uuid, inetSocketAddress, client.getName());
                }

                SmartClientInvocationService::SmartClientInvocationService(ClientContext &client)
                        : AbstractClientInvocationService(client),
                          loadBalancer(*client.getClientConfig().getLoadBalancer()) {}

                void
                SmartClientInvocationService::invokeOnConnection(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                 std::shared_ptr<connection::Connection> connection) {
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnPartitionOwner(
                        std::shared_ptr<impl::ClientInvocation> invocation, int partitionId) {
                    std::shared_ptr<Address> owner = partitionService.getPartitionOwner(partitionId);
                    if (owner.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::invokeOnPartitionOwner")
                                << "Partition does not have an owner. partitionId: " << partitionId).build();
                    }
                    if (!isMember(*owner)) {
                        throw (exception::ExceptionBuilder<exception::TargetNotMemberException>(
                                "SmartClientInvocationService::invokeOnPartitionOwner") << "Partition owner '" << *owner
                                                                                        << "' is not a member.").build();
                    }
                    invocation->getClientMessage()->setPartitionId(partitionId);
                    std::shared_ptr<connection::Connection> connection = getOrTriggerConnect(owner);
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnRandomTarget(
                        std::shared_ptr<impl::ClientInvocation> invocation) {
                    std::shared_ptr<Address> randomAddress = getRandomAddress();
                    if (randomAddress.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::invokeOnRandomTarget")
                                << "No address found to invoke").build();
                    }
                    std::shared_ptr<connection::Connection> connection = getOrTriggerConnect(randomAddress);
                    send(invocation, connection);
                }

                void SmartClientInvocationService::invokeOnTarget(std::shared_ptr<impl::ClientInvocation> invocation,
                                                                  const std::shared_ptr<Address> &target) {
                    if (!isMember(*target)) {
                        throw (exception::ExceptionBuilder<exception::TargetNotMemberException>(
                                "SmartClientInvocationService::invokeOnTarget")
                                << "Target '" << *target << "' is not a member.").build();
                    }
                    std::shared_ptr<connection::Connection> connection = getOrTriggerConnect(target);
                    invokeOnConnection(invocation, connection);

                }

                bool SmartClientInvocationService::isMember(const Address &target) const {
                    std::shared_ptr<Member> member = client.getClientClusterService().getMember(target);
                    return member.get() != NULL;
                }

                std::shared_ptr<connection::Connection>
                SmartClientInvocationService::getOrTriggerConnect(const std::shared_ptr<Address> &target) const {
                    std::shared_ptr<connection::Connection> connection = connectionManager->getOrTriggerConnect(
                            *target);
                    if (connection.get() == NULL) {
                        throw (exception::ExceptionBuilder<exception::IOException>(
                                "SmartClientInvocationService::getOrTriggerConnect")
                                << "No available connection to address " << target).build();
                    }
                    return connection;
                }

                std::shared_ptr<Address> SmartClientInvocationService::getRandomAddress() {
                    // TODO: Change loadBalancer to return pointer as in Java so that it is possible to denote the
                    // case where no address can be provided
                    std::shared_ptr<Address> address;
                    try {
                        Member member = loadBalancer.next();
                        address = std::shared_ptr<Address>(new Address(member.getAddress()));
                    } catch (exception::IllegalStateException &) {
                        // do nothing, there is no available server
                    }
                    return address;
                }

                int ClientMembershipListener::INITIAL_MEMBERS_TIMEOUT_SECONDS = 5;

                ClientMembershipListener::ClientMembershipListener(ClientContext &client)
                        : client(client), logger(client.getLogger()),
                          clusterService(static_cast<ClientClusterServiceImpl &>(client.getClientClusterService())),
                          partitionService((ClientPartitionServiceImpl &) client.getPartitionService()),
                          connectionManager(client.getConnectionManager()) {}

                void ClientMembershipListener::handleMemberEventV10(const Member &member, const int32_t &eventType) {
                    switch (eventType) {
                        case MembershipEvent::MEMBER_ADDED:
                            memberAdded(member);
                            break;
                        case MembershipEvent::MEMBER_REMOVED:
                            memberRemoved(member);
                            break;
                        default:
                            logger.warning("Unknown event type: ", eventType);
                    }
                    partitionService.refreshPartitions();
                }

                void ClientMembershipListener::handleMemberListEventV10(const std::vector<Member> &initialMembers) {
                    std::map<std::string, Member> prevMembers;
                    if (!members.empty()) {
                        for (const Member &member : members) {
                            prevMembers[member.getUuid()] = member;
                        }
                        members.clear();
                    }

                    for (const Member &initialMember : initialMembers) {
                        members.insert(initialMember);
                    }

                    if (prevMembers.empty()) {
                        //this means this is the first time client connected to server
                        logger.info(membersString());
                        clusterService.handleInitialMembershipEvent(
                                InitialMembershipEvent(client.getCluster(), members));
                        initialListFetchedLatch.get()->countDown();
                        return;
                    }

                    std::vector<MembershipEvent> events = detectMembershipEvents(prevMembers);
                    logger.info(membersString());
                    fireMembershipEvent(events);
                    initialListFetchedLatch.get()->countDown();
                }

                void
                ClientMembershipListener::handleMemberAttributeChangeEventV10(const std::string &uuid,
                                                                              const std::string &key,
                                                                              const int32_t &operationType,
                                                                              std::unique_ptr<std::string> &value) {
                    std::vector<Member> members = clusterService.getMemberList();
                    for (Member &target : members) {
                        if (target.getUuid() == uuid) {
                            Member::MemberAttributeOperationType type = (Member::MemberAttributeOperationType) operationType;
                            target.updateAttribute(type, key, value);
                            MemberAttributeEvent memberAttributeEvent(client.getCluster(), target,
                                                                      (MemberAttributeEvent::MemberAttributeOperationType) type,
                                                                      key, value.get() ? (*value) : "");
                            clusterService.fireMemberAttributeEvent(memberAttributeEvent);
                            break;
                        }
                    }

                }

                void ClientMembershipListener::memberAdded(const Member &member) {
                    members.insert(member);
                    logger.info(membersString());
                    MembershipEvent event(client.getCluster(), member, MembershipEvent::MEMBER_ADDED,
                                          std::vector<Member>(members.begin(), members.end()));
                    clusterService.handleMembershipEvent(event);
                }

                std::string ClientMembershipListener::membersString() const {
                    std::stringstream out;
                    out << std::endl << std::endl << "Members [" << members.size() << "]  {";

                    for (const Member &member : members) {
                        out << std::endl << "\t" << member;
                    }
                    out << std::endl << "}" << std::endl;

                    return out.str();
                }

                void ClientMembershipListener::memberRemoved(const Member &member) {
                    members.erase(member);
                    logger.info(membersString());
                    std::shared_ptr<connection::Connection> connection = connectionManager.getActiveConnection(
                            member.getAddress());
                    if (connection.get() != NULL) {
                        connection->close("", newTargetDisconnectedExceptionCausedByMemberLeftEvent(connection));
                    }
                    MembershipEvent event(client.getCluster(), member, MembershipEvent::MEMBER_REMOVED,
                                          std::vector<Member>(members.begin(), members.end()));
                    clusterService.handleMembershipEvent(event);
                }

                std::shared_ptr<exception::IException>
                ClientMembershipListener::newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                        const std::shared_ptr<connection::Connection> &connection) {
                    return (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                            "ClientMembershipListener::newTargetDisconnectedExceptionCausedByMemberLeftEvent")
                            << "The client has closed the connection to this member, after receiving a member left event from the cluster. "
                            << *connection).buildShared();
                }

                std::vector<MembershipEvent>
                ClientMembershipListener::detectMembershipEvents(std::map<std::string, Member> &prevMembers) {
                    std::vector<MembershipEvent> events;

                    const std::set<Member> &eventMembers = members;

                    std::vector<Member> newMembers;
                    for (const Member &member : members) {
                        std::map<std::string, Member>::iterator formerEntry = prevMembers.find(
                                member.getUuid());
                        if (formerEntry != prevMembers.end()) {
                            prevMembers.erase(formerEntry);
                        } else {
                            newMembers.push_back(member);
                        }
                    }

                    // removal events should be added before added events
                    typedef const std::map<std::string, Member> MemberMap;
                    for (const MemberMap::value_type &member : prevMembers) {
                        events.push_back(MembershipEvent(client.getCluster(), member.second,
                                                         MembershipEvent::MEMBER_REMOVED,
                                                         std::vector<Member>(eventMembers.begin(),
                                                                             eventMembers.end())));
                        const Address &address = member.second.getAddress();
                        if (clusterService.getMember(address).get() == NULL) {
                            std::shared_ptr<connection::Connection> connection = connectionManager.getActiveConnection(
                                    address);
                            if (connection.get() != NULL) {
                                connection->close("",
                                                  newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                                                          connection));
                            }
                        }
                    }
                    for (const Member &member : newMembers) {
                        events.push_back(
                                MembershipEvent(client.getCluster(), member, MembershipEvent::MEMBER_ADDED,
                                                std::vector<Member>(eventMembers.begin(),
                                                                    eventMembers.end())));
                    }

                    return events;
                }

                void ClientMembershipListener::fireMembershipEvent(std::vector<MembershipEvent> &events) {
                    for (const MembershipEvent &event : events) {
                        clusterService.handleMembershipEvent(event);
                    }
                }

                void
                ClientMembershipListener::listenMembershipEvents(
                        const std::shared_ptr<ClientMembershipListener> &listener,
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    listener->initialListFetchedLatch = std::shared_ptr<util::CountDownLatch>(
                            new util::CountDownLatch(1));
                    std::unique_ptr<protocol::ClientMessage> clientMessage = protocol::codec::ClientAddMembershipListenerCodec::encodeRequest(
                            false);
                    std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(listener->client,
                                                                                            clientMessage, "",
                                                                                            ownerConnection);
                    invocation->setEventHandler(listener);
                    invocation->invokeUrgent()->get();
                    listener->waitInitialMemberListFetched();
                }

                void ClientMembershipListener::waitInitialMemberListFetched() {
                    bool success = initialListFetchedLatch.get()->await(INITIAL_MEMBERS_TIMEOUT_SECONDS);
                    if (!success) {
                        logger.warning("Error while getting initial member list from cluster!");
                    }
                }

                const int ClientExecutionServiceImpl::SHUTDOWN_CHECK_INTERVAL_SECONDS = 30;

                ClientExecutionServiceImpl::ClientExecutionServiceImpl(const std::string &name,
                                                                       const ClientProperties &clientProperties,
                                                                       int32_t poolSize, util::ILogger &logger)
                        : logger(logger) {

                    int internalPoolSize = clientProperties.getInteger(clientProperties.getInternalExecutorPoolSize());
                    if (internalPoolSize <= 0) {
                        internalPoolSize = util::IOUtil::to_value<int>(
                                ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT);
                    }

                    int32_t executorPoolSize = poolSize;
                    if (executorPoolSize <= 0) {
                        executorPoolSize = util::RuntimeAvailableProcessors::get();
                    }
                    if (executorPoolSize <= 0) {
                        executorPoolSize = 4; // hard coded thread pool count in case we could not get the processor count
                    }

                    internalExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".internal-", internalPoolSize,
                                                                  INT32_MAX));

                    userExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".user-", executorPoolSize,
                                                                  INT32_MAX));
                }

                void ClientExecutionServiceImpl::execute(const std::shared_ptr<util::Runnable> &command) {
                    internalExecutor->execute(command);
                }

                void ClientExecutionServiceImpl::start() {
                    userExecutor->start();
                    internalExecutor->start();
                }

                void ClientExecutionServiceImpl::shutdown() {
                    shutdownExecutor("user", *userExecutor, logger);
                    shutdownExecutor("internal", *internalExecutor, logger);
                }

                void
                ClientExecutionServiceImpl::shutdownExecutor(const std::string &name, util::ExecutorService &executor,
                                                             util::ILogger &logger) {
                    try {
                        int64_t startTimeMilliseconds = util::currentTimeMillis();
                        bool success = false;
                        // Wait indefinitely until the threads gracefully shutdown an log the problem periodically.
                        while (!success) {
                            int64_t waitTimeMillis = 100;
                            auto intervalStartTimeMillis = util::currentTimeMillis();
                            while (!success && util::currentTimeMillis() - intervalStartTimeMillis <
                                               1000 * SHUTDOWN_CHECK_INTERVAL_SECONDS) {
                                executor.shutdown();
                                auto &executorService = static_cast<util::impl::SimpleExecutorService &>(executor);
                                success = executorService.awaitTerminationMilliseconds(waitTimeMillis);
                            }

                            if (!success) {
                                logger.warning(name, " executor awaitTermination could not be completed in ",
                                               (util::currentTimeMillis() - startTimeMilliseconds), " msecs.");
                            }
                        }
                    } catch (exception::InterruptedException &e) {
                        logger.warning(name, " executor await termination is interrupted. ", e);
                    }
                }

                void
                ClientExecutionServiceImpl::scheduleWithRepetition(const std::shared_ptr<util::Runnable> &command,
                                                                   int64_t initialDelayInMillis,
                                                                   int64_t periodInMillis) {
                    internalExecutor->scheduleAtFixedRate(command, initialDelayInMillis, periodInMillis);
                }

                void ClientExecutionServiceImpl::schedule(const std::shared_ptr<util::Runnable> &command,
                                                          int64_t initialDelayInMillis) {
                    internalExecutor->schedule(command, initialDelayInMillis);
                }

                const std::shared_ptr<util::ExecutorService> ClientExecutionServiceImpl::getUserExecutor() const {
                    return userExecutor;
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   int partitionId) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(std::shared_ptr<protocol::ClientMessage>(std::move(clientMessage))),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(partitionId),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), this->clientMessage,
                                                           clientContext.getCallIdSequence())) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   const std::shared_ptr<connection::Connection> &connection) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(std::shared_ptr<protocol::ClientMessage>(std::move(clientMessage))),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        connection(connection),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), std::move(clientMessage),
                                                           clientContext.getCallIdSequence())) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(std::shared_ptr<protocol::ClientMessage>(std::move(clientMessage))),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), this->clientMessage,
                                                           clientContext.getCallIdSequence())) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName, const Address &address) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(std::shared_ptr<protocol::ClientMessage>(std::move(clientMessage))),
                        callIdSequence(clientContext.getCallIdSequence()),
                        address(new Address(address)),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), this->clientMessage,
                                                           clientContext.getCallIdSequence())) {
                }

                ClientInvocation::~ClientInvocation() {
                }

                std::shared_ptr<ClientInvocationFuture> ClientInvocation::invoke() {
                    assert (clientMessage.get() != NULL);
                    clientMessage.get()->setCorrelationId(callIdSequence->next());
                    invokeOnSelection(shared_from_this());
                    return clientInvocationFuture;
                }

                std::shared_ptr<ClientInvocationFuture> ClientInvocation::invokeUrgent() {
                    assert (clientMessage.get() != NULL);
                    clientMessage.get()->setCorrelationId(callIdSequence->forceNext());
                    invokeOnSelection(shared_from_this());
                    return clientInvocationFuture;
                }

                void ClientInvocation::invokeOnSelection(const std::shared_ptr<ClientInvocation> &invocation) {
                    invocation->invokeCount++;
                    try {
                        if (invocation->isBindToSingleConnection()) {
                            invocation->invocationService.invokeOnConnection(invocation, invocation->connection);
                        } else if (invocation->partitionId != UNASSIGNED_PARTITION) {
                            invocation->invocationService.invokeOnPartitionOwner(invocation, invocation->partitionId);
                        } else if (invocation->address.get() != NULL) {
                            invocation->invocationService.invokeOnTarget(invocation, invocation->address);
                        } else {
                            invocation->invocationService.invokeOnRandomTarget(invocation);
                        }
                    } catch (exception::HazelcastOverloadException &) {
                        throw;
                    } catch (exception::IException &e) {
                        invocation->notifyException(std::shared_ptr<exception::IException>(e.clone()));
                    }
                }

                bool ClientInvocation::isBindToSingleConnection() const {
                    return connection.get() != NULL;
                }

                void ClientInvocation::run() {
                    retry();
                }

                void ClientInvocation::retry() {
                    // retry modifies the client message and should not reuse the client message.
                    // It could be the case that it is in write queue of the connection.
                    clientMessage = copyMessage();
                    // first we force a new invocation slot because we are going to return our old invocation slot immediately after
                    // It is important that we first 'force' taking a new slot; otherwise it could be that a sneaky invocation gets
                    // through that takes our slot!
                    clientMessage.get()->setCorrelationId(callIdSequence->forceNext());
                    //we release the old slot
                    callIdSequence->complete();

                    try {
                        invokeOnSelection(shared_from_this());
                    } catch (exception::IException &e) {
                        clientInvocationFuture->complete(std::shared_ptr<exception::IException>(e.clone()));
                    }
                }

                void ClientInvocation::notifyException(const std::shared_ptr<exception::IException> &exception) {
                    if (!lifecycleService.isRunning()) {
                        std::shared_ptr<exception::IException> notActiveException(
                                new exception::HazelcastClientNotActiveException(exception->getSource(),
                                                                                 "Client is shutting down", exception));

                        clientInvocationFuture->complete(notActiveException);
                        return;
                    }

                    if (isNotAllowedToRetryOnSelection(*exception)) {
                        clientInvocationFuture->complete(exception);
                        return;
                    }

                    bool retry = isRetrySafeException(*exception)
                                 || invocationService.isRedoOperation()
                                 || (exception->getErrorCode() == protocol::TARGET_DISCONNECTED &&
                                     clientMessage.get()->isRetryable());

                    if (!retry) {
                        clientInvocationFuture->complete(exception);
                        return;
                    }

                    int64_t timePassed = util::currentTimeMillis() - startTimeMillis;
                    if (timePassed > invocationService.getInvocationTimeoutMillis()) {
                        if (logger.isFinestEnabled()) {
                            std::ostringstream out;
                            out << "Exception will not be retried because invocation timed out. " << exception->what();
                            logger.finest(out.str());
                        }

                        clientInvocationFuture->complete(newOperationTimeoutException(*exception));
                        return;
                    }

                    try {
                        execute();
                    } catch (exception::RejectedExecutionException &) {
                        clientInvocationFuture->complete(exception);
                    }

                }

                bool ClientInvocation::isNotAllowedToRetryOnSelection(exception::IException &exception) {
                    if (isBindToSingleConnection() && exception.getErrorCode() == protocol::IO) {
                        return true;
                    }

                    if (address.get() != NULL && exception.getErrorCode() == protocol::TARGET_NOT_MEMBER &&
                        clientClusterService.getMember(*address).get() == NULL) {
                        //when invocation send over address
                        //if exception is target not member and
                        //address is not available in member list , don't retry
                        return true;
                    }
                    return false;
                }

                bool ClientInvocation::isRetrySafeException(exception::IException &exception) {
                    int32_t errorCode = exception.getErrorCode();
                    if (errorCode == protocol::IO || errorCode == protocol::HAZELCAST_INSTANCE_NOT_ACTIVE ||
                        exception.isRetryable()) {
                        return true;
                    }

                    return false;
                }

                std::shared_ptr<exception::OperationTimeoutException>
                ClientInvocation::newOperationTimeoutException(exception::IException &exception) {
                    int64_t nowInMillis = util::currentTimeMillis();

                    return (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                            "ClientInvocation::newOperationTimeoutException") << *this
                                                                              << " timed out because exception occurred after client invocation timeout "
                                                                              << "Current time :"
                                                                              << invocationService.getInvocationTimeoutMillis()
                                                                              << util::StringUtil::timeToString(
                                                                                      nowInMillis) << ". "
                                                                              << "Start time: "
                                                                              << util::StringUtil::timeToString(
                                                                                      startTimeMillis)
                                                                              << ". Total elapsed time: "
                                                                              << (nowInMillis - startTimeMillis)
                                                                              << " ms. ").buildShared();
                }

                std::ostream &operator<<(std::ostream &os, const ClientInvocation &invocation) {
                    std::ostringstream target;
                    if (invocation.isBindToSingleConnection()) {
                        target << "connection " << *invocation.connection;
                    } else if (invocation.partitionId != -1) {
                        target << "partition " << invocation.partitionId;
                    } else if (invocation.address.get() != NULL) {
                        target << "address " << *invocation.address;
                    } else {
                        target << "random";
                    }
                    ClientInvocation &nonConstInvocation = const_cast<ClientInvocation &>(invocation);
                    os << "ClientInvocation{" << "clientMessage = " << *nonConstInvocation.clientMessage.get()
                       << ", objectName = "
                       << invocation.objectName << ", target = " << target.str() << ", sendConnection = ";
                    std::shared_ptr<connection::Connection> sendConnection = nonConstInvocation.sendConnection.get();
                    if (sendConnection.get()) {
                        os << *sendConnection;
                    } else {
                        os << "null";
                    }
                    os << '}';

                    return os;
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                           const std::string &objectName,
                                                                           int partitionId) {
                    std::shared_ptr<ClientInvocation> invocation = std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, partitionId));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                           const std::string &objectName,
                                                                           const std::shared_ptr<connection::Connection> &connection) {
                    std::shared_ptr<ClientInvocation> invocation = std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, connection));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }


                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                           const std::string &objectName,
                                                                           const Address &address) {
                    std::shared_ptr<ClientInvocation> invocation = std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, address));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                std::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                           std::unique_ptr<protocol::ClientMessage> &clientMessage,
                                                                           const std::string &objectName) {
                    std::shared_ptr<ClientInvocation> invocation = std::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                std::shared_ptr<connection::Connection> ClientInvocation::getSendConnection() {
                    return sendConnection;
                }

                std::shared_ptr<connection::Connection> ClientInvocation::getSendConnectionOrWait() {
                    while (sendConnection.get().get() == NULL && !clientInvocationFuture->isDone()) {
                        // TODO: Make sleep interruptible
                        util::sleepmillis(retryPauseMillis);
                    }
                    return sendConnection;
                }

                void
                ClientInvocation::setSendConnection(const std::shared_ptr<connection::Connection> &sendConnection) {
                    ClientInvocation::sendConnection = sendConnection;
                }

                void ClientInvocation::notify(const std::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    if (clientMessage.get() == NULL) {
                        throw exception::IllegalArgumentException("response can't be null");
                    }
                    clientInvocationFuture->complete(clientMessage);
                }

                const std::shared_ptr<protocol::ClientMessage> ClientInvocation::getClientMessage() {
                    return clientMessage.get();
                }

                const std::shared_ptr<EventHandler<protocol::ClientMessage> > &
                ClientInvocation::getEventHandler() const {
                    return eventHandler;
                }

                void ClientInvocation::setEventHandler(
                        const std::shared_ptr<EventHandler<protocol::ClientMessage> > &eventHandler) {
                    ClientInvocation::eventHandler = eventHandler;
                }

                void ClientInvocation::execute() {
                    if (invokeCount < MAX_FAST_INVOCATION_COUNT) {
                        // fast retry for the first few invocations
                        executionService->execute(std::shared_ptr<util::Runnable>(shared_from_this()));
                    } else {
                        // progressive retry delay
                        int64_t delayMillis = util::min<int64_t>(1 << (invokeCount - MAX_FAST_INVOCATION_COUNT),
                                                                 retryPauseMillis);
                        executionService->schedule(shared_from_this(), delayMillis);
                    }
                }

                const std::string ClientInvocation::getName() const {
                    return "ClientInvocation";
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocation::copyMessage() {
                    return std::shared_ptr<protocol::ClientMessage>(new protocol::ClientMessage(*clientMessage.get()));
                }

                std::shared_ptr<util::Executor> ClientInvocation::getUserExecutor() {
                    return executionService->getUserExecutor();
                }

                ClientContext &impl::ClientTransactionManagerServiceImpl::getClient() const {
                    return client;
                }

                ClientTransactionManagerServiceImpl::ClientTransactionManagerServiceImpl(ClientContext &client,
                                                                                         hazelcast::client::LoadBalancer &loadBalancer)
                        : client(client), loadBalancer(loadBalancer) {}

                std::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::connect() {
                    AbstractClientInvocationService &invocationService = (AbstractClientInvocationService &) client.getInvocationService();
                    int64_t startTimeMillis = util::currentTimeMillis();
                    int64_t invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                    ClientConfig &clientConfig = client.getClientConfig();
                    bool smartRouting = clientConfig.getNetworkConfig().isSmartRouting();

                    while (client.getLifecycleService().isRunning()) {
                        try {
                            if (smartRouting) {
                                return tryConnectSmart();
                            } else {
                                return tryConnectUnisocket();
                            }
                        } catch (exception::HazelcastClientOfflineException &) {
                            throw;
                        } catch (exception::IException &e) {
                            if (util::currentTimeMillis() - startTimeMillis > invocationTimeoutMillis) {
                                throw newOperationTimeoutException(e, invocationTimeoutMillis, startTimeMillis);
                            }
                        }
                        util::sleepmillis(invocationService.getInvocationRetryPauseMillis());
                    }
                    throw exception::HazelcastClientNotActiveException("ClientTransactionManagerServiceImpl::connect",
                                                                       "Client is shutdown");
                }

                std::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::tryConnectSmart() {
                    try {
                        Address address = getRandomAddress();
                        return client.getConnectionManager().getOrConnect(address);
                    } catch (exception::IllegalStateException &) {
                        // loadBalancer.next may throw IllegalStateException when no available member
                        throwException(true);
                    }
                    return std::shared_ptr<connection::Connection>();
                }

                Address ClientTransactionManagerServiceImpl::getRandomAddress() {
                    Member member = loadBalancer.next();
                    return member.getAddress();
                }

                std::shared_ptr<connection::Connection> ClientTransactionManagerServiceImpl::tryConnectUnisocket() {
                    std::shared_ptr<connection::Connection> connection = client.getConnectionManager().getOwnerConnection();

                    if (connection.get()) {
                        return connection;
                    }
                    return throwException(false);
                }

                std::shared_ptr<connection::Connection>
                ClientTransactionManagerServiceImpl::throwException(bool smartRouting) {
                    ClientConfig clientConfig = client.getClientConfig();
                    const config::ClientConnectionStrategyConfig &connectionStrategyConfig = clientConfig.getConnectionStrategyConfig();
                    config::ClientConnectionStrategyConfig::ReconnectMode reconnectMode = connectionStrategyConfig.getReconnectMode();
                    if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC) {
                        throw exception::HazelcastClientOfflineException(
                                "ClientTransactionManagerServiceImpl::throwException", "Hazelcast client is offline");
                    }
                    if (smartRouting) {
                        std::vector<Member> members = client.getCluster().getMembers();
                        std::ostringstream msg;
                        if (members.empty()) {
                            msg
                                    << "No address was return by the LoadBalancer since there are no members in the cluster";
                        } else {
                            msg << "No address was return by the LoadBalancer. "
                                << "But the cluster contains the following members:{";
                            for (std::vector<Member>::const_iterator it = members.begin();;) {
                                msg << (*it);

                                ++it;

                                if (it != members.end()) {
                                    msg << " , ";
                                } else {
                                    break;
                                }
                            }
                            msg << "}";
                        }
                        throw exception::IllegalStateException("ClientTransactionManagerServiceImpl::throwException",
                                                               msg.str());
                    }
                    throw exception::IllegalStateException("ClientTransactionManagerServiceImpl::throwException",
                                                           "No active connection is found");
                }

                exception::OperationTimeoutException
                ClientTransactionManagerServiceImpl::newOperationTimeoutException(exception::IException &throwable,
                                                                                  int64_t invocationTimeoutMillis,
                                                                                  int64_t startTimeMillis) {
                    std::ostringstream sb;
                    sb
                            << "Creating transaction context timed out because exception occurred after client invocation timeout "
                            << invocationTimeoutMillis << " ms. " << "Current time: "
                            << util::StringUtil::timeToString(util::currentTimeMillis()) << ". " << "Start time: "
                            << util::StringUtil::timeToString(startTimeMillis) << ". Total elapsed time: "
                            << (util::currentTimeMillis() - startTimeMillis) << " ms. ";
                    return exception::OperationTimeoutException(
                            "ClientTransactionManagerServiceImpl::newOperationTimeoutException", sb.str(),
                            std::shared_ptr<exception::IException>(throwable.clone()));

                }

                ClientInvocationFuture::ClientInvocationFuture(
                        const std::shared_ptr<Executor> &defaultExecutor, ILogger &logger,
                        const std::shared_ptr<protocol::ClientMessage> &request,
                        const std::shared_ptr<sequence::CallIdSequence> &callIdSequence)
                        : AbstractInvocationFuture<protocol::ClientMessage>(defaultExecutor, logger), request(request),
                          callIdSequence(callIdSequence) {}

                std::string ClientInvocationFuture::invocationToString() const {
                    auto existingInvocation = getInvocation();
                    if (!existingInvocation) {
                        return std::string();
                    }
                    std::ostringstream out;
                    out << existingInvocation;
                    return out.str();
                }

                void ClientInvocationFuture::andThen(
                        const std::shared_ptr<ExecutionCallback<protocol::ClientMessage> > &callback,
                        const std::shared_ptr<Executor> &executor) {
                    AbstractInvocationFuture<protocol::ClientMessage>::andThen(
                            std::shared_ptr<client::ExecutionCallback<protocol::ClientMessage> >(
                                    new InternalDelegatingExecutionCallback(callback, callIdSequence)), executor);
                }

                void ClientInvocationFuture::andThen(
                        const std::shared_ptr<ExecutionCallback<protocol::ClientMessage> > &callback) {
                    AbstractInvocationFuture<protocol::ClientMessage>::andThen(callback);
                }

                void ClientInvocationFuture::onComplete() {
                    callIdSequence->complete();
                }

                std::shared_ptr<protocol::ClientMessage> ClientInvocationFuture::resolveAndThrowIfException(
                        const std::shared_ptr<AbstractInvocationFuture<protocol::ClientMessage>::BaseState> &response) {
                    if (response->getType() == BaseState::Exception) {
                        std::shared_ptr<ExceptionState> exceptionState = std::static_pointer_cast<ExceptionState>(
                                response);
                        std::shared_ptr<IException> exception = exceptionState->getException();
                        int32_t errorCode = exception->getErrorCode();
                        if (errorCode == ExecutionException::ERROR_CODE || errorCode == protocol::CANCELLATION ||
                            errorCode == protocol::INTERRUPTED) {
                            exception->raise();
                        }

                        throw ExecutionException("ClientInvocationFuture::resolveAndThrowIfException",
                                                 "ExecutionException occured.", exception);
                    }
                    std::shared_ptr<AbstractInvocationFuture<protocol::ClientMessage>::ValueState> valueState =
                            std::static_pointer_cast<AbstractInvocationFuture<protocol::ClientMessage>::ValueState>(
                                    response);

                    return valueState->getValue();
                }

                const std::shared_ptr<ClientInvocation> ClientInvocationFuture::getInvocation() const {
                    return invocation.lock();
                }

                void ClientInvocationFuture::setInvocation(
                        const std::weak_ptr<spi::impl::ClientInvocation> &invocation) {
                    this->invocation = invocation;
                }

                ClientInvocationFuture::InternalDelegatingExecutionCallback::InternalDelegatingExecutionCallback(
                        const std::shared_ptr<client::ExecutionCallback<protocol::ClientMessage> > &callback,
                        const std::shared_ptr<sequence::CallIdSequence> &callIdSequence) : callback(callback),
                                                                                           callIdSequence(
                                                                                                   callIdSequence) {
                    this->callIdSequence->forceNext();
                }

                void ClientInvocationFuture::InternalDelegatingExecutionCallback::onResponse(
                        const std::shared_ptr<protocol::ClientMessage> &message) {
                    try {
                        callback->onResponse(message);
                        callIdSequence->complete();
                    } catch (...) {
                        callIdSequence->complete();
                    }
                }

                void ClientInvocationFuture::InternalDelegatingExecutionCallback::onFailure(
                        const std::shared_ptr<exception::IException> &e) {
                    try {
                        callback->onFailure(e);
                        callIdSequence->complete();
                    } catch (...) {
                        callIdSequence->complete();
                    }
                }

                AwsAddressProvider::AwsAddressProvider(config::ClientAwsConfig &awsConfig, int awsMemberPort,
                                                       util::ILogger &logger) : awsMemberPort(
                        util::IOUtil::to_string<int>(awsMemberPort)), logger(logger), awsClient(awsConfig, logger) {
                }

                std::vector<Address> AwsAddressProvider::loadAddresses() {
                    updateLookupTable();
                    std::map<std::string, std::string> lookupTable = getLookupTable();
                    std::vector<Address> addresses;

                    typedef std::map<std::string, std::string> LookupTable;
                    for (const LookupTable::value_type &privateAddress : lookupTable) {
                        std::vector<Address> possibleAddresses = util::AddressHelper::getSocketAddresses(
                                privateAddress.first + ":" + awsMemberPort, logger);
                        addresses.insert(addresses.begin(), possibleAddresses.begin(),
                                         possibleAddresses.end());
                    }
                    return addresses;
                }

                void AwsAddressProvider::updateLookupTable() {
                    try {
                        privateToPublic = awsClient.getAddresses();
                    } catch (exception::IException &e) {
                        logger.warning("Aws addresses failed to load: ", e.getMessage());
                    }
                }

                std::map<std::string, std::string> AwsAddressProvider::getLookupTable() {
                    return privateToPublic;
                }

                AwsAddressProvider::~AwsAddressProvider() {
                }

                Address DefaultAddressTranslator::translate(const Address &address) {
                    return address;
                }

                void DefaultAddressTranslator::refresh() {
                }


                ClientPartitionServiceImpl::RefreshTaskCallback::RefreshTaskCallback(
                        ClientPartitionServiceImpl &partitionService) : partitionService(partitionService) {}

                void ClientPartitionServiceImpl::RefreshTaskCallback::onResponse(
                        const std::shared_ptr<protocol::ClientMessage> &responseMessage) {
                    if (!responseMessage.get()) {
                        return;
                    }
                    protocol::codec::ClientGetPartitionsCodec::ResponseParameters response =
                            protocol::codec::ClientGetPartitionsCodec::ResponseParameters::decode(*responseMessage);
                    partitionService.processPartitionResponse(response.partitions, response.partitionStateVersion,
                                                              response.partitionStateVersionExist);
                }

                void ClientPartitionServiceImpl::RefreshTaskCallback::onFailure(
                        const std::shared_ptr<exception::IException> &t) {
                    if (partitionService.client.getLifecycleService().isRunning()) {
                        partitionService.logger.warning("Error while fetching cluster partition table! Cause:", *t);
                    }
                }

                ClientPartitionServiceImpl::ClientPartitionServiceImpl(ClientContext &client,
                                                                       hazelcast::client::spi::impl::ClientExecutionServiceImpl &executionService)
                        : client(client), logger(client.getLogger()),
                          clientExecutionService(executionService),
                          refreshTaskCallback(new RefreshTaskCallback(*this)), partitionCount(0),
                          lastPartitionStateVersion(0) {
                }

                bool ClientPartitionServiceImpl::processPartitionResponse(
                        const std::vector<std::pair<Address, std::vector<int32_t> > > &partitions,
                        int32_t partitionStateVersion, bool partitionStateVersionExist) {
                    {
                        util::LockGuard guard(lock);
                        if (!partitionStateVersionExist || partitionStateVersion > lastPartitionStateVersion) {
                            typedef std::vector<std::pair<Address, std::vector<int32_t> > > PARTITION_VECTOR;
                            for (const PARTITION_VECTOR::value_type &entry : partitions) {
                                const Address &address = entry.first;
                                for (const std::vector<int32_t>::value_type &partition : entry.second) {
                                    this->partitions.put(partition, std::shared_ptr<Address>(
                                            new Address(address)));
                                }
                            }
                            partitionCount = this->partitions.size();
                            lastPartitionStateVersion = partitionStateVersion;
                            if (logger.isFinestEnabled()) {
                                logger.finest("Processed partition response. partitionStateVersion : ",
                                              (partitionStateVersionExist ? util::IOUtil::to_string<int32_t>(
                                                      partitionStateVersion) : "NotAvailable"), ", partitionCount :",
                                              (int) partitionCount);
                            }
                        }
                    }
                    return partitionCount > 0;
                }

                void ClientPartitionServiceImpl::start() {
                    //scheduling left in place to support server versions before 3.9.
                    clientExecutionService.scheduleWithRepetition(
                            std::shared_ptr<util::Runnable>(new RefreshTask(client, *this)), INITIAL_DELAY, PERIOD);
                }

                void ClientPartitionServiceImpl::listenPartitionTable(
                        const std::shared_ptr<connection::Connection> &ownerConnection) {
                    //when we connect to cluster back we need to reset partition state version
                    lastPartitionStateVersion = -1;
                    if (ownerConnection->getConnectedServerVersion() >=
                        client::impl::BuildInfo::calculateVersion("3.9")) {
                        //Servers after 3.9 supports listeners
                        std::unique_ptr<protocol::ClientMessage> clientMessage =
                                protocol::codec::ClientAddPartitionListenerCodec::encodeRequest();
                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(client, clientMessage,
                                                                                                "", ownerConnection);
                        invocation->setEventHandler(shared_from_this());
                        invocation->invokeUrgent()->get();
                    }
                }

                void ClientPartitionServiceImpl::refreshPartitions() {
                    try {
                        // use internal execution service for all partition refresh process (do not use the user executor thread)
                        clientExecutionService.execute(
                                std::shared_ptr<util::Runnable>(new RefreshTask(client, *this)));
                    } catch (exception::RejectedExecutionException &) {
                        // ignore
                    }
                }

                void ClientPartitionServiceImpl::handlePartitionsEventV15(
                        const std::vector<std::pair<Address, std::vector<int32_t> > > &collection,
                        const int32_t &partitionStateVersion) {
                    processPartitionResponse(collection, partitionStateVersion, true);
                }

                void ClientPartitionServiceImpl::beforeListenerRegister() {
                }

                void ClientPartitionServiceImpl::onListenerRegister() {
                }

                std::shared_ptr<Address> ClientPartitionServiceImpl::getPartitionOwner(int partitionId) {
                    waitForPartitionsFetchedOnce();
                    return partitions.get(partitionId);
                }

                int ClientPartitionServiceImpl::getPartitionId(const serialization::pimpl::Data &key) {
                    int pc = getPartitionCount();
                    if (pc <= 0) {
                        return 0;
                    }
                    int hash = key.getPartitionHash();
                    return util::HashUtil::hashToIndex(hash, pc);
                }

                int ClientPartitionServiceImpl::getPartitionCount() {
                    waitForPartitionsFetchedOnce();
                    return partitionCount;
                }

                std::shared_ptr<client::impl::Partition> ClientPartitionServiceImpl::getPartition(int partitionId) {
                    return std::shared_ptr<client::impl::Partition>(new PartitionImpl(partitionId, client, *this));
                }

                void ClientPartitionServiceImpl::waitForPartitionsFetchedOnce() {
                    while (partitionCount == 0 && client.getConnectionManager().isAlive()) {
                        if (isClusterFormedByOnlyLiteMembers()) {
                            throw exception::NoDataMemberInClusterException(
                                    "ClientPartitionServiceImpl::waitForPartitionsFetchedOnce",
                                    "Partitions can't be assigned since all nodes in the cluster are lite members");
                        }
                        std::unique_ptr<protocol::ClientMessage> requestMessage = protocol::codec::ClientGetPartitionsCodec::encodeRequest();
                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(client,
                                                                                                requestMessage, "");
                        std::shared_ptr<ClientInvocationFuture> future = invocation->invokeUrgent();
                        try {
                            std::shared_ptr<protocol::ClientMessage> responseMessage = future->get();
                            protocol::codec::ClientGetPartitionsCodec::ResponseParameters response =
                                    protocol::codec::ClientGetPartitionsCodec::ResponseParameters::decode(
                                            *responseMessage);
                            processPartitionResponse(response.partitions,
                                                     response.partitionStateVersion,
                                                     response.partitionStateVersionExist);
                        } catch (exception::IException &e) {
                            if (client.getLifecycleService().isRunning()) {
                                logger.warning("Error while fetching cluster partition table!", e);
                            }
                        }
                    }
                }

                bool ClientPartitionServiceImpl::isClusterFormedByOnlyLiteMembers() {
                    ClientClusterService &clusterService = client.getClientClusterService();
                    for (const std::vector<Member>::value_type &member : clusterService.getMemberList()) {
                        if (!member.isLiteMember()) {
                            return false;
                        }
                    }
                    return true;
                }

                void ClientPartitionServiceImpl::stop() {
                    partitions.clear();
                }

                void ClientPartitionServiceImpl::RefreshTask::run() {
                    if (!client.getLifecycleService().isRunning()) {
                        return;
                    }

                    try {
                        connection::ClientConnectionManagerImpl &connectionManager = client.getConnectionManager();
                        std::shared_ptr<connection::Connection> connection = connectionManager.getOwnerConnection();
                        if (!connection.get()) {
                            return;
                        }
                        std::unique_ptr<protocol::ClientMessage> requestMessage = protocol::codec::ClientGetPartitionsCodec::encodeRequest();
                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(client,
                                                                                                requestMessage, "");
                        std::shared_ptr<ClientInvocationFuture> future = invocation->invokeUrgent();
                        future->andThen(partitionService.refreshTaskCallback);
                    } catch (exception::IException &e) {
                        if (client.getLifecycleService().isRunning()) {
                            partitionService.logger.warning(
                                    std::string("Error while fetching cluster partition table! ") + e.what());
                        }
                    }
                }

                const std::string ClientPartitionServiceImpl::RefreshTask::getName() const {
                    return "ClientPartitionServiceImpl::RefreshTask";
                }

                ClientPartitionServiceImpl::RefreshTask::RefreshTask(ClientContext &client,
                                                                     ClientPartitionServiceImpl &partitionService)
                        : client(client), partitionService(partitionService) {}


                int ClientPartitionServiceImpl::PartitionImpl::getPartitionId() const {
                    return partitionId;
                }

                std::shared_ptr<Member> ClientPartitionServiceImpl::PartitionImpl::getOwner() const {
                    std::shared_ptr<Address> owner = partitionService.getPartitionOwner(partitionId);
                    if (owner.get()) {
                        return client.getClientClusterService().getMember(*owner);
                    }
                    return std::shared_ptr<Member>();
                }

                ClientPartitionServiceImpl::PartitionImpl::PartitionImpl(int partitionId, ClientContext &client,
                                                                         ClientPartitionServiceImpl &partitionService)
                        : partitionId(partitionId), client(client), partitionService(partitionService) {
                }


                namespace sequence {
                    CallIdSequenceWithoutBackpressure::CallIdSequenceWithoutBackpressure() : head(0) {}

                    CallIdSequenceWithoutBackpressure::~CallIdSequenceWithoutBackpressure() {
                    }

                    int32_t CallIdSequenceWithoutBackpressure::getMaxConcurrentInvocations() const {
                        return INT32_MAX;
                    }

                    int64_t CallIdSequenceWithoutBackpressure::next() {
                        return forceNext();
                    }

                    int64_t CallIdSequenceWithoutBackpressure::forceNext() {
                        return ++head;
                    }

                    void CallIdSequenceWithoutBackpressure::complete() {
                        // no-op
                    }

                    int64_t CallIdSequenceWithoutBackpressure::getLastCallId() {
                        return head;
                    }

                    AbstractCallIdSequence::AbstractCallIdSequence(int32_t maxConcurrentInvocations) : longs(
                            3 * util::Bits::CACHE_LINE_LENGTH / util::Bits::LONG_SIZE_IN_BYTES) {
                        std::ostringstream out;
                        out << "maxConcurrentInvocations should be a positive number. maxConcurrentInvocations="
                            << maxConcurrentInvocations;
                        util::Preconditions::checkPositive(maxConcurrentInvocations, out.str());

                        this->maxConcurrentInvocations = maxConcurrentInvocations;
                    }

                    AbstractCallIdSequence::~AbstractCallIdSequence() {
                    }

                    int32_t AbstractCallIdSequence::getMaxConcurrentInvocations() const {
                        return maxConcurrentInvocations;
                    }

                    int64_t AbstractCallIdSequence::next() {
                        if (!hasSpace()) {
                            handleNoSpaceLeft();
                        }
                        return forceNext();
                    }

                    int64_t AbstractCallIdSequence::forceNext() {
                        return longs.incrementAndGet(INDEX_HEAD);
                    }

                    void AbstractCallIdSequence::complete() {
                        int64_t newTail = longs.incrementAndGet(INDEX_TAIL);
                        assert(newTail <= longs.get(INDEX_HEAD));
                    }

                    int64_t AbstractCallIdSequence::getLastCallId() {
                        return longs.get(INDEX_HEAD);
                    }

                    bool AbstractCallIdSequence::hasSpace() {
                        return longs.get(INDEX_HEAD) - longs.get(INDEX_TAIL) < maxConcurrentInvocations;
                    }

                    int64_t AbstractCallIdSequence::getTail() {
                        return longs.get(INDEX_TAIL);
                    }

                    const std::unique_ptr<util::concurrent::IdleStrategy> CallIdSequenceWithBackpressure::IDLER(
                            new util::concurrent::BackoffIdleStrategy(
                                    0, 0, std::chrono::duration_cast<std::chrono::nanoseconds>(
                                            std::chrono::microseconds(1000)).count(),
                                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                                            std::chrono::microseconds(MAX_DELAY_MS * 1000)).count()));

                    CallIdSequenceWithBackpressure::CallIdSequenceWithBackpressure(int32_t maxConcurrentInvocations,
                                                                                   int64_t backoffTimeoutMs)
                            : AbstractCallIdSequence(maxConcurrentInvocations) {
                        std::ostringstream out;
                        out << "backoffTimeoutMs should be a positive number. backoffTimeoutMs=" << backoffTimeoutMs;
                        util::Preconditions::checkPositive(backoffTimeoutMs, out.str());

                        backoffTimeoutNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::milliseconds(backoffTimeoutMs)).count();
                    }

                    void CallIdSequenceWithBackpressure::handleNoSpaceLeft() {
                        auto start = std::chrono::system_clock::now();
                        for (int64_t idleCount = 0;; idleCount++) {
                            int64_t elapsedNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::system_clock::now() - start).count();
                            if (elapsedNanos > backoffTimeoutNanos) {
                                throw (exception::ExceptionBuilder<exception::HazelcastOverloadException>(
                                        "CallIdSequenceWithBackpressure::handleNoSpaceLeft")
                                        << "Timed out trying to acquire another call ID."
                                        << " maxConcurrentInvocations = " << getMaxConcurrentInvocations()
                                        << ", backoffTimeout = "
                                        << std::chrono::microseconds(backoffTimeoutNanos / 1000).count()
                                        << " msecs, elapsed:"
                                        << std::chrono::microseconds(elapsedNanos / 1000).count() << " msecs").build();
                            }
                            IDLER->idle(idleCount);
                            if (hasSpace()) {
                                return;
                            }

                        }
                    }

                    FailFastCallIdSequence::FailFastCallIdSequence(int32_t maxConcurrentInvocations)
                            : AbstractCallIdSequence(maxConcurrentInvocations) {}

                    void FailFastCallIdSequence::handleNoSpaceLeft() {
                        throw (exception::ExceptionBuilder<exception::HazelcastOverloadException>(
                                "FailFastCallIdSequence::handleNoSpaceLeft")
                                << "Maximum invocation count is reached. maxConcurrentInvocations = "
                                << getMaxConcurrentInvocations()).build();

                    }

                    std::unique_ptr<CallIdSequence> CallIdFactory::newCallIdSequence(bool isBackPressureEnabled,
                                                                                     int32_t maxAllowedConcurrentInvocations,
                                                                                     int64_t backoffTimeoutMs) {
                        if (!isBackPressureEnabled) {
                            return std::unique_ptr<CallIdSequence>(new CallIdSequenceWithoutBackpressure());
                        } else if (backoffTimeoutMs <= 0) {
                            return std::unique_ptr<CallIdSequence>(
                                    new FailFastCallIdSequence(maxAllowedConcurrentInvocations));
                        } else {
                            return std::unique_ptr<CallIdSequence>(
                                    new CallIdSequenceWithBackpressure(maxAllowedConcurrentInvocations,
                                                                       backoffTimeoutMs));
                        }
                    }
                }

                namespace listener {
                    AbstractClientListenerService::AbstractClientListenerService(ClientContext &clientContext,
                                                                                 int32_t eventThreadCount,
                                                                                 int32_t eventQueueCapacity)
                            : clientContext(clientContext),
                              serializationService(clientContext.getSerializationService()),
                              logger(clientContext.getLogger()),
                              clientConnectionManager(clientContext.getConnectionManager()),
                              eventExecutor(logger, clientContext.getName() + ".event-", eventThreadCount,
                                            eventQueueCapacity),
                              registrationExecutor(logger, clientContext.getName() + ".eventRegistration-", 1) {
                        AbstractClientInvocationService &invocationService = (AbstractClientInvocationService &) clientContext.getInvocationService();
                        invocationTimeoutMillis = invocationService.getInvocationTimeoutMillis();
                        invocationRetryPauseMillis = invocationService.getInvocationRetryPauseMillis();
                    }

                    AbstractClientListenerService::~AbstractClientListenerService() {
                    }

                    std::string
                    AbstractClientListenerService::registerListener(
                            const std::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/
                        std::shared_ptr<util::Callable<std::string> > task(
                                new RegisterListenerTask("AbstractClientListenerService::registerListener",
                                                         shared_from_this(), listenerMessageCodec, handler));
                        return *registrationExecutor.submit<std::string>(task)->get();
                    }

                    bool AbstractClientListenerService::deregisterListener(const std::string &registrationId) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/

                        try {
                            std::shared_ptr<util::Future<bool> > future = registrationExecutor.submit(
                                    std::shared_ptr<util::Callable<bool> >(
                                            new DeregisterListenerTask(
                                                    "AbstractClientListenerService::deregisterListener",
                                                    shared_from_this(), registrationId)));

                            return *future->get();
                        } catch (exception::RejectedExecutionException &) {
                            //RejectedExecutionException executor(hence the client) is already shutdown
                            //listeners are cleaned up by the server side. We can ignore the exception and return true safely
                            return true;
                        }
                    }

                    void AbstractClientListenerService::connectionAdded(
                            const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        registrationExecutor.execute(
                                std::shared_ptr<util::Runnable>(
                                        new ConnectionAddedTask("AbstractClientListenerService::connectionAdded",
                                                                shared_from_this(), connection)));
                    }

                    void AbstractClientListenerService::connectionRemoved(
                            const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/

                        registrationExecutor.execute(
                                std::shared_ptr<util::Runnable>(
                                        new ConnectionRemovedTask("AbstractClientListenerService::connectionRemoved",
                                                                  shared_from_this(), connection)));
                    }

                    void AbstractClientListenerService::addEventHandler(int64_t callId,
                                                                        const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        eventHandlerMap.put(callId, handler);
                    }

                    void AbstractClientListenerService::removeEventHandler(int64_t callId) {
                        eventHandlerMap.remove(callId);
                    }

                    void AbstractClientListenerService::handleClientMessage(
                            const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                            const std::shared_ptr<connection::Connection> &connection) {
                        try {
                            eventExecutor.execute(
                                    std::shared_ptr<util::StripedRunnable>(
                                            new ClientEventProcessor(clientMessage, connection, eventHandlerMap,
                                                                     logger)));
                        } catch (exception::RejectedExecutionException &e) {
                            logger.warning("Event clientMessage could not be handled. ", e);
                        }
                    }

                    void AbstractClientListenerService::shutdown() {
                        clientContext.getClientExecutionService().shutdownExecutor(eventExecutor.getThreadNamePrefix(),
                                                                                   eventExecutor, logger);
                        clientContext.getClientExecutionService().shutdownExecutor(
                                registrationExecutor.getThreadNamePrefix(), registrationExecutor, logger);
                    }

                    void AbstractClientListenerService::start() {
                        registrationExecutor.start();
                        eventExecutor.start();
                        clientConnectionManager.addConnectionListener(shared_from_this());
                    }

                    void AbstractClientListenerService::ClientEventProcessor::run() {
                        int64_t correlationId = clientMessage->getCorrelationId();
                        std::shared_ptr<EventHandler<protocol::ClientMessage> > eventHandler = eventHandlerMap.get(
                                correlationId);
                        if (eventHandler.get() == NULL) {
                            logger.warning("No eventHandler for callId: ", correlationId, ", event: ", *clientMessage);
                            return;
                        }

                        eventHandler->handle(clientMessage);
                    }

                    const std::string AbstractClientListenerService::ClientEventProcessor::getName() const {
                        return "AbstractClientListenerService::ClientEventProcessor";
                    }

                    int32_t AbstractClientListenerService::ClientEventProcessor::getKey() {
                        return clientMessage->getPartitionId();
                    }

                    AbstractClientListenerService::ClientEventProcessor::ClientEventProcessor(
                            const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                            const std::shared_ptr<connection::Connection> &connection,
                            util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > &eventHandlerMap,
                            util::ILogger &logger)
                            : clientMessage(clientMessage), eventHandlerMap(eventHandlerMap), logger(logger) {
                    }

                    AbstractClientListenerService::ClientEventProcessor::~ClientEventProcessor() {
                    }

                    AbstractClientListenerService::RegisterListenerTask::RegisterListenerTask(
                            const std::string &taskName,
                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) : taskName(
                            taskName), listenerService(listenerService), listenerMessageCodec(listenerMessageCodec),
                                                                                                      handler(handler) {}

                    std::shared_ptr<std::string> AbstractClientListenerService::RegisterListenerTask::call() {
                        return std::shared_ptr<std::string>(new std::string(
                                listenerService->registerListenerInternal(listenerMessageCodec, handler)));
                    }

                    const std::string AbstractClientListenerService::RegisterListenerTask::getName() const {
                        return taskName;
                    }

                    AbstractClientListenerService::DeregisterListenerTask::DeregisterListenerTask(
                            const std::string &taskName,
                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::string &registrationId) : taskName(taskName), listenerService(listenerService),
                                                                 registrationId(registrationId) {}

                    std::shared_ptr<bool> AbstractClientListenerService::DeregisterListenerTask::call() {
                        return std::shared_ptr<bool>(
                                new bool(listenerService->deregisterListenerInternal(registrationId)));
                    }

                    const std::string AbstractClientListenerService::DeregisterListenerTask::getName() const {
                        return taskName;
                    }

                    AbstractClientListenerService::ConnectionAddedTask::ConnectionAddedTask(const std::string &taskName,
                                                                                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                                                                                            const std::shared_ptr<connection::Connection> &connection)
                            : taskName(taskName), listenerService(listenerService), connection(connection) {}

                    const std::string AbstractClientListenerService::ConnectionAddedTask::getName() const {
                        return taskName;
                    }

                    void AbstractClientListenerService::ConnectionAddedTask::run() {
                        listenerService->connectionAddedInternal(connection);
                    }


                    AbstractClientListenerService::ConnectionRemovedTask::ConnectionRemovedTask(
                            const std::string &taskName,
                            const std::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::shared_ptr<connection::Connection> &connection) : taskName(taskName),
                                                                                         listenerService(
                                                                                                 listenerService),
                                                                                         connection(connection) {}

                    const std::string AbstractClientListenerService::ConnectionRemovedTask::getName() const {
                        return taskName;
                    }

                    void AbstractClientListenerService::ConnectionRemovedTask::run() {
                        listenerService->connectionRemovedInternal(connection);
                    }

                    std::string AbstractClientListenerService::registerListenerInternal(
                            const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        std::string userRegistrationId = util::UuidUtil::newUnsecureUuidString();

                        ClientRegistrationKey registrationKey(userRegistrationId, handler, listenerMessageCodec);
                        registrations.put(registrationKey, std::shared_ptr<ConnectionRegistrationsMap>(
                                new ConnectionRegistrationsMap()));
                        for (const std::shared_ptr<connection::Connection> &connection : clientConnectionManager.getActiveConnections()) {
                            try {
                                invoke(registrationKey, connection);
                            } catch (exception::IException &e) {
                                if (connection->isAlive()) {
                                    deregisterListenerInternal(userRegistrationId);
                                    throw (exception::ExceptionBuilder<exception::HazelcastException>(
                                            "AbstractClientListenerService::RegisterListenerTask::call")
                                            << "Listener can not be added " << e).build();
                                }
                            }
                        }
                        return userRegistrationId;
                    }

                    bool
                    AbstractClientListenerService::deregisterListenerInternal(const std::string &userRegistrationId) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        ClientRegistrationKey key(userRegistrationId);
                        std::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrations.get(key);
                        if (registrationMap.get() == NULL) {
                            return false;
                        }
                        bool successful = true;

                        for (ConnectionRegistrationsMap::iterator it = registrationMap->begin();
                             it != registrationMap->end();) {
                            ClientEventRegistration &registration = (*it).second;
                            std::shared_ptr<connection::Connection> subscriber = registration.getSubscriber();
                            try {
                                const std::shared_ptr<ListenerMessageCodec> &listenerMessageCodec = registration.getCodec();
                                const std::string &serverRegistrationId = registration.getServerRegistrationId();
                                std::unique_ptr<protocol::ClientMessage> request = listenerMessageCodec->encodeRemoveRequest(
                                        serverRegistrationId);
                                std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                        request, "",
                                                                                                        subscriber);
                                invocation->invoke()->get();
                                removeEventHandler(registration.getCallId());

                                ConnectionRegistrationsMap::iterator oldEntry = it;
                                ++it;
                                registrationMap->erase(oldEntry);
                            } catch (exception::IException &e) {
                                ++it;

                                if (subscriber->isAlive()) {
                                    successful = false;
                                    std::ostringstream endpoint;
                                    if (subscriber->getRemoteEndpoint().get()) {
                                        endpoint << *subscriber->getRemoteEndpoint();
                                    } else {
                                        endpoint << "null";
                                    }
                                    logger.warning("AbstractClientListenerService::deregisterListenerInternal",
                                                   "Deregistration of listener with ID ", userRegistrationId,
                                                   " has failed to address ", subscriber->getRemoteEndpoint(), e);
                                }
                            }
                        }
                        if (successful) {
                            registrations.remove(key);
                        }
                        return successful;
                    }

                    void AbstractClientListenerService::connectionAddedInternal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        for (const ClientRegistrationKey &registrationKey : registrations.keys()) {
                            invokeFromInternalThread(registrationKey, connection);
                        }
                    }

                    void AbstractClientListenerService::connectionRemovedInternal(
                            const std::shared_ptr<connection::Connection> &connection) {
                        typedef std::vector<std::pair<ClientRegistrationKey, std::shared_ptr<ConnectionRegistrationsMap> > > ENTRY_VECTOR;
                        for (const ENTRY_VECTOR::value_type &registrationMapEntry : registrations.entrySet()) {
                            std::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrationMapEntry.second;
                            ConnectionRegistrationsMap::iterator foundRegistration = registrationMap->find(
                                    connection);
                            if (foundRegistration != registrationMap->end()) {
                                removeEventHandler(foundRegistration->second.getCallId());
                                registrationMap->erase(foundRegistration);
                                registrations.put(registrationMapEntry.first,
                                                  registrationMap);
                            }
                        }
                    }

                    void
                    AbstractClientListenerService::invokeFromInternalThread(
                            const ClientRegistrationKey &registrationKey,
                            const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/
                        try {
                            invoke(registrationKey, connection);
                        } catch (exception::IException &e) {
                            logger.warning("Listener ", registrationKey, " can not be added to a new connection: ",
                                           *connection, ", reason: ", e.getMessage());
                        }
                    }

                    void
                    AbstractClientListenerService::invoke(const ClientRegistrationKey &registrationKey,
                                                          const std::shared_ptr<connection::Connection> &connection) {
                        //This method should only be called from registrationExecutor
/*                      TODO
                        assert (Thread.currentThread().getName().contains("eventRegistration"));
*/

                        std::shared_ptr<ConnectionRegistrationsMap> registrationMap = registrations.get(
                                registrationKey);
                        if (registrationMap->find(connection) != registrationMap->end()) {
                            return;
                        }

                        const std::shared_ptr<ListenerMessageCodec> &codec = registrationKey.getCodec();
                        std::unique_ptr<protocol::ClientMessage> request = codec->encodeAddRequest(
                                registersLocalOnly());
                        std::shared_ptr<EventHandler<protocol::ClientMessage> >
                                handler = registrationKey.getHandler();
                        handler->beforeListenerRegister();

                        std::shared_ptr<ClientInvocation> invocation = ClientInvocation::create(clientContext,
                                                                                                request, "",
                                                                                                connection);
                        invocation->setEventHandler(handler);

                        std::shared_ptr<protocol::ClientMessage> clientMessage = invocation->invokeUrgent()->get();

                        std::string serverRegistrationId = codec->decodeAddResponse(*clientMessage);
                        handler->onListenerRegister();
                        int64_t correlationId = invocation->getClientMessage()->getCorrelationId();
                        ClientEventRegistration registration(serverRegistrationId, correlationId, connection, codec);

                        (*registrationMap)[connection] = registration;
                    }

                    bool AbstractClientListenerService::ConnectionPointerLessComparator::operator()(
                            const std::shared_ptr<connection::Connection> &lhs,
                            const std::shared_ptr<connection::Connection> &rhs) const {
                        if (lhs == rhs) {
                            return false;
                        }
                        if (!lhs.get()) {
                            return true;
                        }
                        if (!rhs.get()) {
                            return false;
                        }

                        return *lhs < *rhs;
                    }

                    ClientEventRegistration::ClientEventRegistration(const std::string &serverRegistrationId,
                                                                     int64_t callId,
                                                                     const std::shared_ptr<connection::Connection> &subscriber,
                                                                     const std::shared_ptr<ListenerMessageCodec> &codec)
                            : serverRegistrationId(serverRegistrationId), callId(callId), subscriber(subscriber),
                              codec(codec) {
                    }

                    const std::string &ClientEventRegistration::getServerRegistrationId() const {
                        return serverRegistrationId;
                    }

                    int64_t ClientEventRegistration::getCallId() const {
                        return callId;
                    }

                    const std::shared_ptr<connection::Connection> &ClientEventRegistration::getSubscriber() const {
                        return subscriber;
                    }

                    const std::shared_ptr<ListenerMessageCodec> &ClientEventRegistration::getCodec() const {
                        return codec;
                    }

                    bool ClientEventRegistration::operator==(const ClientEventRegistration &rhs) const {
                        return serverRegistrationId == rhs.serverRegistrationId;
                    }

                    bool ClientEventRegistration::operator!=(const ClientEventRegistration &rhs) const {
                        return !(rhs == *this);
                    }

                    bool ClientEventRegistration::operator<(const ClientEventRegistration &rhs) const {
                        return serverRegistrationId < rhs.serverRegistrationId;
                    }

                    ClientEventRegistration::ClientEventRegistration() {}

                    SmartClientListenerService::SmartClientListenerService(ClientContext &clientContext,
                                                                           int32_t eventThreadCount,
                                                                           int32_t eventQueueCapacity)
                            : AbstractClientListenerService(clientContext, eventThreadCount, eventQueueCapacity) {
                    }


                    void SmartClientListenerService::start() {
                        AbstractClientListenerService::start();

                        registrationExecutor.scheduleAtFixedRate(
                                std::shared_ptr<util::Runnable>(new AsyncConnectToAllMembersTask(
                                        std::static_pointer_cast<SmartClientListenerService>(shared_from_this()))),
                                1000, 1000);
                    }

                    std::string
                    SmartClientListenerService::registerListener(
                            const std::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                            const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        //This method should not be called from registrationExecutor
/*                      TODO
                        assert (!Thread.currentThread().getName().contains("eventRegistration"));
*/
                        trySyncConnectToAllMembers();

                        return AbstractClientListenerService::registerListener(listenerMessageCodec, handler);
                    }

                    void SmartClientListenerService::trySyncConnectToAllMembers() {
                        ClientClusterService &clientClusterService = clientContext.getClientClusterService();
                        int64_t startMillis = util::currentTimeMillis();

                        do {
                            Member lastFailedMember;
                            std::shared_ptr<exception::IException> lastException;

                            for (const Member &member : clientClusterService.getMemberList()) {
                                try {
                                    clientConnectionManager.getOrConnect(member.getAddress());
                                } catch (exception::IException &e) {
                                    lastFailedMember = member;
                                    lastException = e.clone();
                                }
                            }

                            if (lastException.get() == NULL) {
                                // successfully connected to all members, break loop.
                                break;
                            }

                            timeOutOrSleepBeforeNextTry(startMillis, lastFailedMember, lastException);

                        } while (clientContext.getLifecycleService().isRunning());
                    }

                    void SmartClientListenerService::timeOutOrSleepBeforeNextTry(int64_t startMillis,
                                                                                 const Member &lastFailedMember,
                                                                                 std::shared_ptr<exception::IException> &lastException) {
                        int64_t nowInMillis = util::currentTimeMillis();
                        int64_t elapsedMillis = nowInMillis - startMillis;
                        bool timedOut = elapsedMillis > invocationTimeoutMillis;

                        if (timedOut) {
                            throwOperationTimeoutException(startMillis, nowInMillis, elapsedMillis, lastFailedMember,
                                                           lastException);
                        } else {
                            sleepBeforeNextTry();
                        }

                    }

                    void
                    SmartClientListenerService::throwOperationTimeoutException(int64_t startMillis, int64_t nowInMillis,
                                                                               int64_t elapsedMillis,
                                                                               const Member &lastFailedMember,
                                                                               std::shared_ptr<exception::IException> &lastException) {
                        throw (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                                "SmartClientListenerService::throwOperationTimeoutException")
                                << "Registering listeners is timed out."
                                << " Last failed member : " << lastFailedMember << ", "
                                << " Current time: " << util::StringUtil::timeToString(nowInMillis) << ", "
                                << " Start time : " << util::StringUtil::timeToString(startMillis) << ", "
                                << " Client invocation timeout : " << invocationTimeoutMillis << " ms, "
                                << " Elapsed time : " << elapsedMillis << " ms. " << *lastException).build();

                    }

                    void SmartClientListenerService::sleepBeforeNextTry() {
                        // TODO: change with interruptible sleep
                        util::sleepmillis(invocationRetryPauseMillis);
                    }

                    bool SmartClientListenerService::registersLocalOnly() const {
                        return true;
                    }

                    void SmartClientListenerService::asyncConnectToAllMembersInternal() {
                        std::vector<Member> memberList = clientContext.getClientClusterService().getMemberList();
                        for (const Member &member : memberList) {
                            try {
                                if (!clientContext.getLifecycleService().isRunning()) {
                                    return;
                                }
                                clientContext.getConnectionManager().getOrTriggerConnect(
                                        member.getAddress());
                            } catch (exception::IOException &) {
                                return;
                            }
                        }

                    }

                    SmartClientListenerService::AsyncConnectToAllMembersTask::AsyncConnectToAllMembersTask(
                            const std::shared_ptr<SmartClientListenerService> &listenerService) : listenerService(
                            listenerService) {}

                    void SmartClientListenerService::AsyncConnectToAllMembersTask::run() {
                        listenerService->asyncConnectToAllMembersInternal();
                    }

                    const std::string SmartClientListenerService::AsyncConnectToAllMembersTask::getName() const {
                        return "SmartClientListenerService::AsyncConnectToAllMembersTask";
                    }

                    ClientRegistrationKey::ClientRegistrationKey(const std::string &userRegistrationId,
                                                                 const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler,
                                                                 const std::shared_ptr<ListenerMessageCodec> &codec)
                            : userRegistrationId(userRegistrationId), handler(handler), codec(codec) {
                    }

                    ClientRegistrationKey::ClientRegistrationKey(const std::string &userRegistrationId)
                            : userRegistrationId(userRegistrationId) {
                    }

                    const std::string &ClientRegistrationKey::getUserRegistrationId() const {
                        return userRegistrationId;
                    }

                    const std::shared_ptr<EventHandler<protocol::ClientMessage> > &
                    ClientRegistrationKey::getHandler() const {
                        return handler;
                    }

                    const std::shared_ptr<ListenerMessageCodec> &ClientRegistrationKey::getCodec() const {
                        return codec;
                    }

                    bool ClientRegistrationKey::operator==(const ClientRegistrationKey &rhs) const {
                        return userRegistrationId == rhs.userRegistrationId;
                    }

                    bool ClientRegistrationKey::operator!=(const ClientRegistrationKey &rhs) const {
                        return !(rhs == *this);
                    }

                    bool ClientRegistrationKey::operator<(const ClientRegistrationKey &rhs) const {
                        return userRegistrationId < rhs.userRegistrationId;
                    }

                    std::ostream &operator<<(std::ostream &os, const ClientRegistrationKey &key) {
                        os << "ClientRegistrationKey{ userRegistrationId='" << key.userRegistrationId + '\'' + '}';
                        return os;
                    }

                    ClientRegistrationKey::ClientRegistrationKey() {}

                    NonSmartClientListenerService::NonSmartClientListenerService(ClientContext &clientContext,
                                                                                 int32_t eventThreadCount,
                                                                                 int32_t eventQueueCapacity)
                            : AbstractClientListenerService(clientContext, eventThreadCount, eventQueueCapacity) {

                    }

                    bool NonSmartClientListenerService::registersLocalOnly() const {
                        return false;
                    }
                }

            }
        }
    }
}


