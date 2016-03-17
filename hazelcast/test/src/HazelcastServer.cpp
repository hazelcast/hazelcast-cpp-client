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
//  hazelcastInstance.cpp
//  hazelcast
//
//  Created by Sancar on 14.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"
#include <iostream>

namespace hazelcast {
    namespace client {
        namespace test {
            HazelcastServer::HazelcastServer(HazelcastServerFactory& factory)
            :factory(factory)
            , id(factory.getInstanceId(DEFAULT_RETRY_COUNT))
            , isShutDown(false) {
            }

            bool HazelcastServer::start() {
                bool result = false;

                if (isShutDown) {
                    id = factory.getInstanceId();
                    isShutDown = false;
                    result = true;
                }

                return result;
            }

            bool HazelcastServer::shutdown() {
                bool result = false;
                try{
                    if (!isShutDown) {
                        factory.shutdownInstance(id);
                        isShutDown = true;
                        result = true;
                    }
                }catch(std::exception& e){
                    isShutDown = true;
                    std::cerr << e.what() << std::endl;
                }
                return result;
            }

            HazelcastServer::~HazelcastServer() {
                shutdown();
            }

        }
    }
}

