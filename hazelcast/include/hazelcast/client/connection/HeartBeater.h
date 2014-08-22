//
// Created by sancar koyunlu on 20/08/14.
//


#ifndef HAZELCAST_HeartBeater
#define HAZELCAST_HeartBeater


namespace hazelcast {
    namespace util {
        class ThreadArgs;
    }
    namespace client {
        namespace spi{
            class ClientContext;
        }

        namespace connection {
            class HeartBeater {
            public:
                HeartBeater(spi::ClientContext& clientContext);

                static void staticStart(util::ThreadArgs& args);

                void shutdown();

            private:

                time_t getRemainingTimeout(time_t, int heartBeatTimeoutSeconds);

                void run(util::Thread *currentThread);

                volatile bool live;
                spi::ClientContext& clientContext;
                int heartBeatIntervalSeconds;
                int heartBeatTimeoutSeconds;
            };
        }
    }
}


#endif //HAZELCAST_HeartBeater
