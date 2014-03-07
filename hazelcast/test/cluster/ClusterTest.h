//
// Created by sancar koyunlu on 26/02/14.
//


#ifndef HAZELCAST_ClusterTest
#define HAZELCAST_ClusterTest


#include "iTest/iTest.h"


namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ClusterTest : public iTest::iTestFixture<ClusterTest> {

            public:

                ClusterTest(HazelcastServerFactory &);

                ~ClusterTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testClusterListeners();

                void testClusterListenersFromConfig();

                void testListenersWhenClusterDown();

                void testBehaviourWhenClusterNotFound();

            private:
                HazelcastServerFactory & hazelcastInstanceFactory;
            };
        }
    }
}


#endif //HAZELCAST_ClusterTest
