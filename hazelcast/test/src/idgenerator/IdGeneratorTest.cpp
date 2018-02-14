/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "HazelcastServer.h"
#include "ClientTestSupport.h"

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class IdGeneratorTest : public ClientTestSupport {
            public:
                IdGeneratorTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<IdGenerator> generator;

            };

            IdGeneratorTest::IdGeneratorTest()
            : instance(*g_srvFactory)
            , client(getNewClient())
            , generator(new IdGenerator(client->getIdGenerator("clientIdGenerator"))) {
            }

            TEST_F (IdGeneratorTest, testGenerator) {
                int initValue = 3569;
                ASSERT_TRUE(generator->init(initValue));
                ASSERT_FALSE(generator->init(4569));
                for(int i = 0 ; i < 2000 ; i++){
                    ASSERT_EQ(++initValue, generator->newId());
                }
            }

        }
    }
}



