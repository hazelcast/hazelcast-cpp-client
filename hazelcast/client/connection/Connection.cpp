//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "Connection.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address & address, serialization::SerializationService & serializationService)
            : endpoint(address)
            , serializationService(serializationService)
            , socket(address)
            , inputSocketStream(socket)
            , outputSocketStream(socket)
            , connectionId(CONN_ID++) {
            };

            void Connection::connect() {
                socket.connect();
            };

            void Connection::write(vector<byte> const& bytes) {
                outputSocketStream.write(bytes);
            };

            void Connection::write(serialization::Data const & data) {
                data.writeData(outputSocketStream);
            };

            serialization::Data Connection::read(serialization::SerializationContext & serializationContext) {
                serialization::Data data;
                data.readData(inputSocketStream, serializationContext);
                return data;
            };


            Socket const & Connection::getSocket() const {
                return socket;
            };

            const Address & Connection::getEndpoint() const {
                return endpoint;
            };

            void Connection::close() {
                socket.close();
                //TODO may be something with streams
            };


        }
    }
}