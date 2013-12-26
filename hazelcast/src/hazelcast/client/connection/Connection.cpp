//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/serialization/DataOutput.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/serialization/DataAdapter.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address &address, serialization::SerializationService &serializationService, spi::ClusterService &clusterService, OListener &oListener)
            : serializationService(serializationService)
            , socket(address)
            , connectionId(CONN_ID++)
            , live(true)
            , readHandler(*this, clusterService, 16 << 10)
            , writeHandler(*this, oListener, 16 << 10) {
            };

            void Connection::connect() {
                socket.connect();
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

            const Address &Connection::getEndpoint() const {
                return socket.getAddress();
            };

            const Address &Connection::getRemoteEndpoint() const {
                return remoteEndpoint;
            };

            void Connection::setRemoteEndpoint(Address &remoteEndpoint) {
                this->remoteEndpoint = remoteEndpoint;
            };
        }
    }
}