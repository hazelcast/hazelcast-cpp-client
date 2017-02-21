/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 8/26/13.

#include <iostream>
#include <string.h>

#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            HazelcastServerFactory::HazelcastServerFactory(const char *hostAddress)
                    : address(hostAddress, 6543), socket(new internal::socket::TcpSocket(address)), outputSocketStream(*socket),
                      inputSocketStream(*socket), logger(util::ILogger::getLogger()) {
                if (int error = socket->connect(5000)) {
                    char msg[200];
                    util::snprintf(msg, 200,
                                   "[HazelcastServerFactory] Could not connect to socket %s:6543. Errno:%d, %s",
                                   hostAddress, error, strerror(error));
                    logger.severe(msg);
                }
            }

            HazelcastServerFactory::~HazelcastServerFactory() {
                try {
                    outputSocketStream.writeInt(END);
                    inputSocketStream.readInt();
                } catch (std::exception &e) {
                    char msg[200];
                    util::snprintf(msg, 200, "[HazelcastServerFactory] ~HazelcastServerFactory() exception:%s", e.what());
                    logger.severe(msg);
                }
            }

            void HazelcastServerFactory::shutdownInstance(int id) {
                outputSocketStream.writeInt(SHUTDOWN);
                outputSocketStream.writeInt(id);
                int i = inputSocketStream.readInt();
                if (i != OK) {
                    char msg[200];
                    util::snprintf(msg, 200, "[HazelcastServerFactory] shutdownInstance(int id): %d", i);
                    logger.info(msg);
                }
            }

            void HazelcastServerFactory::shutdownAll() {
                outputSocketStream.writeInt(SHUTDOWN_ALL);
                try {
                    int i = inputSocketStream.readInt();
                    if (i != OK) {
                        char msg[200];
                        util::snprintf(msg, 200, "[HazelcastServerFactory] shutdownAll(): %d", i);
                        logger.info(msg);
                    }
                } catch (std::exception &e) {
                    char msg[200];
                    util::snprintf(msg, 200, "[HazelcastServerFactory] shutdownAll exception:%s", e.what());
                    logger.severe(msg);
                }

            }

            int HazelcastServerFactory::getInstanceId(int retryNumber) {
                outputSocketStream.writeInt(START);
                int id = inputSocketStream.readInt();
                if (FAIL == id) {
                    char msg[200];
                    util::snprintf(msg, 200, "[HazelcastServerFactory::getInstanceId] Failed to start server");
                    logger.warning(msg);

                    while (id == FAIL && retryNumber > 0) {
                        util::snprintf(msg, 200, "[HazelcastServerFactory::getInstanceId] Retrying to start server. Retry number:%d", retryNumber);
                        logger.warning(msg);
                        outputSocketStream.writeInt(START);
                        id = inputSocketStream.readInt();
                        --retryNumber;
                    }
                }

                return id;
            }


            const std::string &HazelcastServerFactory::getServerAddress() const {
                return address.getHost();
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
