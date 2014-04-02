//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_OListener
#define HAZELCAST_OListener

#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/util/SocketSet.h"
#include "hazelcast/util/Mutex.h"
#include <map>

namespace hazelcast {
    namespace util {
        class ServerSocket;
    }
    namespace client {
        class Soclet;
        namespace connection {
            class WriteHandler;

            class HAZELCAST_API OutSelector : public IOSelector {
            public:
                OutSelector(ConnectionManager &connectionManager);

                void listenInternal();

                bool start();

            private:
                util::SocketSet wakeUpSocketSet;
            };
        }
    }
}


#endif //HAZELCAST_OListener
