//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClientSemaphoreTest.h"
#include "HazelcastInstanceFactory.h"
#include "hazelcast/client/HazelcastClient.h"


namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ClientSemaphoreTest::ClientSemaphoreTest(HazelcastInstanceFactory &hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701))))
            , s(new ISemaphore(client->getISemaphore("ClientSemaphoreTest"))) {
            };


            ClientSemaphoreTest::~ClientSemaphoreTest() {
            }

            void ClientSemaphoreTest::addTests() {
                addTest(&ClientSemaphoreTest::testAcquire, "testAcquire");
                addTest(&ClientSemaphoreTest::testTryAcquire, "testTryAcquire");
            };

            void ClientSemaphoreTest::beforeClass() {
            };

            void ClientSemaphoreTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void ClientSemaphoreTest::beforeTest() {
                s->reducePermits(100);
                s->release(10);
            };

            void ClientSemaphoreTest::afterTest() {
                s->reducePermits(100);
                s->release(10);
            };

            void testAcquireThread(ISemaphore *s, util::CountDownLatch *latch) {
                s->acquire();
                latch->countDown();
            }

            void ClientSemaphoreTest::testAcquire() {
                assertEqual(10, s->drainPermits());

                util::CountDownLatch latch(1);
                boost::thread t(boost::bind(testAcquireThread, s.get(), &latch));

                boost::this_thread::sleep(boost::posix_time::seconds(1));

                s->release(2);
                assertTrue(latch.await(10 * 1000));
                assertEqual(1, s->availablePermits());

            }

            void testTryAcquireThread(ISemaphore *s, util::CountDownLatch *latch) {
                if (s->tryAcquire(2, 5 * 1000)) {
                    latch->countDown();
                }
            }

            void ClientSemaphoreTest::testTryAcquire() {
                assertTrue(s->tryAcquire());
                assertTrue(s->tryAcquire(9));
                assertEqual(0, s->availablePermits());
                assertFalse(s->tryAcquire(1 * 1000));
                assertFalse(s->tryAcquire(2, 1 * 1000));

                util::CountDownLatch latch(1);

                boost::thread t(boost::bind(testTryAcquireThread, s.get(), &latch));

                s->release(2);
                assertTrue(latch.await(10 * 1000));
                assertEqual(0, s->availablePermits());

            }
        }
    }
}