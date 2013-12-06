//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientQueueTest
#define HAZELCAST_ClientQueueTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IQueue.h"
#include "HazelcastInstance.h"

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class HAZELCAST_API ClientQueueTest : public iTest::iTestFixture<ClientQueueTest> {
            public:
                ClientQueueTest(HazelcastInstanceFactory&);

                ~ClientQueueTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testListener();

                void testOfferPoll();

                void testRemainingCapacity();

                void testRemove();

                void testContains();

                void testDrain();

                void testToArray();

                void testAddAll();

                void testRemoveRetain();

                void testClear();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IQueue< std::string> > q;
            };
        }
    }
}


#endif //HAZELCAST_ClientQueueTest
