/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "HazelcastServerFactory.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/ISemaphore.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientSemaphoreTest : public ClientTestSupport {
            protected:
                virtual void SetUp() {
                }

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
            };

            HazelcastServer *ClientSemaphoreTest::instance = NULL;
            HazelcastClient *ClientSemaphoreTest::client = NULL;

            void testAcquireThread(util::ThreadArgs& args) {
                ISemaphore *s = (ISemaphore *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                s->acquire();
                latch->countDown();
            }

            TEST_F(ClientSemaphoreTest, testSemaphoreInit) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                ASSERT_TRUE(semaphore.init(10));
            }

            TEST_F(ClientSemaphoreTest, testSemaphoreNegInit) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                ASSERT_THROW(semaphore.init(-1), exception::IllegalArgumentException);
            }

            TEST_F(ClientSemaphoreTest, testRelease) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(0);
                semaphore.release();
                ASSERT_EQ(1, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testdrainPermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                ASSERT_EQ(10, semaphore.drainPermits());
            }

            TEST_F(ClientSemaphoreTest, testAvailablePermits_AfterDrainPermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                semaphore.drainPermits();
                ASSERT_EQ(0, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testTryAcquire_whenDrainPermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                semaphore.drainPermits();
                ASSERT_FALSE(semaphore.tryAcquire());
            }

            TEST_F(ClientSemaphoreTest, testAvailablePermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                ASSERT_EQ(10, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testAvailableReducePermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                semaphore.reducePermits(5);
                ASSERT_EQ(5, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testAvailableReducePermits_WhenZero) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(0);
                semaphore.reducePermits(1);
                ASSERT_EQ(-1, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testAvailableIncreasePermits) {
                ISemaphore semaphore = client->getISemaphore(randomString());
                semaphore.init(10);
                semaphore.drainPermits();
                semaphore.increasePermits(5);
                ASSERT_EQ(5, semaphore.availablePermits());
            }

            TEST_F(ClientSemaphoreTest, testSimpleAcquire) {
                ISemaphore semaphore = client->getISemaphore("testSimpleAcquire");
                int numberOfPermits = 20;
                ASSERT_TRUE(semaphore.init(numberOfPermits));
                for (int i = 0; i < numberOfPermits; i++) {
                    ASSERT_EQ(numberOfPermits - i, semaphore.availablePermits());
                    semaphore.acquire();
                }

                ASSERT_EQ(semaphore.availablePermits(), 0);
            }

            TEST_F(ClientSemaphoreTest, testAcquire) {
                ISemaphore semaphore = client->getISemaphore("testAcquire");
                semaphore.init(10);
                ASSERT_EQ(10, semaphore.drainPermits());

                util::CountDownLatch latch(1);
                util::StartedThread t(testAcquireThread, &semaphore, &latch);

                util::sleep(1);

                semaphore.release(2);
                ASSERT_TRUE(latch.await(10 ));
                ASSERT_EQ(1, semaphore.availablePermits());

            }

            void testTryAcquireThread(util::ThreadArgs& args) {
                ISemaphore *s = (ISemaphore *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                if (s->tryAcquire(2, 5 * 1000)) {
                    latch->countDown();
                }
            }

            TEST_F(ClientSemaphoreTest, testTryAcquire) {
                ISemaphore semaphore = client->getISemaphore("testTryAcquire");
                semaphore.init(10);
                ASSERT_TRUE(semaphore.tryAcquire());
                ASSERT_TRUE(semaphore.tryAcquire(9));
                ASSERT_EQ(0, semaphore.availablePermits());
                ASSERT_FALSE(semaphore.tryAcquire(1 * 1000));
                ASSERT_FALSE(semaphore.tryAcquire(2, 1 * 1000));

                util::CountDownLatch latch(1);

                util::StartedThread t(testTryAcquireThread, &semaphore, &latch);

                semaphore.release(2);
                ASSERT_TRUE(latch.await(10));
                ASSERT_EQ(0, semaphore.availablePermits());
            }
        }
    }
}

