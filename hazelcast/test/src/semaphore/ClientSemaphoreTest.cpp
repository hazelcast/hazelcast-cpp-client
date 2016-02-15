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
        namespace test {
            ClientSemaphoreTest::ClientSemaphoreTest()
            : instance(*g_srvFactory)
            , client(getNewClient())
            , s(new ISemaphore(client->getISemaphore("ClientSemaphoreTest"))) {
                s->reducePermits(100);
                s->release(10);
            }

            ClientSemaphoreTest::~ClientSemaphoreTest() {
                s->reducePermits(100);
                s->release(10);                
            }

            void testAcquireThread(util::ThreadArgs& args) {
                ISemaphore *s = (ISemaphore *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                s->acquire();
                latch->countDown();
            }

            TEST_F(ClientSemaphoreTest, testAcquire) {
                ASSERT_EQ(10, s->drainPermits());

                util::CountDownLatch latch(1);
                util::Thread t(testAcquireThread, s.get(), &latch);

                util::sleep(1);

                s->release(2);
                ASSERT_TRUE(latch.await(10 ));
                ASSERT_EQ(1, s->availablePermits());

            }

            void testTryAcquireThread(util::ThreadArgs& args) {
                ISemaphore *s = (ISemaphore *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                if (s->tryAcquire(2, 5 * 1000)) {
                    latch->countDown();
                }
            }

            TEST_F(ClientSemaphoreTest, testTryAcquire) {
                ASSERT_TRUE(s->tryAcquire());
                ASSERT_TRUE(s->tryAcquire(9));
                ASSERT_EQ(0, s->availablePermits());
                ASSERT_FALSE(s->tryAcquire(1 * 1000));
                ASSERT_FALSE(s->tryAcquire(2, 1 * 1000));

                util::CountDownLatch latch(1);

                util::Thread t(testTryAcquireThread, s.get(), &latch);

                s->release(2);
                ASSERT_TRUE(latch.await(10));
                ASSERT_EQ(0, s->availablePermits());
            }
        }
    }
}

