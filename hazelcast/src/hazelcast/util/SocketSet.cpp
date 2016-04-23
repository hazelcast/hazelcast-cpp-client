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

#include <assert.h>
#include <iosfwd>
#include <string.h>

#include "hazelcast/util/Util.h"
#include "hazelcast/util/SocketSet.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {
        void SocketSet::insertSocket(client::Socket const *socket) {
            assert(NULL != socket);

            int socketId = socket->getSocketId();
            assert(socketId >= 0);

            if (socketId >= 0) {
                LockGuard lockGuard(accessLock);
                sockets.insert(socketId);
            } else {
                char msg[200];
                util::snprintf(msg, 200, "[SocketSet::insertSocket] Socket id:%d, Should be 0 or greater than 0.",
                               socketId);
                util::ILogger::getLogger().warning(msg);
            }
        }

        void SocketSet::removeSocket(client::Socket const *socket) {
            assert(NULL != socket);

            int socketId = socket->getSocketId();
            assert(socketId >= 0);

            bool found = false;

            if (socketId >= 0) {
                LockGuard lockGuard(accessLock);

                for (std::set<int>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
                    if (socketId == *it) { // found
                        sockets.erase(it);
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                char msg[200];
                util::snprintf(msg, 200,
                               "[SocketSet::removeSocket] Socket with id %d  was not found among the sockets.",
                               socketId);
                util::ILogger::getLogger().finest(msg);
            }
        }

        SocketSet::FdRange SocketSet::fillFdSet(fd_set &resultSet) {
            FdRange result;
            memset(&result, 0, sizeof(FdRange));

            FD_ZERO(&resultSet);

            LockGuard lockGuard(accessLock);

            if (!sockets.empty()) {
                for (std::set<int>::const_iterator it = sockets.begin(); it != sockets.end(); it++) {
                    FD_SET(*it, &resultSet);
                }

                result.max = *sockets.rbegin();
                result.min = *sockets.begin();
            }

            return result;
        }
    }
}
