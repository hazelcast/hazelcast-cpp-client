/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 9/18/13.

#include "ClientTxnQueueTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace test {
            ClientTxnQueueTest::ClientTxnQueueTest()
            : instance(*g_srvFactory)
            , client(getNewClient()) {
            }
            
            ClientTxnQueueTest::~ClientTxnQueueTest() {
            }

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPoll1) {
                std::string name = "defQueue";

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalQueue<std::string> q = context.getQueue<std::string>(name);
                ASSERT_TRUE(q.offer("ali"));
                ASSERT_EQ(1, q.size());
                ASSERT_EQ("ali", *(q.poll()));
                ASSERT_EQ(0, q.size());
                context.commitTransaction();
                ASSERT_EQ(0, client->getQueue<std::string>(name).size());
            }

            void testTransactionalOfferPoll2Thread(util::ThreadArgs& args) {
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg0;
                HazelcastClient *client = (HazelcastClient *)args.arg1;                
                latch->await();
                client->getQueue<std::string>("defQueue0").offer("item0");
            }

            TEST_F(ClientTxnQueueTest, testTransactionalOfferPoll2) {
                util::CountDownLatch latch(1);
                util::Thread t(testTransactionalOfferPoll2Thread, &latch, client.get());
                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalQueue<std::string> q0 = context.getQueue<std::string>("defQueue0");
                TransactionalQueue<std::string> q1 = context.getQueue<std::string>("defQueue1");
                hazelcast::util::SharedPtr<std::string> s;
                latch.countDown();
                s = q0.poll(10 * 1000);
                ASSERT_EQ("item0", *s);
                ASSERT_TRUE(q1.offer(*s));

                ASSERT_NO_THROW(context.commitTransaction());

                ASSERT_EQ(0, client->getQueue<std::string>("defQueue0").size());
                ASSERT_EQ("item0", *(client->getQueue<std::string>("defQueue1").poll()));
            }
        }
    }
}

