/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by İhsan Demir on Mar 6 2016.
//
#include <gtest/gtest.h>
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class ConcurentQueueTest : public ::testing::Test
                {
                protected:
                    static void ConcurrentQueueTask(hazelcast::util::ThreadArgs &args) {
                        hazelcast::util::ConcurrentQueue<int> *q = (hazelcast::util::ConcurrentQueue<int> *)args.arg0;
                        hazelcast::util::CountDownLatch *startLatch = (hazelcast::util::CountDownLatch *)args.arg1;
                        hazelcast::util::CountDownLatch *startRemoveLatch = (hazelcast::util::CountDownLatch *)args.arg2;
                        int *removalValue = (int *)args.arg3;

                        int numItems = 1000;

                        std::vector<int> values((size_t)numItems);

                        startLatch->countDown();

                        ASSERT_TRUE(startLatch->await(10));

                        // insert items
                        for (int i = 0; i < numItems; ++i) {
                            values[i] = i;
                            q->offer(&values[i]);
                        }

                        q->offer(removalValue);
                        startRemoveLatch->countDown();

                        // poll items
                        for (int i = 0; i < numItems; ++i) {
                            values[i] = i;
                            ASSERT_NE((int *)NULL, q->poll());
                        }
                    }
                };

                TEST_F(ConcurentQueueTest, testSingleThread) {
                    hazelcast::util::ConcurrentQueue<int> q;

                    ASSERT_EQ((int *)NULL, q.poll());

                    int val1, val2;

                    q.offer(&val1);

                    ASSERT_EQ(&val1, q.poll());

                    ASSERT_EQ((int *)NULL, q.poll());

                    q.offer(&val1);
                    q.offer(&val2);
                    q.offer(&val2);
                    q.offer(&val1);

                    ASSERT_EQ(2, q.removeAll(&val2));
                    ASSERT_EQ(0, q.removeAll(&val2));

                    ASSERT_EQ(&val1, q.poll());
                    ASSERT_EQ(&val1, q.poll());

                    ASSERT_EQ((int *)NULL, q.poll());
                }

                TEST_F(ConcurentQueueTest, testMultiThread) {
                    int numThreads = 40;

                    hazelcast::util::CountDownLatch startLatch(numThreads);

                    hazelcast::util::CountDownLatch startRemoveLatch(numThreads);

                    hazelcast::util::ConcurrentQueue<int> q;

                    int removalValue = 10;

                    std::vector<hazelcast::util::Thread *> threads((size_t)numThreads);
                    for (int i = 0; i < numThreads; ++i) {
                        // I would prefer using scoped_ptr or boost:::scoped_array for array if there was one available
                        threads[i] = new hazelcast::util::Thread(ConcurentQueueTest::ConcurrentQueueTask, &q, &startLatch, &startRemoveLatch, &removalValue);
                    }

                    // wait for the remove start
                    ASSERT_TRUE(startRemoveLatch.await(30));

                    int numRemoved = q.removeAll(&removalValue);

                    int numRemaining = numThreads - numRemoved;

                    char msg[200];
                    hazelcast::util::snprintf(msg, 200, "Was able to remove %d items and left %d items", (numThreads - numRemaining), numRemaining);
                    hazelcast::util::ILogger::getLogger().info(msg);

                    for (int j = 0; j < numRemaining; ++j) {
                        ASSERT_NE((int *)NULL, q.poll());
                    }
                    ASSERT_EQ(0, q.removeAll(&removalValue));

                    for (int i = 0; i < numThreads; ++i) {
                        ASSERT_TRUE(threads[i]->join());
                    }

                    for (int i = 0; i < numThreads; ++i) {
                        delete threads[i];
                    }
                }
            }
        }
    }
}
