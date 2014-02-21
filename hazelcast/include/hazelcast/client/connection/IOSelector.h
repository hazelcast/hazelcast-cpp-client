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

                virtual void start() = 0;

                void initListenSocket(util::SocketSet &wakeUpSocketSet);

                void listen();

                virtual void listenInternal() = 0;

                void addTask(ListenerTask *listenerTask);

                void wakeUp();

                void shutdown();

                void addSocket(const Socket &socket);

                void removeSocket(const Socket &socket);

            protected:

                struct timeval t;
                util::SocketSet socketSet;
                int wakeUpListenerSocketId;
                ConnectionManager &connectionManager;
            private:
                void processListenerQueue();

                std::auto_ptr<Socket> wakeUpSocket;
                std::auto_ptr<Socket> sleepingSocket;
                util::ConcurrentQueue<ListenerTask> listenerTasks;
                boost::atomic<bool> isAlive;
            };
        }
    }
}

#endif //HAZELCAST_NIOListener
