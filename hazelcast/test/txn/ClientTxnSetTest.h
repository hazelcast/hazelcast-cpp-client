//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientTxnSetTest
#define HAZELCAST_ClientTxnSetTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ClientTxnSetTest : public iTest::iTestFixture<ClientTxnSetTest> {

            public:

                ClientTxnSetTest(HazelcastServerFactory &);

                ~ClientTxnSetTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testAddRemove();

            private:
                HazelcastServerFactory & hazelcastInstanceFactory;
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
            };

        }
    }
}


#endif //HAZELCAST_ClientTxnSetTest
