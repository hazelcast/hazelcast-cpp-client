//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/IListener.h"
#include "hazelcast/client/connection/ReadHandler.h"

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
                        (std::cerr << "Ilistener thread select : " << std::string(strerror(errno)) << std::endl);
                        continue;
                    }
                    std::set<Socket const *>::iterator it;
                    for (it = socketSet.sockets.begin(); it != socketSet.sockets.end(); ++it) {
                        if (FD_ISSET((*it)->getSocketId(), &read_fds)) {
                            if (wakeUpListenerSocketId == (*it)->getSocketId()) {
                                int wakeUpSignal;
                                (*it)->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                                continue;
                            }
                            ioHandlers[(*it)->getSocketId()]->handle();
                        }
                    }
                }
            }
        }
    }
}
