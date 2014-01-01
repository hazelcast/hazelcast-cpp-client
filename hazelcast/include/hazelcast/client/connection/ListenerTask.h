//
// Created by sancar koyunlu on 24/12/13.
//




#ifndef HAZELCAST_ListenerTask
#define HAZELCAST_ListenerTask

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class IOListener;

            class HAZELCAST_API ListenerTask {
            public:
//                void init(IOListener *nioListener) {
//                    this->nioListener = nioListener;
//                };

                virtual void run() = 0;

                virtual ~ListenerTask() {

                };
            protected:
//                IOListener *nioListener;
            };
        }
    }
}


#endif //HAZELCAST_ListenerTask
