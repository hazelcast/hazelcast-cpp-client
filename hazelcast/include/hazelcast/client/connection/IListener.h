//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_IListener
#define HAZELCAST_IListener

#include "hazelcast/client/connection/IOListener.h"
#include "hazelcast/util/SocketSet.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class ReadHandler;

            class HAZELCAST_API IListener : public IOListener {
            public:
                IListener();

                void listen();

            private:
                std::map<int, ReadHandler* > readHandlers; //TODO who fills this map ?
            };
        }
    }
}

#endif //HAZELCAST_IListener
