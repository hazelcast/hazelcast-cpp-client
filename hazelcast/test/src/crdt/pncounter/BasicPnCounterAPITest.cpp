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

#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/HazelcastClient.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace crdt {
                namespace pncounter {
                    class BasicPnCounterAPITest : public ClientTestSupport {
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

                        static HazelcastServer *instance;
                        static ClientConfig *clientConfig;
                        static HazelcastClient *client;
                    };

                    HazelcastServer *BasicPnCounterAPITest::instance = NULL;
                    ClientConfig *BasicPnCounterAPITest::clientConfig = NULL;
                    HazelcastClient *BasicPnCounterAPITest::client = NULL;

                    TEST_F(BasicPnCounterAPITest, testGetStart) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndAdd) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->getAndAdd(5));
                    }

                    TEST_F(BasicPnCounterAPITest, testAddAndGet) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(5, pnCounter->addAndGet(5));
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndAddExisting) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());

                                assertEquals(0, pnCounter->getAndAdd(2));
                                assertEquals(2, pnCounter->getAndAdd(3));
                                assertEquals(5, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndIncrement) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->getAndIncrement());
                                assertEquals(1, pnCounter->getAndIncrement());
                                assertEquals(2, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testIncrementAndGet) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(1, pnCounter->incrementAndGet());
                                assertEquals(1, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndDecrementFromDefault) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->getAndDecrement());
                                assertEquals(-1, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndDecrement) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(1, pnCounter->incrementAndGet());
                                assertEquals(1, pnCounter->getAndDecrement());
                                assertEquals(0, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testGetAndSubtract) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(0, pnCounter->getAndSubtract(2));
                                assertEquals(-2, pnCounter->get());
                    }

                    TEST_F(BasicPnCounterAPITest, testSubtractAndGet) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                assertEquals(-3, pnCounter->subtractAndGet(3));
                    }

                    TEST_F(BasicPnCounterAPITest, testReset) {
                        boost::shared_ptr<client::crdt::pncounter::PNCounter> pnCounter = client->getPNCounter(
                                testing::UnitTest::GetInstance()->current_test_info()->name());
                                pnCounter->reset();
                    }
                }
            }

        }
    }
}
