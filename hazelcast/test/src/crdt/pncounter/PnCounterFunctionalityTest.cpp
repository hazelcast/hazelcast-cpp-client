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
#include "HazelcastServerFactory.h"
#include "ClientTestSupport.h"
#include "HazelcastServer.h"



#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/HazelcastClient.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace crdt {
                namespace pncounter {
                    class PnCounterFunctionalityTest : public ClientTestSupport {
                    public:
                        static void SetUpTestCase() {
                            instance = new HazelcastServer(*g_srvFactory);
                            client = new HazelcastClient;
                        }

                        static void TearDownTestCase() {
                            delete client;
                            delete instance;

                            client = NULL;
                            instance = NULL;
                        }

                    protected:
                        class IncrementerTask : public util::Runnable {
                        public:
                            IncrementerTask(int loopsPerThread, std::atomic<int64_t> &finalValue,
                                            const std::shared_ptr<client::crdt::pncounter::PNCounter> &counter1,
                                            const std::shared_ptr<client::crdt::pncounter::PNCounter> &counter2)
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
                            std::atomic<int64_t> &finalValue;
                            std::shared_ptr<client::crdt::pncounter::PNCounter> counter1;
                            std::shared_ptr<client::crdt::pncounter::PNCounter> counter2;
                        };

                        static HazelcastServer *instance;
                        static HazelcastClient *client;
                    };

                    HazelcastServer *PnCounterFunctionalityTest::instance = NULL;
                    HazelcastClient *PnCounterFunctionalityTest::client = NULL;

                    TEST_F(PnCounterFunctionalityTest, testSimpleReplication) {
                        const char *name = testing::UnitTest::GetInstance()->current_test_info()->name();
                        std::shared_ptr<client::crdt::pncounter::PNCounter> counter1 = client->getPNCounter(name);
                        std::shared_ptr<client::crdt::pncounter::PNCounter> counter2 = client->getPNCounter(name);

                        assertEquals(5, counter1->addAndGet(5));

                        ASSERT_EQ_EVENTUALLY(5, counter1->get());
                        ASSERT_EQ_EVENTUALLY(5, counter2->get());
                    }

                    TEST_F(PnCounterFunctionalityTest, testParallelism) {
                        const char *name = testing::UnitTest::GetInstance()->current_test_info()->name();
                        std::shared_ptr<client::crdt::pncounter::PNCounter> counter1 = client->getPNCounter(name);
                        std::shared_ptr<client::crdt::pncounter::PNCounter> counter2 = client->getPNCounter(name);

                        int parallelism = 5;
                        int loopsPerThread = 100;
                        std::atomic<int64_t> finalValue(0);

                        std::vector<std::shared_ptr<util::Thread> > futures;
                        for (int i = 0; i < parallelism; i++) {
                            std::shared_ptr<Thread> thread = std::shared_ptr<util::Thread>(new util::Thread(std::shared_ptr<util::Runnable>(
                                                                new IncrementerTask(loopsPerThread, finalValue, counter1, counter2)), getLogger()));
                            thread->start();
                            futures.push_back(thread);
                        }

                        for (const std::shared_ptr<util::Thread> &future  : futures) {
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
