//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "Connection.h"
#include "ObjectDataOutput.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            Connection::Connection(const Address & address, serialization::SerializationService & serializationService)
            : serializationService(serializationService)
            , socket(address)
            , inputSocketStream(socket)
            , outputSocketStream(socket)
            , connectionId(CONN_ID++) {
            };

            void Connection::connect() {
                socket.connect();
            };

            void Connection::write(std::vector<byte> const& bytes) {
                outputSocketStream.write(bytes);
            };

            void Connection::write(serialization::Data const & data) {
                serialization::ObjectDataOutput out;
                data.writeData(out);
                std::auto_ptr<std::vector<byte> > buffer = out.toByteArray();
                outputSocketStream.write(&((*buffer.get())[0]), buffer->size());
            };

            serialization::Data Connection::read(serialization::SerializationContext & serializationContext) {
                serialization::Data data;
                inputSocketStream.setSerializationContext(&serializationContext);
                data.readData(inputSocketStream);
                lastRead = clock();
                return data;
            };


            int Connection::getConnectionId() const {
                return connectionId;
            };

            Socket const & Connection::getSocket() const {
                return socket;
            };

            const Address & Connection::getEndpoint() const {
                return endpoint;
            };


            clock_t Connection::getLastReadTime() const {
                return lastRead;
            }

            void Connection::setEndpoint(Address & address) {
                endpoint = address;
            };

        }
    }
}