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

            IdGeneratorTest::IdGeneratorTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<IdGeneratorTest>("IdGeneratorTest" , &serverFactory)
            , instance(serverFactory)
            , client(getNewClient())
            , generator(new IdGenerator(client->getIdGenerator("clientIdGenerator"))) {
            }


            IdGeneratorTest::~IdGeneratorTest() {
            }

            void IdGeneratorTest::addTests() {
                addTest(&IdGeneratorTest::testGenerator, "clientIdGenerator");
            }

            void IdGeneratorTest::beforeClass() {

            }

            void IdGeneratorTest::afterClass() {
                client.reset();
                instance.shutdown();
            }

            void IdGeneratorTest::beforeTest() {
            }

            void IdGeneratorTest::afterTest() {
            }

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



