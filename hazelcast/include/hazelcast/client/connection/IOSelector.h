//
// Created by sancar koyunlu on 16/12/13.
//


#ifndef HAZELCAST_NIOListener
#define HAZELCAST_NIOListener

#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/SocketSet.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/ThreadArgs.h"
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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

                virtual bool start() = 0;

                bool initListenSocket(hazelcast::util::SocketSet &wakeUpSocketSet);

                static void staticListen(hazelcast::util::ThreadArgs& args);

                void listen();

                virtual void listenInternal() = 0;

                void addTask(ListenerTask *listenerTask);

                void wakeUp();

                void shutdown();

                void addSocket(const Socket &socket);

                void removeSocket(const Socket &socket);

            protected:

                struct timeval t;
                hazelcast::util::SocketSet socketSet;
                int wakeUpListenerSocketId;
                ConnectionManager &connectionManager;
            private:
                void processListenerQueue();

                std::auto_ptr<Socket> wakeUpSocket;
                std::auto_ptr<Socket> sleepingSocket;
                hazelcast::util::ConcurrentQueue<ListenerTask> listenerTasks;
                hazelcast::util::AtomicBoolean isAlive;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_NIOListener

