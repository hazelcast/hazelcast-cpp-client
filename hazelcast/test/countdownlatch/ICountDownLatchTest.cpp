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

            ICountDownLatchTest::ICountDownLatchTest(HazelcastServerFactory &serverFactory)
            : iTestFixture<ICountDownLatchTest>("ICountDownLatchTest")
            , instance(serverFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(serverFactory.getServerAddress(), 5701))))
            , l(new ICountDownLatch(client->getICountDownLatch("ICountDownLatchTest"))) {
            }


            ICountDownLatchTest::~ICountDownLatchTest() {
            }

            void ICountDownLatchTest::addTests() {
                addTest(&ICountDownLatchTest::testLatch, "ICountDownLatchTest");
            }

            void ICountDownLatchTest::beforeClass() {

            }

            void ICountDownLatchTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void ICountDownLatchTest::beforeTest() {

            }

            void ICountDownLatchTest::afterTest() {
            }

            void testLatchThread(util::ThreadArgs &args) {
                ICountDownLatch *l = (ICountDownLatch *) args.arg0;
                for (int i = 0; i < 20; i++) {
                    l->countDown();
                }
            }

            void ICountDownLatchTest::testLatch() {
                assertTrue(l->trySetCount(20));
                assertFalse(l->trySetCount(10));
                assertEqual(20, l->getCount());

                util::Thread t(testLatchThread, l.get());

                assertTrue(l->await(10 * 1000));
            }

        }
    }
}


