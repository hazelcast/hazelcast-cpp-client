//
//  IdGeneratorTest.h
//  hazelcast
//
//  Created by Batikan Turkmen on 02.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#include "IdGeneratorTest.h"
#include "HazelcastInstanceFactory.h"
#include "HazelcastClient.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            using namespace iTest;

            IdGeneratorTest::IdGeneratorTest(HazelcastInstanceFactory& hazelcastInstanceFactory)
            :hazelcastInstanceFactory(hazelcastInstanceFactory)
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701))))
            , generator(new IdGenerator(client->getIdGenerator("clientIdGenerator"))) {
            };


            IdGeneratorTest::~IdGeneratorTest() {
            }

            void IdGeneratorTest::addTests() {
                addTest(&IdGeneratorTest::testGenerator, "clientIdGenerator");
            };

            void IdGeneratorTest::beforeClass() {

            };

            void IdGeneratorTest::afterClass() {
                client.reset();
                instance.shutdown();
            };

            void IdGeneratorTest::beforeTest() {
            };

            void IdGeneratorTest::afterTest() {
            };

            void IdGeneratorTest::testGenerator() {

                assertTrue(generator->init(3569));
                assertFalse(generator->init(4569));
                assertEqual(3570, generator->newId());
            }

        }
    }
}


