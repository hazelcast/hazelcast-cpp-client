//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/serialization/pimpl/Packet.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/impl/ServerException.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/impl/ClientPingRequest.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            InvocationService::InvocationService(spi::ClientContext& clientContext)
            : clientContext(clientContext)
            , callIdGenerator(10) {
                redoOperation = clientContext.getClientConfig().isRedoOperation();
                ClientProperties& properties = clientContext.getClientProperties();
                retryWaitTime = properties.getRetryWaitTime().getInteger();
                retryCount = properties.getRetryCount().getInteger();
                heartbeatTimeout = properties.getHeartbeatTimeout().getInteger();
                if (retryWaitTime <= 0) {
                    retryWaitTime = util::IOUtil::to_value<int>(ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT);
                }
                if (retryCount <= 0) {
                    retryCount = util::IOUtil::to_value<int>(ClientProperties::PROP_REQUEST_RETRY_COUNT_DEFAULT);
                }
                if (heartbeatTimeout <= 0) {
                    heartbeatTimeout = util::IOUtil::to_value<int>(ClientProperties::PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT);
                }
            }

            void InvocationService::start() {

            }

            connection::CallFuture  InvocationService::invokeOnRandomTarget(const impl::ClientRequest *request) {
                return invokeOnRandomTarget(request, NULL);
            }

            connection::CallFuture  InvocationService::invokeOnPartitionOwner(const impl::ClientRequest *request, int partitionId) {
                return invokeOnPartitionOwner(request, NULL, partitionId);
            }

            connection::CallFuture  InvocationService::invokeOnTarget(const impl::ClientRequest *request, const Address& address) {
                return invokeOnTarget(request, NULL, address);
            }

            connection::CallFuture  InvocationService::invokeOnRandomTarget(const impl::ClientRequest *request, impl::BaseEventHandler *eventHandler) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                std::auto_ptr<impl::BaseEventHandler> managedEventHandler(eventHandler);
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getRandomConnection(retryCount);
                return doSend(managedRequest, managedEventHandler, connection, -1);
            }

            connection::CallFuture  InvocationService::invokeOnTarget(const impl::ClientRequest *request, impl::BaseEventHandler *eventHandler, const Address& address) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                std::auto_ptr<impl::BaseEventHandler> managedEventHandler(eventHandler);
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(address, retryCount);
                return doSend(managedRequest, managedEventHandler, connection, -1);
            }

            connection::CallFuture  InvocationService::invokeOnPartitionOwner(const impl::ClientRequest *request, impl::BaseEventHandler *handler, int partitionId) {
                boost::shared_ptr<Address> owner = clientContext.getPartitionService().getPartitionOwner(partitionId);
                if (owner.get() != NULL) {
                    std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                    std::auto_ptr<impl::BaseEventHandler> managedEventHandler(handler);
                    boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(*owner, retryCount);
                    return doSend(managedRequest, managedEventHandler, connection, partitionId);
                }
                return invokeOnRandomTarget(request, handler);
            }

            connection::CallFuture  InvocationService::invokeOnConnection(const impl::ClientRequest *request, boost::shared_ptr<connection::Connection> connection) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                return doSend(managedRequest, std::auto_ptr<impl::BaseEventHandler>(NULL), connection, -1);
            }

            bool InvocationService::isRedoOperation() const {
                return redoOperation;
            }

            int InvocationService::getRetryWaitTime() const {
                return retryWaitTime;
            }

            int InvocationService::getRetryCount() const {
                return retryCount;
            }

            void InvocationService::removeEventHandler(int callId) {
                std::vector<boost::shared_ptr<connection::Connection> > connections = clientContext.getConnectionManager().getConnections();
                std::vector<boost::shared_ptr<connection::Connection> >::iterator it;
                for (it = connections.begin(); it != connections.end(); ++it) {
                    boost::shared_ptr<connection::Connection>& connectionPtr = *it;
                    if (deRegisterEventHandler(*connectionPtr, callId) != NULL) {
                        return;
                    }
                }
            }

            connection::CallFuture  InvocationService::doSend(std::auto_ptr<const impl::ClientRequest> request, std::auto_ptr<impl::BaseEventHandler> eventHandler, boost::shared_ptr<connection::Connection> connection, int partitionId) {
                boost::shared_ptr<connection::CallPromise> promise(new connection::CallPromise());
                promise->setRequest(request);
                promise->setEventHandler(eventHandler);

                boost::shared_ptr<connection::Connection> conn = registerAndEnqueue(connection, promise, partitionId);
                return connection::CallFuture(promise, conn, heartbeatTimeout, this);
            }

            int InvocationService::getNextCallId() {
                return callIdGenerator++;
            }


            bool InvocationService::isAllowedToSentRequest(connection::Connection& connection, impl::ClientRequest const& request) {
                util::ILogger& logger = util::ILogger::getLogger();
                if (!connection.live) {
                    if (logger.isEnabled(FINEST)) {
                        std::stringstream message;
                        message << " Connection(" << connection.getRemoteEndpoint();
                        message << ") is closed, won't write packet with callId : " << request.callId;
                        logger.finest(message.str());
                    }
                    return false;
                }

                if (!connection.isHeartBeating()) {
                    if (dynamic_cast<const impl::ClientPingRequest *>(&request)) {
                        return true;//ping request should be send even though heart is not beating
                    }
                    std::stringstream message;
                    if (logger.isEnabled(FINEST)) {
                        message << " Connection(" << connection.getRemoteEndpoint();
                        message << ") is not heart-beating, won't write packet with callId : " << request.callId;
                        logger.finest(message.str());
                    }
                    return false;
                }
                return true;
            }

            boost::shared_ptr<connection::Connection> InvocationService::resend(boost::shared_ptr<connection::CallPromise> promise, const std::string& lastTriedAddress) {
                util::sleep(getRetryWaitTime());

                if (promise->getRequest().isBindToSingleConnection()) {
                    promise->setException(exception::pimpl::ExceptionHandler::INSTANCE_NOT_ACTIVE, lastTriedAddress);
                    return boost::shared_ptr<connection::Connection>();
                }
                if (promise->incrementAndGetResendCount() > getRetryCount()) {
                    promise->setException(exception::pimpl::ExceptionHandler::INSTANCE_NOT_ACTIVE, lastTriedAddress);
                    return boost::shared_ptr<connection::Connection>();
                }

                boost::shared_ptr<connection::Connection> connection;
                try {
                    connection::ConnectionManager& cm = clientContext.getConnectionManager();
                    connection = cm.getRandomConnection(getRetryCount());
                } catch (exception::IOException&) {
                    promise->setException(exception::pimpl::ExceptionHandler::INSTANCE_NOT_ACTIVE, lastTriedAddress);
                    return boost::shared_ptr<connection::Connection>();
                }
                return registerAndEnqueue(connection, promise, -1);
            }

            boost::shared_ptr<connection::Connection> InvocationService::registerAndEnqueue(boost::shared_ptr<connection::Connection> connection, boost::shared_ptr<connection::CallPromise> promise, int partitionId) {
                registerCall(*connection, promise); //Don't change the order with following line
                impl::ClientRequest const& request = promise->getRequest();
                if (!isAllowedToSentRequest(*connection, request)) {
                    deRegisterCall(*connection, request.callId);
                    std::string address = util::IOUtil::to_string(connection->getRemoteEndpoint());
                    return resend(promise, address);
                }
                serialization::pimpl::SerializationService& ss = clientContext.getSerializationService();
                serialization::pimpl::Data data = ss.toData<impl::ClientRequest>(&(promise->getRequest()));
                serialization::pimpl::PortableContext& portableContext = ss.getPortableContext();
                serialization::pimpl::Packet *packet = new serialization::pimpl::Packet(portableContext, data);
                packet->setPartitionId(partitionId);
                connection->write(packet);
                return connection;
            }

            void InvocationService::registerCall(connection::Connection& connection, boost::shared_ptr<connection::CallPromise> promise) {
                int callId = getNextCallId();
                promise->getRequest().callId = callId;
                getCallPromiseMap(connection)->put(callId, promise);
                if (promise->getEventHandler() != NULL) {
                    registerEventHandler(connection, promise);
                }
            }

            boost::shared_ptr<connection::CallPromise> InvocationService::deRegisterCall(connection::Connection& connection, int callId) {
                return getCallPromiseMap(connection)->remove(callId);
            }

            void InvocationService::registerEventHandler(connection::Connection& connection, boost::shared_ptr<connection::CallPromise> promise) {
                getEventHandlerPromiseMap(connection)->put(promise->getRequest().callId, promise);
            }

            void InvocationService::handlePacket(connection::Connection& connection, const serialization::pimpl::Packet& packet) {
                const serialization::pimpl::Data& data = packet.getData();
                serialization::pimpl::SerializationService& serializationService = clientContext.getSerializationService();
                boost::shared_ptr<connection::ClientResponse> response = serializationService.toObject<connection::ClientResponse>(data);
                if (packet.isHeaderSet(serialization::pimpl::Packet::HEADER_EVENT)) {
                    boost::shared_ptr<connection::CallPromise> promise = getEventHandlerPromise(connection, response->getCallId());
                    if (promise.get() != NULL) {
                        promise->getEventHandler()->handle(response->getData());
                    }
                    return;
                }

                boost::shared_ptr<connection::CallPromise> promise = deRegisterCall(connection, response->getCallId());
                if (promise.get() == NULL) {
                    return;
                }
                std::string address = util::IOUtil::to_string(connection.getRemoteEndpoint());
                if (!handleException(response, promise, address))
                    return;//if response is exception,then return

                if (!handleEventUuid(response, promise))
                    return; //if response is event uuid,then return.

                promise->setResponse(response->getData());

            }

            /* returns shouldSetResponse */
            bool InvocationService::handleException(boost::shared_ptr<connection::ClientResponse> response, boost::shared_ptr<connection::CallPromise> promise, const std::string& address) {
                if (response->isException()) {
                    serialization::pimpl::SerializationService& serializationService = clientContext.getSerializationService();
                    serialization::pimpl::Data const& data = response->getData();
                    boost::shared_ptr<impl::ServerException> ex = serializationService.toObject<impl::ServerException>(data);

                    std::string exceptionClassName = ex->name;
                    if (exceptionClassName == "com.hazelcast.core.HazelcastInstanceNotActiveException") {
                        tryResend(promise, address);
                    } else {
                        promise->setException(ex->name, ex->message + ":" + ex->details + "\n");
                    }
                    return false;
                }

                return true;
            }

            /* returns shouldSetResponse */
            bool InvocationService::handleEventUuid(boost::shared_ptr<connection::ClientResponse> response, boost::shared_ptr<connection::CallPromise> promise) {
                serialization::pimpl::SerializationService& serializationService = clientContext.getSerializationService();
                impl::BaseEventHandler *eventHandler = promise->getEventHandler();
                if (eventHandler != NULL) {
                    if (eventHandler->registrationId.size() == 0) //if uuid is not set, it means it is first time that we are getting uuid.
                        return true;                    // then no need to handle it, just set as normal response
                    boost::shared_ptr<std::string> alias = serializationService.toObject<std::string>(response->getData());
                    int callId = promise->getRequest().callId;
                    clientContext.getServerListenerService().reRegisterListener(eventHandler->registrationId, alias, callId);
                    return false;
                }
                //if it does not have event handler associated with it, then it is a normal response.
                return true;
            }

            void InvocationService::tryResend(boost::shared_ptr<connection::CallPromise> promise, const std::string& lastTriedAddress) {
                if (promise->getRequest().isRetryable() || isRedoOperation()) {
                    resend(promise, lastTriedAddress);
                    return;
                }
                promise->setException(exception::pimpl::ExceptionHandler::INSTANCE_NOT_ACTIVE, lastTriedAddress);

            }

            boost::shared_ptr<connection::CallPromise> InvocationService::getEventHandlerPromise(connection::Connection& connection, int callId) {
                return getEventHandlerPromiseMap(connection)->get(callId);
            }

            boost::shared_ptr<connection::CallPromise> InvocationService::deRegisterEventHandler(connection::Connection& connection, int callId) {
                return getEventHandlerPromiseMap(connection)->remove(callId);
            }

            void InvocationService::cleanResources(connection::Connection& connection) {
                typedef std::vector<std::pair<int, boost::shared_ptr<connection::CallPromise> > > Entry_Set;
                {
                    Entry_Set entrySet = getCallPromiseMap(connection)->clear();
                    Entry_Set::iterator it;
                    for (it = entrySet.begin(); it != entrySet.end(); ++it) {
                        std::string address = util::IOUtil::to_string(connection.getRemoteEndpoint());
                        tryResend(it->second, address);
                    }
                }
                cleanEventHandlers(connection);
            }

            void InvocationService::cleanEventHandlers(connection::Connection& connection) {
                typedef std::vector<std::pair<int, boost::shared_ptr<connection::CallPromise> > > Entry_Set;
                Entry_Set entrySet = getEventHandlerPromiseMap(connection)->clear();
                Entry_Set::iterator it;
                for (it = entrySet.begin(); it != entrySet.end(); ++it) {
                    clientContext.getServerListenerService().retryFailedListener(it->second);
                }
            }


            boost::shared_ptr<util::SynchronizedMap<int, connection::CallPromise> > InvocationService::getCallPromiseMap(connection::Connection& connection) {
                return callPromises.getOrPutIfAbsent(&connection);
            }


            boost::shared_ptr<util::SynchronizedMap<int, connection::CallPromise> > InvocationService::getEventHandlerPromiseMap(connection::Connection& connection) {
                return eventHandlerPromises.getOrPutIfAbsent(&connection);
            }
        }
    }
}
