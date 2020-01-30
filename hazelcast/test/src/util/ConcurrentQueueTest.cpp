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
//
// Created by İhsan Demir on Mar 6 2016.
//

#include <memory>

#include <ClientTestSupport.h>
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class ConcurentQueueTest : public ClientTestSupport
                {
                protected:
                    class ConcurrentQueueTask : public hazelcast::util::Runnable {
                    public:
                        ConcurrentQueueTask(hazelcast::util::ConcurrentQueue<int> &q,
                                            hazelcast::util::CountDownLatch &startLatch,
                                            hazelcast::util::CountDownLatch &startRemoveLatch, int removalValue) : q(q),
                                                                                                                   startLatch(
                                                                                                                           startLatch),
                                                                                                                   startRemoveLatch(
                                                                                                                           startRemoveLatch),
                                                                                                                   removalValue(
                                                                                                                           removalValue) {}

                    private:
                        virtual void run() {
                            int numItems = 1000;

                            std::vector<int> values((size_t)numItems);

                            startLatch.countDown();

                            ASSERT_TRUE(startLatch.await(10));

                            // insert items
                            for (int i = 0; i < numItems; ++i) {
                                values[i] = i;
                                q.offer(&values[i]);
                            }

                            q.offer(&removalValue);
                            startRemoveLatch.countDown();

                            // poll items
                            for (int i = 0; i < numItems; ++i) {
                                values[i] = i;
                                ASSERT_NE((int *)NULL, q.poll());
                            }
                        }

                        virtual const std::string getName() const {
                            return "ConcurrentQueueTask";
                        }

                    private:
                        hazelcast::util::ConcurrentQueue<int> &q;
                        hazelcast::util::CountDownLatch &startLatch;
                        hazelcast::util::CountDownLatch &startRemoveLatch;
                        int removalValue;
                    };
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

                    std::vector<std::shared_ptr<hazelcast::util::Thread> > allThreads;
                    for (int i = 0; i < numThreads; i++) {
                        std::shared_ptr<hazelcast::util::Thread> t(
                                new hazelcast::util::Thread(std::shared_ptr<hazelcast::util::Runnable>(
                                        new ConcurrentQueueTask(q, startLatch, startRemoveLatch, removalValue)),
                                                getLogger()));
                        t->start();
                        allThreads.push_back(t);
                    }

                    // wait for the remove start
                    ASSERT_TRUE(startRemoveLatch.await(30));

                    int numRemoved = q.removeAll(&removalValue);

                    int numRemaining = numThreads - numRemoved;

                    for (int j = 0; j < numRemaining; ++j) {
                        ASSERT_NE((int *)NULL, q.poll());
                    }
                    ASSERT_EQ(0, q.removeAll(&removalValue));

                }
            }
        }
    }
}
