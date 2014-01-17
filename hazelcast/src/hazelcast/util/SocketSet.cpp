//
// Created by sancar koyunlu on 16/12/13.
//

#include "hazelcast/util/SocketSet.h"

namespace hazelcast {
    namespace util {

        fd_set SocketSet::get_fd_set() const {
            fd_set fdSet;
            FD_ZERO(&fdSet);
            std::set<client::Socket const *>::iterator it;
            for (it = sockets.begin(); it != sockets.end(); it++) {
                FD_SET((*it)->getSocketId(), &fdSet);
            }
            return fdSet;
        }

        int SocketSet::getHighestSocketId() const {
            if (!sockets.empty())
                return (*(sockets.begin()))->getSocketId();
            else
                return 0;
        }
    }
}