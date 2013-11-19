//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include "hazelcast/client/connection/Socket.h"
#include "hazelcast/client/serialization/InputSocketStream.h"
#include "hazelcast/client/serialization/OutputSocketStream.h"
#include "hazelcast/client/serialization/Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization{
		class SerializationService;
	}

        class Address;

        namespace connection {
            static int CONN_ID = 1;

            class Connection {
            public:
                Connection(const Address& address, serialization::SerializationService&);

                void connect();

                void write(const std::vector<byte>& bytes);

                void write(const serialization::Data&);

                serialization::Data read();

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
