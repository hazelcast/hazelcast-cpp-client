//
// Created by sancar koyunlu on 16/12/13.
//


#ifndef HAZELCAST_NIOListener
#define HAZELCAST_NIOListener

#include "SocketSet.h"
#include <algorithm>


namespace hazelcast {
    namespace client {
        namespace connection {
            class NIOListener {
            public:
                NIOListener() {
                    t.tv_sec = 10;
                    t.tv_usec = 0;
                    isAlive = true;
                };


                void listen(util::SocketSet &readers, util::SocketSet &writers) {
                    while (isAlive) {
                        int n = std::max(readers.getHighestSocketId(), writers.getHighestSocketId());
                        fd_set read_fds = readers.get_fd_set();
                        fd_set write_fds = writers.get_fd_set();
                        switch (select(n, &read_fds, &write_fds, NULL, &t)) {
                            case 0:
                                continue;
                            case 1:
                                throw "exception";
                        }
                        std::set<Socket>::iterator it;
                        for (it = readers.sockets.begin(); it != readers.sockets.end(); ++it) {
                            if (FD_ISSET(it->getSocketId(), &read_fds)) {
                                //recv
                            }
                        }

                        for (it = writers.sockets.begin(); it != writers.sockets.end(); ++it) {
                            if (FD_ISSET(it->getSocketId(), &write_fds)) {
                                //send
                            }
                        }
                    }


                }

            private:
                struct timeval t;
                bool isAlive;
            };
        }
    }
}

#endif //HAZELCAST_NIOListener
