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
    namespace util {
        class ServerSocket;
    }
    namespace client {
        class Soclet;
        namespace connection {
            class WriteHandler;

            class HAZELCAST_API OListener : public IOListener {
            public:
                OListener();

                ~OListener();

                void listen();

            private:
                util::SocketSet wakeUpSocketSet;

            };
        }
    }
}


#endif //HAZELCAST_OListener
