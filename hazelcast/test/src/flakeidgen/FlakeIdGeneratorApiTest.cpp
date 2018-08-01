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

#include <vector>
#include <boost/foreach.hpp>

#include "HazelcastServer.h"
#include "ClientTestSupport.h"

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"

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
                    clientConfig = getConfig().release();
                    boost::shared_ptr<config::ClientFlakeIdGeneratorConfig> flakeIdConfig(
                            new config::ClientFlakeIdGeneratorConfig("test*"));
                    flakeIdConfig->setPrefetchCount(10).setPrefetchValidityMillis(20000);
                    clientConfig->addFlakeIdGeneratorConfig(flakeIdConfig);
                    client = new HazelcastClient(*clientConfig);
                }

                static void TearDownTestCase() {
                    delete client;
                    delete clientConfig;
                    delete instance;

                    client = NULL;
                    clientConfig = NULL;
                    instance = NULL;
                }

            protected:
                class SmokeRunner : public util::Runnable {
                public:
                    SmokeRunner(FlakeIdGenerator &flakeIdGenerator,
                                util::CountDownLatch &startLatch, util::Atomic<boost::shared_ptr<set<int64_t> > > &ids)
                            : flakeIdGenerator(flakeIdGenerator), startLatch(startLatch), ids(ids) {}

                    virtual const string getName() const {
                        return "SmokeRunner";
                    }

                    virtual void run() {
                        boost::shared_ptr<std::set<int64_t> > localIds = ids;
                        startLatch.await();
                        for (int j = 0; j < 100000; ++j) {
                            localIds->insert(flakeIdGenerator.newId());
                        }

                        ids = localIds;
                    }

                private:
                    FlakeIdGenerator &flakeIdGenerator;
                    util::CountDownLatch &startLatch;
                    util::Atomic<boost::shared_ptr<std::set<int64_t> > > &ids;
                };

                static HazelcastServer *instance;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;

                FlakeIdGenerator flakeIdGenerator;
            };

            HazelcastServer *FlakeIdGeneratorApiTest::instance = NULL;
            ClientConfig *FlakeIdGeneratorApiTest::clientConfig = NULL;
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
                std::vector<boost::shared_ptr<util::Thread> > threads(4);
                util::Atomic<boost::shared_ptr<std::set<int64_t> > > ids[4];

                for (int i = 0; i < 4; ++i) {
                    ids[i] = boost::shared_ptr<std::set<int64_t> >(new std::set<int64_t>());
                    boost::shared_ptr<util::Thread> t(new util::Thread(boost::shared_ptr<util::Runnable>(
                            new SmokeRunner(flakeIdGenerator, startLatch, ids[i]))));
                    t->start();
                    threads[i] = t;
                }

                startLatch.countDown();

                for (int i = 0; i < 4; ++i) {
                    threads[i]->join();
                }

                std::set<int64_t> allIds;
                for (int i = 0; i < 4; ++i) {
                    boost::shared_ptr<std::set<int64_t> > threadIdSet = ids[i];
                    BOOST_FOREACH(const int64_t &value, *threadIdSet) {
                                    allIds.insert(value);
                                }
                }

                // if there were duplicate IDs generated, there will be less items in the set than expected
                        assertEquals(4 * 100000, (int) allIds.size());
            }
        }
    }
}
