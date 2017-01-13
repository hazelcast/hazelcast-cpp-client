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



#include "ClientTxnMultiMapTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/CountDownLatch.h"

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            ClientTxnMultiMapTest::ClientTxnMultiMapTest()
            : instance(*g_srvFactory)
            , client(getNewClient()) {
            }
            
            ClientTxnMultiMapTest::~ClientTxnMultiMapTest() {
            }

            void putGetRemoveTestThread(util::ThreadArgs& args) {
                util::ILogger &logger = util::ILogger::getLogger();
                logger.info("[ClientTxnMultiMapTest::putGetRemoveTestThread] Thread started.");
                
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string > *)args.arg0;
                HazelcastClient *client = (HazelcastClient *)args.arg1;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg2;
                std::string key = util::IOUtil::to_string(util::Thread::getThreadID());
                client->getMultiMap<std::string, std::string>("testPutGetRemove").put(key, "value");
                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalMultiMap<std::string, std::string> multiMap = context.getMultiMap<std::string, std::string >("testPutGetRemove");
                ASSERT_FALSE(multiMap.put(key, "value"));
                ASSERT_TRUE(multiMap.put(key, "value1"));
                ASSERT_TRUE(multiMap.put(key, "value2"));
                ASSERT_EQ(3, (int)multiMap.get(key).size());
                ASSERT_EQ(3, (int)multiMap.valueCount(key));
                context.commitTransaction();

                ASSERT_EQ(3, (int)mm->get(key).size());

                latch->countDown();

                logger.info("[ClientTxnMultiMapTest::putGetRemoveTestThread] Thread finished");
            }

            TEST_F(ClientTxnMultiMapTest, testRemoveIfExists) {
                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalMultiMap<std::string, std::string> multiMap = context.getMultiMap<std::string, std::string >("testRemoveIfExists");
                std::string key("MyKey");
                ASSERT_TRUE(multiMap.put(key, "value"));
                ASSERT_TRUE(multiMap.put(key, "value1"));
                ASSERT_TRUE(multiMap.put(key, "value2"));
                ASSERT_EQ(3, (int)multiMap.get(key).size());

                ASSERT_FALSE(multiMap.remove(key, "NonExistentValue"));
                ASSERT_TRUE(multiMap.remove(key, "value1"));

                ASSERT_EQ(2, multiMap.size());
                ASSERT_EQ(2, (int)multiMap.valueCount(key));

                context.commitTransaction();

                MultiMap<std::string, std::string> mm = client->getMultiMap<std::string, std::string >("testRemoveIfExists");
                ASSERT_EQ(2, (int)mm.get(key).size());
            }

            TEST_F(ClientTxnMultiMapTest, testPutGetRemove) {
                MultiMap<std::string, std::string > mm = client->getMultiMap<std::string, std::string>("testPutGetRemove");
                int n = 10;
                util::CountDownLatch latch(n);
                std::vector<util::Thread*> threads(n);
                for (int i = 0; i < n; i++) {
                    threads[i] = new util::Thread(putGetRemoveTestThread, &mm, client.get(), &latch);
                }
                ASSERT_TRUE(latch.await(1));
                for (int i = 0; i < n; i++) {
                    delete threads[i] ;
                }
            }
        }
    }
}

