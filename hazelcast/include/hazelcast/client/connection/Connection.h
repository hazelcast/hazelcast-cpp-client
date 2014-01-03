//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/InputHandler.h"
#include "hazelcast/client/connection/OutputHandler.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class SerializationService;

            class Data;
        }

        class Address;

        namespace connection {
            class OListener;

            class IListener;

            static int CONN_ID = 1;

            class HAZELCAST_API Connection {
            public:
                Connection(const Address &address, connection::ConnectionManager &connectionManager, serialization::SerializationService &, spi::ClusterService &clusterService, IListener &iListener, OListener &listener);

                void connect();

                void close();

                bool write(serialization::Data const &data);

                const Address &getRemoteEndpoint() const;

                void setRemoteEndpoint(Address &remoteEndpoint);

                int getConnectionId() const;

                const Socket &getSocket() const;

                void writeBlocking(serialization::Data const &data);

                serialization::Data readBlocking();

                boost::atomic<clock_t> lastRead;
                boost::atomic<clock_t> lastWrite;
                boost::atomic<bool> live;
            private:
                serialization::SerializationService &serializationService;
                connection::ConnectionManager &connectionManager;
                Socket socket;
                int connectionId;
                Address remoteEndpoint;
                InputHandler readHandler;
                OutputHandler writeHandler;
            };


            inline std::ostream &operator <<(std::ostream &strm, const Connection &a) {
                return strm << "Connection [id " << a.getConnectionId() << "][" << a.getRemoteEndpoint()
                        << " -> " << a.getSocket().getAddress() << "]";
            };
        }
    }
}


#endif //HAZELCAST_CONNECTION
