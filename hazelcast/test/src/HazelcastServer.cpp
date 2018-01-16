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
//  hazelcastInstance.cpp
//  hazelcast
//
//  Created by Sancar on 14.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"
#include <sstream>

#include <hazelcast/util/ILogger.h>

namespace hazelcast {
    namespace client {
        namespace test {
            HazelcastServer::HazelcastServer(HazelcastServerFactory& factory)
            :factory(factory) {
                start();
            }

            bool HazelcastServer::start() {
                try {
                    factory.startServer(member);
                    factory.setAttributes(member);
                    return true;
                } catch (TException &tx) {
                    std::ostringstream out;
                    out << "Could not start new member!!! " << tx.what();
                    util::ILogger::getLogger().severe(out.str());
                    return false;
                }
            }

            bool HazelcastServer::shutdown() {
                try {
                    factory.shutdownServer(member);
                    return true;
                } catch (TException &tx) {
                    std::ostringstream out;
                    out << "Could not shutdown member " << member.uuid <<  " !!! " << tx.what();
                    util::ILogger::getLogger().severe(out.str());
                    return false;
                }
            }

            HazelcastServer::~HazelcastServer() {
                shutdown();
            }

        }
    }
}

