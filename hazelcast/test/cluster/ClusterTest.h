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

            class HazelcastInstanceFactory;

            class HAZELCAST_API ClusterTest : public iTest::iTestFixture<ClusterTest> {

            public:

                ClusterTest(HazelcastInstanceFactory&);

                ~ClusterTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testClusterListeners();

                void testClusterListenersFromConfig();

            private:
                HazelcastInstanceFactory& hazelcastInstanceFactory;
            };
        }
    }
}


#endif //HAZELCAST_ClusterTest
