//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientMapTest
#define HAZELCAST_ClientMapTest

#include "iTest.h"
#include "ClientConfig.h"
#include "IMap.h"
#include "HazelcastInstance.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ClientMapTest : public iTest::iTestFixture<ClientMapTest> {
            public:
                ClientMapTest(HazelcastInstanceFactory&);

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

                void testAsyncPutWithTtl();

                void testIssue537();

                void testListener();

                void testTryLock();

                void testForceUnlock();

                void testLockTtl2();

                void testLockTtl();

                void testLock();

                void testBasicPredicate();

                void testPutTransient();

                void testSet();

                void testAsyncGet();

                void testAsyncPut();

                void testAsyncRemove();

                void testValues();

                void testGetAllPutAll();

                void testPutIfAbsent();

                void testPutIfAbsentTtl();

                void destroy();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IMap<std::string, std::string> > iMap;
            };
        }
    }
}

#endif //HAZELCAST_ClientMapTest
