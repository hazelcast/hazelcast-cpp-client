//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CountDownLatch
#define HAZELCAST_CountDownLatch

#include "iTest.h"
#include "ClientConfig.h"
#include "HazelcastInstance.h"
#include "ICountDownLatch.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class ICountDownLatchTest : public iTest::iTestFixture<ICountDownLatchTest> {
            public:

                ICountDownLatchTest(HazelcastInstanceFactory&);

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testLatch();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<ICountDownLatch > l;
            };
        }
    }
}

#endif //HAZELCAST_CountDownLatch
