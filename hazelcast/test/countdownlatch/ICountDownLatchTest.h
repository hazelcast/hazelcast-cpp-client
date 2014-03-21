//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ICountDownLatch
#define HAZELCAST_ICountDownLatch

#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastServer.h"
#include "hazelcast/client/ICountDownLatch.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ICountDownLatchTest : public iTest::iTestFixture<ICountDownLatchTest> {
            public:

                ICountDownLatchTest(HazelcastServerFactory &);

                ~ICountDownLatchTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testLatch();

            private:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<ICountDownLatch > l;
            };
        }
    }
}

#endif //HAZELCAST_ICountDownLatch
