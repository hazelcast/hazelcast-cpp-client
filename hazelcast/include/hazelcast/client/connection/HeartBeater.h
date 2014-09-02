//
// Created by sancar koyunlu on 20/08/14.
//


#ifndef HAZELCAST_HeartBeater
#define HAZELCAST_HeartBeater

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/AtomicBoolean.h"

namespace hazelcast {
    namespace util {
        class ThreadArgs;

        class Thread;
    }
    namespace client {
        namespace spi{
            class ClientContext;
        }

        namespace connection {
            class HAZELCAST_API HeartBeater {
            public:
                HeartBeater(spi::ClientContext& clientContext);

                static void staticStart(util::ThreadArgs& args);

                void shutdown();

            private:
                void run(util::Thread *currentThread);

                util::AtomicBoolean live;
                spi::ClientContext& clientContext;
                int heartBeatIntervalSeconds;
                int heartBeatTimeoutSeconds;
            };
        }
    }
}


#endif //HAZELCAST_HeartBeater
