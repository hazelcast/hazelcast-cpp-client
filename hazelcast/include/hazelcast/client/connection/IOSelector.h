/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

                bool initListenSocket(util::SocketSet &wakeUpSocketSet);

                static void staticListen(util::ThreadArgs& args);

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
                std::auto_ptr<Socket> sleepingSocket;
            private:
                void processListenerQueue();

                std::auto_ptr<Socket> wakeUpSocket;
                util::ConcurrentQueue<ListenerTask> listenerTasks;
                util::AtomicBoolean isAlive;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_NIOListener

