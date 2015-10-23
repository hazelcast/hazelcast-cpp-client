/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

