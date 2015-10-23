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
// Created by sancar koyunlu on 8/27/13.




#ifndef HAZELCAST_ClientMapTest
#define HAZELCAST_ClientMapTest

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IMap.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientMapTest : public ClientTestSupport<ClientMapTest> {
            public:
                ClientMapTest(HazelcastServerFactory &);

                ~ClientMapTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void fillMap();

                void testGet();

                void testRemoveAndDelete();

                void testRemoveIfSame();

                void testContains();

                void testReplace();

                void testPutTtl();

                void testTryPutRemove();

                void testIssue537();

                void testEvictAllEvent();

                void testClearEvent();

                void testPredicateListenerWithPortableKey();

                void testListener();

                void testTryLock();

                void testForceUnlock();

                void testLockTtl2();

                void testLockTtl();

                void testLock();

                void testBasicPredicate();

                void testKeySetAndValuesWithPredicates();

                void testSet();

                void testValues();

                void testGetAllPutAll();

                void testPutIfAbsent();

                void testPutIfAbsentTtl();

                void testMapWithPortable();

                void testMapStoreRelatedRequests();

            private:
                HazelcastServer instance;
                HazelcastServer instance2;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IMap<std::string, std::string> > imap;
            };
        }
    }
}

#endif //HAZELCAST_ClientMapTest

