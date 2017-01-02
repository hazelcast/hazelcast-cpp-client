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
// Created by İhsan Demir on 6 June 2016.
//
#include <gtest/gtest.h>

#include "hazelcast/util/Atomic.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class BlockingConcurrentQueueTest : public ::testing::Test
                {
                protected:
                    static void PushDelayed(hazelcast::util::ThreadArgs &args) {
                        hazelcast::util::BlockingConcurrentQueue<int> *q = (hazelcast::util::BlockingConcurrentQueue<int> *)args.arg0;
                        int *val = (int *)args.arg1;
                        unsigned long *milliseconds = (unsigned long *)args.arg2;
                        hazelcast::util::sleepmillis(*milliseconds);
                        q->push(*val);
                    }
                    static void Pop(hazelcast::util::ThreadArgs &args) {
                        hazelcast::util::BlockingConcurrentQueue<int> *q = (hazelcast::util::BlockingConcurrentQueue<int> *)args.arg0;
                        hazelcast::util::Atomic<int> *val = (hazelcast::util::Atomic<int> *)args.arg1;
                        *val = q->pop();
                    }
                };

                TEST_F(BlockingConcurrentQueueTest, testPushDelyaed) {
                    size_t capacity = 3;

                    hazelcast::util::BlockingConcurrentQueue<int> q(capacity);

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_NO_THROW(q.push(i));
                    }

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_EQ((int)i, q.pop());
                    }

                    int val = 7;

                    unsigned long sleepTime = 3000U;
                    hazelcast::util::Thread t(PushDelayed, &q, &val, &sleepTime);

                    int64_t start = hazelcast::util::currentTimeMillis();
                    ASSERT_EQ(val, q.pop());
                    int64_t diff = hazelcast::util::currentTimeMillis() - start;
                    ASSERT_TRUE(diff > 1000);
                }

                TEST_F(BlockingConcurrentQueueTest, testPop) {
                    size_t capacity = 3;

                    hazelcast::util::BlockingConcurrentQueue<int> q(capacity);

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_NO_THROW(q.push(i));
                    }

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_EQ((int)i, q.pop());
                    }

                    hazelcast::util::Atomic<int> val(-1);
                    int testValue = 7;
                    unsigned long sleepTime = 3000U;
                    hazelcast::util::Thread t(Pop, &q, &val, &sleepTime);

                    hazelcast::util::sleepmillis(2000);
                    ASSERT_NO_THROW(q.push(testValue));
                    hazelcast::util::sleepmillis(1000);
                    ASSERT_EQ(testValue, (int)val);
                }
            }
        }
    }
}
