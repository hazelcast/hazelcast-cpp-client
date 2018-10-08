/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_CONNECTION_IOSELECTOR_H_
#define HAZELCAST_CLIENT_CONNECTION_IOSELECTOR_H_

#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/SocketSet.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/Runnable.h"
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {
        class Socket;

        namespace connection {
            class ListenerTask;

            class IOHandler;

            class ClientConnectionManagerImpl;

            class HAZELCAST_API IOSelector : public util::Runnable {
            public:
                IOSelector(ClientConnectionManagerImpl &connectionManager);

                virtual ~IOSelector();

                virtual bool start() = 0;

                bool initListenSocket(util::SocketSet &wakeUpSocketSet);

                void run();

                void addTask(ListenerTask *listenerTask);

                void cancelTask(ListenerTask *listenerTask);

                void wakeUp();

                void shutdown();

                void addSocket(const Socket &socket);

                void removeSocket(const Socket &socket);

            protected:
                struct timeval t;
                util::SocketSet socketSet;
                int wakeUpListenerSocketId;
                ClientConnectionManagerImpl &connectionManager;
                std::auto_ptr<Socket> sleepingSocket;
                util::ILogger &logger;

                virtual void listenInternal() = 0;

                /**
                 * @return true if should return from the calling method
                 */
                bool checkError(const char *messagePrefix, int numSelected) const;

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

#endif //HAZELCAST_CLIENT_CONNECTION_IOSELECTOR_H_

