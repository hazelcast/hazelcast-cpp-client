//
// Created by sancar koyunlu on 8/27/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientMapTest
#define HAZELCAST_ClientMapTest

#include "iTest.h"
#include "ClientConfig.h"
#include "IMap.h"

namespace hazelcast {
    namespace client {

//        template class IMap<std::string, std::string>;

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ClientMapTest : public iTest::iTestFixture<ClientMapTest> {
            public:
                ClientMapTest(HazelcastInstanceFactory&);

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void fillMap();

                void testGet();

                void testRemoveAndDelete();

                void testRemoveIfSame();

                void flush();

                void clear();

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
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IMap<std::string, std::string> > iMap;
                HazelcastInstanceFactory& hazelcastInstanceFactory;
            };
        }
    }
}

class ClientMapTest {

};


#endif //HAZELCAST_ClientMapTest
