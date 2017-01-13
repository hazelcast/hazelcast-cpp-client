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
// Created by sancar koyunlu on 9/5/13.




#ifndef HAZELCAST_ICountDownLatch
#define HAZELCAST_ICountDownLatch

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"
#include "hazelcast/client/ICountDownLatch.h"

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ICountDownLatchTest : public ClientTestSupport {
            public:
                ICountDownLatchTest();

                ~ICountDownLatchTest();
            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<ICountDownLatch> l;
            };
        }
    }
}

#endif //HAZELCAST_ICountDownLatch

