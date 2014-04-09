//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_WriteHandler
#define HAZELCAST_WriteHandler

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/client/serialization/pimpl/DataAdapter.h"
#include "hazelcast/util/AtomicBoolean.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class DataAdapter;
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

                void enqueueData(const serialization::pimpl::Data &data);

                void run();

            private:
                util::ByteBuffer buffer;
                util::ConcurrentQueue<serialization::pimpl::DataAdapter> writeQueue;
                serialization::pimpl::DataAdapter *lastData;
                bool ready;
                util::AtomicBoolean informSelector;


            };
        }
    }
}


#endif //HAZELCAST_WriteHandler

