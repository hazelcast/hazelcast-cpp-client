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

/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"

#include <string>
#include <HazelcastServer.h>
#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/EntryAdapter.h"

#include "hazelcast/client/mixedtype/MultiMap.h"

using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class MixedMultiMapTest : public ClientTestSupport {
            protected:
                class MyMultiMapListener : public MixedEntryListener {
                public:
                    MyMultiMapListener(util::CountDownLatch& addedLatch, util::CountDownLatch& removedLatch)
                            : addedLatch(addedLatch), removedLatch(removedLatch) {
                    }

                    virtual void entryUpdated(const MixedEntryEvent &event) {
                    }

                    virtual void entryEvicted(const MixedEntryEvent &event) {
                    }

                    virtual void entryExpired(const MixedEntryEvent &event) {
                    }

                    virtual void entryMerged(const MixedEntryEvent &event) {
                    }

                    virtual void mapEvicted(const MapEvent &event) {
                    }

                    virtual void mapCleared(const MapEvent &event) {
                    }

                    virtual void entryAdded(const MixedEntryEvent &event) {
                        addedLatch.countDown();

                    }

                    virtual void entryRemoved(const MixedEntryEvent &event) {
                        removedLatch.countDown();
                    }

                private:
                    util::CountDownLatch& addedLatch;
                    util::CountDownLatch& removedLatch;
                };

                static void lockTtlThread(util::ThreadArgs& args) {
                    mixedtype::MultiMap *map = (mixedtype::MultiMap *)args.arg0;
                    util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;

                    if (!map->tryLock<std::string>("key1")) {
                        latch->countDown();
                    }

                    if (map->tryLock<std::string>("key1", 5 * 1000)) {
                        latch->countDown();
                    }
                }

                static void forceUnlockThread(util::ThreadArgs& args) {
                    mixedtype::MultiMap *mm = (mixedtype::MultiMap *)args.arg0;
                    util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                    mm->forceUnlock<std::string>("key1");
                    latch->countDown();
                }

                static void lockThread(util::ThreadArgs& args) {
                    mixedtype::MultiMap *mm = (mixedtype::MultiMap *)args.arg0;
                    util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                    if (!mm->tryLock<std::string>("key1")) {
                        latch->countDown();
                    }
                }

                static void tryLockThread(util::ThreadArgs& args) {
                    mixedtype::MultiMap *mm = (mixedtype::MultiMap *)args.arg0;
                    util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                    try {
                        if (!mm->tryLock<std::string>("key1", 2)) {
                            latch->countDown();
                        }
                    } catch (...) {
                        std::cerr << "Unexpected exception at MixedMultiMapTest tryLockThread" << std::endl;
                    }
                }

                static void tryLockThread2(util::ThreadArgs& args) {
                    mixedtype::MultiMap *mm = (mixedtype::MultiMap *)args.arg0;
                    util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;
                    try {
                        if (mm->tryLock<std::string>("key1", 20 * 1000)) {
                            latch->countDown();
                        }
                    } catch (...) {
                        std::cerr << "Unexpected exception at MixedMultiMapTest lockThread2" << std::endl;
                    }
                }

                virtual void TearDown() {
                    // clear mm
                    mm->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    clientConfig = new ClientConfig();
                    client = new HazelcastClient(*clientConfig);
                    mm = new mixedtype::MultiMap(client->toMixedType().getMultiMap("MixedMultimapTestMap"));
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
                static mixedtype::MultiMap *mm;
            };

            HazelcastServer *MixedMultiMapTest::instance = NULL;
            ClientConfig *MixedMultiMapTest::clientConfig = NULL;
            HazelcastClient *MixedMultiMapTest::client = NULL;
            mixedtype::MultiMap *MixedMultiMapTest::mm = NULL;

            TEST_F(MixedMultiMapTest, testPutGetRemove) {
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value1")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value2")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value3")));

                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value4")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value5")));

                ASSERT_EQ(3, mm->valueCount<std::string>("key1"));
                ASSERT_EQ(2, mm->valueCount<std::string>("key2"));
                ASSERT_EQ(5, mm->size());

                std::vector<TypedData> coll = mm->get<std::string>("key1");
                ASSERT_EQ(3, (int)coll.size());

                coll = mm->remove<std::string>("key2");
                ASSERT_EQ(2, (int)coll.size());
                ASSERT_EQ(0, mm->valueCount<std::string>("key2"));
                ASSERT_EQ(0, (int)mm->get<std::string>("key2").size());

                ASSERT_FALSE((mm->remove<std::string, std::string>("key1", "value4")));
                ASSERT_EQ(3, mm->size());

                ASSERT_TRUE((mm->remove<std::string, std::string>("key1", "value2")));
                ASSERT_EQ(2, mm->size());

                ASSERT_TRUE((mm->remove<std::string, std::string>("key1", "value1")));
                ASSERT_EQ(1, mm->size());
                coll = mm->get<std::string>("key1");
                std::auto_ptr<std::string> val = coll[0].get<std::string>();
                ASSERT_NE((std::string *)NULL, val.get());
                ASSERT_EQ("value3", *val);
            }

            TEST_F(MixedMultiMapTest, testKeySetEntrySetAndValues) {
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value1")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value2")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value3")));

                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value4")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value5")));


                ASSERT_EQ(2, (int)mm->keySet().size());
                ASSERT_EQ(5, (int)mm->values().size());
                ASSERT_EQ(5, (int)mm->entrySet().size());
            }

            TEST_F(MixedMultiMapTest, testContains) {
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value1")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value2")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key1", "value3")));

                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value4")));
                ASSERT_TRUE((mm->put<std::string, std::string>("key2", "value5")));

                ASSERT_FALSE(mm->containsKey<std::string>("key3"));
                ASSERT_TRUE(mm->containsKey<std::string>("key1"));

                ASSERT_FALSE(mm->containsValue<std::string>("value6"));
                ASSERT_TRUE(mm->containsValue<std::string>("value4"));

                ASSERT_FALSE((mm->containsEntry<std::string, std::string>("key1", "value4")));
                ASSERT_FALSE((mm->containsEntry<std::string, std::string>("key2", "value3")));
                ASSERT_TRUE((mm->containsEntry<std::string, std::string>("key1", "value1")));
                ASSERT_TRUE((mm->containsEntry<std::string, std::string>("key2", "value5")));
            }

            TEST_F(MixedMultiMapTest, testListener) {
                util::CountDownLatch latch1Add(8);
                util::CountDownLatch latch1Remove(4);

                util::CountDownLatch latch2Add(3);
                util::CountDownLatch latch2Remove(3);

                MyMultiMapListener mmener1(latch1Add, latch1Remove);
                MyMultiMapListener mmener2(latch2Add, latch2Remove);

                std::string id1 = mm->addEntryListener(mmener1, true);
                std::string id2 = mm->addEntryListener<std::string>(mmener2, "key3", true);

                mm->put<std::string, std::string>("key1", "value1");
                mm->put<std::string, std::string>("key1", "value2");
                mm->put<std::string, std::string>("key1", "value3");
                mm->put<std::string, std::string>("key2", "value4");
                mm->put<std::string, std::string>("key2", "value5");

                mm->remove<std::string, std::string>("key1", "value2");

                mm->put<std::string, std::string>("key3", "value6");
                mm->put<std::string, std::string>("key3", "value7");
                mm->put<std::string, std::string>("key3", "value8");

                mm->remove<std::string>("key3");

                ASSERT_TRUE(latch1Add.await(20));
                ASSERT_TRUE(latch1Remove.await(20));

                ASSERT_TRUE(latch2Add.await(20));
                ASSERT_TRUE(latch2Remove.await(20));

                ASSERT_TRUE(mm->removeEntryListener(id1));
                ASSERT_TRUE(mm->removeEntryListener(id2));

            }

            TEST_F(MixedMultiMapTest, testLock) {
                mm->lock<std::string>("key1");
                util::CountDownLatch latch(1);
                util::Thread t(lockThread, mm, &latch);
                ASSERT_TRUE(latch.await(5));
                mm->forceUnlock<std::string>("key1");
                t.join();
            }

            TEST_F(MixedMultiMapTest, testLockTtl) {
                mm->lock<std::string>("key1", 3 * 1000);
                util::CountDownLatch latch(2);
                util::Thread t(lockTtlThread, mm, &latch);
                ASSERT_TRUE(latch.await(10));
                mm->forceUnlock<std::string>("key1");
                t.join();
            }

            TEST_F(MixedMultiMapTest, testTryLock) {
                ASSERT_TRUE(mm->tryLock<std::string>("key1", 2 * 1000));
                util::CountDownLatch latch(1);
                util::Thread t(tryLockThread, mm, &latch);
                ASSERT_TRUE(latch.await(100));
                ASSERT_TRUE(mm->isLocked<std::string>("key1"));

                util::CountDownLatch latch2(1);
                util::Thread t2(tryLockThread2, mm, &latch2);

                util::sleep(1);
                mm->unlock<std::string>("key1");
                ASSERT_TRUE(latch2.await(100));
                ASSERT_TRUE(mm->isLocked<std::string>("key1"));
                mm->forceUnlock<std::string>("key1");
            }

            TEST_F(MixedMultiMapTest, testForceUnlock) {
                mm->lock<std::string>("key1");
                util::CountDownLatch latch(1);
                util::Thread t(forceUnlockThread, mm, &latch);
                ASSERT_TRUE(latch.await(100));
                ASSERT_FALSE(mm->isLocked<std::string>("key1"));
            }
        }
    }
}
