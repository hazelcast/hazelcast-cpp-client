//
// Created by sancar koyunlu on 16/12/13.
//


#ifndef HAZELCAST_NIOListener
#define HAZELCAST_NIOListener

#include "hazelcast/util/ConcurrentQueue.h"
#include <map>

namespace hazelcast {
    namespace util {
        class SocketSet;
    }
    namespace client {
        class Socket;

        namespace connection {
            class ReadHandler;

            class ListenerTask;

            int KILO_BYTE = 1024;
            int BUFFER_SIZE = 16 << 10; // 32k

            int socketReceiveBufferSize = 32 * KILO_BYTE;

            class NIOListener {
                friend class AddWriteSocketTask;
            public:
                NIOListener(util::SocketSet &readers, util::SocketSet &writers);

                void listenReaders();

                void listenWriters();

                void addReadTask(ListenerTask* listenerTask);

                void addWriteTask(ListenerTask* listenerTask);


            private:
                void processListenerQueue();

                void addWriteSocket(Socket &socket);

                struct timeval t;
                bool isAlive;
                util::SocketSet &readers;
                util::SocketSet &writers;
                util::ConcurrentQueue<ListenerTask> readListenerTask;
                util::ConcurrentQueue<ListenerTask> writeListenerTask;
                std::map<int, ReadHandler > readHandlers;
//                std::map<int , WriteHandler > writeHandlers;
            };
        }
    }
}

#endif //HAZELCAST_NIOListener
