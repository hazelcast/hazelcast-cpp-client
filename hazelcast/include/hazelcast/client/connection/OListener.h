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
    namespace util{
        class ServerSocket;
    }
    namespace client {
        class Soclet;
        namespace connection {
            class OutputHandler;

            class HAZELCAST_API OListener : public IOListener{
            public:
                OListener();

                ~OListener();

                void wakeUp();

                void init();

                void listen();

            private:
                void acceptThread(util::ServerSocket * serverSocket);
                util::SocketSet wakeUpSocketSet;
                Socket* wakeUpSocket;
            };
        }
    }
}


#endif //HAZELCAST_OListener
