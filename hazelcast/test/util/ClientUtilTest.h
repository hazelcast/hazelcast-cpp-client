//
// Created by sancar koyunlu on 22/08/14.
//


#ifndef HAZELCAST_CLientUtilTest
#define HAZELCAST_CLientUtilTest


#include "iTest/iTest.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class HAZELCAST_API ClientUtilTest : public iTest::iTestFixture<ClientUtilTest> {
            public:

                ClientUtilTest();

                void addTests();

                void beforeClass();

                void afterClass();

                void beforeTest();

                void afterTest();

                void testConditionWaitTimeout();

                void testConditionWakeUpTime();

                void testConditionAcquireLock_afterWakeUp();

                void testConditionAcquireLock_afterTimeout();

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
