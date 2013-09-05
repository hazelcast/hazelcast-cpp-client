//
//  ClientAtomiclLong.h
//  hazelcast
//
//  Created by Batikan Turkmen on 02.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#include "IAtomicLongTest.h"
#include "HazelcastInstanceFactory.h"
#include "HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            IAtomicLongTest::IAtomicLongTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory.newHazelcastInstance())
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701))))
            , atom(new IAtomicLong(client->getIAtomicLong("clientAtomicLong"))) {
            };


            void IAtomicLongTest::addTests() {
                addTest(&IAtomicLongTest::test, "ClientAtomicLongTest");
            };

            void IAtomicLongTest::beforeClass() {

            };

            void IAtomicLongTest::afterClass() {

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

