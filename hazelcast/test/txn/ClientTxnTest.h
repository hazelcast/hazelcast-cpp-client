//
// Created by sancar koyunlu on 9/18/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientTxnTest
#define HAZELCAST_ClientTxnTest

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientTxnTest : public ClientTestSupport<ClientTxnTest> {

            public:

                ClientTxnTest(HazelcastServerFactory &);

                ~ClientTxnTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testTxnRollback();

                void testTxnRollbackOnServerCrash();

            private:
                HazelcastServerFactory & hazelcastInstanceFactory;
                std::auto_ptr<HazelcastServer> server;
                std::auto_ptr<HazelcastServer> second;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<LoadBalancer> loadBalancer;
            };

        }
    }
}


#endif //HAZELCAST_ClientTxnTest

