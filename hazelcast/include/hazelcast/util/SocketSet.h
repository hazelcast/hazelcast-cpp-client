//
// Created by sancar koyunlu on 16/12/13.
//



#ifndef HAZELCAST_SocketSet
#define HAZELCAST_SocketSet

#include "hazelcast/client/Socket.h"
#include "hazelcast/util/Mutex.h"
#include <set>

namespace hazelcast {
    namespace util {
        class SocketSet {
        public:
            static fd_set get_fd_set(std::set<client::Socket const *, client::socketPtrComp> sockets);

            static int getHighestSocketId(std::set<client::Socket const *, client::socketPtrComp> sockets);

            std::set<client::Socket const *, client::socketPtrComp> getSockets();

            void insertSocket(client::Socket const *);

            void removeSocket(client::Socket const *);

        private:
            std::set<client::Socket const *, client::socketPtrComp> sockets;
            util::Mutex accessLock;

        };

    }
}


#endif //HAZELCAST_SocketSet

