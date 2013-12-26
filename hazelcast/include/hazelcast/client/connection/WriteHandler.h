//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_WriteHandler
#define HAZELCAST_WriteHandler

#include "hazelcast/util/CircularBuffer.h"
#include "hazelcast/util/ConcurrentQueue.h"


namespace hazelcast {
    namespace client {
        namespace serialization {
            class DataAdapter;

            class Data;
        }
        namespace connection {
            class Connection;

            class OListener;

            class ConnectionManager;

            class HAZELCAST_API WriteHandler {
            public:
                WriteHandler(Connection &connection, OListener &oListener, int bufferSize);

                void handle();

                void enqueueData(const serialization::Data &data);

            private:
                util::CircularBuffer buffer;
                util::ConcurrentQueue<serialization::DataAdapter> writeQueue;
                Connection &connection;
                OListener &oListener;
                serialization::DataAdapter *lastData;
                bool initialized;
            };
        }
    }
}


#endif //HAZELCAST_WriteHandler
