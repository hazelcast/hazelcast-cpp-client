//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/WriteHandler.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class SerializationService;

            class Data;
        }


        class Address;

        namespace connection {
            class OListener;

            static int CONN_ID = 1;

            class HAZELCAST_API Connection {
            public:
                Connection(const Address &address, serialization::SerializationService &,spi::ClusterService& clusterService, OListener& listener);

                void connect();

                void close();

                bool write(serialization::Data const &data);

                const Address &getEndpoint() const;

                const Address &getRemoteEndpoint() const;

                void setRemoteEndpoint(Address& remoteEndpoint);

                int getConnectionId() const;

                const Socket &getSocket() const;

                boost::atomic<clock_t> lastRead;
                boost::atomic<clock_t> lastWrite;
                boost::atomic<bool> live;
            private:
                serialization::SerializationService &serializationService;
                Socket socket;
                int connectionId;
                Address remoteEndpoint;
                ReadHandler readHandler;
                WriteHandler writeHandler;
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
