//
// Created by sancar koyunlu on 27/02/14.
//


#ifndef HAZELCAST_MemberAttributeTest
#define HAZELCAST_MemberAttributeTest

#include "ClientTestSupport.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class MemberAttributeTest : public ClientTestSupport<MemberAttributeTest> {

            public:

            MemberAttributeTest(HazelcastServerFactory &);

            ~MemberAttributeTest();

            void addTests();

            void beforeClass();

            void afterClass();

            void beforeTest();

            void afterTest();

            void testInitialValues();

            void testChangeWithListeners();

            private:
            HazelcastServerFactory & hazelcastInstanceFactory;
        };
    }
}
}



#endif //HAZELCAST_MemberAttributeTest

