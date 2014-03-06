//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientTxnQueueTest
#define HAZELCAST_ClientTxnQueueTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ClientTxnQueueTest : public iTest::iTestFixture<ClientTxnQueueTest> {

            public:

                ClientTxnQueueTest(HazelcastServerFactory &);

                ~ClientTxnQueueTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testTransactionalOfferPoll1();

                void testTransactionalOfferPoll2();

            private:
                HazelcastServerFactory & hazelcastInstanceFactory;
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
            };

        }
    }
}


#endif //HAZELCAST_ClientTxnQueueTest
