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
#include "hazelcast/client/connection/OutputSocketStream.h"
#include "hazelcast/client/connection/InputSocketStream.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/impl/ServerException.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address &address, spi::ClientContext &clientContext, InSelector &iListener, OutSelector &oListener)
            : live(true)
            , clientContext(clientContext)
            , socket(address)
            , readHandler(*this, iListener, 16 << 10)
            , writeHandler(*this, oListener, 16 << 10) {

            };

            Connection::~Connection() {
                live = false;
            }

            void Connection::connect() {
                int error = socket.connect();
                if (error) {
                    throw exception::IOException("Socket::connect", strerror(error));
                }
            };

            void Connection::init(const std::vector<byte> &PROTOCOL) {
                connection::OutputSocketStream outputSocketStream(socket);
                outputSocketStream.write(PROTOCOL);
            }

            void Connection::close() {
                if (!_isOwnerConnection) {
                    removeConnectionCalls();
                }
                live = false;
                socket.close();
            }

            void Connection::resend(boost::shared_ptr<CallPromise> promise) {
                if (promise->getRequest().isBindToSingleConnection()) {
                    std::string address = util::IOUtil::to_string(socket.getRemoteEndpoint());
                    promise->setException(exception::ExceptionHandler::INSTANCE_NOT_ACTIVE, address);
                    return;
                }
                if (promise->incrementAndGetResendCount() > spi::InvocationService::RETRY_COUNT) {
                    std::string address = util::IOUtil::to_string(socket.getRemoteEndpoint());
                    promise->setException(exception::ExceptionHandler::INSTANCE_NOT_ACTIVE, address);
                    return;
                }

                boost::shared_ptr<Connection> connection;
                try {
                    ConnectionManager &cm = clientContext.getConnectionManager();
                    connection = cm.getRandomConnection(spi::InvocationService::RETRY_COUNT);
                } catch(exception::IOException &e) {
                    std::string address = util::IOUtil::to_string(socket.getRemoteEndpoint());
                    promise->setException(exception::ExceptionHandler::INSTANCE_NOT_ACTIVE, address);
                    return;
                }
                connection->registerAndEnqueue(promise);
            };

            void Connection::registerAndEnqueue(boost::shared_ptr<CallPromise> promise) {
                registerCall(promise); //Don't change the order with following line
                serialization::pimpl::Data data = clientContext.getSerializationService().toData<impl::PortableRequest>(&(promise->getRequest()));
                if (!live) {
                    deRegisterCall(promise->getRequest().callId);
                    resend(promise);
                    return;
                }
                writeHandler.enqueueData(data);
            }


            void Connection::handlePacket(const serialization::pimpl::Data &data) {
                serialization::pimpl::SerializationService &serializationService = clientContext.getSerializationService();
                boost::shared_ptr<ClientResponse> response = serializationService.toObject<ClientResponse>(data);
                if (response->isEvent()) {
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
                serialization::pimpl::SerializationService &serializationService = clientContext.getSerializationService();
                if (response->isException()) {
                    serialization::pimpl::Data const &data = response->getData();
                    boost::shared_ptr<impl::ServerException> ex = serializationService.toObject<impl::ServerException>(data);

                    std::string exceptionClassName = ex->name;
                    if (exceptionClassName == "HazelcastInstanceNotActiveException") {
                        targetNotActive(promise);
                    } else {
                        exception::ExceptionHandler::rethrow(ex->name, ex->message + ":" + ex->details + "\n");
                    }
                    return false;
                }

                return true;
            }

            /* returns shouldSetResponse */
            bool Connection::handleEventUuid(boost::shared_ptr<ClientResponse> response, boost::shared_ptr<CallPromise> promise) {
                serialization::pimpl::SerializationService &serializationService = clientContext.getSerializationService();
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

            Socket const &Connection::getSocket() const {
                return socket;
            };

            const Address &Connection::getRemoteEndpoint() const {
                return socket.getRemoteEndpoint();
            };

            void Connection::setRemoteEndpoint(Address &remoteEndpoint) {
                socket.setRemoteEndpoint(remoteEndpoint);
            };

            void Connection::writeBlocking(serialization::pimpl::Data const &data) {
                connection::OutputSocketStream outputSocketStream(socket);
                outputSocketStream.writeData(data);
            }

            serialization::pimpl::Data Connection::readBlocking() {
                connection::InputSocketStream inputSocketStream(socket);
                inputSocketStream.setSerializationContext(&(clientContext.getSerializationService().getSerializationContext()));
                serialization::pimpl::Data data;
                inputSocketStream.readData(data);
                return data;
            }

            void Connection::registerCall(boost::shared_ptr<CallPromise> promise) {
                int callId = clientContext.getConnectionManager().getNextCallId();
                promise->getRequest().callId = callId;
                callPromises.put(callId, promise);
                if (promise->getEventHandler() != NULL) {
                    registerEventHandler(promise);
                }
            }

            ReadHandler &Connection::getReadHandler() {
                return readHandler;
            }

            WriteHandler &Connection::getWriteHandler() {
                return writeHandler;
            }

            boost::shared_ptr<CallPromise> Connection::deRegisterCall(int callId) {
                return callPromises.remove(callId);
            }


            void Connection::registerEventHandler(boost::shared_ptr<CallPromise> promise) {
                eventHandlerPromises.put(promise->getRequest().callId, promise);
            }


            boost::shared_ptr<CallPromise > Connection::getEventHandlerPromise(int callId) {
                return eventHandlerPromises.get(callId);
            }

            boost::shared_ptr<CallPromise > Connection::deRegisterEventHandler(int callId) {
                return eventHandlerPromises.remove(callId);
            }

            void Connection::setAsOwnerConnection(bool isOwnerConnection) {
                _isOwnerConnection = isOwnerConnection;
            }

            void Connection::removeConnectionCalls() {
                clientContext.getConnectionManager().removeConnection(socket.getRemoteEndpoint());
                typedef std::vector<std::pair<int, boost::shared_ptr<CallPromise> > > Entry_Set;
                {
                    Entry_Set entrySet = callPromises.clear();
                    Entry_Set::iterator it;
                    for (it = entrySet.begin(); it != entrySet.end(); ++it) {
                        targetNotActive(it->second);
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

            void Connection::targetNotActive(boost::shared_ptr<CallPromise> promise) {
                spi::InvocationService &invocationService = clientContext.getInvocationService();
                if (promise->getRequest().isRetryable() || invocationService.isRedoOperation()) {
                    resend(promise);
                    return;
                }
                std::string address = util::IOUtil::to_string(socket.getRemoteEndpoint());
                promise->setException(exception::ExceptionHandler::INSTANCE_NOT_ACTIVE, address);

            }
        }
    }
}