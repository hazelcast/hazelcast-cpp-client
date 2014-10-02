//
// Created by sancar koyunlu on 24/12/13.
//

#ifndef HAZELCAST_ListenerTask
#define HAZELCAST_ListenerTask

namespace hazelcast {
    namespace client {
        namespace connection {
            class ListenerTask {
            public:
                virtual void run() = 0;

                virtual ~ListenerTask() {

                };
            };
        }
    }
}


#endif //HAZELCAST_ListenerTask

