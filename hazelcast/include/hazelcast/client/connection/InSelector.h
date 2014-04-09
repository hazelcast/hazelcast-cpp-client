//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_IListener
#define HAZELCAST_IListener

#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/util/SocketSet.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            class HAZELCAST_API InSelector : public IOSelector {
            public:
                InSelector(ConnectionManager &connectionManager);

                void listenInternal();

                bool start();
            };
        }
    }
}

#endif //HAZELCAST_IListener

