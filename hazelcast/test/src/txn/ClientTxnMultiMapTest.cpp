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
//
// Created by sancar koyunlu on 9/18/13.
#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnMultiMapTest : public ClientTestSupport {
            public:
                ClientTxnMultiMapTest();

                ~ClientTxnMultiMapTest();

            protected:
                class PutGetRemoveTestTask : public util::Runnable {
                public:
                    PutGetRemoveTestTask(HazelcastClient &client, MultiMap<std::string, std::string> &mm,
                                         util::CountDownLatch &latch) : client(client), mm(mm), latch(latch) {}

                    virtual void run() {
                        std::string key = util::IOUtil::to_string(util::getCurrentThreadId());
                        std::string key2 = key + "2";
                        client.getMultiMap<std::string, std::string>("testPutGetRemove").put(key, "value");
                        TransactionContext context = client.newTransactionContext();
                        context.beginTransaction();
                        TransactionalMultiMap<std::string, std::string> multiMap = context.getMultiMap<std::string, std::string>(
                                "testPutGetRemove");
                        ASSERT_FALSE(multiMap.put(key, "value"));
                        ASSERT_TRUE(multiMap.put(key, "value1"));
                        ASSERT_TRUE(multiMap.put(key, "value2"));
                        ASSERT_TRUE(multiMap.put(key2, "value21"));
                        ASSERT_TRUE(multiMap.put(key2, "value22"));
                        ASSERT_EQ(3, (int) multiMap.get(key).size());
                        ASSERT_EQ(3, (int) multiMap.valueCount(key));
                        vector<std::string> removedValues = multiMap.remove(key2);
                        ASSERT_EQ(2U, removedValues.size())  ;
                        ASSERT_TRUE((removedValues[0] == "value21" && removedValues[1] == "value22") ||
                                    (removedValues[1] == "value21" && removedValues[0] == "value22"));
                        context.commitTransaction();

                        ASSERT_EQ(3, (int) mm.get(key).size());

                        latch.countDown();
                    }

                    virtual const string getName() const {
                        return "PutGetRemoveTestTask";
                    }

                private:
                    HazelcastClient &client;
                    MultiMap<std::string, std::string> &mm;
                    util::CountDownLatch &latch;
                };

                HazelcastServer instance;
                HazelcastClient client;
            };

            ClientTxnMultiMapTest::ClientTxnMultiMapTest()
                    : instance(*g_srvFactory), client(getNewClient()) {
            }

            ClientTxnMultiMapTest::~ClientTxnMultiMapTest() {
            }

            TEST_F(ClientTxnMultiMapTest, testRemoveIfExists) {
                TransactionContext context = client.newTransactionContext();
                context.beginTransaction();
                TransactionalMultiMap<std::string, std::string> multiMap = context.getMultiMap<std::string, std::string>(
                        "testRemoveIfExists");
                std::string key("MyKey");
                ASSERT_TRUE(multiMap.put(key, "value"));
                ASSERT_TRUE(multiMap.put(key, "value1"));
                ASSERT_TRUE(multiMap.put(key, "value2"));
                ASSERT_EQ(3, (int) multiMap.get(key).size());

                ASSERT_FALSE(multiMap.remove(key, "NonExistentValue"));
                ASSERT_TRUE(multiMap.remove(key, "value1"));

                ASSERT_EQ(2, multiMap.size());
                ASSERT_EQ(2, (int) multiMap.valueCount(key));

                context.commitTransaction();

                MultiMap<std::string, std::string> mm = client.getMultiMap<std::string, std::string>(
                        "testRemoveIfExists");
                ASSERT_EQ(2, (int) mm.get(key).size());
            }

            TEST_F(ClientTxnMultiMapTest, testPutGetRemove) {
                MultiMap<std::string, std::string> mm = client.getMultiMap<std::string, std::string>(
                        "testPutGetRemove");
                int n = 10;
                util::CountDownLatch latch(n);

                std::vector<std::shared_ptr<util::Thread> > allThreads;
                for (int i = 0; i < n; i++) {
                    std::shared_ptr<util::Thread> t(
                            new util::Thread(
                                    std::shared_ptr<util::Runnable>(new PutGetRemoveTestTask(client, mm, latch)),
                                    getLogger()));
                    t->start();
                    allThreads.push_back(t);
                }
                ASSERT_OPEN_EVENTUALLY(latch);
            }
        }
    }
}

