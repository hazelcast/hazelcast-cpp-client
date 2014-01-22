//
// Created by sancar koyunlu on 9/6/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ExecuterServiceTest
#define HAZELCAST_ExecuterServiceTest


#include "iTest/iTest.h"
#include "hazelcast/client/ClientConfig.h"
#include "HazelcastInstance.h"
#include "hazelcast/client/IExecutorService.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            class IExecutorServiceTest : public iTest::iTestFixture<IExecutorServiceTest> {
            public:

                IExecutorServiceTest(HazelcastInstanceFactory&);

                ~IExecutorServiceTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testSubmitWithResult();

                void submitCallable1();

                void submitCallable2();

                void submitCallable3();

                void submitCallable4();

                void submitFailingCallable();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
                HazelcastInstance instance;
                HazelcastInstance second;
                HazelcastInstance third;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IExecutorService > service;
            };
        }
    }
}


#endif //HAZELCAST_ExecuterServiceTest
