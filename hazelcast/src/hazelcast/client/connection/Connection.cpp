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

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address &address, connection::ConnectionManager& connectionManager,serialization::SerializationService &serializationService, spi::ClusterService &clusterService, IListener &iListener, OListener &oListener)
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
        }
    }
}