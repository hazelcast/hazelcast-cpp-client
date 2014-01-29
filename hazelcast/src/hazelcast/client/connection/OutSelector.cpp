//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/OutSelector.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"


namespace hazelcast {
    namespace client {
        namespace connection {

            OutSelector::OutSelector(ConnectionManager &connectionManager)
            :IOSelector(connectionManager) {
                initListenSocket(wakeUpSocketSet);
            }

            OutSelector::~OutSelector() {
                std::set<Socket const *>::iterator it = wakeUpSocketSet.sockets.begin();
                delete (*it);
            };


            void OutSelector::listen() {
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
                        (std::cerr << "Olistener thread select : " << std::string(strerror(errno)) << std::endl);
                        continue;
                    }
                    std::set<Socket const *>::iterator it;
                    for (it = wakeUpSocketSet.sockets.begin(); it != wakeUpSocketSet.sockets.end(); ++it) {
                        if (FD_ISSET((*it)->getSocketId(), &wakeUp_fd)) {
                            int wakeUpSignal;
                            (*it)->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                        }
                    }

                    it = socketSet.sockets.begin();
                    while (it != socketSet.sockets.end()) {
                        Socket const *currentSocket = *it;
                        ++it;
                        int id = currentSocket->getSocketId();
                        if (FD_ISSET(id, &write_fds)) {
                            socketSet.sockets.erase(currentSocket);
                            boost::shared_ptr<Connection> conn = connectionManager.getConnectionIfAvailable(currentSocket->getRemoteEndpoint());
                            if (conn.get() != NULL)
                                conn->getWriteHandler().handle();
                            else
                                std::cerr << "OutSelector null connection " << std::endl;
                        }
                    }

                }
            }
        }
    }
}
