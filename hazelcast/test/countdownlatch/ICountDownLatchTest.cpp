//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ICountDownLatchTest.h"
#include "HazelcastInstanceFactory.h"
#include "HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            ICountDownLatchTest::ICountDownLatchTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701))))
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

