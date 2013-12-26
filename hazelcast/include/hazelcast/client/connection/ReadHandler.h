//
// Created by sancar koyunlu on 24/12/13.
//


#ifndef HAZELCAST_ReadHandler
#define HAZELCAST_ReadHandler

#include "hazelcast/util/CircularBuffer.h"


namespace hazelcast {
    namespace client {
        namespace serialization {
            class DataAdapter;
        }
        namespace spi {
                      class ClusterService;
        }
        namespace connection {
            class Connection;

            class ConnectionManager;

            class HAZELCAST_API ReadHandler {
            public:
                ReadHandler(Connection &connection, spi::ClusterService &clusterService, int bufferSize);

                void handle();

            private:
                util::CircularBuffer buffer;
                Connection &connection;
                spi::ClusterService &clusterService;
                serialization::DataAdapter *dataAdapter;
            };
        }
    }
}

#endif //HAZELCAST_ReadHandler
