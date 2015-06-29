//
// Created by sancar koyunlu on 25/12/13.
//

#include <string.h>
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
                fd_set write_fds;
               util::SocketSet::FdRange socketRange = socketSet.fillFdSet(write_fds);

                fd_set wakeUp_fds;
                util::SocketSet::FdRange wakeupSocketRange = wakeUpSocketSet.fillFdSet(wakeUp_fds);

                int maxFd = (socketRange.max > wakeupSocketRange.max ? socketRange.max : wakeupSocketRange.max);

                errno = 0;
		t.tv_sec = 5;
		t.tv_usec = 0;
                int numSelected = select(maxFd + 1, &wakeUp_fds, &write_fds, NULL, &t);
		 if (numSelected == 0) {
                    return;
                }
                if (numSelected == -1) {
                    if (errno != EINTR) {
                        util::ILogger::getLogger().severe(std::string("Exception OutSelector::listen => ") + strerror(errno));
                    } else{
                        util::ILogger::getLogger().finest(std::string("Exception OutSelector::listen => ") + strerror(errno));
                    }
                    return;
                }
		
                if (FD_ISSET(wakeUpListenerSocketId, &wakeUp_fds)) {
                    int wakeUpSignal;
                    sleepingSocket->receive(&wakeUpSignal, sizeof(int), MSG_WAITALL);
                    --numSelected;
                }

                for (int fd = socketRange.min;numSelected > 0 && fd <= socketRange.max; ++fd) {
                    if (FD_ISSET(fd, &write_fds)) {
                        --numSelected;
                        boost::shared_ptr<Connection> conn = connectionManager.getConnectionIfAvailable(fd);

                        if (conn.get() != NULL) {
                            socketSet.removeSocket(&conn->getSocket());
                            conn->getWriteHandler().handle();
                        }
                    }
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

