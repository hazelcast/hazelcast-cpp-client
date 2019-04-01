/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/Pipelining.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

using namespace std;

namespace hazelcast {
    namespace client {
        namespace test {
            class PipeliningTest : public ClientTestSupport {
            public:
                PipeliningTest() : map(client->getMap<int, int>(getTestName())) {
                }

            public:
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig;
                    client = new HazelcastClient(clientConfig);
                }

                static void ShutdownTestCase() {
                    delete instance;
                    instance = NULL;
                    delete client;
                    client = NULL;
                }

            protected:
                void testPipelining(const boost::shared_ptr<Pipelining<int> > &pipelining) {
                    std::vector<int> expected;
                    for (int k = 0; k < MAP_SIZE; ++k) {
                        int item = rand();
                        expected.push_back(item);
                        map.put(k, item);
                    }

                    for (int k = 0; k < MAP_SIZE; k++) {
                        pipelining->add(map.getAsync(k));
                    }

                    vector<boost::shared_ptr<int> > results = pipelining->results();
                    ASSERT_EQ(expected.size(), results.size());
                    for (int k = 0; k < MAP_SIZE; ++k) {
                        ASSERT_EQ_PTR(expected[k], results[k].get(), int);
                    }
                }

                IMap<int, int> map;
                static HazelcastServer *instance;
                static HazelcastClient *client;
                static const int MAP_SIZE = 1000;
            };

            HazelcastServer *PipeliningTest::instance = NULL;
            HazelcastClient *PipeliningTest::client = NULL;

            TEST_F(PipeliningTest, testConstructor_whenNegativeDepth) {
                ASSERT_THROW(Pipelining<string>::create(0), exception::IllegalArgumentException);
                ASSERT_THROW(Pipelining<string>::create(-1), exception::IllegalArgumentException);
            }

            TEST_F(PipeliningTest, add_whenNull) {
                boost::shared_ptr<Pipelining<string> > pipelining = Pipelining<string>::create(1);
                ASSERT_THROW(pipelining->add(boost::shared_ptr<ICompletableFuture<string> >()),
                             exception::NullPointerException);
            }

            TEST_F(PipeliningTest, testPipeliningFunctionalityDepthOne) {
                testPipelining(Pipelining<int>::create(1));
            }

            TEST_F(PipeliningTest, testPipeliningFunctionalityDepth10) {
                testPipelining(Pipelining<int>::create(10));
            }
        }
    }
}
