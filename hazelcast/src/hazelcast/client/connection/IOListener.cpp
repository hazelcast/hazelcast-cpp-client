//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/IOListener.h"
#include "hazelcast/client/connection/ListenerTask.h"


namespace hazelcast {
    namespace client {
        namespace connection {


            IOListener::IOListener() {
                t.tv_sec = 10;
                t.tv_usec = 0;
                isAlive = true;
            };


            IOListener::~IOListener() {
                shutdown();
            }


            void IOListener::shutdown() {
                isAlive = false;
            }

            void IOListener::addTask(ListenerTask *listenerTask) {
                listenerTask->init(this);
                listenerTasks.offer(listenerTask);
            }

            void IOListener::addSocket(const Socket &socket) {
                socketSet.sockets.insert(&socket);
            }

            void IOListener::processListenerQueue() {
                while (ListenerTask *task = listenerTasks.poll()) {
                    task->process();
                    delete task;
                }
            }
        }
    }
}

