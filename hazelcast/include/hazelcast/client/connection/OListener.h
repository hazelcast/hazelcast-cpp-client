//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_OListener
#define HAZELCAST_OListener

#include "hazelcast/client/connection/IOListener.h"
#include "hazelcast/util/SocketSet.h"
#include <boost/thread/mutex.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/condition_variable.hpp>
#include <map>

namespace hazelcast {
    namespace client {
        namespace connection {
            class WriteHandler;

            class HAZELCAST_API OListener : public IOListener{
            public:
                OListener();

                void listen();

            private:
                std::map<int, WriteHandler*> writeHandlers; //TODO who fills this map ?
            };
        }
    }
}


#endif //HAZELCAST_OListener
