//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientTxnMapTest
#define HAZELCAST_ClientTxnMapTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastInstance.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ClientTxnMapTest : public iTest::iTestFixture<ClientTxnMapTest> {

            public:

                ClientTxnMapTest(HazelcastInstanceFactory&);

                ~ClientTxnMapTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testPutGet();

                void testKeySetAndValuesWithPredicates();

                void testKeySetValues();

                void testExecuteTxn();

                void testExecuteTxnWithException();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
            };

        }
    }
}

#endif //HAZELCAST_CLientTxnMapTest
