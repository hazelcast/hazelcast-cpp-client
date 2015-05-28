//
// Created by sancar koyunlu on 21/04/14.
//


#ifndef HAZELCAST_IssueTest
#define HAZELCAST_IssueTest

#include "ClientTestSupport.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class IssueTest : public ClientTestSupport<IssueTest> {

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
