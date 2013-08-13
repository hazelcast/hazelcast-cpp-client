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

                void connect();

                void write(const std::vector<byte>& bytes);

                void write(const serialization::Data&);

                serialization::Data read(serialization::SerializationContext & serializationContext);

                const Address& getEndpoint() const;

                void setEndpoint(Address& address);

                clock_t getLastReadTime() const;

                int getConnectionId() const;

                const Socket& getSocket() const;

            private:
                Address endpoint;
                serialization::SerializationService& serializationService;
                Socket socket;
                serialization::InputSocketStream inputSocketStream;
                serialization::OutputSocketStream outputSocketStream;
                int connectionId;
                clock_t lastRead;
            };


            inline std::ostream& operator <<(std::ostream &strm, const Connection &a) {
                return strm << "Connection [id " << util::to_string(a.getConnectionId()) << "][" << a.getEndpoint()
                        << " -> " << a.getSocket().getHost() << ":"
                        << util::to_string(a.getSocket().getPort()) << "]";
            };
        }
    }
}


#endif //HAZELCAST_CONNECTION
