//
// Created by sancar koyunlu on 24/12/13.
//

#include "hazelcast/client/connection/NIOListener.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/util/SocketSet.h"
#include "hazelcast/client/connection/ListenerTask.h"


namespace hazelcast {
    namespace client {
        namespace connection {


            NIOListener::NIOListener(util::SocketSet &readers, util::SocketSet &writers)
            : readers(readers)
            , writers(writers) {
                t.tv_sec = 10;
                t.tv_usec = 0;
                isAlive = true;
            };

            void NIOListener::listenReaders() {
                while (isAlive) {
                    processListenerQueue();

                    int n = std::max(readers.getHighestSocketId(), writers.getHighestSocketId());
                    fd_set read_fds = readers.get_fd_set();
                    switch (select(n, &read_fds, NULL, NULL, &t)) {
                        case 0:
                            continue;
                        case 1:
                            throw "exception";
                    }
                    std::set<Socket>::iterator it;
                    for (it = readers.sockets.begin(); it != readers.sockets.end(); ++it) {
                        if (FD_ISSET(it->getSocketId(), &read_fds)) {
                            readHandlers[it->getSocketId()].handle();
                        }
                    }
                }
            }

            void NIOListener::listenWriters() {
                while (isAlive) {
                    int n = std::max(readers.getHighestSocketId(), writers.getHighestSocketId());
                    fd_set write_fds = writers.get_fd_set();
                    switch (select(n, NULL, &write_fds, NULL, &t)) {
                        case 0:
                            continue;
                        case 1:
                            throw "exception";
                    }
                    std::set<Socket>::iterator it;
                    for (it = writers.sockets.begin(); it != writers.sockets.end(); ++it) {
                        if (FD_ISSET(it->getSocketId(), &write_fds)) {
                            //send
                        }
                    }
                }
            }


            void NIOListener::addReadTask(ListenerTask *listenerTask) {
                readListenerTask.offer(listenerTask);
            }

            void NIOListener::addWriteTask(ListenerTask *listenerTask) {
                writeListenerTask.offer(listenerTask);
            }


            void NIOListener::addWriteSocket(Socket &socket) {
                writers.sockets.insert(socket);
            }

            void NIOListener::processListenerQueue() {
                while (ListenerTask *task = readListenerTask.poll()) {
                    task->process();
                }
            }
        }
    }
}

