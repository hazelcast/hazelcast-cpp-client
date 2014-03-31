#include "lock/ClientLockTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientLockTest::ClientLockTest(HazelcastServerFactory &hazelcastInstanceFactory)
            : iTestFixture<ClientLockTest>("ClientLockTest")
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701))))
            , l(new ILock(client->getILock("ClientLockTest"))) {
            };


            ClientLockTest::~ClientLockTest() {
            }

            void ClientLockTest::addTests() {
                addTest(&ClientLockTest::testLock, "testLock");
                addTest(&ClientLockTest::testLockTtl, "testLockTtl");
                addTest(&ClientLockTest::testTryLock, "testTryLock");
                addTest(&ClientLockTest::testForceUnlock, "testForceUnlock");
                addTest(&ClientLockTest::testStats, "testStats");
            };

            void ClientLockTest::beforeClass() {
            };

            void ClientLockTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void ClientLockTest::beforeTest() {
            };

            void ClientLockTest::afterTest() {
                l->forceUnlock();
            };

            void testLockThread(ILock *l, util::CountDownLatch *latch) {
                if (!l->tryLock())
                    latch->countDown();
            }

            void ClientLockTest::testLock() {
                l->lock();
                util::CountDownLatch latch(1);
                boost::thread(boost::bind(testLockThread, l.get(), &latch));

                assertTrue(latch.await(5 * 1000));
                l->forceUnlock();
            }

            void testLockTtlThread(ILock *l, util::CountDownLatch *latch) {
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
                boost::thread(boost::bind(testLockTtlThread, l.get(), &latch));
                assertTrue(latch.await(10 * 1000));
                l->forceUnlock();
            }

            void testTryLockThread1(ILock *l, util::CountDownLatch *latch) {
                if (!l->tryLock(2 * 1000)) {
                    latch->countDown();
                }
            }

            void testTryLockThread2(ILock *l, util::CountDownLatch *latch) {
                if (l->tryLock(20 * 1000)) {
                    latch->countDown();
                }
            }

            void ClientLockTest::testTryLock() {

                assertTrue(l->tryLock(2 * 1000));
                util::CountDownLatch latch(1);
                boost::thread(boost::bind(testTryLockThread1, l.get(), &latch));
                assertTrue(latch.await(100 * 1000));

                assertTrue(l->isLocked());

                util::CountDownLatch latch2(1);
                boost::thread(boost::bind(testTryLockThread2, l.get(), &latch2));
                boost::this_thread::sleep(boost::posix_time::seconds(1));
                l->unlock();
                assertTrue(latch2.await(100 * 1000));
                assertTrue(l->isLocked());
                l->forceUnlock();
            }

            void testForceUnlockThread(ILock *l, util::CountDownLatch *latch) {
                l->forceUnlock();
                latch->countDown();
            }

            void ClientLockTest::testForceUnlock() {
                l->lock();
                util::CountDownLatch latch(1);
                boost::thread(boost::bind(testForceUnlockThread, l.get(), &latch));
                assertTrue(latch.await(100 * 1000));
                assertFalse(l->isLocked());
            }


            void testStatsThread(ILock *l, util::CountDownLatch *latch) {
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
                boost::thread(boost::bind(testStatsThread, l.get(), &latch));
                assertTrue(latch.await(1 * 1000 * 60));
            }
        }
    }
}
