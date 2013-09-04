//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ClientQueueTest
#define HAZELCAST_ClientQueueTest

#include "iTest.h"
#include "ClientConfig.h"
#include "IQueue.h"
#include "HazelcastInstance.h"

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ClientQueueTest : public iTest::iTestFixture<ClientQueueTest> {
            public:
                ClientQueueTest(HazelcastInstanceFactory&);

                ~ClientQueueTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testAddAll();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IQueue< std::string> > q;
            };
        }
    }
}


#endif //HAZELCAST_ClientQueueTest
