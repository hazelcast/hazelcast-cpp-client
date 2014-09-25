//
// Created by sancar koyunlu on 22/08/14.
//

#include "ClientUtilTest.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/util/Thread.h"
#include <ctime>

namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;


            ClientUtilTest::ClientUtilTest(int dummy)
            : iTestFixture<ClientUtilTest>("ClientUtilTest") {

            }

            void ClientUtilTest::addTests() {
                addTest(&ClientUtilTest::testConditionWaitTimeout, "testConditionWaitTimeout");
                addTest(&ClientUtilTest::testConditionWakeUpTime, "testConditionWakeUpTime");
                addTest(&ClientUtilTest::testFutureWaitTimeout, "testFutureWaitTimeout");
                addTest(&ClientUtilTest::testFutureSetValue, "testFutureSetValue");
                addTest(&ClientUtilTest::testFutureSetException, "testFutureSetException");
                addTest(&ClientUtilTest::testFutureSetValue_afterSomeTime, "testFutureSetValue_afterSomeTime");
                addTest(&ClientUtilTest::testFutureSetException_afterSomeTime, "testFutureSetException_afterSomeTime");
                addTest(&ClientUtilTest::testThreadName, "testThreadName");
                addTest(&ClientUtilTest::testThreadInterruptibleSleep, "testThreadInterruptibleSleep");
            }

            void ClientUtilTest::beforeClass() {

            }

            void ClientUtilTest::afterClass() {

            }

            void ClientUtilTest::beforeTest() {

            }

            void ClientUtilTest::afterTest() {

            }

            void ClientUtilTest::testConditionWaitTimeout() {
                util::Mutex mutex;
                util::ConditionVariable conditionVariable;
                int waitSeconds = 3;
                {
                    util::LockGuard lockGuard(mutex);
                    time_t beg = time(NULL);
                    time_t end = 0;
                    bool wokenUpByInterruption = conditionVariable.waitFor(mutex, waitSeconds);
                    if (!wokenUpByInterruption) {
                        end = time(NULL);
                    }
                    assertEqualWithEpsilon((int)(end - beg), waitSeconds , 1);
                }
            }


            void wakeTheConditionUp(util::ThreadArgs& args) {
                util::Mutex *mutex = (util::Mutex *)args.arg0;
                util::ConditionVariable *cv = (util::ConditionVariable *)args.arg1;
                int wakeUpTime = *(int *)args.arg2;
                util::sleep(wakeUpTime);

                util::LockGuard lockGuard(*mutex);
                cv->notify();
            }

            void ClientUtilTest::testConditionWakeUpTime() {
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
                    assertEqualWithEpsilon((int)(end - beg), wakeUpTime , 1);
                }

            }

            void ClientUtilTest::testFutureWaitTimeout() {
                util::Future<int> future;
                int waitSeconds = 3;
                time_t beg = time(NULL);
                time_t end = 0;
                try {
                    future.get(waitSeconds);
                } catch (exception::TimeoutException&) {
                    end = time(NULL);
                }
                assertEqualWithEpsilon((int)(end - beg), waitSeconds , 1);
            }


            void ClientUtilTest::testFutureSetValue() {
                util::Future<int> future;
                int waitSeconds = 3;
                int expectedValue = 2;
                future.set_value(expectedValue);
                int value = future.get(waitSeconds);
                assertEqual(expectedValue, value);
            }

            void ClientUtilTest::testFutureSetException() {
                util::Future<int> future;
                int waitSeconds = 3;
                bool gotException = false;
                future.set_exception("exceptionName", "details");
                try {
                    future.get(waitSeconds);
                } catch (exception::IException&) {
                    gotException = true;
                }
                assertEqual(true, gotException);
            }

            void setValueToFuture(util::ThreadArgs& args) {
                util::Future<int> *future = (util::Future<int> *)args.arg0;
                int value = *(int *)args.arg1;
                int wakeUpTime = *(int *)args.arg2;
                util::sleep(wakeUpTime);
                future->set_value(value);
            }

            void setExceptionToFuture(util::ThreadArgs& args) {
                util::Future<int> *future = (util::Future<int> *)args.arg0;
                int wakeUpTime = *(int *)args.arg1;
                util::sleep(wakeUpTime);
                future->set_exception("exceptionName", "details");
            }

            void ClientUtilTest::testFutureSetValue_afterSomeTime() {
                util::Future<int> future;
                int waitSeconds = 30;
                int wakeUpTime = 3;
                int expectedValue = 2;
                util::Thread thread(setValueToFuture, &future, &expectedValue, &wakeUpTime);
                int value = future.get(waitSeconds);
                assertEqual(expectedValue, value);

            }

            void ClientUtilTest::testFutureSetException_afterSomeTime() {
                util::Future<int> future;
                int waitSeconds = 30;
                int wakeUpTime = 3;
                bool gotException = false;
                util::Thread thread(setExceptionToFuture, &future, &wakeUpTime);
                try {
                    future.get(waitSeconds);
                } catch (exception::IException&) {
                    gotException = true;
                }
                assertEqual(true, gotException);
            }

            void dummyThread(util::ThreadArgs& args) {

            }

            void ClientUtilTest::testThreadName() {
                std::string threadName = "myThreadName";
                util::Thread thread(threadName, dummyThread);
                assertEqual(threadName, thread.getThreadName());
            }

            void sleepyThread(util::ThreadArgs& args) {
                int sleepTime = *(int *)args.arg0;
                args.currentThread->interruptibleSleep(sleepTime);
            }

            void ClientUtilTest::testThreadInterruptibleSleep() {
                int sleepTime = 30;
                int wakeUpTime = 3;
                time_t beg = time(NULL);
                util::Thread thread(sleepyThread, &sleepTime);
                util::sleep(wakeUpTime);
                thread.interrupt();
                thread.join();
                time_t end = time(NULL);
                assertEqualWithEpsilon((int)(end - beg), wakeUpTime , 1);
            }

        }
    }
}

