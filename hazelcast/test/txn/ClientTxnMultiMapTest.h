//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientTxnMultiMApTest
#define HAZELCAST_ClientTxnMultiMApTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastInstance.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class HAZELCAST_API ClientTxnMultiMapTest : public iTest::iTestFixture<ClientTxnMultiMapTest> {

            public:

                ClientTxnMultiMapTest(HazelcastInstanceFactory&);

                ~ClientTxnMultiMapTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testPutGetRemove();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
            };

        }
    }
}

#endif //HAZELCAST_ClientTxnMultiMApTest
