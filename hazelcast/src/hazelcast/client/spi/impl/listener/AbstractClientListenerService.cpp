/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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



#include "hazelcast/client/spi/impl/AbstractClientInvocationService.h"
#include "hazelcast/util/UuidUtil.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/ListenerMessageCodec.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
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
                            logger.warning() << "Event clientMessage could not be handled. " << e;
                        }
                    }

                    void AbstractClientListenerService::shutdown() {
                        eventExecutor.shutdown();
                        eventExecutor.awaitTerminationSeconds(
                                ClientExecutionServiceImpl::SHUTDOWN_CHECK_INTERVAL_SECONDS);
                        registrationExecutor.shutdown();
                        registrationExecutor.awaitTerminationSeconds(
                                ClientExecutionServiceImpl::SHUTDOWN_CHECK_INTERVAL_SECONDS);
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
                            logger.warning() << "No eventHandler for callId: " << correlationId << ", event: "
                                             << *clientMessage;
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
                        return std::shared_ptr<std::string>(new std::string(listenerService->registerListenerInternal(listenerMessageCodec, handler)));
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
                        return std::shared_ptr<bool>(new bool(listenerService->deregisterListenerInternal(registrationId)));
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
                                    logger.warning() << "AbstractClientListenerService::deregisterListenerInternal"
                                                     << "Deregistration of listener with ID " << userRegistrationId
                                                     << " has failed to address " << subscriber->getRemoteEndpoint()
                                                     << e;
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
                            logger.warning() << "Listener " << registrationKey
                                             << " can not be added to a new connection: "
                                             << *connection << ", reason: " << e.getMessage();
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
                        std::unique_ptr<protocol::ClientMessage> request = codec->encodeAddRequest(registersLocalOnly());
                        std::shared_ptr<EventHandler<protocol::ClientMessage> > handler = registrationKey.getHandler();
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

                }
            }
        }

    }
}
