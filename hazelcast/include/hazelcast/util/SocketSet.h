//
// Created by sancar koyunlu on 16/12/13.
//



#ifndef HAZELCAST_SocketSet
#define HAZELCAST_SocketSet

#include "hazelcast/client/Socket.h"
#include <set>

namespace hazelcast {
    namespace util {
        class SocketSet {
        public:
            fd_set get_fd_set() const;

            int getHighestSocketId() const;

            std::set<client::Socket const *> sockets;
        };

    }
}


#endif //HAZELCAST_SocketSet
