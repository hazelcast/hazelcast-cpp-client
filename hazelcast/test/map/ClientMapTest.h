//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientMapTest
#define HAZELCAST_ClientMapTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IMap.h"
#include "HazelcastServer.h"

using iTest::iTestFixture;
using std::string;
using std::auto_ptr;

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ClientMapTest : public iTestFixture<ClientMapTest> {
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

                void testMultipleThreadPut();

                void testPredicateListenerWithPortableKey();

                void testListener();

                void testTryLock();

                void testForceUnlock();

                void testLockTtl2();

                void testLockTtl();

                void testLock();

                void testBasicPredicate();

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
                auto_ptr<HazelcastClient> client;
                auto_ptr<IMap<string, string> > imap;
            };
        }
    }
}

#endif //HAZELCAST_ClientMapTest
