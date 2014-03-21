//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "countdownlatch/ICountDownLatchTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ICountDownLatchTest::ICountDownLatchTest(HazelcastServerFactory &hazelcastInstanceFactory)
            : iTestFixture("ICountDownLatchTest")
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701))))
            , l(new ICountDownLatch(client->getICountDownLatch("ICountDownLatchTest"))) {
            };


            ICountDownLatchTest::~ICountDownLatchTest() {
            }

            void ICountDownLatchTest::addTests() {
                addTest(&ICountDownLatchTest::testLatch, "ICountDownLatchTest");
            };

            void ICountDownLatchTest::beforeClass() {

            };

            void ICountDownLatchTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void ICountDownLatchTest::beforeTest() {

            };

            void ICountDownLatchTest::afterTest() {
            };

            void testLatchThread(ICountDownLatch *l) {
                for (int i = 0; i < 20; i++) {
                    l->countDown();
                    boost::this_thread::sleep(boost::posix_time::milliseconds(60));
                }
            }

            void ICountDownLatchTest::testLatch() {
                assertTrue(l->trySetCount(20));
                assertFalse(l->trySetCount(10));
                assertEqual(20, l->getCount());

                boost::thread t(boost::bind(testLatchThread, l.get()));

                assertFalse(l->await(1 * 1000));

                assertTrue(l->await(5 * 1000));
            }

        }
    }
}

