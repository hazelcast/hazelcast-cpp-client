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
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalMultiMap.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace adaptor {
                class RawPointerTxnMultiMapTest : public ClientTestSupport {
                protected:
                    static void SetUpTestCase() {
                        instance = new HazelcastServer(*g_srvFactory);
                        client = new HazelcastClient(getConfig());
                    }

                    static void TearDownTestCase() {
                        delete client;
                        delete clientConfig;
                        delete instance;

                        client = NULL;
                        clientConfig = NULL;
                        instance = NULL;
                    }

                    class GetRemoveTestTask : public util::Runnable {
                    public:
                        GetRemoveTestTask(MultiMap<string, string> &mm, util::CountDownLatch &latch) : mm(mm),
                                                                                                       latch(latch) {}

                        virtual void run() {
                            std::string key = util::IOUtil::to_string(util::getCurrentThreadId());
                            client->getMultiMap<std::string, std::string>("testPutGetRemove").put(key, "value");
                            TransactionContext context = client->newTransactionContext();
                            context.beginTransaction();
                            TransactionalMultiMap<std::string, std::string> originalMultiMap = context.getMultiMap<std::string, std::string>(
                                    "testPutGetRemove");
                            client::adaptor::RawPointerTransactionalMultiMap<std::string, std::string> multiMap(
                                    originalMultiMap);
                            ASSERT_FALSE(multiMap.put(key, "value"));
                            ASSERT_TRUE(multiMap.put(key, "value1"));
                            ASSERT_TRUE(multiMap.put(key, "value2"));
                            ASSERT_EQ(3, (int) multiMap.get(key)->size());
                            context.commitTransaction();

                            ASSERT_EQ(3, (int) mm.get(key).size());

                            latch.countDown();
                        }

                        virtual const string getName() const {
                            return "GetRemoveTestTask";
                        }

                    private:
                        MultiMap<std::string, std::string> &mm;
                        util::CountDownLatch &latch;
                    };

                    static HazelcastServer *instance;
                    static ClientConfig *clientConfig;
                    static HazelcastClient *client;
                };

                HazelcastServer *RawPointerTxnMultiMapTest::instance = NULL;
                ClientConfig *RawPointerTxnMultiMapTest::clientConfig = NULL;
                HazelcastClient *RawPointerTxnMultiMapTest::client = NULL;

                TEST_F(RawPointerTxnMultiMapTest, testPutGetRemove) {
                    MultiMap<std::string, std::string> mm = client->getMultiMap<std::string, std::string>(
                            "testPutGetRemove");
                    int n = 10;
                    util::CountDownLatch latch(n);

                    std::vector<std::shared_ptr<util::Thread> > allThreads;
                    for (int i = 0; i < n; i++) {
                        std::shared_ptr<util::Thread> t(
                                new util::Thread(std::shared_ptr<util::Runnable>(new GetRemoveTestTask(mm, latch)),
                                        getLogger()));
                        t->start();
                        allThreads.push_back(t);
                    }
                    ASSERT_OPEN_EVENTUALLY(latch);
                }
            }
        }
    }
}

