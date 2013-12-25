//
// Created by sancar koyunlu on 24/12/13.
//


#ifndef HAZELCAST_ReadHandler
#define HAZELCAST_ReadHandler

#include "util/CircularBuffer.h"

namespace hazelcast {
    namespace client {
        namespace serialization{
            class DataAdapter;
        }
        namespace connection {
            class Connection;

            class ReadHandler {
            public:
                ReadHandler(Connection& connection, int bufferSize);

                void handle();

            private:
                util::CircularBuffer buffer;
                Connection& connection;
                serialization::DataAdapter *dataAdapter;
            };
        }
    }
}

#endif //HAZELCAST_ReadHandler
