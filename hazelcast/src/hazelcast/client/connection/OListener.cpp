//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/OListener.h"
#include "hazelcast/client/connection/IOHandler.h"
#include <boost/thread/detail/thread.hpp>


namespace hazelcast {
    namespace client {
        namespace connection {

            OListener::OListener() {
                initListenSocket(wakeUpSocketSet);
            }

            OListener::~OListener() {
                std::set<Socket const *>::iterator it = wakeUpSocketSet.sockets.begin();
                delete (*it);
            };


            void OListener::listen() {
                while (isAlive) {
                    processListenerQueue();
                    int n = std::max(wakeUpSocketSet.getHighestSocketId(), socketSet.getHighestSocketId());
                    fd_set write_fds = socketSet.get_fd_set();
                    fd_set wakeUp_fd = wakeUpSocketSet.get_fd_set();
                    int err = select(n + 1, &wakeUp_fd, &write_fds, NULL, &t);
                    if (err == 0) {
                        continue;
                    }
                    if (err == -1) {
                        perror("select");
                        continue;
                    }
                    std::set<Socket const *>::iterator it;
                    for (it = wakeUpSocketSet.sockets.begin(); it != wakeUpSocketSet.sockets.end(); ++it) {
                        if (FD_ISSET((*it)->getSocketId(), &wakeUp_fd)) {
                            int wakeUpSignal;
                            (*it)->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                            std::cerr << "writer wokeup " << wakeUpSignal << std::endl;
                        }
                    }

                    std::set<Socket const *>::iterator temp;
                    it = socketSet.sockets.begin();
                    std::cout << "--" << std::endl;
                    while (it != socketSet.sockets.end()) {
                        temp = it++;
                        std::cout << "checking write sockets " << (*temp)->getSocketId() << std::endl;
                        if (FD_ISSET((*temp)->getSocketId(), &write_fds)) {
                            socketSet.sockets.erase(temp);
                            std::cout << "handle write of " << (*temp)->getSocketId() << std::endl;
                            ioHandlers[(*temp)->getSocketId()]->handle();
                        }
                    }

                }
            }
        }
    }
}