//
// Created by sancar koyunlu on 25/12/13.
//

#include <signal.h>
#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            InSelector::InSelector(ConnectionManager& connectionManager)
            : IOSelector(connectionManager) {

            }

            bool InSelector::start() {
                return initListenSocket(socketSet);
            }

            void InSelector::listenInternal() {
                std::set<Socket const *, socketPtrComp> currentSockets = socketSet.getSockets();
                int n = util::SocketSet::getHighestSocketId(currentSockets);
                fd_set read_fds = util::SocketSet::get_fd_set(currentSockets);

                errno = 0;
                int err = select(n + 1, &read_fds, NULL, NULL, &t);
                if (err == 0) {
                    return;
                }
                if (err == -1) {
                    if (errno != EINTR) {
                        util::ILogger::getLogger().severe(std::string("Exception InSelector::listen => ") + strerror(errno));
                    } else{
                        util::ILogger::getLogger().finest(std::string("Exception InSelector::listen => ") + strerror(errno));
                    }
                    return;
                }

                std::set<Socket const *, client::socketPtrComp>::iterator it;
                it = currentSockets.begin();
                while (it != currentSockets.end()) {
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
