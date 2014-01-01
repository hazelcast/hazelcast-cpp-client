//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/IListener.h"
#include "hazelcast/client/connection/InputHandler.h"

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
                    int err = select(n, &read_fds, NULL, NULL, &t);
                    if (err == 0) {
                        std::cout << "wait for read one more second" << std::endl;
                        continue;
                    }
                    if (err == -1) {
                        perror("select");
                        continue;
                    }
                    std::set<Socket const *>::iterator it;
                    for (it = socketSet.sockets.begin(); it != socketSet.sockets.end(); ++it) {
                        if (FD_ISSET((*it)->getSocketId(), &read_fds)) {
                            std::cout << "handle read" << std::endl;
                            ioHandlers[(*it)->getSocketId()]->handle();
                        }
                    }
                }
            }
        }
    }
}