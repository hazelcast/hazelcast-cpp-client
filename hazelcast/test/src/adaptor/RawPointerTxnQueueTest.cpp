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

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalQueue.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace adaptor {
                class RawPointerTxnQueueTest : public ClientTestSupport {
                protected:
                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(getConfig());
                    }

                    static void TearDownTestCase() {
                        delete client;
                        delete instance;

                        client = NULL;
                        instance = NULL;
                    }

                    static HazelcastServer *instance;
                    static HazelcastClient *client;
                };

                HazelcastServer *RawPointerTxnQueueTest::instance = NULL;
                HazelcastClient *RawPointerTxnQueueTest::client = NULL;

                TEST_F(RawPointerTxnQueueTest, testTransactionalOfferPoll1) {
                    std::string name = "defQueue";

                    TransactionContext context = client->newTransactionContext();
                    context.beginTransaction();
                    TransactionalQueue<std::string> queue = context.getQueue<std::string>(name);
                    client::adaptor::RawPointerTransactionalQueue<std::string> q(queue);
                    ASSERT_TRUE(q.offer("ali"));
                    std::unique_ptr<std::string> item = q.poll();
                    ASSERT_NE((std::string *)NULL, item.get());
                    ASSERT_EQ("ali", *item);
                    context.commitTransaction();
                    ASSERT_EQ(0, client->getQueue<std::string>(name).size());
                }

                void testTransactionalOfferPoll2Thread(util::ThreadArgs& args) {
                    util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                    HazelcastClient *client = (HazelcastClient *)args.arg1;
                    latch->await();
                    client->getQueue<std::string>("defQueue0").offer("item0");
                }

                TEST_F(RawPointerTxnQueueTest, testTransactionalOfferPoll2) {
                    util::CountDownLatch latch(1);
                    util::StartedThread t(testTransactionalOfferPoll2Thread, &latch, client);
                    TransactionContext context = client->newTransactionContext();
                    context.beginTransaction();
                    TransactionalQueue<std::string> queue0 = context.getQueue<std::string>("defQueue0");
                    client::adaptor::RawPointerTransactionalQueue<std::string> q0(queue0);
                    TransactionalQueue<std::string> queue1 = context.getQueue<std::string>("defQueue1");
                    client::adaptor::RawPointerTransactionalQueue<std::string> q1(queue1);
                    latch.countDown();
                    std::unique_ptr<std::string> item = q0.poll(10 * 1000);
                    ASSERT_NE((std::string *)NULL, item.get());
                    ASSERT_EQ("item0", *item);
                    ASSERT_TRUE(q1.offer(*item));

                    ASSERT_NO_THROW(context.commitTransaction());

                    ASSERT_EQ(0, client->getQueue<std::string>("defQueue0").size());
                    ASSERT_EQ("item0", *(client->getQueue<std::string>("defQueue1").poll()));
                }
            }
        }
    }
}

