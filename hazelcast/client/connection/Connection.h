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
                Connection(const Address& address, serialization::SerializationService&);

                void write(const vector<byte>& bytes);

                void write(const serialization::Data&);

                serialization::Data read(serialization::SerializationContext & serializationContext);

                void close();

                const Address& getEndpoint() const;

                const Socket& getSocket() const;

            private:
                Address endpoint;
                serialization::SerializationService& serializationService;
                Socket socket;
                serialization::InputSocketStream inputSocketStream;
                serialization::OutputSocketStream outputSocketStream;
                int connectionId;
            };
        }
    }
}

inline std::ostream& operator <<(std::ostream &strm, const hazelcast::client::connection::Connection &a) {
    return strm << std::string("Connection [") << a.getEndpoint()
            << std::string(" -> ") << a.getSocket().getHost() << std::string(":")
            << hazelcast::util::to_string(a.getSocket().getPort()) << std::string("]");
};

#endif //HAZELCAST_CONNECTION
