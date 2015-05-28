//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientSemaphoreTest
#define HAZELCAST_ClientSemaphoreTest

#include "ClientTestSupport.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/ISemaphore.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientSemaphoreTest : public ClientTestSupport<ClientSemaphoreTest> {
            public:

                ClientSemaphoreTest(HazelcastServerFactory &);

                ~ClientSemaphoreTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testAcquire();

                void testTryAcquire();

            private:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<ISemaphore> s;
            };

        }
    }
}


#endif //HAZELCAST_ClientSemaphoreTest

