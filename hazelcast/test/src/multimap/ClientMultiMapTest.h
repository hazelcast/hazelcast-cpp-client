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
//  ClientMultiMapTest.h
//  hazelcast
//
//  Created by Sancar on 16.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef hazelcast_ClientMultiMapTest_h
#define hazelcast_ClientMultiMapTest_h

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/MultiMap.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {
            class ClientMultiMapTest : public ClientTestSupport {
            public:
                ClientMultiMapTest();

                ~ClientMultiMapTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<MultiMap<std::string, std::string> > mm;
            };
        }
    }
}

#endif

