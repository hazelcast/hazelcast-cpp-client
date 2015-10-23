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
// Created by sancar koyunlu on 9/13/13.




#ifndef HAZELCAST_ClientListTest
#define HAZELCAST_ClientListTest

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"
#include "hazelcast/client/IList.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientListTest : public ClientTestSupport<ClientListTest> {
            public:
                ClientListTest(HazelcastServerFactory &);

                ~ClientListTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testAddAll();

                void testAddSetRemove();

                void testIndexOf();

                void testToArray();

                void testContains();

                void testRemoveRetainAll();

                void testListener();

            private:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IList<std::string> > list;
            };
        }
    }
}


#endif //HAZELCAST_ClientListTest

