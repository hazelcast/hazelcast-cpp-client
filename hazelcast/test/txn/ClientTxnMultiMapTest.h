//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientTxnMultiMApTest
#define HAZELCAST_ClientTxnMultiMApTest

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientTxnMultiMapTest : public ClientTestSupport<ClientTxnMultiMapTest> {

            public:

                ClientTxnMultiMapTest(HazelcastServerFactory &);

                ~ClientTxnMultiMapTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testPutGetRemove();

            private:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
            };

        }
    }
}

#endif //HAZELCAST_ClientTxnMultiMApTest

