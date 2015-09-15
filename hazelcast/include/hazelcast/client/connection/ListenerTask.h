//
// Created by sancar koyunlu on 24/12/13.
//

#ifndef HAZELCAST_ListenerTask
#define HAZELCAST_ListenerTask

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class HAZELCAST_API ListenerTask {
            public:
                virtual void run() = 0;

                virtual ~ListenerTask();
            };
        }
    }
}


#endif //HAZELCAST_ListenerTask

