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

            class IOHandler;

            class ConnectionManager;

            class HAZELCAST_API IOSelector {
            public:
                IOSelector(ConnectionManager &connectionManager);

                virtual ~IOSelector();

                void initListenSocket(util::SocketSet &wakeUpSocketSet);

                virtual void listen() = 0;

                void addTask(ListenerTask *listenerTask);

                void wakeUp();

                void shutdown();

                void addSocket(const Socket &socket);

                void removeSocket(const Socket &socket);

            protected:

                void processListenerQueue();

                struct timeval t;
                boost::atomic<bool> isAlive;
                util::SocketSet socketSet;
                util::ConcurrentQueue<ListenerTask> listenerTasks;
                Socket *wakeUpSocket;
                int wakeUpListenerSocketId;
                ConnectionManager &connectionManager;
            };
        }
    }
}

#endif //HAZELCAST_NIOListener
