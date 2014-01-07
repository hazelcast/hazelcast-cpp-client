//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/serialization/DataOutput.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/serialization/DataAdapter.h"
#include "hazelcast/client/serialization/OutputSocketStream.h"
#include "hazelcast/client/serialization/InputSocketStream.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/util/CallPromise.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address &address, connection::ConnectionManager &connectionManager, serialization::SerializationService &serializationService, spi::ClusterService &clusterService, IListener &iListener, OListener &oListener)
            : serializationService(serializationService)
            , connectionManager(connectionManager)
            , socket(address)
            , connectionId(CONN_ID++)
            , live(true)
            , readHandler(*this, iListener, clusterService, 16 << 10)
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

            bool Connection::write(serialization::Data const &data) {
                if (!live)
                    return false;
                writeHandler.enqueueData(data);
                return true;
            };

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
                inputSocketStream.setSerializationContext(&(serializationService.getSerializationContext()));
                serialization::Data data;
                data.readData(inputSocketStream);
                return data;
            }

            // USED BY CLUSTER SERVICE
            util::CallPromise *Connection::registerCall(util::CallPromise *promise) {
                int callId = connectionManager.getNextCallId();
                promise->getRequest().callId = callId;
                callPromises.put(callId, promise);
                if (promise->getEventHandler() != NULL) {
                    registerEventHandler(promise);
                }
                return promise;
            }

            void Connection::reRegisterCall(util::CallPromise *promise) {
                int callId = connectionManager.getNextCallId();
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