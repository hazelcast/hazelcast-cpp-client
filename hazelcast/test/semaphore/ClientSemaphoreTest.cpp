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



#include "hazelcast/util/Util.h"
#include "ClientSemaphoreTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Thread.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientSemaphoreTest::ClientSemaphoreTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<ClientSemaphoreTest>("ClientSemaphoreTest", &serverFactory)
            , instance(serverFactory)
            , client(getNewClient())
            , s(new ISemaphore(client->getISemaphore("ClientSemaphoreTest"))) {
            }


            ClientSemaphoreTest::~ClientSemaphoreTest() {
            }

            void ClientSemaphoreTest::addTests() {
                addTest(&ClientSemaphoreTest::testAcquire, "testAcquire");
                addTest(&ClientSemaphoreTest::testTryAcquire, "testTryAcquire");
            }

            void ClientSemaphoreTest::beforeClass() {
            }

            void ClientSemaphoreTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ClientSemaphoreTest::beforeTest() {
                s->reducePermits(100);
                s->release(10);
            }

            void ClientSemaphoreTest::afterTest() {
                s->reducePermits(100);
                s->release(10);
            }

            void testAcquireThread(util::ThreadArgs& args) {
                ISemaphore *s = (ISemaphore *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                s->acquire();
                latch->countDown();
            }

            void ClientSemaphoreTest::testAcquire() {
                assertEqual(10, s->drainPermits());

                util::CountDownLatch latch(1);
                util::Thread t(testAcquireThread, s.get(), &latch);

                util::sleep(1);

                s->release(2);
                assertTrue(latch.await(10 ));
                assertEqual(1, s->availablePermits());

            }

            void testTryAcquireThread(util::ThreadArgs& args) {
                ISemaphore *s = (ISemaphore *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                if (s->tryAcquire(2, 5 * 1000)) {
                    latch->countDown();
                }
            }

            void ClientSemaphoreTest::testTryAcquire() {
                assertTrue(s->tryAcquire());
                assertTrue(s->tryAcquire(9));
                assertEqual(0, s->availablePermits());
                assertFalse(s->tryAcquire(1 * 1000));
                assertFalse(s->tryAcquire(2, 1 * 1000));

                util::CountDownLatch latch(1);

                util::Thread t(testTryAcquireThread, s.get(), &latch);

                s->release(2);
                assertTrue(latch.await(10 ));
                assertEqual(0, s->availablePermits());

            }
        }
    }
}

