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

#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include <boost/foreach.hpp>

#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/util/Atomic.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace crdt {
                namespace pncounter {
                    class PnCounterFunctionalityTest : public ClientTestSupport {
                    public:
                        static void SetUpTestCase() {
                            instance = new HazelcastServer(*g_srvFactory);
                            clientConfig = new ClientConfig();
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
                        class IncrementerTask : public util::Runnable {
                        public:
                            IncrementerTask(int loopsPerThread, util::Atomic<int64_t> &finalValue,
                                            const boost::shared_ptr<client::crdt::pncounter::PNCounter> &counter1,
                                            const boost::shared_ptr<client::crdt::pncounter::PNCounter> &counter2)
                                    : loopsPerThread(loopsPerThread), finalValue(finalValue), counter1(counter1),
                                      counter2(counter2) {}

                            virtual const string getName() const {
                                return "IncrementerTask";
                            }

                            virtual void run() {
                                for (int j = 0; j < loopsPerThread; j++) {
                                    counter1->addAndGet(5);
                                    finalValue += 5;
                                    counter2->addAndGet(-2);
                                    finalValue += -2;
                                }
                            }

                        private:
                            int loopsPerThread;
                            util::Atomic<int64_t> &finalValue;
                            boost::shared_ptr<client::crdt::pncounter::PNCounter> counter1;
                            boost::shared_ptr<client::crdt::pncounter::PNCounter> counter2;
                        };

                        static HazelcastServer *instance;
                        static ClientConfig *clientConfig;
                        static HazelcastClient *client;
                    };

                    HazelcastServer *PnCounterFunctionalityTest::instance = NULL;
                    ClientConfig *PnCounterFunctionalityTest::clientConfig = NULL;
                    HazelcastClient *PnCounterFunctionalityTest::client = NULL;

                    TEST_F(PnCounterFunctionalityTest, testSimpleReplication) {
                        const char *name = testing::UnitTest::GetInstance()->current_test_info()->name();
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> counter1 = client->getPNCounter(name);
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> counter2 = client->getPNCounter(name);

                        assertEquals(5, counter1->addAndGet(5));

                        ASSERT_EQ_EVENTUALLY(5, counter1->get());
                        ASSERT_EQ_EVENTUALLY(5, counter2->get());
                    }

                    TEST_F(PnCounterFunctionalityTest, testParallelism) {
                        const char *name = testing::UnitTest::GetInstance()->current_test_info()->name();
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> counter1 = client->getPNCounter(name);
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> counter2 = client->getPNCounter(name);

                        int parallelism = 5;
                        int loopsPerThread = 100;
                        util::Atomic<int64_t> finalValue(0);

                        std::vector<boost::shared_ptr<util::Thread> > futures;
                        for (int i = 0; i < parallelism; i++) {
                            boost::shared_ptr<Thread> thread = boost::shared_ptr<util::Thread>(new util::Thread(boost::shared_ptr<util::Runnable>(
                                                                new IncrementerTask(loopsPerThread, finalValue, counter1, counter2)), getLogger()));
                            thread->start();
                            futures.push_back(thread);
                        }

                        BOOST_FOREACH(const boost::shared_ptr<util::Thread> &future , futures) {
                            future->join();
                        }

                        int64_t finalExpectedValue = 3 * (int64_t) loopsPerThread * parallelism;
                        ASSERT_EQ_EVENTUALLY(finalExpectedValue, counter1->get());
                        ASSERT_EQ_EVENTUALLY(finalExpectedValue, counter2->get());
                    }

                }
            }

        }
    }
}
