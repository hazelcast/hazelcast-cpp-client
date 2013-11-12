//
// Created by sancar koyunlu on 25/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#ifndef HAZELCAST_ClientMapIssueTest
#define HAZELCAST_ClientMapIssueTest

#include "iTest.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastInstanceFactory;

            using namespace iTest;

            class ClientMapIssueTest : public iTest::iTestFixture<ClientMapIssueTest> {
            public:
                ClientMapIssueTest(HazelcastInstanceFactory &hazelcastInstanceFactory);

                void testClientDisconnectionWhenQuerying();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

            private:
                HazelcastInstanceFactory &hazelcastInstanceFactory;

            };

        }
    }
}


#endif //HAZELCAST_ClientMapIssueTest
