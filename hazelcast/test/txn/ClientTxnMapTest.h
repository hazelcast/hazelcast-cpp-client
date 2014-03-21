//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientTxnMapTest
#define HAZELCAST_ClientTxnMapTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ClientTxnMapTest : public iTest::iTestFixture<ClientTxnMapTest> {
            public:

                ClientTxnMapTest(HazelcastServerFactory &);

                ~ClientTxnMapTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testPutGet();

                void testKeySetAndValuesWithPredicates();

                void testKeySetValues();

            private:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
            };

        }
    }
}

#endif //HAZELCAST_CLientTxnMapTest
