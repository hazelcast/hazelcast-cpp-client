/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
//  hazelcastInstance.cpp
//  hazelcast
//
//  Created by Sancar on 14.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#include "HazelcastServer.h"

#include <hazelcast/client/exception/IllegalStateException.h>

namespace hazelcast {
    namespace client {
        namespace test {
            HazelcastServer::HazelcastServer(HazelcastServerFactory &factory) : factory(factory), isStarted(false),
                                                                                logger(new util::ILogger(
                                                                                        "HazelcastServer",
                                                                                        "HazelcastServer",
                                                                                        "testversion",
                                                                                        config::LoggerConfig())) {
                start();
            }

            bool HazelcastServer::start() {
                if (isStarted) {
                    return true;
                }

                try {
                    member = factory.startServer();
                    isStarted = true;
                    return true;
                } catch (exception::IllegalStateException &illegalStateException) {
                    std::ostringstream out;
                    out << "Could not start new member!!! " << illegalStateException.what();
                    logger->severe(out.str());
                    return false;
                }
            }

            bool HazelcastServer::shutdown() {
                if (!isStarted) {
                    return true;
                }

                if (!factory.shutdownServer(member)) {
                    return false;
                }

                isStarted = false;
                return true;
            }

            bool HazelcastServer::terminate() {
                if (!isStarted) {
                    return true;
                }

                if (!factory.terminateServer(member)) {
                    return false;
                }

                isStarted = false;
                return true;
            }

            HazelcastServer::~HazelcastServer() {
                shutdown();
            }

            bool HazelcastServer::setAttributes(int memberStartOrder) {
                if (!isStarted) {
                    return false;
                }
                return factory.setAttributes(memberStartOrder);
            }

            const Member &HazelcastServer::getMember() const {
                return member;
            }

        }
    }
}

