//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/serialization/DataOutput.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/serialization/OutputSocketStream.h"
#include "hazelcast/client/serialization/InputSocketStream.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/util/CallPromise.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address &address, spi::ClientContext &clientContext, InSelector &iListener, OutSelector &oListener)
            : clientContext(clientContext)
            , socket(address)
            , live(true)
            , readHandler(*this, iListener, 16 << 10)
            , writeHandler(*this, oListener, 16 << 10) {

            };

            void Connection::connect() {
                int error = socket.connect();
                if (error) {
                    throw client::exception::IOException("Socket::connect", strerror(error));
                }
            };


            void Connection::init() {
                serialization::OutputSocketStream outputSocketStream(socket);
                outputSocketStream.write(protocol::ProtocolConstants::PROTOCOL);
            }

            void Connection::close() {
                socket.close();
            }

            void Connection::resend(boost::shared_ptr<util::CallPromise> promise) {
                if (promise->incrementAndGetResendCount() > spi::InvocationService::RETRY_COUNT) {
                    exception::InstanceNotActiveException instanceNotActiveException(remoteEndpoint.getHost());
                    promise->setException(instanceNotActiveException);  // TargetNotMemberException
                    return;
                } // MTODO there is already resend mechanism in connectionManager

                boost::shared_ptr<connection::Connection> connection;
                try {
                    ConnectionManager &cm = clientContext.getConnectionManager();
                    connection = cm.getRandomConnection(spi::InvocationService::RETRY_COUNT);
                } catch(exception::IOException &e) {
                    exception::InstanceNotActiveException instanceNotActiveException(remoteEndpoint.getHost());
                    promise->setException(instanceNotActiveException);  // TargetNotMemberException
                    return;
                }
                connection->registerAndEnqueue(promise);
            };

            void Connection::registerAndEnqueue(boost::shared_ptr<util::CallPromise> promise) {
                registerCall(promise); //Don't change the order with following line
                serialization::Data data = clientContext.getSerializationService().toData<impl::PortableRequest>(&(promise->getRequest()));
                if (!live) {
                    deRegisterCall(promise->getRequest().callId);
                    resend(promise);
                    return;
                }
                writeHandler.enqueueData(data);
            }


            void Connection::handlePacket(const serialization::Data &data) {
                serialization::SerializationService &serializationService = clientContext.getSerializationService();
                boost::shared_ptr<connection::ClientResponse> response = serializationService.toObject<connection::ClientResponse>(data);
                if (response->isEvent()) {
                    boost::shared_ptr<util::CallPromise> promise = getEventHandlerPromise(response->getCallId());
                    if (promise.get() != NULL) {
                        promise->getEventHandler()->handle(response->getData());
                    }
                    return;
                }
                boost::shared_ptr<util::CallPromise> promise = deRegisterCall(response->getCallId());
                if (response->isException()) {
                    exception::ServerException const &ex = response->getException();
                    if (ex.isInstanceNotActiveException()) {
                        targetNotActive(promise);
                        return;
                    }
                    promise->setException(ex);
                    return;
                }

                if (!handleEventUuid(response, promise))
                    return; //if event uuid return.

                promise->setResponse(response->getData());

            }

            /* returns shouldSetResponse */
            bool Connection::handleEventUuid(boost::shared_ptr<connection::ClientResponse> response, boost::shared_ptr<util::CallPromise> promise) {
                serialization::SerializationService &serializationService = clientContext.getSerializationService();
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
                return remoteEndpoint;
            };

            void Connection::setRemoteEndpoint(Address &remoteEndpoint) {
                this->remoteEndpoint = remoteEndpoint;
            };

            void Connection::writeBlocking(serialization::Data const &data) {
                serialization::OutputSocketStream outputSocketStream(socket);
                data.writeData(outputSocketStream);
            }

            serialization::Data Connection::readBlocking() {
                serialization::InputSocketStream inputSocketStream(socket);
                inputSocketStream.setSerializationContext(&(clientContext.getSerializationService().getSerializationContext()));
                serialization::Data data;
                data.readData(inputSocketStream);
                return data;
            }


            boost::shared_ptr<util::CallPromise> Connection::registerCall(boost::shared_ptr<util::CallPromise> promise) {
                int callId = clientContext.getConnectionManager().getNextCallId();
                promise->getRequest().callId = callId;
                callPromises.put(callId, promise);
                if (promise->getEventHandler() != NULL) {
                    registerEventHandler(promise);
                }
                return promise;
            }


            ReadHandler &Connection::getReadHandler() {
                return readHandler;
            }

            // USED BY CLUSTER SERVICE

            boost::shared_ptr<util::CallPromise> Connection::deRegisterCall(int callId) {
                return callPromises.remove(callId);
            }


            void Connection::registerEventHandler(boost::shared_ptr<util::CallPromise> promise) {
                eventHandlerPromises.put(promise->getRequest().callId, promise);
            }


            boost::shared_ptr<util::CallPromise > Connection::getEventHandlerPromise(int callId) {
                return eventHandlerPromises.get(callId);
            }

            boost::shared_ptr<util::CallPromise > Connection::deRegisterEventHandler(int callId) {
                return eventHandlerPromises.remove(callId);
            }

            void Connection::removeConnectionCalls() {
                clientContext.getConnectionManager().removeConnection(remoteEndpoint);
//            partitionService.runRefresher(); MTODO
                typedef std::vector<std::pair<int, boost::shared_ptr<util::CallPromise> > > Entry_Set;
                Address const &address = getRemoteEndpoint();
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
                        targetNotActive(it->second);
                    }
                }
            }

            void Connection::targetNotActive(boost::shared_ptr<util::CallPromise> promise) {
                Address const &address = getRemoteEndpoint();
                spi::InvocationService &invocationService = clientContext.getInvocationService();
                if (promise->getRequest().isRetryable() || invocationService.isRedoOperation()) {
                    resend(promise);
                }

            }
        }
    }
}