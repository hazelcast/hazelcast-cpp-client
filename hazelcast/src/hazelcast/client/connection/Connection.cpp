//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/serialization/pimpl/Packet.h"
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/connection/InputSocketStream.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/impl/ServerException.h"
#include "hazelcast/client/impl/RemoveAllListeners.h"
#include "hazelcast/client/connection/CallFuture.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address& address, spi::ClientContext& clientContext, InSelector& iListener, OutSelector& oListener)
            : live(true)
            , clientContext(clientContext)
            , invocationService(clientContext.getInvocationService())
            , socket(address)
            , readHandler(*this, iListener, 16 << 10)
            , writeHandler(*this, oListener, 16 << 10)
            , _isOwnerConnection(false) {

            }

            Connection::~Connection() {
                live = false;
                socket.close();
            }

            void Connection::connect(int timeoutInMillis) {
                int error = socket.connect(timeoutInMillis);
                if (error) {
                    throw exception::IOException("Socket::connect", strerror(error));
                }
            }

            void Connection::init(const std::vector<byte>& PROTOCOL) {
                connection::OutputSocketStream outputSocketStream(socket);
                outputSocketStream.write(PROTOCOL);
            }

            void Connection::close() {
                if (!live.compareAndSet(true, false)) {
                    return;
                }

                std::stringstream message;
                message << "Closing connection to " << getRemoteEndpoint() << std::endl;
                util::ILogger::getLogger().finest(message.str());
                socket.close();
                if (_isOwnerConnection) {
                    return;
                }

                clientContext.getConnectionManager().onConnectionClose(socket.getRemoteEndpoint());
                cleanResources();
            }

            void Connection::resend(boost::shared_ptr<CallPromise> promise) {
                util::sleep(invocationService.getRetryWaitTime());

                if (promise->getRequest().isBindToSingleConnection()) {
                    std::string address = util::IOUtil::to_string(socket.getRemoteEndpoint());
                    promise->setException(exception::pimpl::ExceptionHandler::INSTANCE_NOT_ACTIVE, address);
                    return;
                }
                if (promise->incrementAndGetResendCount() > invocationService.getRetryCount()) {
                    std::string address = util::IOUtil::to_string(socket.getRemoteEndpoint());
                    promise->setException(exception::pimpl::ExceptionHandler::INSTANCE_NOT_ACTIVE, address);
                    return;
                }

                boost::shared_ptr<Connection> connection;
                try {
                    ConnectionManager& cm = clientContext.getConnectionManager();
                    connection = cm.getRandomConnection(invocationService.getRetryCount());
                } catch (exception::IOException&) {
                    std::string address = util::IOUtil::to_string(socket.getRemoteEndpoint());
                    promise->setException(exception::pimpl::ExceptionHandler::INSTANCE_NOT_ACTIVE, address);
                    return;
                }
                connection->registerAndEnqueue(promise, -1);
            }

            void Connection::registerAndEnqueue(boost::shared_ptr<CallPromise> promise, int partitionId) {
                registerCall(promise); //Don't change the order with following line
                if (!live || !isHeartBeating()) {
                    deRegisterCall(promise->getRequest().callId);
                    resend(promise);
                    return;
                }
                serialization::pimpl::Data data = clientContext.getSerializationService().toData<impl::ClientRequest>(&(promise->getRequest()));
                serialization::pimpl::Packet *packet = new serialization::pimpl::Packet(getPortableContext(), data);
                packet->setPartitionId(partitionId);
                writeHandler.enqueueData(packet);
            }

            void Connection::handlePacket(const serialization::pimpl::Packet& packet) {
                const serialization::pimpl::Data& data = packet.getData();
                serialization::pimpl::SerializationService& serializationService = clientContext.getSerializationService();
                boost::shared_ptr<ClientResponse> response = serializationService.toObject<ClientResponse>(data);
                if (packet.isHeaderSet(serialization::pimpl::Packet::HEADER_EVENT)) {
                    boost::shared_ptr<CallPromise> promise = getEventHandlerPromise(response->getCallId());
                    if (promise.get() != NULL) {
                        promise->getEventHandler()->handle(response->getData());
                    }
                    return;
                }

                boost::shared_ptr<CallPromise> promise = deRegisterCall(response->getCallId());
                if (!handleException(response, promise))
                    return;//if response is exception,then return

                if (!handleEventUuid(response, promise))
                    return; //if response is event uuid,then return.

                promise->setResponse(response->getData());

            }

            /* returns shouldSetResponse */
            bool Connection::handleException(boost::shared_ptr<ClientResponse> response, boost::shared_ptr<CallPromise> promise) {
                serialization::pimpl::SerializationService& serializationService = clientContext.getSerializationService();
                if (response->isException()) {
                    serialization::pimpl::Data const& data = response->getData();
                    boost::shared_ptr<impl::ServerException> ex = serializationService.toObject<impl::ServerException>(data);

                    std::string exceptionClassName = ex->name;
                    if (exceptionClassName == "com.hazelcast.core.HazelcastInstanceNotActiveException") {
                        handleTargetNotActive(promise);
                    } else {
                        promise->setException(ex->name, ex->message + ":" + ex->details + "\n");
                    }
                    return false;
                }

                return true;
            }

            /* returns shouldSetResponse */
            bool Connection::handleEventUuid(boost::shared_ptr<ClientResponse> response, boost::shared_ptr<CallPromise> promise) {
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

            Socket const& Connection::getSocket() const {
                return socket;
            }

            const Address& Connection::getRemoteEndpoint() const {
                return socket.getRemoteEndpoint();
            }

            void Connection::setRemoteEndpoint(Address& remoteEndpoint) {
                socket.setRemoteEndpoint(remoteEndpoint);
            }

            boost::shared_ptr<connection::ClientResponse> Connection::sendAndReceive(const impl::ClientRequest& clientRequest) {
                serialization::pimpl::SerializationService& service = clientContext.getSerializationService();
                serialization::pimpl::Data request = service.toData<impl::ClientRequest>(&clientRequest);
                serialization::pimpl::Packet packet(service.getPortableContext(), request);
                writeBlocking(packet);
                serialization::pimpl::Packet responsePacket = readBlocking();
                return service.toObject<ClientResponse>(responsePacket.getData());
            }

            void Connection::writeBlocking(serialization::pimpl::Packet const& packet) {
                connection::OutputSocketStream outputSocketStream(socket);
                outputSocketStream.writePacket(packet);
            }

            serialization::pimpl::Packet Connection::readBlocking() {
                connection::InputSocketStream inputSocketStream(socket);
                serialization::pimpl::PortableContext& portableContext = clientContext.getSerializationService().getPortableContext();
                inputSocketStream.setPortableContext(&portableContext);
                serialization::pimpl::Packet packet(portableContext);
                inputSocketStream.readPacket(packet);
                return packet;
            }

            void Connection::registerCall(boost::shared_ptr<CallPromise> promise) {
                int callId = clientContext.getConnectionManager().getNextCallId();
                promise->getRequest().callId = callId;
                callPromises.put(callId, promise);
                if (promise->getEventHandler() != NULL) {
                    registerEventHandler(promise);
                }
            }

            ReadHandler& Connection::getReadHandler() {
                return readHandler;
            }

            WriteHandler& Connection::getWriteHandler() {
                return writeHandler;
            }

            boost::shared_ptr<CallPromise> Connection::deRegisterCall(int callId) {
                return callPromises.remove(callId);
            }


            void Connection::registerEventHandler(boost::shared_ptr<CallPromise> promise) {
                eventHandlerPromises.put(promise->getRequest().callId, promise);
            }


            boost::shared_ptr<CallPromise> Connection::getEventHandlerPromise(int callId) {
                return eventHandlerPromises.get(callId);
            }

            boost::shared_ptr<CallPromise> Connection::deRegisterEventHandler(int callId) {
                return eventHandlerPromises.remove(callId);
            }

            void Connection::setAsOwnerConnection(bool isOwnerConnection) {
                _isOwnerConnection = isOwnerConnection;
            }

            serialization::pimpl::PortableContext& Connection::getPortableContext() {
                return clientContext.getSerializationService().getPortableContext();
            }

            void Connection::cleanResources() {
                typedef std::vector<std::pair<int, boost::shared_ptr<CallPromise> > > Entry_Set;
                {
                    Entry_Set entrySet = callPromises.clear();
                    Entry_Set::iterator it;
                    for (it = entrySet.begin(); it != entrySet.end(); ++it) {
                        handleTargetNotActive(it->second);
                    }
                }
                {
                    Entry_Set entrySet = eventHandlerPromises.clear();
                    Entry_Set::iterator it;
                    for (it = entrySet.begin(); it != entrySet.end(); ++it) {
                        clientContext.getServerListenerService().retryFailedListener(it->second);
                    }
                }
            }

            void Connection::handleTargetNotActive(boost::shared_ptr<CallPromise> promise) {
                spi::InvocationService& invocationService = clientContext.getInvocationService();
                if (promise->getRequest().isRetryable() || invocationService.isRedoOperation()) {
                    resend(promise);
                    return;
                }
                std::string address = util::IOUtil::to_string(socket.getRemoteEndpoint());
                promise->setException(exception::pimpl::ExceptionHandler::INSTANCE_NOT_ACTIVE, address);

            }


            void Connection::heartBeatingFailed() {
                failedHeartBeat++;
                std::stringstream errorMessage;
                errorMessage << "Heartbeat to connection  " << getRemoteEndpoint() << " is failed. " << std::endl;
                errorMessage << "Retrycount is  " << failedHeartBeat << " , max allowed  " << invocationService.getMaxFailedHeartbeatCount() << std::endl;
                util::ILogger::getLogger().warning(errorMessage.str());
                ConnectionManager& connectionManager = clientContext.getConnectionManager();
                if (failedHeartBeat == invocationService.getMaxFailedHeartbeatCount()) {
                    connectionManager.onDetectingUnresponsiveConnection(*this);
                    std::vector<boost::shared_ptr<CallPromise> > waitingPromises = eventHandlerPromises.values();
                    std::vector<boost::shared_ptr<CallPromise> >::iterator it;
                    for (it = waitingPromises.begin(); it != waitingPromises.end(); ++it) {
                        handleTargetNotActive(*it);
                    }
                }
            }

            void Connection::heartBeatingSucceed() {
                int lastFailedHeartBeat = failedHeartBeat;
                failedHeartBeat = 0;
                if (lastFailedHeartBeat != 0) {
                    if (lastFailedHeartBeat >= invocationService.getMaxFailedHeartbeatCount()) {
                        try {
                            impl::RemoveAllListeners *request = new impl::RemoveAllListeners();
                            spi::InvocationService& invocationService = clientContext.getInvocationService();
                            CallFuture future = invocationService.invokeOnTarget(request, getRemoteEndpoint());
                            future.get();
                        } catch (exception::IException& e) {
                            std::stringstream errorMessage;
                            errorMessage << "Clearing listeners upon recovering from heart-attack failed.";
                            errorMessage << "Exception message :" << e.what();
                            util::ILogger::getLogger().warning(errorMessage.str());
                        }
                    }
                }
            }


            bool Connection::isHeartBeating() {
                return failedHeartBeat < invocationService.getMaxFailedHeartbeatCount();
            }
        }
    }
}
