//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/IListener.h"
#include "hazelcast/client/connection/ReadHandler.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            IListener::IListener() {

            }
            void IListener::listen() {
                while (isAlive) {
                    processListenerQueue();

                    int n = socketSet.getHighestSocketId();
                    fd_set read_fds = socketSet.get_fd_set();
                    switch (select(n, &read_fds, NULL, NULL, &t)) {
                        case 0:
                            continue;
                        case 1:
                            throw "exception";
                    }
                    std::set<Socket const *>::iterator it;
                    for (it = socketSet.sockets.begin(); it != socketSet.sockets.end(); ++it) {
                        if (FD_ISSET((*it)->getSocketId(), &read_fds)) {
                            readHandlers[(*it)->getSocketId()]->handle();
                        }
                    }
                }
            }
        }
    }
}