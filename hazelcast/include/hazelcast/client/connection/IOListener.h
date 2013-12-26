//
// Created by sancar koyunlu on 16/12/13.
//


#ifndef HAZELCAST_NIOListener
#define HAZELCAST_NIOListener

#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/SocketSet.h"

namespace hazelcast {
    namespace client {
        class Socket;

        namespace connection {
            class ListenerTask;

            class HAZELCAST_API IOListener {
            public:
                IOListener();

                ~IOListener();

                virtual void listen() = 0;

                void addTask(ListenerTask *listenerTask);

                void shutdown();

                void addSocket(const Socket &socket);
            protected:

                void processListenerQueue();

                struct timeval t;
                boost::atomic<bool> isAlive;
                util::SocketSet socketSet;
                util::ConcurrentQueue<ListenerTask> listenerTasks;
            };
        }
    }
}

#endif //HAZELCAST_NIOListener
