/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include <ClientTestSupport.h>

#include "hazelcast/util/Util.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/impl/ExecutionCallback.h"

#include <ctime>
#include <errno.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientUtilTest : public ClientTestSupport {
            protected:
                class LatchExecutionCallback : public impl::ExecutionCallback<int> {
                public:
                    LatchExecutionCallback(util::CountDownLatch &successLatch, util::CountDownLatch &failLatch)
                            : successLatch(successLatch), failLatch(failLatch) {}

                    virtual void onResponse(const int &response) {
                        successLatch.countDown();
                    }

                    virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
                        failLatch.countDown();
                    }

                private:
                    util::CountDownLatch &successLatch;
                    util::CountDownLatch &failLatch;
                };

                static void wakeTheConditionUp(util::ThreadArgs &args) {
                    util::Mutex *mutex = (util::Mutex *) args.arg0;
                    util::ConditionVariable *cv = (util::ConditionVariable *) args.arg1;
                    int wakeUpTime = *(int *) args.arg2;
                    util::sleep(wakeUpTime);

                    util::LockGuard lockGuard(*mutex);
                    cv->notify();
                }

                static void setValueToFuture(util::ThreadArgs &args) {
                    util::Future<int> *future = (util::Future<int> *) args.arg0;
                    int value = *(int *) args.arg1;
                    int wakeUpTime = *(int *) args.arg2;
                    util::sleep(wakeUpTime);
                    future->set_value(value);
                }

                static void setExceptionToFuture(util::ThreadArgs &args) {
                    util::Future<int> *future = (util::Future<int> *) args.arg0;
                    int wakeUpTime = *(int *) args.arg1;
                    util::sleep(wakeUpTime);
                    std::auto_ptr<client::exception::IException> exception(
                            new exception::IOException("exceptionName", "details"));
                    future->set_exception(exception);
                }

                static void cancelJoinFromRunningThread(util::ThreadArgs &args) {
                    util::Thread *currentThread = args.currentThread;
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    currentThread->cancel();
                    ASSERT_FALSE(currentThread->join());
                    latch->countDown();
                }

                static void notifyExitingThread(util::ThreadArgs &args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *) args.arg0;
                    latch->countDown();
                }
            };

            TEST_F(ClientUtilTest, testConditionWaitTimeout) {
                util::Mutex mutex;
                util::ConditionVariable conditionVariable;
                int wakeUpTime = 3;
                util::StartedThread thread(wakeTheConditionUp, &mutex, &conditionVariable, &wakeUpTime);
                int waitSeconds = 30;
                {
                    util::LockGuard lockGuard(mutex);
                    time_t beg = time(NULL);
                    time_t end = 0;
                    bool wokenUpByInterruption = conditionVariable.waitFor(mutex, waitSeconds * 1000);
                    if (wokenUpByInterruption) {
                        end = time(NULL);
                    }
                    ASSERT_NEAR((double) (end - beg), (double) wakeUpTime, 1);
                }

            }

            TEST_F(ClientUtilTest, testConditionWaitMillisTimeout) {
                util::Mutex mutex;
                util::ConditionVariable conditionVariable;
                ASSERT_FALSE(conditionVariable.waitFor(mutex, 100));
            }

            TEST_F(ClientUtilTest, testConditionWaitNanosTimeout) {
                util::Mutex mutex;
                util::ConditionVariable conditionVariable;
                ASSERT_FALSE(conditionVariable.waitNanos(mutex, 1000));
            }

            TEST_F(ClientUtilTest, testConditionVariableForEINVAL) {
                util::Mutex mutex;
                util::ConditionVariable conditionVariable;
                int wakeUpTime = 1;
                util::StartedThread thread(wakeTheConditionUp, &mutex, &conditionVariable, &wakeUpTime);
                {
                    util::LockGuard lockGuard(mutex);
                    // the following call should not fail with assertion for EINVAL
                    conditionVariable.waitFor(mutex, 19999);
                }
            }

            TEST_F (ClientUtilTest, testFutureWaitTimeout) {
                util::Future<int> future(getLogger());
                int waitSeconds = 3;
                time_t beg = time(NULL);
                ASSERT_FALSE(future.waitFor(waitSeconds * 1000));
                ASSERT_NEAR((double) (time(NULL) - beg), (double) waitSeconds, 1);
            }

            TEST_F (ClientUtilTest, testFutureSetValue) {
                util::Future<int> future(getLogger());
                int waitSeconds = 3;
                int expectedValue = 2;
                future.set_value(expectedValue);
                ASSERT_TRUE(future.waitFor(waitSeconds * 1000));
                int value = future.get();
                ASSERT_EQ(expectedValue, value);
            }

            TEST_F (ClientUtilTest, testFutureSetException) {
                util::Future<int> future(getLogger());

                std::auto_ptr<client::exception::IException> exception(
                        new client::exception::IOException("testFutureSetException", "details"));
                future.set_exception(exception);

                ASSERT_THROW(future.get(), client::exception::IOException);
            }

            TEST_F (ClientUtilTest, testFutureCancel) {
                util::Future<int> future(getLogger());

                ASSERT_FALSE(future.isCancelled());

                future.cancel();

                ASSERT_TRUE(future.isCancelled());

                ASSERT_THROW(future.get(), util::concurrent::CancellationException);
            }

            TEST_F (ClientUtilTest, testFutureSetUnclonedIOException) {
                util::Future<int> future(getLogger());

                std::auto_ptr<client::exception::IException> ioe(
                        new client::exception::IOException("testFutureSetUnclonedIOException", "details"));
                future.set_exception(ioe);

                try {
                    future.get();
                } catch (client::exception::IOException &) {
                    // success
                } catch (client::exception::IException &) {
                    FAIL();
                }
            }

            TEST_F (ClientUtilTest, testFutureSetValue_afterSomeTime) {
                util::Future<int> future(getLogger());
                int waitSeconds = 30;
                int wakeUpTime = 3;
                int expectedValue = 2;
                util::StartedThread thread(ClientUtilTest::setValueToFuture, &future, &expectedValue, &wakeUpTime);
                ASSERT_TRUE(future.waitFor(waitSeconds * 1000));
                int value = future.get();
                ASSERT_EQ(expectedValue, value);

            }

            TEST_F (ClientUtilTest, testFutureSetException_afterSomeTime) {
                util::Future<int> future(getLogger());
                int waitSeconds = 30;
                int wakeUpTime = 3;
                util::StartedThread thread(ClientUtilTest::setExceptionToFuture, &future, &wakeUpTime);
                ASSERT_TRUE(future.waitFor(waitSeconds * 1000));

                try {
                    future.get();
                    FAIL();
                } catch (exception::IException &) {
                    // expect exception here
                }
            }

            TEST_F (ClientUtilTest, testFutureSetValueAndThen) {
                util::Future<int> future(getLogger());
                util::CountDownLatch successLatch(1);
                util::CountDownLatch failLatch(1);
                util::impl::SimpleExecutorService executorService(getLogger(), "testFutureAndThen", 3);
                future.andThen(boost::shared_ptr<impl::ExecutionCallback<int> >(
                        new LatchExecutionCallback(successLatch, failLatch)), executorService);

                int wakeUpTime = 0;
                int expectedValue = 2;
                util::StartedThread thread(ClientUtilTest::setValueToFuture, &future, &expectedValue, &wakeUpTime);

                ASSERT_OPEN_EVENTUALLY(successLatch);
            }

            TEST_F (ClientUtilTest, testFutureSetValueBeforeAndThen) {
                util::Future<int> future(getLogger());
                util::CountDownLatch successLatch(1);
                util::CountDownLatch failLatch(1);
                util::impl::SimpleExecutorService executorService(getLogger(), "testFutureAndThen", 3);

                int value = 5;
                future.set_value(value);
                future.andThen(boost::shared_ptr<impl::ExecutionCallback<int> >(
                        new LatchExecutionCallback(successLatch, failLatch)), executorService);

                ASSERT_OPEN_EVENTUALLY(successLatch);
            }

            TEST_F (ClientUtilTest, testFutureSetExceptionAndThen) {
                util::Future<int> future(getLogger());
                util::CountDownLatch successLatch(1);
                util::CountDownLatch failLatch(1);
                util::impl::SimpleExecutorService executorService(getLogger(), "testFutureAndThen", 3);
                future.andThen(boost::shared_ptr<impl::ExecutionCallback<int> >(
                        new LatchExecutionCallback(successLatch, failLatch)), executorService);

                int wakeUpTime = 0;
                util::StartedThread thread(ClientUtilTest::setExceptionToFuture, &future, &wakeUpTime);

                ASSERT_OPEN_EVENTUALLY(failLatch);
                ASSERT_THROW(future.get(), exception::IOException);
            }

            TEST_F (ClientUtilTest, testFutureSetExceptionBeforeAndThen) {
                util::Future<int> future(getLogger());
                util::CountDownLatch successLatch(1);
                util::CountDownLatch failLatch(1);
                util::impl::SimpleExecutorService executorService(getLogger(), "testFutureAndThen", 3);

                future.set_exception(std::auto_ptr<client::exception::IException>(
                        new exception::IOException("exceptionName", "details")));
                future.andThen(boost::shared_ptr<impl::ExecutionCallback<int> >(
                        new LatchExecutionCallback(successLatch, failLatch)), executorService);

                ASSERT_OPEN_EVENTUALLY(failLatch);
                ASSERT_THROW(future.get(), exception::IOException);
            }

            TEST_F (ClientUtilTest, testThreadName) {
                std::string threadName = "myThreadName";
                // We use latch so that we guarantee that the object instance thread is not destructed at the time when
                // StartedThread::run is being executed.
                util::CountDownLatch latch(1);
                util::StartedThread thread(threadName, notifyExitingThread, &latch);
                ASSERT_EQ(threadName, thread.getName());
                ASSERT_TRUE(latch.await(120));
            }

            TEST_F (ClientUtilTest, testThreadJoinAfterThreadExited) {
                std::string threadName = "myThreadName";
                util::CountDownLatch latch(1);
                util::StartedThread thread(threadName, notifyExitingThread, &latch);
                ASSERT_TRUE(latch.await(2));
                // guarantee that the thread exited
                util::sleep(1);

                // call join after thread exit
                thread.join();
            }

            TEST_F (ClientUtilTest, testCancelJoinItselfFromTheRunningThread) {
                std::string threadName = "myThreadName";
                util::CountDownLatch latch(1);
                util::StartedThread thread(threadName, cancelJoinFromRunningThread, &latch);
                ASSERT_TRUE(latch.await(1000));
            }

            void sleepyThread(util::ThreadArgs &args) {
                int sleepTime = *(int *) args.arg0;
                args.currentThread->interruptibleSleep(sleepTime);
            }

            TEST_F (ClientUtilTest, testThreadInterruptibleSleep) {
                int sleepTime = 30;
                int wakeUpTime = 3;
                time_t beg = time(NULL);
                util::StartedThread thread(sleepyThread, &sleepTime);
                util::sleep(wakeUpTime);
                thread.cancel();
                thread.join();
                ASSERT_NEAR((double) (time(NULL) - beg), (double) wakeUpTime, 1);
            }

            TEST_F (ClientUtilTest, testDateConversion) {
                std::string date("2016-04-20");
                util::gitDateToHazelcastLogDate(date);
                ASSERT_EQ("20160420", date);

                date = "NOT_FOUND";
                util::gitDateToHazelcastLogDate(date);
                ASSERT_EQ("NOT_FOUND", date);
            }

            TEST_F (ClientUtilTest, testStrError) {
                #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                int error = WSAEINPROGRESS;
                std::string expectedErrorString("A blocking operation is currently executing.\r\n");
                #else
                int error = EINPROGRESS;
                std::string expectedErrorString("Operation now in progress");
                #endif

                char msg[100];
                const std::string prefix = "testStrError prefix message";
                ASSERT_EQ(0, util::strerror_s(error, msg, 100, prefix.c_str()));
                ASSERT_STREQ((prefix + " " + expectedErrorString).c_str(), msg);

                ASSERT_EQ(0, util::strerror_s(error, msg, prefix.length() + 1, prefix.c_str()));
                ASSERT_STREQ(prefix.c_str(), msg);

                ASSERT_EQ(0, util::strerror_s(error, msg, 100));
                ASSERT_STREQ(expectedErrorString.c_str(), msg);
            }

            TEST_F (ClientUtilTest, testAvailableCoreCount) {
                ASSERT_GT(util::getAvailableCoreCount(), 0);
            }

            TEST_F (ClientUtilTest, testStringUtilTimeToString) {
                std::string timeString = util::StringUtil::timeToString(util::currentTimeMillis());
                //expected format is "%Y-%m-%d %H:%M:%S.%f" it will be something like 2018-03-20 15:36:07.280300
                ASSERT_EQ((size_t) 26, timeString.length());
                ASSERT_EQ(timeString[0], '2');
                ASSERT_EQ(timeString[1], '0');
                ASSERT_EQ(timeString[4], '-');
                ASSERT_EQ(timeString[7], '-');
            }

            TEST_F (ClientUtilTest, testStringUtilTimeToStringFriendly) {
                ASSERT_EQ("never", util::StringUtil::timeToStringFriendly(0));
            }

            TEST_F (ClientUtilTest, testLockSupport) {
                // we can not set less then a millisecond since windows platform can not support less than a millisecond
                int64_t parkDurationNanos = 1000 * 1000;
                int64_t start = util::currentTimeNanos();
                util::concurrent::locks::LockSupport::parkNanos(parkDurationNanos);
                int64_t end = util::currentTimeNanos();
                int64_t actualDuration = end - start;
                ASSERT_GE(actualDuration, parkDurationNanos);
            }
        }
    }
}

