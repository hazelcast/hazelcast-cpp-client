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
// Created by ihsan demir on24/3/16.
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"
#include <ClientTestSupport.h>
#include <HazelcastServer.h>

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalMultiMap.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace adaptor {
                class RawPointerTxnMultiMapTest : public ClientTestSupport {
                protected:
                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        clientConfig = new ClientConfig();
                        clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
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

                HazelcastServer *RawPointerTxnMultiMapTest::instance = NULL;
                ClientConfig *RawPointerTxnMultiMapTest::clientConfig = NULL;
                HazelcastClient *RawPointerTxnMultiMapTest::client = NULL;

                void putGetRemoveTestThread(util::ThreadArgs& args) {
                    MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string > *)args.arg0;
                    HazelcastClient *client = (HazelcastClient *)args.arg1;
                    util::CountDownLatch *latch = (util::CountDownLatch *)args.arg2;
                    std::string key = util::IOUtil::to_string(util::Thread::getThreadID());
                    client->getMultiMap<std::string, std::string>("testPutGetRemove").put(key, "value");
                    TransactionContext context = client->newTransactionContext();
                    context.beginTransaction();
                    TransactionalMultiMap<std::string, std::string> originalMultiMap = context.getMultiMap<std::string, std::string >("testPutGetRemove");
                    client::adaptor::RawPointerTransactionalMultiMap<std::string, std::string> multiMap(originalMultiMap);
                    ASSERT_FALSE(multiMap.put(key, "value"));
                    ASSERT_TRUE(multiMap.put(key, "value1"));
                    ASSERT_TRUE(multiMap.put(key, "value2"));
                    ASSERT_EQ(3, (int)multiMap.get(key)->size());
                    context.commitTransaction();

                    ASSERT_EQ(3, (int)mm->get(key).size());

                    latch->countDown();
                }

                TEST_F(RawPointerTxnMultiMapTest, testPutGetRemove) {
                    MultiMap<std::string, std::string> mm = client->getMultiMap<std::string, std::string >("testPutGetRemove");
                    int n = 10;
                    util::CountDownLatch latch(n);
                    std::vector<util::Thread*> threads(n);
                    for (int i = 0; i < n; i++) {
                        threads[i] = new util::Thread(putGetRemoveTestThread, &mm, client, &latch);
                    }
                    ASSERT_TRUE(latch.await(1));
                    for (int i = 0; i < n; i++) {
                        delete threads[i] ;
                    }
                }
            }
        }
    }
}

