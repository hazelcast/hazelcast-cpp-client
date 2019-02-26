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
// Created by sancar koyunlu on 8/27/13.
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/MultiMap.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientMultiMapTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    // clear mm
                    mm->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    clientConfig->addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                    client = new HazelcastClient(*clientConfig);
                    mm = new MultiMap<std::string, std::string>(client->getMultiMap<std::string, std::string>("MyMultiMap"));
                }

                static void TearDownTestCase() {
                    delete mm;
                    delete client;
                    delete clientConfig;
                    delete instance;

                    mm = NULL;
                    client = NULL;
                    clientConfig = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static ClientConfig *clientConfig;
                static HazelcastClient *client;
                static MultiMap<std::string, std::string> *mm;
            };

            HazelcastServer *ClientMultiMapTest::instance = NULL;
            ClientConfig *ClientMultiMapTest::clientConfig = NULL;
            HazelcastClient *ClientMultiMapTest::client = NULL;
            MultiMap<std::string, std::string> *ClientMultiMapTest::mm = NULL;

            TEST_F(ClientMultiMapTest, testPutGetRemove) {
                ASSERT_TRUE(mm->put("key1", "value1"));
                ASSERT_TRUE(mm->put("key1", "value2"));
                ASSERT_TRUE(mm->put("key1", "value3"));

                ASSERT_TRUE(mm->put("key2", "value4"));
                ASSERT_TRUE(mm->put("key2", "value5"));

                ASSERT_EQ(3, mm->valueCount("key1"));
                ASSERT_EQ(2, mm->valueCount("key2"));
                ASSERT_EQ(5, mm->size());

                std::vector<std::string> coll = mm->get("key1");
                ASSERT_EQ(3, (int)coll.size());

                coll = mm->remove("key2");
                ASSERT_EQ(2, (int)coll.size());
                ASSERT_EQ(0, mm->valueCount("key2"));
                ASSERT_EQ(0, (int)mm->get("key2").size());

                ASSERT_FALSE(mm->remove("key1", "value4"));
                ASSERT_EQ(3, mm->size());

                ASSERT_TRUE(mm->remove("key1", "value2"));
                ASSERT_EQ(2, mm->size());

                ASSERT_TRUE(mm->remove("key1", "value1"));
                ASSERT_EQ(1, mm->size());
                ASSERT_EQ("value3", mm->get("key1")[0]);
            }


            TEST_F(ClientMultiMapTest, testKeySetEntrySetAndValues) {
                ASSERT_TRUE(mm->put("key1", "value1"));
                ASSERT_TRUE(mm->put("key1", "value2"));
                ASSERT_TRUE(mm->put("key1", "value3"));

                ASSERT_TRUE(mm->put("key2", "value4"));
                ASSERT_TRUE(mm->put("key2", "value5"));


                ASSERT_EQ(2, (int)mm->keySet().size());
                ASSERT_EQ(5, (int)mm->values().size());
                ASSERT_EQ(5, (int)mm->entrySet().size());
            }


            TEST_F(ClientMultiMapTest, testContains) {
                ASSERT_TRUE(mm->put("key1", "value1"));
                ASSERT_TRUE(mm->put("key1", "value2"));
                ASSERT_TRUE(mm->put("key1", "value3"));

                ASSERT_TRUE(mm->put("key2", "value4"));
                ASSERT_TRUE(mm->put("key2", "value5"));

                ASSERT_FALSE(mm->containsKey("key3"));
                ASSERT_TRUE(mm->containsKey("key1"));

                ASSERT_FALSE(mm->containsValue("value6"));
                ASSERT_TRUE(mm->containsValue("value4"));

                ASSERT_FALSE(mm->containsEntry("key1", "value4"));
                ASSERT_FALSE(mm->containsEntry("key2", "value3"));
                ASSERT_TRUE(mm->containsEntry("key1", "value1"));
                ASSERT_TRUE(mm->containsEntry("key2", "value5"));
            }

            class MyMultiMapListener : public EntryAdapter<std::string, std::string>{
            public:
                MyMultiMapListener(util::CountDownLatch& addedLatch, util::CountDownLatch& removedLatch)
                        : addedLatch(addedLatch), removedLatch(removedLatch) {
                }

                void entryAdded(const EntryEvent<std::string, std::string>& event) {
                    addedLatch.countDown();
                }

                void entryRemoved(const EntryEvent<std::string, std::string>& event) {
                    removedLatch.countDown();
                }

            private:
                util::CountDownLatch& addedLatch;
                util::CountDownLatch& removedLatch;
            };

            TEST_F(ClientMultiMapTest, testListener) {
                util::CountDownLatch latch1Add(8);
                util::CountDownLatch latch1Remove(4);

                util::CountDownLatch latch2Add(3);
                util::CountDownLatch latch2Remove(3);

                MyMultiMapListener listener1(latch1Add, latch1Remove);
                MyMultiMapListener listener2(latch2Add, latch2Remove);

                std::string id1 = mm->addEntryListener(listener1, true);
                std::string id2 = mm->addEntryListener(listener2, "key3", true);

                mm->put("key1", "value1");
                mm->put("key1", "value2");
                mm->put("key1", "value3");
                mm->put("key2", "value4");
                mm->put("key2", "value5");

                mm->remove("key1", "value2");

                mm->put("key3", "value6");
                mm->put("key3", "value7");
                mm->put("key3", "value8");

                mm->remove("key3");

                ASSERT_TRUE(latch1Add.await(20));
                ASSERT_TRUE(latch1Remove.await(20));

                ASSERT_TRUE(latch2Add.await(20));
                ASSERT_TRUE(latch2Remove.await(20));

                ASSERT_TRUE(mm->removeEntryListener(id1));
                ASSERT_TRUE(mm->removeEntryListener(id2));

            }

            void lockThread(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                if (!mm->tryLock("key1")) {
                    latch->countDown();
                }
            }

            TEST_F(ClientMultiMapTest, testLock) {
                mm->lock("key1");
                util::CountDownLatch latch(1);
                util::StartedThread t(lockThread, mm, &latch);
                ASSERT_TRUE(latch.await(5));
                mm->forceUnlock("key1");
            }

            void lockTtlThread(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;

                if (!mm->tryLock("key1")) {
                    latch->countDown();
                }

                if (mm->tryLock("key1", 5 * 1000)) {
                    latch->countDown();
                }
            }

            TEST_F(ClientMultiMapTest, testLockTtl) {
                mm->lock("key1", 3 * 1000);
                util::CountDownLatch latch(2);
                util::StartedThread t(lockTtlThread, mm, &latch);
                ASSERT_TRUE(latch.await(10));
                mm->forceUnlock("key1");
            }


            void tryLockThread(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                try {
                    if (!mm->tryLock("key1", 2)) {
                        latch->countDown();
                    }
                } catch (...) {
                    std::cerr << "Unexpected exception at ClientMultiMapTest tryLockThread" << std::endl;
                }
            }

            void tryLockThread2(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                try {
                    if (mm->tryLock("key1", 20 * 1000)) {
                        latch->countDown();
                    }
                } catch (...) {
                    std::cerr << "Unexpected exception at ClientMultiMapTest lockThread2" << std::endl;
                }
            }

            TEST_F(ClientMultiMapTest, testTryLock) {
                ASSERT_TRUE(mm->tryLock("key1", 2 * 1000));
                util::CountDownLatch latch(1);
                util::StartedThread t(tryLockThread, mm, &latch);
                ASSERT_TRUE(latch.await(100));
                ASSERT_TRUE(mm->isLocked("key1"));

                util::CountDownLatch latch2(1);
                util::StartedThread t2(tryLockThread2, mm, &latch2);

                util::sleep(1);
                mm->unlock("key1");
                ASSERT_TRUE(latch2.await(100));
                ASSERT_TRUE(mm->isLocked("key1"));
                mm->forceUnlock("key1");
            }

            void forceUnlockThread(util::ThreadArgs& args) {
                MultiMap<std::string, std::string> *mm = (MultiMap<std::string, std::string> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                mm->forceUnlock("key1");
                latch->countDown();
            }

            TEST_F(ClientMultiMapTest, testForceUnlock) {
                mm->lock("key1");
                util::CountDownLatch latch(1);
                util::StartedThread t(forceUnlockThread, mm, &latch);
                ASSERT_TRUE(latch.await(100));
                ASSERT_FALSE(mm->isLocked("key1"));
            }
        }
    }
}

