//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/OListener.h"
#include "hazelcast/client/connection/WriteHandler.h"


namespace hazelcast {
    namespace client {
        namespace connection {
            OListener::OListener() {

            }

            void OListener::listen() {
                while (isAlive) {
                    processListenerQueue();
                    int n = socketSet.getHighestSocketId();
                    fd_set write_fds = socketSet.get_fd_set();
                    switch (select(n, NULL, &write_fds, NULL, &t)) {
                        case 0:
                            continue;
                        case 1:
                            throw "exception";
                        default:;
                    }
                    std::set<Socket const *>::iterator it;
                    for (it = socketSet.sockets.begin(); it != socketSet.sockets.end(); ++it) {
                        if (FD_ISSET((*it)->getSocketId(), &write_fds)) {
                            writeHandlers[(*it)->getSocketId()]->handle();
                        }
                    }
                    socketSet.sockets.clear();

                }
            }
        }
    }
}