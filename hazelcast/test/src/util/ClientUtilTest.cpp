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

#include "hazelcast/util/Util.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/util/Thread.h"

#include <ctime>
#include <gtest/gtest.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientUtilTest : public ::testing::Test {
            public:
                static void wakeTheConditionUp(util::ThreadArgs& args) {
                    util::Mutex *mutex = (util::Mutex *)args.arg0;
                    util::ConditionVariable *cv = (util::ConditionVariable *)args.arg1;
                    int wakeUpTime = *(int *)args.arg2;
                    util::sleep(wakeUpTime);

                    util::LockGuard lockGuard(*mutex);
                    cv->notify();
                }

                static void setValueToFuture(util::ThreadArgs& args) {
                    util::Future<int> *future = (util::Future<int> *)args.arg0;
                    int value = *(int *)args.arg1;
                    int wakeUpTime = *(int *)args.arg2;
                    util::sleep(wakeUpTime);
                    future->set_value(value);
                }

                static void setExceptionToFuture(util::ThreadArgs& args) {
                    util::Future<int> *future = (util::Future<int> *)args.arg0;
                    int wakeUpTime = *(int *)args.arg1;
                    util::sleep(wakeUpTime);
                    future->set_exception("exceptionName", "details");
                }
            };

            TEST_F(ClientUtilTest, testConditionWaitTimeout) {
                util::Mutex mutex;
                util::ConditionVariable conditionVariable;
                int wakeUpTime = 3;
                util::Thread thread(wakeTheConditionUp, &mutex, &conditionVariable, &wakeUpTime);
                int waitSeconds = 30;
                {
                    util::LockGuard lockGuard(mutex);
                    time_t beg = time(NULL);
                    time_t end = 0;
                    bool wokenUpByInterruption = conditionVariable.waitFor(mutex, waitSeconds);
                    if (wokenUpByInterruption) {
                        end = time(NULL);
                    }
                    ASSERT_NEAR((double)(end-beg), (double)wakeUpTime, 1);
                }

            }

            TEST_F (ClientUtilTest, testFutureWaitTimeout) {
                util::Future<int> future;
                int waitSeconds = 3;
                time_t beg = time(NULL);
                time_t end = 0;
                try {
                    future.get(waitSeconds);
                } catch (exception::TimeoutException&) {
                    end = time(NULL);
                }
                ASSERT_NEAR((double)(end-beg), (double)waitSeconds, 1);
            }

            TEST_F (ClientUtilTest, testFutureSetValue) {
                util::Future<int> future;
                int waitSeconds = 3;
                int expectedValue = 2;
                future.set_value(expectedValue);
                int value = future.get(waitSeconds);
                ASSERT_EQ(expectedValue, value);
            }

            TEST_F (ClientUtilTest, testFutureSetException) {
                util::Future<int> future;
                int waitSeconds = 3;

                future.set_exception("exceptionName", "details");

                ASSERT_THROW(future.get(waitSeconds), exception::IException);
            }


            TEST_F (ClientUtilTest, testFutureSetValue_afterSomeTime) {
                util::Future<int> future;
                int waitSeconds = 30;
                int wakeUpTime = 3;
                int expectedValue = 2;
                util::Thread thread(ClientUtilTest::setValueToFuture, &future, &expectedValue, &wakeUpTime);
                int value = future.get(waitSeconds);
                ASSERT_EQ(expectedValue, value);

            }

            TEST_F (ClientUtilTest, testFutureSetException_afterSomeTime) {
                util::Future<int> future;
                int waitSeconds = 30;
                int wakeUpTime = 3;
                bool gotException = false;
                util::Thread thread(ClientUtilTest::setExceptionToFuture, &future, &wakeUpTime);
                try {
                    future.get(waitSeconds);
                } catch (exception::IException&) {
                    gotException = true;
                }
                ASSERT_EQ(true, gotException);
            }

            void dummyThread(util::ThreadArgs& args) {

            }

            TEST_F (ClientUtilTest, testThreadName) {
                std::string threadName = "myThreadName";
                util::Thread thread(threadName, dummyThread);
                ASSERT_EQ(threadName, thread.getThreadName());
            }

            void sleepyThread(util::ThreadArgs& args) {
                int sleepTime = *(int *)args.arg0;
                args.currentThread->interruptibleSleep(sleepTime);
            }

            TEST_F (ClientUtilTest, testThreadInterruptibleSleep) {
                int sleepTime = 30;
                int wakeUpTime = 3;
                time_t beg = time(NULL);
                util::Thread thread(sleepyThread, &sleepTime);
                util::sleep(wakeUpTime);
                thread.cancel();
                thread.join();
                time_t end = time(NULL);
                ASSERT_NEAR((double)(end - beg), (double)wakeUpTime , 1);
            }

            TEST_F (ClientUtilTest, testDateConversion) {
                std::string date("2016-04-20");
                util::gitDateToHazelcastLogDate(date);
                ASSERT_EQ("20160420", date);

                date = "NOT_FOUND";
                util::gitDateToHazelcastLogDate(date);
                ASSERT_EQ("NOT_FOUND", date);
            }
        }
    }
}

