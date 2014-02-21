//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/OutSelector.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            OutSelector::OutSelector(ConnectionManager &connectionManager)
            :IOSelector(connectionManager) {

            }


            void OutSelector::start() {
                initListenSocket(wakeUpSocketSet);
            }

            void OutSelector::listenInternal() {

                using std::max;
                int n = max(wakeUpSocketSet.getHighestSocketId(), socketSet.getHighestSocketId());
                fd_set write_fds = socketSet.get_fd_set();
                fd_set wakeUp_fd = wakeUpSocketSet.get_fd_set();
                int err = select(n + 1, &wakeUp_fd, &write_fds, NULL, &t);
                if (err == 0) {
                    return;
                }
                if (err == -1) {
                    hazelcast::util::ILogger::warning("Olistener::listen ", std::string(strerror(errno)));
                    return;
                }
                std::set<Socket const *, client::socketPtrComp>::iterator it;
                for (it = wakeUpSocketSet.sockets.begin(); it != wakeUpSocketSet.sockets.end(); ++it) {
                    if (FD_ISSET((*it)->getSocketId(), &wakeUp_fd)) {
                        int wakeUpSignal;
                        (*it)->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                        return;
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
                    }
                }

            }
        }
    }
}
