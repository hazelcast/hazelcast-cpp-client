//
// Created by sancar koyunlu on 16/12/13.
//

#include <assert.h>
#include <iosfwd>
#include <string.h>

#include "hazelcast/util/SocketSet.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {
        void SocketSet::insertSocket(client::Socket const *socket) {
            assert(NULL != socket);

            LockGuard lockGuard(accessLock);
            std::pair<SocketContainer::iterator, bool> result = sockets.insert(socket);
            if (!result.second) {
                ILogger &logger = util::ILogger::getLogger();
                std::ostringstream out;
                out << "[SocketSet::insertSocket] Trying to add an already added socket with id " <<
                socket->getSocketId() << ".";
                logger.warning(out.str());
            }
        }

        void SocketSet::removeSocket(client::Socket const *socket) {
            assert(NULL != socket);

            int searchedSocketId = socket->getSocketId();
            bool found = false;

            LockGuard lockGuard(accessLock);

            for (SocketContainer::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
                if (searchedSocketId ==  (*it)->getSocketId()) { // found
                    sockets.erase(it);
                    found = true;
                    break;
                }
            }
            if (!found) {
                ILogger &logger = util::ILogger::getLogger();
                std::ostringstream out;
                out << "[SocketSet::removeSocket] Trying to remove an already removed socket with id " << searchedSocketId << ".";
                logger.warning(out.str());
            }
        }

        SocketSet::FdRange SocketSet::fillFdSet(fd_set &resultSet) {
            FdRange result;
            memset(&result, 0, sizeof(FdRange));

            FD_ZERO(&resultSet);

            LockGuard lockGuard(accessLock);

            if (!sockets.empty()) {
                for (SocketContainer::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
                    int socketId = (*it)->getSocketId();
                    FD_SET(socketId, &resultSet);
                }

                result.max = (*(sockets.begin()))->getSocketId();
                result.min = (*(sockets.rbegin()))->getSocketId();
            }

            return result;
        }
    }
}
