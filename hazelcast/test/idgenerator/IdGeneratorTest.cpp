//
//  IdGeneratorTest.h
//  hazelcast
//
//  Created by Sancar on 02.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#include "idgenerator/IdGeneratorTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            using namespace iTest;

            IdGeneratorTest::IdGeneratorTest(HazelcastServerFactory &hazelcastInstanceFactory)
            : iTestFixture<IdGeneratorTest>("IdGeneratorTest")
            , instance(hazelcastInstanceFactory)
            , client(new HazelcastClient(clientConfig.addAddress(Address(HOST, 5701))))
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
                int initValue = 3569;
                assertTrue(generator->init(initValue));
                assertFalse(generator->init(4569));
                for(int i = 0 ; i < 2000 ; i++){
                    assertEqual(++initValue, generator->newId());
                }
            }

        }
    }
}



