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
#include <vector>


#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include "ClientTestSupport.h"

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Sync.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class FlakeIdGeneratorApiTest : public ClientTestSupport {
            public:
                FlakeIdGeneratorApiTest() : flakeIdGenerator(
                        client->getFlakeIdGenerator(testing::UnitTest::GetInstance()->current_test_info()->name())) {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    ClientConfig clientConfig = getConfig();
                    std::shared_ptr<config::ClientFlakeIdGeneratorConfig> flakeIdConfig(
                            new config::ClientFlakeIdGeneratorConfig("test*"));
                    flakeIdConfig->setPrefetchCount(10).setPrefetchValidityMillis(20000);
                    clientConfig.addFlakeIdGeneratorConfig(flakeIdConfig);
                    client = new HazelcastClient(clientConfig);
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    client = NULL;
                    instance = NULL;
                }

            protected:
                class SmokeRunner : public util::Runnable {
                public:
                    SmokeRunner(FlakeIdGenerator &flakeIdGenerator,
                                util::CountDownLatch &startLatch, util::Sync<std::shared_ptr<set<int64_t> > > &ids)
                            : flakeIdGenerator(flakeIdGenerator), startLatch(startLatch), ids(ids) {}

                    virtual const string getName() const {
                        return "SmokeRunner";
                    }

                    virtual void run() {
                        std::shared_ptr<std::set<int64_t> > localIds = ids;
                        startLatch.await();
                        for (int j = 0; j < 100000; ++j) {
                            localIds->insert(flakeIdGenerator.newId());
                        }

                        ids = localIds;
                    }

                private:
                    FlakeIdGenerator &flakeIdGenerator;
                    util::CountDownLatch &startLatch;
                    util::Sync<std::shared_ptr<std::set<int64_t> > > &ids;
                };

                static HazelcastServer *instance;
                static HazelcastClient *client;

                FlakeIdGenerator flakeIdGenerator;
            };

            HazelcastServer *FlakeIdGeneratorApiTest::instance = NULL;
            HazelcastClient *FlakeIdGeneratorApiTest::client = NULL;

            TEST_F (FlakeIdGeneratorApiTest, testStartingValue) {
                flakeIdGenerator.newId();
            }

            TEST_F (FlakeIdGeneratorApiTest, testInit) {
                int64_t currentId = flakeIdGenerator.newId();
                        assertTrue(flakeIdGenerator.init(currentId / 2));
                        assertFalse(flakeIdGenerator.init(currentId * 2));
            }

            TEST_F (FlakeIdGeneratorApiTest, testSmoke) {
                util::CountDownLatch startLatch(1);
                std::vector<std::shared_ptr<util::Thread> > threads(4);
                util::Sync<std::shared_ptr<std::set<int64_t> > > ids[4];

                for (int i = 0; i < 4; ++i) {
                    ids[i] = std::shared_ptr<std::set<int64_t> >(new std::set<int64_t>());
                    std::shared_ptr<util::Thread> t(new util::Thread(std::shared_ptr<util::Runnable>(
                            new SmokeRunner(flakeIdGenerator, startLatch, ids[i])), getLogger()));
                    t->start();
                    threads[i] = t;
                }

                startLatch.countDown();

                for (int i = 0; i < 4; ++i) {
                    threads[i]->join();
                }

                std::set<int64_t> allIds;
                for (int i = 0; i < 4; ++i) {
                    std::shared_ptr<std::set<int64_t> > threadIdSet = ids[i];
                    for (const int64_t &value : *threadIdSet) {
                                    allIds.insert(value);
                                }
                }

                // if there were duplicate IDs generated, there will be less items in the set than expected
                        assertEquals(4 * 100000, (int) allIds.size());
            }
        }
    }
}
