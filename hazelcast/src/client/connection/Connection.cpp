//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include <iostream>
#include "Connection.h"
#include "../serialization/BufferedDataOutput.h"

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

            void Connection::write(vector<byte> const& bytes) {
                outputSocketStream.write(bytes);
            };

            void Connection::write(serialization::Data const & data) {
                serialization::BufferedDataOutput out;
                data.writeData(out);
                auto_ptr<vector<byte> > buffer = out.toByteArray();
                outputSocketStream.write(&((*buffer.get())[0]), buffer->size());
            };

            serialization::Data Connection::read(serialization::SerializationContext & serializationContext) {
                serialization::Data data;
                data.readData(inputSocketStream, serializationContext);
                lastRead = clock();
                return data;
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

            void Connection::close() {
                socket.close();
                outputSocketStream.close();
                inputSocketStream.close();
            };


        }
    }
}