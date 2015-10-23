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
// Created by sancar koyunlu on 9/5/13.



#include "countdownlatch/ICountDownLatchTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ICountDownLatchTest::ICountDownLatchTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<ICountDownLatchTest>("ICountDownLatchTest" , &serverFactory)
            , instance(serverFactory)
            , client(getNewClient())
            , l(new ICountDownLatch(client->getICountDownLatch("ICountDownLatchTest"))) {
            }


            ICountDownLatchTest::~ICountDownLatchTest() {
            }

            void ICountDownLatchTest::addTests() {
                addTest(&ICountDownLatchTest::testLatch, "ICountDownLatchTest");
            }

            void ICountDownLatchTest::beforeClass() {

            }

            void ICountDownLatchTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ICountDownLatchTest::beforeTest() {

            }

            void ICountDownLatchTest::afterTest() {
            }

            void testLatchThread(util::ThreadArgs &args) {
                ICountDownLatch *l = (ICountDownLatch *) args.arg0;
                for (int i = 0; i < 20; i++) {
                    l->countDown();
                }
            }

            void ICountDownLatchTest::testLatch() {
                assertTrue(l->trySetCount(20));
                assertFalse(l->trySetCount(10));
                assertEqual(20, l->getCount());

                util::Thread t(testLatchThread, l.get());

                assertTrue(l->await(10 * 1000));
            }

        }
    }
}


