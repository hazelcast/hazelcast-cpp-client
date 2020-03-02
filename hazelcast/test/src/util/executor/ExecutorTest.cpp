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

#include <memory>

#include <gtest/gtest.h>
#include <hazelcast/util/Executor.h>
#include <hazelcast/util/CountDownLatch.h>
#include <hazelcast/util/Util.h>
#include <hazelcast/util/impl/SimpleExecutorService.h>
#include <TestHelperFunctions.h>
#include <ClientTestSupport.h>

using namespace hazelcast::util;

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                namespace executor {
                    class ExecutorTest : public ClientTestSupport {
                    protected:
                        class StripedIntRunable : public StripedRunnable {
                        public:
                            StripedIntRunable(int32_t key, CountDownLatch &latch, std::atomic<int64_t> &threadId,
                                              bool controlThread)
                                    : key(key), latch(latch), threadId(threadId), isControlThread(controlThread) {}

                            virtual void run() {
                                if (isControlThread) {
                                    if (threadId.load() == hazelcast::util::getCurrentThreadId()) {
                                        latch.countDown();
                                    }
                                } else {
                                    threadId = hazelcast::util::getCurrentThreadId();
                                    latch.countDown();
                                }
                            }

                            virtual int32_t getKey() {
                                return key;
                            }

                            virtual const std::string getName() const {
                                return "StripedIntRunable";
                            }

                        private:
                            int32_t key;
                            CountDownLatch &latch;
                            std::atomic<int64_t> &threadId;
                            bool isControlThread;
                        };

                        class ValueReflector : public Callable<int> {
                        public:
                            ValueReflector(int returnValue) : returnValue(new int(returnValue)) {}

                            virtual std::shared_ptr<int> call() {
                                return returnValue;
                            }

                            virtual const std::string getName() const {
                                return "Multiplier";
                            }

                        protected:
                            std::shared_ptr<int> returnValue;
                        };

                        class LatchDecrementer : public Runnable {
                        public:
                            LatchDecrementer(CountDownLatch &latch) : latch(latch) {}

                            virtual void run() {
                                latch.countDown();
                            }

                            virtual const std::string getName() const {
                                return "LatchDecrementer";
                            }

                        protected:
                            CountDownLatch &latch;
                        };

                        class SequentialLatchDecrementer : public LatchDecrementer {
                        public:
                            SequentialLatchDecrementer(CountDownLatch &latch, int threadIndex, int totalNumberOfThreads)
                                    : LatchDecrementer(latch),
                                      threadIndex(threadIndex),
                                      totalNumberOfThreads(totalNumberOfThreads) {}

                            virtual void run() {
                                if (latch.get() == totalNumberOfThreads - threadIndex) {
                                    latch.countDown();
                                }
                            }

                            virtual const std::string getName() const {
                                return "SequentialLatchDecrementer";
                            }

                        private:
                            int threadIndex;
                            int totalNumberOfThreads;
                        };

                    };

                    TEST_F(ExecutorTest, testSingleThreadSequentialExecution) {
                        hazelcast::util::impl::SimpleExecutorService singleThreadExecutor(getLogger(),
                                                                                          "testGetPossibleSocketAddresses",
                                                                                          1);

                        singleThreadExecutor.start();

                        int numThreads = 10;
                        CountDownLatch latch(numThreads);

                        for (int i = 0; i < numThreads; ++i) {
                            singleThreadExecutor.execute(
                                    std::shared_ptr<Runnable>(new SequentialLatchDecrementer(latch, i, numThreads)));
                        }

                        ASSERT_OPEN_EVENTUALLY(latch);
                    }

                    TEST_F(ExecutorTest, testMultiThreadExecution) {
                        int32_t numThreads = 10;
                        hazelcast::util::impl::SimpleExecutorService executorService(getLogger(),
                                                                                     "testMultiThreadExecution",
                                                                                     numThreads);

                        CountDownLatch latch(numThreads);

                        for (int i = 0; i < numThreads; ++i) {
                            executorService.execute(std::shared_ptr<Runnable>(new LatchDecrementer(latch)));
                        }

                        ASSERT_OPEN_EVENTUALLY(latch);

                        executorService.shutdown();
                    }

                    TEST_F(ExecutorTest, testRejectExecuteAfterShutdown) {
                        int32_t numThreads = 10;
                        hazelcast::util::impl::SimpleExecutorService executorService(getLogger(),
                                                                                     "testRejectExecuteAfterShutdown",
                                                                                     numThreads);
                        executorService.shutdown();
                        CountDownLatch latch(numThreads);
                        ASSERT_THROW(executorService.execute(std::shared_ptr<Runnable>(new LatchDecrementer(latch))),
                                     client::exception::RejectedExecutionException);
                        ASSERT_THROW(
                                executorService.submit<int>(std::shared_ptr<Callable<int> >(new ValueReflector(1))),
                                client::exception::RejectedExecutionException);
                    }

                    TEST_F(ExecutorTest, testExecutorSubmit) {
                        int32_t numThreads = 10;
                        int32_t numJobs = 5 * numThreads;
                        hazelcast::util::impl::SimpleExecutorService executorService(getLogger(),
                                                                                     "testExecutorSubmit",
                                                                                     numThreads);


                        std::vector<std::shared_ptr<Future<int> > > futures;
                        for (int i = 0; i < numJobs; ++i) {
                            futures.push_back(
                                    executorService.submit<int>(
                                            std::shared_ptr<Callable<int> >(new ValueReflector(i))));
                        }

                        for (int i = 0; i < numJobs; ++i) {
                            ASSERT_EQ(i, *futures[i]->get());
                        }
                    }

                    TEST_F(ExecutorTest, testStripedExecutorAlwaysRunsAtTheSameThread) {
                        int32_t numThreads = 10;
                        hazelcast::util::impl::SimpleExecutorService executorService(getLogger(),
                                                                                      "testMultiThreadExecution",
                                                                                      numThreads);

                        CountDownLatch latch(1);
                        std::atomic<int64_t> threadId(0);
                        int32_t key = 5;
                        // the following gets the thread id fr the key
                        executorService.execute(
                                std::shared_ptr<StripedRunnable>(new StripedIntRunable(key, latch, threadId, false)));
                        ASSERT_OPEN_EVENTUALLY(latch);

                        CountDownLatch latch2(1);
                        // this makes sure that the execution occured at the same thread as the previous runnable
                        executorService.execute(std::shared_ptr<StripedRunnable>(
                                new StripedIntRunable(key, latch2, threadId, true)));
                        ASSERT_OPEN_EVENTUALLY(latch2);
                    }

                }
            }
        }
    }
}

