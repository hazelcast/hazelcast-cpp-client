//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientSemaphoreTest
#define HAZELCAST_ClientSemaphoreTest

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastInstance.h"
#include "ISemaphore.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ClientSemaphoreTest : public iTest::iTestFixture<ClientSemaphoreTest> {

            public:

                ClientSemaphoreTest(HazelcastInstanceFactory&);

                ~ClientSemaphoreTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testAcquire();

                void testTryAcquire();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<ISemaphore> s;
            };

        }
    }
}


#endif //HAZELCAST_ClientSemaphoreTest
