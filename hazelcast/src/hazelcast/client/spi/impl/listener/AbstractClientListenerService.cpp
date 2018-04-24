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

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/EventHandler.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    void AbstractClientListenerService::addEventHandler(int64_t callId,
                                                                        const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) {
                        eventHandlerMap.put(callId, handler);
                    }

                    void AbstractClientListenerService::removeEventHandler(int64_t callId) {
                        eventHandlerMap.remove(callId);
                    }

                    void AbstractClientListenerService::handleClientMessage(
                            const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                            const boost::shared_ptr<connection::Connection> &connection) {
                        try {
                            eventExecutor.execute(
                                    boost::shared_ptr<util::StripedRunnable>(
                                            new ClientEventProcessor(clientMessage, connection, eventHandlerMap,
                                                                     logger)));
                        } catch (exception::RejectedExecutionException &e) {
                            logger.warning() << "Event clientMessage could not be handled. " << e;
                        }
                    }

                    AbstractClientListenerService::AbstractClientListenerService(ClientContext &clientContext,
                                                                                 int32_t eventThreadCount,
                                                                                 int32_t eventQueueCapacity)
                            : clientContext(clientContext),
                              serializationService(clientContext.getSerializationService()),
                              logger(util::ILogger::getLogger()),
                              eventExecutor(logger, clientContext.getName() + ".event-", eventThreadCount, eventQueueCapacity),
                              registrationExecutor(logger, clientContext.getName() + ".eventRegistration-", 1) {
                    }

                    void AbstractClientListenerService::shutdown() {
                        eventExecutor.shutdown();
                        registrationExecutor.shutdown();
                    }

                    void AbstractClientListenerService::start() {
                    }

                    AbstractClientListenerService::~AbstractClientListenerService() {
                    }

                    void AbstractClientListenerService::ClientEventProcessor::run() {
                        try {
                            int64_t correlationId = clientMessage->getCorrelationId();
                            boost::shared_ptr<EventHandler<protocol::ClientMessage> > eventHandler = eventHandlerMap.get(
                                    correlationId);
                            if (eventHandler.get() == NULL) {
                                logger.warning() << "No eventHandler for callId: " << correlationId << ", event: "
                                                 << *clientMessage << ", connection: " << *connection;
                                return;
                            }

                            eventHandler->handle(clientMessage);
                        } catch (...) {
                            connection->decrementPendingPacketCount();
                        }
                    }

                    const std::string AbstractClientListenerService::ClientEventProcessor::getName() const {
                        return "AbstractClientListenerService::ClientEventProcessor";
                    }

                    int32_t AbstractClientListenerService::ClientEventProcessor::getKey() {
                        return clientMessage->getPartitionId();
                    }

                    AbstractClientListenerService::ClientEventProcessor::ClientEventProcessor(
                            const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                            const boost::shared_ptr<connection::Connection> &connection,
                            util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > &eventHandlerMap,
                            util::ILogger &logger)
                            : clientMessage(clientMessage), connection(connection), eventHandlerMap(eventHandlerMap),
                              logger(logger) {
                    }

                    AbstractClientListenerService::ClientEventProcessor::~ClientEventProcessor() {
                    }

                    AbstractClientListenerService::RegisterListenerTask::RegisterListenerTask(
                            const std::string &taskName,
                            const boost::shared_ptr<AbstractClientListenerService> &listenerService,
                            const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                            const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) : taskName(
                            taskName), listenerService(listenerService), listenerMessageCodec(listenerMessageCodec),
                                                                                                       handler(handler) {}

                    std::string AbstractClientListenerService::RegisterListenerTask::call() {
                        return listenerService->registerListenerInternal(listenerMessageCodec, handler);
                    }

                    const std::string AbstractClientListenerService::RegisterListenerTask::getName() const {
                        return taskName;
                    }

                    AbstractClientListenerService::DeregisterListenerTask::DeregisterListenerTask(
                            const std::string &taskName,
                            const boost::shared_ptr<AbstractClientListenerService> &listenerService,
                            const std::string &registrationId) : taskName(taskName), listenerService(listenerService),
                                                                 registrationId(registrationId) {}

                    bool AbstractClientListenerService::DeregisterListenerTask::call() {
                        return listenerService->deregisterListenerInternal(registrationId);
                    }

                    const std::string AbstractClientListenerService::DeregisterListenerTask::getName() const {
                        return taskName;
                    }

                    AbstractClientListenerService::ConnectionAddedTask::ConnectionAddedTask(const std::string &taskName,
                                                                                            const boost::shared_ptr<AbstractClientListenerService> &listenerService,
                                                                                            const boost::shared_ptr<connection::Connection> &connection)
                            : taskName(taskName), listenerService(listenerService), connection(connection) {}

                    const std::string AbstractClientListenerService::ConnectionAddedTask::getName() const {
                        return taskName;
                    }

                    void AbstractClientListenerService::ConnectionAddedTask::run() {
                        listenerService->connectionAddedInternal(connection);
                    }
                }
            }
        }

    }
}
