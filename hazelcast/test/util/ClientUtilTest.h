//
// Created by sancar koyunlu on 22/08/14.
//


#ifndef HAZELCAST_CLientUtilTest
#define HAZELCAST_CLientUtilTest


#include "ClientTestSupport.h"
#include "ClientTestSupport.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientUtilTest : public ClientTestSupport<ClientUtilTest> {
            public:

                ClientUtilTest(int dummy);

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testConditionWaitTimeout();

                void testConditionWakeUpTime();

                void testFutureWaitTimeout();

                void testFutureSetValue();

                void testFutureSetException();

                void testFutureSetValue_afterSomeTime();

                void testFutureSetException_afterSomeTime();

                void testThreadName();

                void testThreadInterruptibleSleep();

            };

        }
    }
}


#endif //HAZELCAST_CLientUtilTest
