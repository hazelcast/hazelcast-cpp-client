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
// Created by sancar koyunlu on 25/12/13.
//

#include <string.h>
#include "hazelcast/client/connection/InSelector.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            InSelector::InSelector(ConnectionManager& connectionManager)
            : IOSelector(connectionManager) {

            }

            bool InSelector::start() {
                return initListenSocket(socketSet);
            }

            void InSelector::listenInternal() {
                fd_set read_fds;
                util::SocketSet::FdRange socketRange = socketSet.fillFdSet(read_fds);
                errno = 0;
                t.tv_sec = 5;
                t.tv_usec = 0;
                int numSelected = select(socketRange.max + 1, &read_fds, NULL, NULL, &t);
                if (numSelected == 0) {
                    return;
                }
                if (numSelected == -1) {
                    if (EINTR == errno || EBADF == errno /* This case may happen if socket closed by cluster listener thread */) {
                        util::ILogger::getLogger().finest(std::string("Exception InSelector::listen => ") + strerror(errno));
                    } else{
                        util::ILogger::getLogger().severe(std::string("Exception InSelector::listen => ") + strerror(errno));
                    }
                    return;
                }
                for (int fd = socketRange.min;numSelected > 0 && fd <= socketRange.max; ++fd) {
                    if (FD_ISSET(fd, &read_fds)) {
                        --numSelected;
                        if (wakeUpListenerSocketId == fd) {
                            int wakeUpSignal;
                            sleepingSocket->receive(&wakeUpSignal, sizeof(int));
                        } else {
                            boost::shared_ptr<Connection> conn = connectionManager.getConnectionIfAvailable(fd);
                            if (conn.get() != NULL) {
                                conn->getReadHandler().handle();
                            }
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
