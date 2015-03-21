//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/OutSelector.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror
#endif

namespace hazelcast {
    namespace client {
        namespace connection {

            OutSelector::OutSelector(ConnectionManager &connectionManager)
            :IOSelector(connectionManager) {

            }


            bool OutSelector::start() {
                return initListenSocket(wakeUpSocketSet);
            }

            void OutSelector::listenInternal() {
                using std::max;
                std::set<Socket const *, socketPtrComp> currentWakeUpSockets = wakeUpSocketSet.getSockets();
                std::set<Socket const *, socketPtrComp> currentSockets = socketSet.getSockets();
                int id1 = util::SocketSet::getHighestSocketId(currentSockets);
                int id2 = util::SocketSet::getHighestSocketId(currentWakeUpSockets);
                int n = max(id1, id2);

                fd_set write_fds = util::SocketSet::get_fd_set(currentSockets);
                fd_set wakeUp_fd =  util::SocketSet::get_fd_set(currentWakeUpSockets);

                errno = 0;
                int err = select(n + 1, &wakeUp_fd, &write_fds, NULL, &t);
                if (err == 0) {
                    return;
                }
                if (err == -1) {
                    if (errno != EINTR) {
                        util::ILogger::getLogger().severe(std::string("Exception OutSelector::listen => ") + strerror(errno));
                    } else{
                        util::ILogger::getLogger().finest(std::string("Exception OutSelector::listen => ") + strerror(errno));
                    }
                    return;
                }
                std::set<Socket const *, client::socketPtrComp>::iterator it;
                for (it = currentWakeUpSockets.begin(); it != currentWakeUpSockets.end(); ++it) {
                    if (FD_ISSET((*it)->getSocketId(), &wakeUp_fd)) {
                        int wakeUpSignal;
                        (*it)->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                        return;
                    }
                }

                it = currentSockets.begin();
                while (it != currentSockets.end()) {
                    Socket const *currentSocket = *it;
                    ++it;
                    int id = currentSocket->getSocketId();
                    if (FD_ISSET(id, &write_fds)) {
                        socketSet.removeSocket(currentSocket);
                        boost::shared_ptr<Connection> conn = connectionManager.getConnectionIfAvailable(currentSocket->getRemoteEndpoint());
                        if (conn.get() != NULL)
                            conn->getWriteHandler().handle();
                    }
                }

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

