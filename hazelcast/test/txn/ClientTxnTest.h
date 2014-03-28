//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientTxnTest
#define HAZELCAST_ClientTxnTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ClientTxnTest : public iTest::iTestFixture<ClientTxnTest> {

            public:

                ClientTxnTest(HazelcastServerFactory &);

                ~ClientTxnTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testTxnWithMultipleNodes();

                void testTxnRollback();

            private:
                HazelcastServerFactory & hazelcastInstanceFactory;
            };

        }
    }
}


#endif //HAZELCAST_ClientTxnTest
