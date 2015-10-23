/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 22/08/14.
//


#ifndef HAZELCAST_CLientUtilTest
#define HAZELCAST_CLientUtilTest


#include "ClientTestSupport.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {

            class HazelcastServerFactory;

            class ClientUtilTest : public ClientTestSupport<ClientUtilTest> {
            public:

                ClientUtilTest();

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
