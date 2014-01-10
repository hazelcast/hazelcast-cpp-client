//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/serialization/DataOutput.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/serialization/DataAdapter.h"
#include "hazelcast/client/serialization/OutputSocketStream.h"
#include "hazelcast/client/serialization/InputSocketStream.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/util/CallPromise.h"
#include "hazelcast/client/connection/ClientResponse.h"
#include "hazelcast/client/impl/EventHandlerWrapper.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address &address, spi::ClientContext& clientContext, IListener &iListener, OListener &oListener)
            : clientContext(clientContext)
            , socket(address)
            , connectionId(CONN_ID++)
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

            void Connection::send(util::CallPromise *promise) {
                registerCall(promise);
                write(promise);
            };

            void Connection::resend(util::CallPromise *promise) {
                reRegisterCall(promise);
                write(promise);
            };

            void Connection::write(util::CallPromise *promise) {
                serialization::Data data = clientContext.getSerializationService().toData<impl::PortableRequest>(&(promise->getRequest()));
                if (!live) {
                    promise->targetIsNotAlive(remoteEndpoint);
                    removeConnectionCalls();
                }
                writeHandler.enqueueData(data);
            }


            void Connection::handlePacket(const serialization::Data &data) {
                serialization::SerializationService &serializationService = clientContext.getSerializationService();
                boost::shared_ptr<connection::ClientResponse> response = serializationService.toObject<connection::ClientResponse>(data);
                if (response->isEvent()) {
                    util::CallPromise *promise = getEventHandler(response->getCallId());
                    if (promise != NULL) {
                        promise->getEventHandler()->handle(response->getData());
                        return;
                    }
                    return;
                }
                {
//                    boost::lock_guard<boost::mutex> l(connectionLock);TODO
                    util::CallPromise *promise = deRegisterCall(response->getCallId());
                    if (response->isException()) {
                        promise->setException(response->getException());
                    } else {

                        if (promise->getEventHandler() != NULL) {
                            //TODO may require lock here
                            boost::shared_ptr<std::string> alias = serializationService.toObject<std::string>(response->getData());
                            boost::shared_ptr<std::string> uuid = serializationService.toObject<std::string>(promise->getFuture().get());
                            int callId = promise->getRequest().callId;
                            clientContext.getServerListenerService().reRegisterListener(*uuid, *alias, callId);
                            return;
                        }
                        promise->setResponse(response->getData());
                    }

                }
            }

            int Connection::getConnectionId() const {
                return connectionId;
            };

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


            util::CallPromise *Connection::registerCall(util::CallPromise *promise) {
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
            void Connection::reRegisterCall(util::CallPromise *promise) {
                int callId = clientContext.getConnectionManager().getNextCallId();
                promise->getRequest().callId = callId;
                callPromises.put(callId, promise);
                if (promise->getEventHandler() != NULL) {
                    registerEventHandler(promise);
                }
            }

            util::CallPromise *Connection::deRegisterCall(int callId) {
                return callPromises.remove(callId);
            }


            void Connection::registerEventHandler(util::CallPromise *promise) {
                eventHandlerPromises.put(promise->getRequest().callId, promise);
            }


            util::CallPromise *Connection::getEventHandler(int callId) {
                return eventHandlerPromises.get(callId);
            }

            util::CallPromise *Connection::deRegisterEventHandler(int callId) {
                return eventHandlerPromises.remove(callId);
            }

            void Connection::removeConnectionCalls() {
//            partitionService.runRefresher(); TODO
                hazelcast::client::Address const &address = getRemoteEndpoint();
                {
                    std::vector<util::CallPromise *> v = callPromises.values();
                    std::vector<util::CallPromise *>::iterator it;
                    for (it = v.begin(); it != v.end(); ++it) {
                        (*it)->targetDisconnected(address);
                    }
                    v.clear();
                }
                {
                    std::vector<util::CallPromise *> v = eventHandlerPromises.values();
                    std::vector<util::CallPromise *>::iterator it;
                    for (it = v.begin(); it != v.end(); ++it) {
                        (*it)->targetDisconnected(address);
                    }
                    v.clear();
                }
            }

        }
    }
}