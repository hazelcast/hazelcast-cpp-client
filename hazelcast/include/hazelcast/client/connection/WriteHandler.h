//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_WriteHandler
#define HAZELCAST_WriteHandler

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/util/AtomicBoolean.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

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

            class WriteHandler : public IOHandler {
            public:
                WriteHandler(Connection &connection, OutSelector &oListener, int bufferSize);

                ~WriteHandler();

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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_WriteHandler

