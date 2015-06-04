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
            struct FdRange {
                int min;
                int max;
            };

            /**
             * @return Returns the maximum file descriptor id in the existing set, returns 0 if no fd in the set.
             */
            FdRange fillFdSet(fd_set &resultSet);

            void insertSocket(client::Socket const *);

            void removeSocket(client::Socket const *);
        private:
            typedef std::set<client::Socket const *, client::socketPtrComp> SocketContainer;
            SocketContainer sockets;
            util::Mutex accessLock;
        };

    }
}


#endif //HAZELCAST_SocketSet

