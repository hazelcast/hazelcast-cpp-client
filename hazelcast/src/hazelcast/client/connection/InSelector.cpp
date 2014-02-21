//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            InSelector::InSelector(ConnectionManager &connectionManager)
            : IOSelector(connectionManager) {

            }


            void InSelector::start() {
                initListenSocket(socketSet);
            }

            void InSelector::listenInternal() {
                int n = socketSet.getHighestSocketId();
                fd_set read_fds = socketSet.get_fd_set();
                int err = select(n + 1, &read_fds, NULL, NULL, &t);
                if (err == 0) {
                    return;
                }
                if (err == -1) {
                    hazelcast::util::ILogger::warning("InSelector::listen ", std::string(strerror(errno)));
                    return;
                }

                std::set<Socket const *, client::socketPtrComp>::iterator it;
                it = socketSet.sockets.begin();
                while (it != socketSet.sockets.end()) {
                    Socket const *currentSocket = *it;
                    ++it;
                    int id = currentSocket->getSocketId();
                    if (FD_ISSET(id, &read_fds)) {
                        if (wakeUpListenerSocketId == id) {
                            int wakeUpSignal;
                            currentSocket->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                            return;
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
