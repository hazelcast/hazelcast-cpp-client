//
// Created by sancar koyunlu on 16/12/13.
//

#include "hazelcast/util/SocketSet.h"

namespace hazelcast {
    namespace util {

        fd_set SocketSet::get_fd_set(std::set<client::Socket const *, client::socketPtrComp> sockets) {
            fd_set fdSet;
            FD_ZERO(&fdSet);
            std::set<client::Socket const *, client::socketPtrComp>::iterator it;
            for (it = sockets.begin(); it != sockets.end(); it++) {
                FD_SET((*it)->getSocketId(), &fdSet);
            }
            return fdSet;
        }

        int SocketSet::getHighestSocketId(std::set<client::Socket const *, client::socketPtrComp> sockets) {
            if (!sockets.empty())
                return (*(sockets.begin()))->getSocketId();
            else
                return 0;
        }


        std::set<client::Socket const *, client::socketPtrComp> SocketSet::getSockets() {
            LockGuard lockGuard(accessLock);
            return sockets;
        }

        void SocketSet::insertSocket(client::Socket const *socket) {
            LockGuard lockGuard(accessLock);
            sockets.insert(socket);
        }

        void SocketSet::removeSocket(client::Socket const *socket) {
            LockGuard lockGuard(accessLock);
            sockets.erase(socket);
        }
    }
}
