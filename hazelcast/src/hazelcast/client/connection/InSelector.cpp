//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            InSelector::InSelector(ConnectionManager &connectionManager)
            : IOSelector(connectionManager) {
                initListenSocket(socketSet);
            }

            void InSelector::listen() {
                while (isAlive) {
                    processListenerQueue();

                    int n = socketSet.getHighestSocketId();
                    fd_set read_fds = socketSet.get_fd_set();
                    int err = select(n + 1, &read_fds, NULL, NULL, &t);
                    if (err == 0) {
                        continue;
                    }
                    if (err == -1) {
                        (std::cerr << "InSelector thread select : " << std::string(strerror(errno)) << std::endl);
                        continue;
                    }


                    std::set<Socket const *>::iterator it;
                    it = socketSet.sockets.begin();
                    while (it != socketSet.sockets.end()) {
                        Socket const *currentSocket = *it;
                        ++it;
                        int id = currentSocket->getSocketId();
                        if (FD_ISSET(id, &read_fds)) {
                            if (wakeUpListenerSocketId == id) {
                                int wakeUpSignal;
                                currentSocket->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                                continue;
                            }

                            boost::shared_ptr<Connection> conn = connectionManager.getConnectionIfAvailable(currentSocket->getRemoteEndpoint());
                            if (conn.get() != NULL)
                                conn->getReadHandler().handle();
                        }
                    }


                }
            }
        }
    }
}
