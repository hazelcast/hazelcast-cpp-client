//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class SerializationService;

            class DataAdapter;
        }


        class Address;

        namespace connection {
            static int CONN_ID = 1;

            class HAZELCAST_API Connection {
            public:
                Connection(const Address &address, serialization::SerializationService &);

                void connect();

                void close();

                void write(serialization::DataAdapter const &data);

                const Address &getEndpoint() const;

                int getConnectionId() const;

                const Socket &getSocket() const;

                clock_t lastRead;
                clock_t lastWrite;
                boost::atomic<bool> live;
            private:
                serialization::SerializationService &serializationService;
                Socket socket;
                int connectionId;
            };


            inline std::ostream &operator <<(std::ostream &strm, const Connection &a) {
                return strm << "Connection [id " << util::to_string(a.getConnectionId()) << "][" << a.getEndpoint()
                        << " -> " << a.getSocket().getHost() << ":"
                        << util::to_string(a.getSocket().getPort()) << "]";
            };
        }
    }
}


#endif //HAZELCAST_CONNECTION
