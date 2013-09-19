//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientTxnQueueTest
#define HAZELCAST_ClientTxnQueueTest

#include "iTest.h"
#include "ClientConfig.h"
#include "HazelcastInstance.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ClientTxnQueueTest : public iTest::iTestFixture<ClientTxnQueueTest> {

            public:

                ClientTxnQueueTest(HazelcastInstanceFactory&);

                ~ClientTxnQueueTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testTransactionalOfferPoll1();

                void testTransactionalOfferPoll2();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
            };

        }
    }
}


#endif //HAZELCAST_ClientTxnQueueTest
