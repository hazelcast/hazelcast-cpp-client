//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_WriteHandler
#define HAZELCAST_WriteHandler

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/util/AtomicBoolean.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Packet;

                class Data;
            }
        }
        namespace connection {
            class Connection;

            class OutSelector;

            class ConnectionManager;

            class HAZELCAST_API WriteHandler : public IOHandler {
            public:
                WriteHandler(Connection &connection, OutSelector &oListener, int bufferSize);
                void handle();

                void enqueueData(serialization::pimpl::Packet *packet);

                void run();

            private:
                util::ByteBuffer buffer;
                util::ConcurrentQueue<serialization::pimpl::Packet> writeQueue;
                serialization::pimpl::Packet *lastData;
                bool ready;
                util::AtomicBoolean informSelector;


            };
        }
    }
}


#endif //HAZELCAST_WriteHandler

