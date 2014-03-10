//
//  ClientAtomiclLong.h
//  hazelcast
//
//  Created by Sancar on 02.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#include "IAtomicLongTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            IAtomicLongTest::IAtomicLongTest(HazelcastServerFactory &hazelcastInstanceFactory)
            : iTestFixture("IAtomicLongTest")
            ,hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701))))
            , atom(new IAtomicLong(client->getIAtomicLong("clientAtomicLong"))) {
            };


            IAtomicLongTest::~IAtomicLongTest() {
            }

            void IAtomicLongTest::addTests() {
                addTest(&IAtomicLongTest::test, "ClientAtomicLongTest");
            };

            void IAtomicLongTest::beforeClass() {

            };

            void IAtomicLongTest::afterClass() {
                client.reset();
                client.reset();
                client.reset();
                instance.shutdown();
            };

            void IAtomicLongTest::beforeTest() {
                atom->set(0);
            };

            void IAtomicLongTest::afterTest() {
                atom->set(0);
            };

            void IAtomicLongTest::test() {
                assertEqual(0, atom->getAndAdd(2));
                assertEqual(2, atom->get());
                atom->set(5);
                assertEqual(5, atom->get());
                assertEqual(8, atom->addAndGet(3));
                assertFalse(atom->compareAndSet(7, 4));
                assertEqual(8, atom->get());
                assertTrue(atom->compareAndSet(8, 4));
                assertEqual(4, atom->get());
                assertEqual(3, atom->decrementAndGet());
                assertEqual(3, atom->getAndIncrement());
                assertEqual(4, atom->getAndSet(9));
                assertEqual(10, atom->incrementAndGet());
            }

        }
    }
}

