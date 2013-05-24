//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#include "Socket.h"
#include "../serialization/InputSocketStream.h"
#include "../serialization/OutputSocketStream.h"
#include "../serialization/Data.h"

#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

namespace hazelcast {
    namespace client {
        class serialization::SerializationService;

        class Address;

        namespace connection {
            static int CONN_ID = 1;

            class Connection {
            public:
                Connection(const hazelcast::client::Address& address, hazelcast::client::serialization::SerializationService&);

                void write(const vector<byte>& bytes);

                void write(const hazelcast::client::serialization::Data&);

                void read(hazelcast::client::serialization::Data&);

                void close();

                hazelcast::client::Address& getEndpoint();

            private:
                hazelcast::client::Address endpoint;
                hazelcast::client::serialization::SerializationService& serializationService;
                Socket socket;
                hazelcast::client::serialization::InputSocketStream inputSocketStream;
                hazelcast::client::serialization::OutputSocketStream outputSocketStream;
                int connectionId;
            };
        }
    }
}

#endif //HAZELCAST_CONNECTION
