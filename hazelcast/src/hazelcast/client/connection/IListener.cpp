//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/IListener.h"
#include "ReadHandler.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            IListener::IListener() {
                initListenSocket(socketSet);
            }

            void IListener::listen() {
                while (isAlive) {
                    processListenerQueue();

                    int n = socketSet.getHighestSocketId();
                    fd_set read_fds = socketSet.get_fd_set();
                    int err = select(n + 1, &read_fds, NULL, NULL, &t);
                    if (err == 0) {
                        continue;
                    }
                    if (err == -1) {
                        perror("select");
                        continue;
                    }
                    std::set<Socket const *>::iterator it;
                    std::cout << "||" << std::endl;
                    for (it = socketSet.sockets.begin(); it != socketSet.sockets.end(); ++it) {
                        if (FD_ISSET((*it)->getSocketId(), &read_fds)) {
                            if (wakeUpListenerSocketId == (*it)->getSocketId()) {
                                int wakeUpSignal;
                                (*it)->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                                std::cerr << "reader wokeup " << wakeUpSignal << std::endl;
                                continue;
                            }
                            std::cout << "handle read of " << (*it)->getSocketId() << std::endl;
                            ioHandlers[(*it)->getSocketId()]->handle();
                        }
                    }
                }
            }
        }
    }
}