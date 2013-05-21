//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "Connection.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address & address, hazelcast::client::serialization::SerializationService & serializationService)
            : endpoint(address)
            , serializationService(serializationService)
            , socket(endpoint)
            , inputSocketStream(socket)
            , outputSocketStream(socket)
            , connectionId(CONN_ID++) {
            }

            void Connection::write(hazelcast::client::serialization::Data const & data) {
                data.writeData(outputSocketStream);
            }

            void Connection::read(hazelcast::client::serialization::Data & data) {
                data.readData(inputSocketStream);
            }

            void Connection::close() {
                socket.close();
                //TODO may be something with streams
            }


        }
    }
}