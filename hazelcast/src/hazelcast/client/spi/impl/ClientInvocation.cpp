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

#include <hazelcast/client/protocol/ClientProtocolErrorCodes.h>
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/spi/ClientInvocationService.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   int partitionId) :
                        logger(util::ILogger::getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService()),
                        clientMessage(clientMessage),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(partitionId),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(new ClientInvocationFuture(logger, callIdSequence,
                                                                          clientContext.getClientExecutionService())) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   const boost::shared_ptr<connection::Connection> &connection) :
                        logger(util::ILogger::getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService()),
                        clientMessage(clientMessage),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        connection(connection),
                        clientInvocationFuture(new ClientInvocationFuture(logger, callIdSequence,
                                                                          clientContext.getClientExecutionService())) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName) :
                        logger(util::ILogger::getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService()),
                        clientMessage(clientMessage),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        clientInvocationFuture(new ClientInvocationFuture(logger, callIdSequence,
                                                                          clientContext.getClientExecutionService())) {
                }

                boost::shared_ptr<ClientInvocationFuture>
                ClientInvocation::invoke(boost::shared_ptr<ClientInvocation> &invocation) {
                    assert (invocation->clientMessage.get() != NULL);
                    invocation->clientMessage->setCorrelationId(invocation->callIdSequence.next());
                    invocation->invokeOnSelection(invocation);
                    return invocation->clientInvocationFuture;
                }

                boost::shared_ptr<ClientInvocationFuture>
                ClientInvocation::invokeUrgent(boost::shared_ptr<ClientInvocation> &invocation) {
                    assert (invocation->clientMessage.get() != NULL);
                    invocation->clientMessage->setCorrelationId(invocation->callIdSequence.forceNext());
                    invocation->invokeOnSelection(invocation);
                    return invocation->clientInvocationFuture;
                }

                void ClientInvocation::invokeOnSelection(const boost::shared_ptr<ClientInvocation> &invocation) {
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
                    } catch (exception::HazelcastOverloadException &e) {
                        throw;
                    } catch (exception::IException &e) {
                        invocation->notifyException(e);
                    }
                }

                bool ClientInvocation::isBindToSingleConnection() const {
                    return connection.get() != NULL;
                }

                void ClientInvocation::run() {
                    retry();
                }

                void ClientInvocation::retry() {
                    // first we force a new invocation slot because we are going to return our old invocation slot immediately after
                    // It is important that we first 'force' taking a new slot; otherwise it could be that a sneaky invocation gets
                    // through that takes our slot!
                    clientMessage->setCorrelationId(callIdSequence.forceNext());
                    //we release the old slot
                    callIdSequence.complete();

                    try {
                        invokeOnSelection(shared_from_this());
                    } catch (exception::IException &e) {
                        clientInvocationFuture->complete(e);
                    }
                }

                void ClientInvocation::notifyException(exception::IException &exception) {
                    if (!lifecycleService.isRunning()) {
                        clientInvocationFuture->complete(exception::HazelcastClientNotActiveException(
                                exception.getSource(), exception.getMessage(), exception));
                        return;
                    }

                    if (isNotAllowedToRetryOnSelection(exception)) {
                        clientInvocationFuture->complete(exception);
                        return;
                    }

                    bool retry = isRetrySafeException(exception)
                                 || invocationService.isRedoOperation()
                                 || (exception.getErrorCode() == protocol::TARGET_DISCONNECTED &&
                                     clientMessage->isRetryable());

                    if (!retry) {
                        clientInvocationFuture->complete(exception);
                        return;
                    }

                    int64_t timePassed = util::currentTimeMillis() - startTimeMillis;
                    if (timePassed > invocationService.getInvocationTimeoutMillis()) {
                        if (logger.isFinestEnabled()) {
                            std::ostringstream out;
                            out << "Exception will not be retried because invocation timed out. " << exception.what();
                            logger.finest(out.str());
                        }

                        clientInvocationFuture->complete(newOperationTimeoutException(exception));
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
                    return errorCode == protocol::IO ||
                           errorCode == protocol::HAZELCAST_INSTANCE_NOT_ACTIVE ||
                           errorCode == protocol::RETRYABLE_HAZELCAST;
                }

                exception::OperationTimeoutException
                ClientInvocation::newOperationTimeoutException(exception::IException &exception) {
                    std::ostringstream sb;
                    int64_t nowInMillis = util::currentTimeMillis();
                    sb << *this << " timed out because exception occurred after client invocation timeout "
                       << invocationService.getInvocationTimeoutMillis() << util::StringUtil::timeToString(nowInMillis)
                       << ". " << "Start time: " << util::StringUtil::timeToString(startTimeMillis)
                       << ". Total elapsed time: "
                       << (nowInMillis - startTimeMillis) << " ms. ";
                    return exception::OperationTimeoutException("ClientInvocation::newOperationTimeoutException",
                                                                sb.str());
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
                    os << "ClientInvocation{" << "clientMessage = " << *invocation.clientMessage << ", objectName = "
                       << invocation.objectName << ", target = " << target.str() << ", sendConnection = ";
                    boost::shared_ptr<connection::Connection> sendConnection = (const_cast<ClientInvocation &>(invocation)).sendConnection.get();
                    if (sendConnection.get()) {
                        os << *sendConnection;
                    } else {
                        os << "null";
                    }
                    os << '}';

                    return os;
                }

                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName,
                                                                             int partitionId) {
                    boost::shared_ptr<ClientInvocation> invocation = boost::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, partitionId));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName,
                                                                             const boost::shared_ptr<connection::Connection> &connection) {
                    boost::shared_ptr<ClientInvocation> invocation = boost::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, connection));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName) {
                    boost::shared_ptr<ClientInvocation> invocation = boost::shared_ptr<ClientInvocation>(
                                                new ClientInvocation(clientContext, clientMessage, objectName));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                boost::shared_ptr<connection::Connection> ClientInvocation::getSendConnection() {
                    return sendConnection;
                }

                void
                ClientInvocation::setSendConnection(const boost::shared_ptr<connection::Connection> &sendConnection) {
                    ClientInvocation::sendConnection = sendConnection;
                }

                void ClientInvocation::notify(const boost::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    if (clientMessage.get() == NULL) {
                        throw exception::IllegalArgumentException("response can't be null");
                    }
                    clientInvocationFuture->complete(clientMessage);
                }

                const boost::shared_ptr<protocol::ClientMessage> &ClientInvocation::getClientMessage() const {
                    return clientMessage;
                }

                bool ClientInvocation::shouldBypassHeartbeatCheck() const {
                    return bypassHeartbeatCheck;
                }

                void ClientInvocation::setBypassHeartbeatCheck(bool bypassHeartbeatCheck) {
                    ClientInvocation::bypassHeartbeatCheck = bypassHeartbeatCheck;
                }

                const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &
                ClientInvocation::getEventHandler() const {
                    return eventHandler;
                }

                void ClientInvocation::setEventHandler(
                        const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &eventHandler) {
                    ClientInvocation::eventHandler = eventHandler;
                }

                void ClientInvocation::execute() {
                    if (invokeCount < MAX_FAST_INVOCATION_COUNT) {
                        // fast retry for the first few invocations
                        executionService.execute(boost::shared_ptr<util::Runnable>(shared_from_this()));
                    } else {
                        // progressive retry delay
                        long delayMillis = util::min<int64_t>(1 << ((int64_t) invokeCount - MAX_FAST_INVOCATION_COUNT),
                                                              retryPauseMillis);
                        executionService.schedule(shared_from_this(), delayMillis);
                    }
                }

                const std::string ClientInvocation::getName() const {
                    std::ostringstream out;
                    out << *this;
                    return out.str();
                }

                ClientInvocation::~ClientInvocation() {
                }
            }
        }
    }
}
