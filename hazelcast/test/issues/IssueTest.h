//
// Created by sancar koyunlu on 21/04/14.
//


#ifndef HAZELCAST_IssueTest
#define HAZELCAST_IssueTest

#include "iTest/iTest.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API IssueTest : public iTest::iTestFixture<IssueTest> {

            public:
                IssueTest(HazelcastServerFactory &);

                ~IssueTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testOperationRedo_smartRoutingDisabled();

            private:
                HazelcastServerFactory& serverFactory;

            };
        }
    }
}


#endif //HAZELCAST_IssueTest
