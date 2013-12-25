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
            Connection::Connection(const Address &address, serialization::SerializationService &serializationService)
            : serializationService(serializationService)
            , socket(address)
            , connectionId(CONN_ID++)
            , live(true){
            };

            void Connection::connect() {
                socket.connect();
            };


            void Connection::close() {
                socket.close();
            }

            void Connection::write(serialization::DataAdapter const &data) {
                //Add to write Queue
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

        }
    }
}