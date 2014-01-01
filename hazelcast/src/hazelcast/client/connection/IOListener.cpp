//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/IOListener.h"
#include "hazelcast/client/connection/ListenerTask.h"
#include "hazelcast/client/connection/IOHandler.h"


namespace hazelcast {
    namespace client {
        namespace connection {


            IOListener::IOListener() {
                t.tv_sec = 1;
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
//                listenerTask->init(this);
                listenerTasks.offer(listenerTask);
            }

            void IOListener::addSocket(const Socket &socket) {
                std::cout << ">" << socket.getSocketId() << std::endl;
                socketSet.sockets.insert(&socket);
            }

            void IOListener::removeSocket(const Socket &socket) {
                socketSet.sockets.erase(&socket);
            }

            void IOListener::addHandler(int socketId, IOHandler *handler) {
                ioHandlers[socketId] = handler;
            }

            void IOListener::processListenerQueue() {
                while (ListenerTask *task = listenerTasks.poll()) {
                    task->run();
                }
            }
        }
    }
}

