//
// Created by sancar koyunlu on 20/08/14.
//


#ifndef HAZELCAST_HeartBeater
#define HAZELCAST_HeartBeater

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/AtomicBoolean.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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

                static void staticStart(hazelcast::util::ThreadArgs& args);

                void shutdown();

            private:
                void run(hazelcast::util::Thread *currentThread);

                hazelcast::util::AtomicBoolean live;
                spi::ClientContext& clientContext;
                int heartBeatIntervalSeconds;
                int heartBeatTimeoutSeconds;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_HeartBeater
