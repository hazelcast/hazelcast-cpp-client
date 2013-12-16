//
// Created by sancar koyunlu on 16/12/13.
//



#ifndef HAZELCAST_SocketSet
#define HAZELCAST_SocketSet

#include "Socket.h"
#include <set>

namespace hazelcast {
    namespace util {

        struct socketCompare {
            bool operator ()(const client::Socket &lhs, const client::Socket &rhs) const {
                return lhs.getSocketId() < rhs.getSocketId();
            }
        };

        class SocketSet {
        public:
            fd_set get_fd_set() const;

            int getHighestSocketId() const;

            std::set<client::Socket> sockets;
        };

    }
}


#endif //HAZELCAST_SocketSet
