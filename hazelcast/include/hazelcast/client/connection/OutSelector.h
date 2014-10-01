//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_OListener
#define HAZELCAST_OListener

#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/util/SocketSet.h"
#include "hazelcast/util/Mutex.h"
#include <map>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_OListener

