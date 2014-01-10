//
// Created by sancar koyunlu on 24/12/13.
//


#ifndef HAZELCAST_ReadHandler
#define HAZELCAST_ReadHandler

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/connection/IOHandler.h"

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

            class IListener;

            class HAZELCAST_API ReadHandler : public IOHandler{
            public:
                ReadHandler(Connection &connection, IListener&iListener, int bufferSize);

                void handle();

                void run();

            private:
                util::ByteBuffer buffer;
                serialization::DataAdapter *lastData;


            };
        }
    }
}

#endif //HAZELCAST_ReadHandler
