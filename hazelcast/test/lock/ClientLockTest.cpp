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
#include "lock/ClientLockTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientLockTest::ClientLockTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<ClientLockTest>("ClientLockTest", &serverFactory)
            , instance(serverFactory)
            , client(getNewClient())
            , l(new ILock(client->getILock("ClientLockTest"))) {
            }


            ClientLockTest::~ClientLockTest() {
            }

            void ClientLockTest::addTests() {
                addTest(&ClientLockTest::testLock, "testLock");
                addTest(&ClientLockTest::testLockTtl, "testLockTtl");
                addTest(&ClientLockTest::testTryLock, "testTryLock");
                addTest(&ClientLockTest::testForceUnlock, "testForceUnlock");
                addTest(&ClientLockTest::testStats, "testStats");
            }

            void ClientLockTest::beforeClass() {
            }

            void ClientLockTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ClientLockTest::beforeTest() {
            }

            void ClientLockTest::afterTest() {
                l->forceUnlock();
            }

            void testLockLockThread(util::ThreadArgs &args) {
                ILock *l = (ILock *) args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg1;
                if (!l->tryLock())
                    latch->countDown();
            }

            void ClientLockTest::testLock() {
                l->lock();
                util::CountDownLatch latch(1);
                util::Thread t(testLockLockThread, l.get(), &latch);

                assertTrue(latch.await(5));
                l->forceUnlock();
            }

            void testLockTtlThread(util::ThreadArgs &args) {
                ILock *l = (ILock *) args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg1;
                if (!l->tryLock()) {
                    latch->countDown();
                }
                if (l->tryLock(5 * 1000)) {
                    latch->countDown();
                }
            }

            void ClientLockTest::testLockTtl() {
                l->lock(3 * 1000);
                util::CountDownLatch latch(2);
                util::Thread t(testLockTtlThread, l.get(), &latch);
                assertTrue(latch.await(10));
                l->forceUnlock();
            }

            void testLockTryLockThread1(util::ThreadArgs &args) {
                ILock *l = (ILock *) args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg1;
                if (!l->tryLock(2 * 1000)) {
                    latch->countDown();
                }
            }

            void testLockTryLockThread2(util::ThreadArgs &args) {
                ILock *l = (ILock *) args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg1;
                if (l->tryLock(20 * 1000)) {
                    latch->countDown();
                }
            }

            void ClientLockTest::testTryLock() {

                assertTrue(l->tryLock(2 * 1000));
                util::CountDownLatch latch(1);
                util::Thread t1(testLockTryLockThread1, l.get(), &latch);
                assertTrue(latch.await(100));

                assertTrue(l->isLocked());

                util::CountDownLatch latch2(1);
                util::Thread t2(testLockTryLockThread2, l.get(), &latch2);
                util::sleep(1);
                l->unlock();
                assertTrue(latch2.await(100));
                assertTrue(l->isLocked());
                l->forceUnlock();
            }

            void testLockForceUnlockThread(util::ThreadArgs &args) {
                ILock *l = (ILock *) args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg1;
                l->forceUnlock();
                latch->countDown();
            }

            void ClientLockTest::testForceUnlock() {
                l->lock();
                util::CountDownLatch latch(1);
                util::Thread t(testLockForceUnlockThread, l.get(), &latch);
                assertTrue(latch.await(100));
                assertFalse(l->isLocked());
            }


            void testStatsThread(util::ThreadArgs &args) {
                ILock *l = (ILock *) args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *) args.arg1;
                try {
                    assertTrue(l->isLocked(), "l->isLocked()");
                    assertFalse(l->isLockedByCurrentThread(), "isLockedByCurrentThread");
                    assertEqual(1, l->getLockCount(), "getLockCount");
                    assertTrue(l->getRemainingLeaseTime() > 1000 * 30, "getRemainingLeaseTime");
                    latch->countDown();
                } catch(iTest::iTestException &e) {
                    (std::cout << e.message << std::endl);
                }
            }

            void ClientLockTest::testStats() {
                l->lock();
                assertTrue(l->isLocked());
                assertTrue(l->isLockedByCurrentThread());
                assertEqual(1, l->getLockCount());

                l->unlock();
                assertFalse(l->isLocked());
                assertEqual(0, l->getLockCount());
                assertEqual(-1L, l->getRemainingLeaseTime());

                l->lock(1 * 1000 * 60);
                assertTrue(l->isLocked());
                assertTrue(l->isLockedByCurrentThread());
                assertEqual(1, l->getLockCount());
                assertTrue(l->getRemainingLeaseTime() > 1000 * 30);

                util::CountDownLatch latch(1);
                util::Thread t(testStatsThread, l.get(), &latch);
                assertTrue(latch.await(60));
            }
        }
    }
}

