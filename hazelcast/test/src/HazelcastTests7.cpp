/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include<boost/thread/barrier.hpp>

#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include "ClientTestSupport.h"
#include <regex>
#include <vector>
#include "hazelcast/client/EntryListener.h"
#include "ringbuffer/StartsWithStringFilter.h"
#include "ClientTestSupportBase.h"
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/exception/IllegalStateException.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/impl/Partition.h>
#include <hazelcast/client/spi/impl/ClientInvocation.h>
#include <gtest/gtest.h>
#include <thread>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/connection/Connection.h>
#include <ClientTestSupport.h>
#include <memory>
#include <hazelcast/client/proxy/PNCounterImpl.h>
#include <hazelcast/client/serialization/pimpl/DataInput.h>
#include <hazelcast/util/AddressUtil.h>
#include <hazelcast/client/serialization/pimpl/DataOutput.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/client/exception/IOException.h>
#include <hazelcast/client/protocol/ClientExceptionFactory.h>
#include <hazelcast/util/IOUtil.h>
#include <ClientTestSupportBase.h>
#include <hazelcast/util/Util.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <hazelcast/util/ILogger.h>
#include <ctime>
#include <cerrno>
#include <hazelcast/client/LifecycleListener.h>
#include "serialization/Serializables.h"
#include <hazelcast/client/SerializationConfig.h>
#include <hazelcast/client/HazelcastJsonValue.h>
#include <cstdint>
#include <hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <HazelcastServer.h>
#include "TestHelperFunctions.h"
#include <cmath>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h>
#include <iostream>
#include <string>
#include "executor/tasks/Tasks.h"
#include <hazelcast/client/serialization/serialization.h>
#include <cstdlib>
#include <fstream>
#include <boost/asio.hpp>
#include <cassert>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/connection/Connection.h"

#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/Pipelining.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientMultiMapTest : public ClientTestSupport {
            protected:

                EntryListener makeAddRemoveListener(boost::latch &addedLatch, boost::latch &removedLatch) {
                    return EntryListener().
                        on_added([&addedLatch](EntryEvent &&) {
                            addedLatch.count_down();
                        }).
                        on_removed([&removedLatch](EntryEvent &&) {
                            removedLatch.count_down();
                        });
                }

                static void fillData() {
                    ASSERT_TRUE(mm->put("key1", "value1").get());
                    ASSERT_TRUE(mm->put("key1", "value2").get());
                    ASSERT_TRUE(mm->put("key1", "value3").get());
                    ASSERT_TRUE(mm->put("key2", "value4").get());
                    ASSERT_TRUE(mm->put("key2", "value5").get());
                }
                
                void TearDown() override {
                    // clear mm
                    mm->clear().get();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    mm = client->getMultiMap("MyMultiMap");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    mm = nullptr;
                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<MultiMap> mm;
            };

            HazelcastServer *ClientMultiMapTest::instance = nullptr;
            HazelcastClient *ClientMultiMapTest::client = nullptr;
            std::shared_ptr<MultiMap> ClientMultiMapTest::mm;

            TEST_F(ClientMultiMapTest, testPutGetRemove) {
                fillData();
                ASSERT_EQ(3, mm->valueCount("key1").get());
                ASSERT_EQ(2, mm->valueCount("key2").get());
                ASSERT_EQ(5, mm->size().get());

                auto coll = mm->get<std::string, std::string>("key1").get();
                ASSERT_EQ(3, (int) coll.size());

                coll = mm->remove<std::string, std::string>("key2").get();
                ASSERT_EQ(2, (int) coll.size());
                ASSERT_EQ(0, mm->valueCount("key2").get());
                ASSERT_EQ(0, ((int) mm->get<std::string, std::string>("key2").get().size()));

                ASSERT_FALSE(mm->remove("key1", "value4").get());
                ASSERT_EQ(3, mm->size().get());

                ASSERT_TRUE(mm->remove("key1", "value2").get());
                ASSERT_EQ(2, mm->size().get());

                ASSERT_TRUE(mm->remove("key1", "value1").get());
                ASSERT_EQ(1, mm->size().get());
                ASSERT_EQ("value3", (mm->get<std::string, std::string>("key1").get()[0]));
            }


            TEST_F(ClientMultiMapTest, testKeySetEntrySetAndValues) {
                fillData();
                ASSERT_EQ(2, (int) mm->keySet<std::string>().get().size());
                ASSERT_EQ(5, (int) mm->values<std::string>().get().size());
                ASSERT_EQ(5, ((int) mm->entrySet<std::string, std::string>().get().size()));
            }


            TEST_F(ClientMultiMapTest, testContains) {
                fillData();
                ASSERT_FALSE(mm->containsKey<std::string>("key3").get());
                ASSERT_TRUE(mm->containsKey<std::string>("key1").get());

                ASSERT_FALSE(mm->containsValue<std::string>("value6").get());
                ASSERT_TRUE(mm->containsValue<std::string>("value4").get());

                ASSERT_FALSE(mm->containsEntry("key1", "value4").get());
                ASSERT_FALSE(mm->containsEntry("key2", "value3").get());
                ASSERT_TRUE(mm->containsEntry("key1", "value1").get());
                ASSERT_TRUE(mm->containsEntry("key2", "value5").get());
            }

            TEST_F(ClientMultiMapTest, testListener) {
                boost::latch latch1Add(8);
                boost::latch latch1Remove(4);
                boost::latch latch2Add(3);
                boost::latch latch2Remove(3);
                auto listener1 = makeAddRemoveListener(latch1Add, latch1Remove);
                auto listener2 = makeAddRemoveListener(latch2Add, latch2Remove);

                auto id1 = mm->addEntryListener(std::move(listener1), true).get();
                auto id2 = mm->addEntryListener(std::move(listener2), "key3", true).get();

                fillData();

                mm->remove("key1", "value2").get();

                mm->put("key3", "value6").get();
                mm->put("key3", "value7").get();
                mm->put("key3", "value8").get();

                mm->remove<std::string, std::string>("key3").get();

                ASSERT_OPEN_EVENTUALLY(latch1Add);
                ASSERT_OPEN_EVENTUALLY(latch1Remove);
                ASSERT_OPEN_EVENTUALLY(latch2Add);
                ASSERT_OPEN_EVENTUALLY(latch2Remove);

                ASSERT_TRUE(mm->removeEntryListener(id1).get());
                ASSERT_TRUE(mm->removeEntryListener(id2).get());
            }

            TEST_F(ClientMultiMapTest, testLock) {
                mm->lock("key1").get();
                boost::latch latch1(1);
                std::thread([&]() {
                    if (!mm->tryLock("key1").get()) {
                        latch1.count_down();
                    }
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                mm->forceUnlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testLockTtl) {
                mm->lock("key1", std::chrono::seconds(2)).get();
                boost::latch latch1(2);
                std::thread([&]() {
                    if (!mm->tryLock("key1").get()) {
                        latch1.count_down();
                    }
                    if (mm->tryLock("key1", std::chrono::seconds(5)).get()) {
                        latch1.count_down();
                    }
                }).detach();

                ASSERT_OPEN_EVENTUALLY(latch1);
                mm->forceUnlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testTryLock) {
                ASSERT_TRUE(mm->tryLock("key1", std::chrono::seconds(2)).get());
                boost::latch latch1(1);
                std::thread([&]() {
                    if (!mm->tryLock("key1", std::chrono::milliseconds(500)).get()) {
                        latch1.count_down();
                    }
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_TRUE(mm->isLocked("key1").get());

                boost::latch latch2(1);
                boost::barrier b(2);
                std::thread([&]() {
                    b.count_down_and_wait();
                    if (mm->tryLock("key1", std::chrono::seconds(20)).get()) {
                        latch2.count_down();
                    }
                }).detach();
                b.count_down_and_wait();
                mm->unlock("key1").get();
                ASSERT_OPEN_EVENTUALLY(latch2);
                ASSERT_TRUE(mm->isLocked("key1").get());
                mm->forceUnlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testForceUnlock) {
                mm->lock("key1").get();
                boost::latch latch1(1);
                std::thread([&]() {
                    mm->forceUnlock("key1").get();
                    latch1.count_down();
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_FALSE(mm->isLocked("key1").get());
            }

            TEST_F(ClientMultiMapTest, testTryLockTtl) {
                ASSERT_TRUE(mm->tryLock("key1", std::chrono::seconds(2), std::chrono::seconds(1)).get());
                boost::latch latch1(1);
                std::thread([&]() {
                    if (!mm->tryLock("key1", std::chrono::milliseconds(500)).get()) {
                        latch1.count_down();
                    }
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_TRUE(mm->isLocked("key1").get());

                boost::latch latch2(1);
                boost::barrier b(2);
                std::thread([&]() {
                    b.count_down_and_wait();
                    if (mm->tryLock("key1", std::chrono::seconds(20)).get()) {
                        latch2.count_down();
                    }
                }).detach();
                b.count_down_and_wait();
                mm->unlock("key1").get();
                ASSERT_OPEN_EVENTUALLY(latch2);
                ASSERT_TRUE(mm->isLocked("key1").get());
                mm->forceUnlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testTryLockTtlTimeout) {
                ASSERT_TRUE(mm->tryLock("key1", std::chrono::seconds(1), std::chrono::seconds(200)).get());
                boost::latch latch1(1);
                std::thread([&]() {
                    if (!mm->tryLock("key1", std::chrono::seconds(2)).get()) {
                        latch1.count_down();
                    }
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_TRUE(mm->isLocked("key1").get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientListTest : public ClientTestSupport {
            protected:

                void TearDown() override {
                    // clear list
                    list->clear().get();
                }

                static void SetUpTestCase() {
#ifdef HZ_BUILD_WITH_SSL
                    sslFactory = new HazelcastServerFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                    instance = new HazelcastServer(*sslFactory);
#else
                    instance = new HazelcastServer(*g_srvFactory);
#endif

#ifdef HZ_BUILD_WITH_SSL
                    ClientConfig clientConfig = getConfig(true);
                    clientConfig.getNetworkConfig().getSSLConfig().setCipherList("HIGH");
#else
                    ClientConfig clientConfig = getConfig();
#endif // HZ_BUILD_WITH_SSL
                    client = new HazelcastClient(clientConfig);
                    list = client->getList("MyList");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;
                    delete sslFactory;

                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<IList> list;
                static HazelcastServerFactory *sslFactory;
            };

            std::shared_ptr<IList> ClientListTest::list;
            HazelcastServer *ClientListTest::instance = nullptr;
            HazelcastClient *ClientListTest::client = nullptr;
            HazelcastServerFactory *ClientListTest::sslFactory = nullptr;

            TEST_F(ClientListTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_TRUE(list->addAll(l).get());

                ASSERT_TRUE(list->addAll(1, l).get());
                ASSERT_EQ(4, list->size().get());

                auto item = list->get<std::string>(0).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());
                item = list->get<std::string>(1).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());
                item = list->get<std::string>(2).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item2", item.value());
                item = list->get<std::string>(3).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item2", item.value());
            }

            TEST_F(ClientListTest, testAddSetRemove) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                list->add(0, "item3").get();
                ASSERT_EQ(3, list->size().get());
                auto temp = list->set<std::string>(2, "item4").get();
                ASSERT_EQ("item2", temp.value());

                ASSERT_EQ(3, list->size().get());
                ASSERT_EQ("item3", list->get<std::string>(0).get().value());
                ASSERT_EQ("item1", list->get<std::string>(1).get().value());
                ASSERT_EQ("item4", list->get<std::string>(2).get().value());

                ASSERT_FALSE(list->remove("item2").get());
                ASSERT_TRUE(list->remove("item3").get());

                temp = list->remove<std::string>(1).get();
                ASSERT_TRUE(temp.has_value());
                ASSERT_EQ("item4", temp.value());

                ASSERT_EQ(1, list->size().get());
                ASSERT_EQ("item1", list->get<std::string>(0).get().value());
            }

            TEST_F(ClientListTest, testIndexOf) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                ASSERT_EQ(-1, list->indexOf("item5").get());
                ASSERT_EQ(0, list->indexOf("item1").get());

                ASSERT_EQ(-1, list->lastIndexOf("item6").get());
                ASSERT_EQ(2, list->lastIndexOf("item1").get());
            }

            TEST_F(ClientListTest, testToArray) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                std::vector<std::string> ar = list->toArray<std::string>().get();

                ASSERT_EQ("item1", ar[0]);
                ASSERT_EQ("item2", ar[1]);
                ASSERT_EQ("item1", ar[2]);
                ASSERT_EQ("item4", ar[3]);

                std::vector<std::string> arr2 = list->subList<std::string>(1, 3).get();

                ASSERT_EQ(2, (int) arr2.size());
                ASSERT_EQ("item2", arr2[0]);
                ASSERT_EQ("item1", arr2[1]);
            }

            TEST_F(ClientListTest, testContains) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                ASSERT_FALSE(list->contains("item3").get());
                ASSERT_TRUE(list->contains("item2").get());

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_FALSE(list->containsAll(l).get());
                ASSERT_TRUE(list->add("item3").get());
                ASSERT_TRUE(list->containsAll(l).get());
            }

            TEST_F(ClientListTest, testRemoveRetainAll) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(list->removeAll(l).get());
                ASSERT_EQ(3, (int) list->size().get());
                ASSERT_FALSE(list->removeAll(l).get());
                ASSERT_EQ(3, (int) list->size().get());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(list->retainAll(l).get());
                ASSERT_EQ(3, (int) list->size().get());

                l.clear();
                ASSERT_TRUE(list->retainAll(l).get());
                ASSERT_EQ(0, (int) list->size().get());
            }

            TEST_F(ClientListTest, testListener) {
                boost::latch latch1(1);

                ItemListener listener;

                listener.on_added([&latch1](ItemEvent &&itemEvent) {
                    auto type = itemEvent.getEventType();
                    ASSERT_EQ(ItemEventType::ADDED, type);
                    ASSERT_EQ("MyList", itemEvent.getName());
                    std::string host = itemEvent.getMember().getAddress().getHost();
                    ASSERT_TRUE(host == "localhost" || host == "127.0.0.1");
                    ASSERT_EQ(5701, itemEvent.getMember().getAddress().getPort());
                    ASSERT_EQ("item-1", itemEvent.getItem().get<std::string>().value());
                    latch1.count_down();
                });

                auto registrationId = list->addItemListener(std::move(listener), true).get();

                list->add("item-1").get();

                ASSERT_EQ(boost::cv_status::no_timeout, latch1.wait_for(boost::chrono::seconds(20)));

                ASSERT_TRUE(list->removeItemListener(registrationId).get());
            }

            TEST_F(ClientListTest, testIsEmpty) {
                ASSERT_TRUE(list->isEmpty().get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_FALSE(list->isEmpty().get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientQueueTest : public ClientTestSupport {
            protected:
                void offer(int numberOfItems) {
                    for (int i = 1; i <= numberOfItems; ++i) {
                        ASSERT_TRUE(q->offer(std::string("item") + std::to_string(i)).get());
                    }
                }
                
                void TearDown() override {
                    q->clear();
                }
                
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    q = client->getQueue("MyQueue");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    q = nullptr;
                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<IQueue> q;
            };

            HazelcastServer *ClientQueueTest::instance = nullptr;
            HazelcastClient *ClientQueueTest::client = nullptr;
            std::shared_ptr<IQueue> ClientQueueTest::q;

            TEST_F(ClientQueueTest, testListener) {
                ASSERT_EQ(0, q->size().get());

                boost::latch latch1(5);

                auto listener = ItemListener()
                    .on_added([&latch1](ItemEvent &&itemEvent) {
                        latch1.count_down();
                    });

                auto id = q->addItemListener(std::move(listener), true).get();
                
                for (int i = 0; i < 5; i++) {
                    ASSERT_TRUE(q->offer(std::string("event_item") + std::to_string(i)).get());
                }

                ASSERT_EQ(boost::cv_status::no_timeout, latch1.wait_for(boost::chrono::seconds(5)));
                ASSERT_TRUE(q->removeItemListener(id).get());

                // added for test coverage
                ASSERT_NO_THROW(q->destroy().get());
            }

            void testOfferPollThread2(hazelcast::util::ThreadArgs &args) {
                auto *q = (IQueue *) args.arg0;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                q->offer("item1");
            }

            TEST_F(ClientQueueTest, testOfferPoll) {
                for (int i = 0; i < 10; i++) {
                    ASSERT_TRUE(q->offer("item").get());
                }
                ASSERT_EQ(10, q->size().get());
                q->poll<std::string>().get();
                ASSERT_TRUE(q->offer("item", std::chrono::milliseconds(5)).get());

                for (int i = 0; i < 10; i++) {
                    ASSERT_TRUE(q->poll<std::string>().get().has_value());
                }
                ASSERT_EQ(0, q->size().get());

                hazelcast::util::StartedThread t2(testOfferPollThread2, q.get());

                boost::optional<std::string> item = q->poll<std::string>(std::chrono::seconds(30)).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());
                t2.join();
            }

            TEST_F(ClientQueueTest, testPeek) {
                offer(3);
                boost::optional<std::string> item = q->peek<std::string>().get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());
            }

            TEST_F(ClientQueueTest, testTake) {
                q->put("peek 1").get();
                ASSERT_TRUE(q->offer("peek 2").get());
                ASSERT_TRUE(q->offer("peek 3").get());

                boost::optional<std::string> item = q->take<std::string>().get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("peek 1", item.value());

                item = q->take<std::string>().get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("peek 2", item.value());

                item = q->take<std::string>().get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("peek 3", item.value());

                ASSERT_TRUE(q->isEmpty().get());

                // start a thread to insert an item
                hazelcast::util::StartedThread t2(testOfferPollThread2, q.get());

                item = q->take<std::string>().get();  //  should block till it gets an item
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());

                t2.join();
            }

            TEST_F(ClientQueueTest, testRemainingCapacity) {
                int capacity = q->remainingCapacity().get();
                ASSERT_TRUE(capacity > 10000);
                q->offer("item");
                ASSERT_EQ(capacity - 1, q->remainingCapacity().get());
            }


            TEST_F(ClientQueueTest, testRemove) {
                offer(3);
                ASSERT_FALSE(q->remove("item4").get());
                ASSERT_EQ(3, q->size().get());

                ASSERT_TRUE(q->remove("item2").get());

                ASSERT_EQ(2, q->size().get());

                ASSERT_EQ("item1", q->poll<std::string>().get().value());
                ASSERT_EQ("item3", q->poll<std::string>().get().value());
            }


            TEST_F(ClientQueueTest, testContains) {
                offer(5);
                ASSERT_TRUE(q->contains("item3").get());
                ASSERT_FALSE(q->contains("item").get());

                std::vector<std::string> list;
                list.emplace_back("item4");
                list.emplace_back("item2");

                ASSERT_TRUE(q->containsAll(list).get());

                list.emplace_back("item");
                ASSERT_FALSE(q->containsAll(list).get());
            }

            TEST_F(ClientQueueTest, testDrain) {
                offer(5);
                std::vector<std::string> list;
                size_t result = q->drainTo(list, 2).get();
                ASSERT_EQ(2U, result);
                ASSERT_EQ("item1", list[0]);
                ASSERT_EQ("item2", list[1]);

                std::vector<std::string> list2;
                result = q->drainTo(list2).get();
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item3", list2[0]);
                ASSERT_EQ("item4", list2[1]);
                ASSERT_EQ("item5", list2[2]);

                offer(3);
                list2.clear();
                result = q->drainTo(list2, 5).get();
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item1", list2[0]);
                ASSERT_EQ("item2", list2[1]);
                ASSERT_EQ("item3", list2[2]);
            }

            TEST_F(ClientQueueTest, testToArray) {
                offer(5);
                std::vector<std::string> array = q->toArray<std::string>().get();
                size_t size = array.size();
                for (size_t i = 0; i < size; i++) {
                    ASSERT_EQ(std::string("item") + std::to_string(i + 1), array[i]);
                }
            }

            TEST_F(ClientQueueTest, testAddAll) {
                std::vector<std::string> coll;
                coll.emplace_back("item1");
                coll.emplace_back("item2");
                coll.emplace_back("item3");
                coll.emplace_back("item4");

                ASSERT_TRUE(q->addAll(coll).get());
                int size = q->size().get();
                ASSERT_EQ(size, (int) coll.size());
            }

            TEST_F(ClientQueueTest, testRemoveRetain) {
                offer(5);
                std::vector<std::string> list{"item8", "item9"};
                ASSERT_FALSE(q->removeAll(list).get());
                ASSERT_EQ(5, q->size().get());

                list.emplace_back("item3");
                list.emplace_back("item4");
                list.emplace_back("item1");
                ASSERT_TRUE(q->removeAll(list).get());
                ASSERT_EQ(2, q->size().get());

                list.clear();
                list.emplace_back("item2");
                list.emplace_back("item5");
                ASSERT_FALSE(q->retainAll(list).get());
                ASSERT_EQ(2, q->size().get());

                list.clear();
                ASSERT_TRUE(q->retainAll(list).get());
                ASSERT_EQ(0, q->size().get());
            }

            TEST_F(ClientQueueTest, testClear) {
                offer(5);
                q->clear().get();
                ASSERT_EQ(0, q->size().get());
                ASSERT_FALSE(q->poll<std::string>().get().has_value());
            }

            TEST_F(ClientQueueTest, testIsEmpty) {
                ASSERT_TRUE(q->isEmpty().get());
                ASSERT_TRUE(q->offer("item1").get());
                ASSERT_FALSE(q->isEmpty().get());
            }

            TEST_F(ClientQueueTest, testPut) {
                q->put("item1").get();
                ASSERT_EQ(1, q->size().get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace executor {
                namespace tasks {
                    bool SelectAllMembers::select(const hazelcast::client::Member &member) const {
                        return true;
                    }

                    void SelectAllMembers::toString(std::ostream &os) const {
                        os << "SelectAllMembers";
                    }

                    bool SelectNoMembers::select(const hazelcast::client::Member &member) const {
                        return false;
                    }

                    void SelectNoMembers::toString(std::ostream &os) const {
                        os << "SelectNoMembers";
                    }
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientExecutorServiceTest : public ClientTestSupport {
            protected:
                static constexpr const char *APPENDAGE = ":CallableResult";

                static const size_t numberOfMembers;

                void TearDown() override {
                }

                static void SetUpTestCase() {
                    factory = new HazelcastServerFactory(g_srvFactory->getServerAddress(),
                                                         "hazelcast/test/resources/hazelcast-test-executor.xml");
                    for (size_t i = 0; i < numberOfMembers; ++i) {
                        instances.push_back(new HazelcastServer(*factory));
                    }
                    client = new HazelcastClient(ClientConfig().setClusterName("executor-test"));
                }

                static void TearDownTestCase() {
                    delete client;
                    for (HazelcastServer *server : instances) {
                        server->shutdown();
                        delete server;
                    }

                    client = nullptr;
                }

                class FailingExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    FailingExecutionCallback(const std::shared_ptr<boost::latch> &latch1) : latch1(
                            latch1) {}

                    void onResponse(const boost::optional<std::string> &response) override {
                    }

                    void onFailure(std::exception_ptr e) override {
                        exception = e;
                        latch1->count_down();
                    }

                    std::exception_ptr getException() {
                        return exception;
                    }

                private:
                    const std::shared_ptr<boost::latch> latch1;
                    hazelcast::util::Sync<std::exception_ptr> exception;
                };

                class SuccessfullExecutionCallback : public ExecutionCallback<boost::uuids::uuid> {
                public:
                    SuccessfullExecutionCallback(const std::shared_ptr<boost::latch> &latch1) : latch1(latch1) {}

                    void onResponse(const boost::optional<boost::uuids::uuid> &response) override {
                        latch1->count_down();
                    }

                    void onFailure(std::exception_ptr e) override {
                    }

                private:
                    const std::shared_ptr<boost::latch> latch1;
                };

                template<typename T>
                class ResultSettingExecutionCallback : public ExecutionCallback<T> {
                public:
                    explicit ResultSettingExecutionCallback(const std::shared_ptr<boost::latch> &latch1) : latch1(latch1) {}

                    void onResponse(const boost::optional<T> &response) override {
                        result.set(response);
                        latch1->count_down();
                    }

                    void onFailure(std::exception_ptr e) override {
                    }

                    boost::optional<T> getResult() {
                        return result.get();
                    }

                private:
                    const std::shared_ptr<boost::latch> latch1;
                    hazelcast::util::Sync<boost::optional<T>> result;
                };

                class MultiExecutionCompletionCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionCompletionCallback(std::string msg,
                                                     std::shared_ptr<boost::latch> responseLatch,
                                                     const std::shared_ptr<boost::latch> &completeLatch) : msg(std::move(msg)),
                                                                                                           responseLatch(std::move(
                                                                                                                   responseLatch)),
                                                                                                           completeLatch(
                                                                                                                   completeLatch) {}

                    void onResponse(const Member &member, const boost::optional<std::string> &response) override {
                        if (response && *response == msg + APPENDAGE) {
                            responseLatch->count_down();
                        }
                    }

                    void
                    onFailure(const Member &member, std::exception_ptr exception) override {
                    }

                    void onComplete(const std::unordered_map<Member, boost::optional<std::string> > &values,
                                            const std::unordered_map<Member, std::exception_ptr> &exceptions) override {
                        typedef std::unordered_map<Member, boost::optional<std::string> > VALUE_MAP;
                        std::string expectedValue(msg + APPENDAGE);
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (entry.second && *entry.second == expectedValue) {
                                completeLatch->count_down();
                            }
                        }
                    }

                private:
                    std::string msg;
                    const std::shared_ptr<boost::latch> responseLatch;
                    const std::shared_ptr<boost::latch> completeLatch;
                };

                class MultiExecutionNullCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionNullCallback(std::shared_ptr<boost::latch> responseLatch,
                                               std::shared_ptr<boost::latch> completeLatch)
                            : responseLatch(std::move(responseLatch)), completeLatch(std::move(completeLatch)) {}

                    void onResponse(const Member &member, const boost::optional<std::string> &response) override {
                        if (!response) {
                            responseLatch->count_down();
                        }
                    }

                    void
                    onFailure(const Member &member, std::exception_ptr exception) override {
                    }

                    void onComplete(const std::unordered_map<Member, boost::optional<std::string> > &values,
                                            const std::unordered_map<Member, std::exception_ptr> &exceptions) override {
                        typedef std::unordered_map<Member, boost::optional<std::string> > VALUE_MAP;
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (!entry.second) {
                                completeLatch->count_down();
                            }
                        }
                    }

                private:
                    const std::shared_ptr<boost::latch> responseLatch;
                    const std::shared_ptr<boost::latch> completeLatch;
                };

                static std::vector<HazelcastServer *> instances;
                static HazelcastClient *client;
                static HazelcastServerFactory *factory;
            };

            std::vector<HazelcastServer *>ClientExecutorServiceTest::instances;
            HazelcastClient *ClientExecutorServiceTest::client = nullptr;
            HazelcastServerFactory *ClientExecutorServiceTest::factory = nullptr;
            const size_t ClientExecutorServiceTest::numberOfMembers = 4;

            TEST_F(ClientExecutorServiceTest, testIsTerminated) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isTerminated().get());
            }

            TEST_F(ClientExecutorServiceTest, testIsShutdown) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isShutdown().get());
            }

            TEST_F(ClientExecutorServiceTest, testShutdown) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown().get());
            }

            TEST_F(ClientExecutorServiceTest, testShutdownMultipleTimes) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();
                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown().get());
            }

            TEST_F(ClientExecutorServiceTest, testCancellationAwareTask_whenTimeOut) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task{INT64_MAX};

                auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);

                ASSERT_EQ(boost::future_status::timeout, promise.get_future().wait_for(boost::chrono::seconds(1)));
            }

            TEST_F(ClientExecutorServiceTest, testFutureAfterCancellationAwareTaskTimeOut) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task{INT64_MAX};

                auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);
                auto future = promise.get_future();

                ASSERT_EQ(boost::future_status::timeout, future.wait_for(boost::chrono::seconds(1)));

                ASSERT_FALSE(future.is_ready());
            }

            TEST_F(ClientExecutorServiceTest, testGetFutureAfterCancel) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task{INT64_MAX};

                auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);

                auto future = promise.get_future();
                ASSERT_EQ(boost::future_status::timeout, future.wait_for(boost::chrono::seconds(1)));

                ASSERT_TRUE(promise.cancel(true));

                ASSERT_THROW(future.get(), exception::CancellationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::FailingCallable task;

                auto future = service->submit<executor::tasks::FailingCallable, std::string>(task).get_future();

                ASSERT_THROW(future.get(), exception::IllegalStateException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));

                executor::tasks::FailingCallable task;
                std::shared_ptr<ExecutionCallback<std::string> > callback(new FailingExecutionCallback(latch1));

                service->submit<executor::tasks::FailingCallable, std::string>(task, callback);

                ASSERT_OPEN_EVENTUALLY(*latch1);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableReasonExceptionCause) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                auto failingFuture = service->submit<executor::tasks::FailingCallable, std::string>(
                        executor::tasks::FailingCallable()).get_future();

                ASSERT_THROW(failingFuture.get(), exception::IllegalStateException);
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withNoMemberSelected) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string mapName = randomMapName();

                executor::tasks::SelectNoMembers selector;

                ASSERT_THROW(service->execute<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>>(
                        executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>{mapName, boost::uuids::random_generator()()}, selector),
                             exception::RejectedExecutionException);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable{0};

                auto future = service->submitToKeyOwner<executor::tasks::SerializedCounterCallable, int, std::string>(
                        counterCallable, name).get_future();
                auto value = future.get();
                ASSERT_TRUE(value);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce_submitToAddress) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable{0};

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_FALSE(members.empty());
                auto future = service->submitToMember<executor::tasks::SerializedCounterCallable, int>(
                        counterCallable, members[0]).get_future();
                auto value = future.get();
                ASSERT_TRUE(value);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClient) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                auto future = service->submit<executor::tasks::TaskWithUnserializableResponse, bool>(
                        taskWithUnserializableResponse).get_future();

                ASSERT_THROW(future.get(), exception::HazelcastSerializationException);
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClientCallback) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));

                std::shared_ptr<FailingExecutionCallback> callback(new FailingExecutionCallback(latch1));

                service->submit<executor::tasks::TaskWithUnserializableResponse, std::string>(
                        taskWithUnserializableResponse, callback);

                ASSERT_OPEN_EVENTUALLY(*latch1);

                auto exception = callback->getException();
                ASSERT_THROW(std::rethrow_exception(exception), exception::HazelcastSerializationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMember) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                auto future = service->submitToMember<executor::tasks::GetMemberUuidTask, boost::uuids::uuid>(
                        task, members[0]).get_future();

                auto uuid = future.get();
                ASSERT_TRUE(uuid);
                ASSERT_EQ(members[0].getUuid(), uuid);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                auto futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, boost::uuids::uuid>(task,
                                                                                                            members);

                for (const Member &member : members) {
                    ASSERT_EQ(1U, futuresMap.count(member));
                    auto it = futuresMap.find(member);
                    ASSERT_NE(futuresMap.end(), it);
                    auto uuid = (*it).second.get_future().get();
                    ASSERT_TRUE(uuid);
                    ASSERT_EQ(member.getUuid(), uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withMemberSelector) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};
                executor::tasks::SelectAllMembers selectAll;

                auto f = service->submit<executor::tasks::AppendCallable, std::string>(callable, selectAll).get_future();

                auto result = f.get();
                ASSERT_TRUE(result);
                ASSERT_EQ(msg + APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers_withMemberSelector) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;
                executor::tasks::SelectAllMembers selectAll;

                auto futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, boost::uuids::uuid>(
                        task, selectAll);

                for (auto &pair : futuresMap) {
                    const Member &member = pair.first;
                    auto future = pair.second.get_future();

                    auto uuid = future.get();
                    ASSERT_TRUE(uuid);
                    ASSERT_EQ(member.getUuid(), uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};

                auto futuresMap = service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(callable);

                for (auto &pair : futuresMap) {
                    auto future = pair.second.get_future();

                    auto result = future.get();
                    ASSERT_TRUE(result);
                    ASSERT_EQ(msg + APPENDAGE, *result);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withExecutionCallback) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid> callable(testName, boost::uuids::random_generator()());

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                std::shared_ptr<SuccessfullExecutionCallback> callback(new SuccessfullExecutionCallback(latch1));

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                service->submitToMember<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>, boost::uuids::uuid>(callable, members[0],
                                                                                                    callback);

                auto map = client->getMap(testName);

                ASSERT_OPEN_EVENTUALLY(*latch1);
                ASSERT_EQ(1, map->size().get());
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> responseLatch(new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(new boost::latch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};
                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, members, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallable_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};
                executor::tasks::SelectAllMembers selector;
                std::shared_ptr<boost::latch> responseLatch(new boost::latch(1));
                auto callback = std::make_shared<ResultSettingExecutionCallback<std::string>>(responseLatch);

                service->submit<executor::tasks::AppendCallable, std::string>(callable, selector,
                                                                              std::static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                auto message = callback->getResult();
                ASSERT_TRUE(message.has_value());
                ASSERT_EQ(msg + APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMembers_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> responseLatch(
                        new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(
                        new boost::latch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};
                executor::tasks::SelectAllMembers selector;

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, selector, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> responseLatch(
                        new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(
                        new boost::latch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*completeLatch);
                ASSERT_OPEN_EVENTUALLY(*responseLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableWithNullResultToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> responseLatch(new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(new boost::latch(numberOfMembers));

                executor::tasks::NullCallable callable;

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionNullCallback(responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::NullCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};

                auto result = service->submit<executor::tasks::AppendCallable, std::string>(callable).get_future();

                auto message = result.get();
                ASSERT_TRUE(message);
                ASSERT_EQ(msg + APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                auto callback = std::make_shared<ResultSettingExecutionCallback<std::string>>(latch1);

                service->submit<executor::tasks::AppendCallable, std::string>(callable, std::static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*latch1);
                auto value = callback->getResult();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(msg + APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};

                auto f = service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(callable, "key").get_future();

                auto result = f.get();
                ASSERT_TRUE(result);
                ASSERT_EQ(msg + APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable{msg};

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                auto callback = std::make_shared<ResultSettingExecutionCallback<std::string>>(latch1);

                service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(callable, "key",
                                                                                                     std::static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                                             callback));

                ASSERT_OPEN_EVENTUALLY(*latch1);
                auto value = callback->getResult();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(msg + APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                auto key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid> callable{testName, key};

                auto f = service->submit<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>, boost::uuids::uuid>(
                        callable).get_future();

                auto result = f.get();
                ASSERT_TRUE(result);
                ASSERT_EQ(member.getUuid(), *result);
                ASSERT_TRUE(map->containsKey(member.getUuid()).get());
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware_WithExecutionCallback) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                auto key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid> callable(testName, key);

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                auto callback = std::make_shared<ResultSettingExecutionCallback<boost::uuids::uuid>>(latch1);

                service->submit<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>, boost::uuids::uuid>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*latch1);
                auto value = std::static_pointer_cast<ResultSettingExecutionCallback<boost::uuids::uuid>>(callback)->getResult();
                ASSERT_TRUE(value);
                ASSERT_EQ(member.getUuid(), *value);
                ASSERT_TRUE(map->containsKey(member.getUuid()).get());
            }

            TEST_F(ClientExecutorServiceTest, testExecute) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                service->execute(
                        executor::tasks::MapPutPartitionAwareCallable<std::string>(testName, "key"));

                auto map = client->getMap(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withMemberSelector) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);
                executor::tasks::SelectAllMembers selector;

                service->execute(
                        executor::tasks::MapPutPartitionAwareCallable<std::string>(testName, "key"), selector);
                auto map = client->getMap(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnKeyOwner) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap(testName);
                map->put(1, 1).get();

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                auto targetUuid = member.getUuid();
                auto key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid> callable(testName, key);

                service->executeOnKeyOwner<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>, boost::uuids::uuid>(callable, key);

                ASSERT_TRUE_EVENTUALLY(map->containsKey(targetUuid).get());
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMember) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                Member &member = members[0];
                auto targetUuid = member.getUuid();

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                service->executeOnMember(callable, member);

                ASSERT_TRUE_EVENTUALLY(map->containsKey(targetUuid).get());
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap(testName);

                std::vector<Member> allMembers = client->getCluster().getMembers();
                std::vector<Member> members(allMembers.begin(), allMembers.begin() + 2);

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                service->executeOnMembers(callable, members);

                ASSERT_TRUE_EVENTUALLY(map->containsKey(members[0].getUuid()).get() && map->containsKey(members[1].getUuid()).get());
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withEmptyCollection) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

               auto map = client->getMap(testName);

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                service->executeOnMembers(callable, std::vector<Member>());

                assertSizeEventually(0, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withSelector) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap(testName);

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                executor::tasks::SelectAllMembers selector;

                service->executeOnMembers(callable, selector);

                assertSizeEventually((int) numberOfMembers, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnAllMembers) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap(testName);

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                service->executeOnAllMembers(callable);

                assertSizeEventually((int) numberOfMembers, map);
            }
        }
    }
}

#ifdef HZ_BUILD_WITH_SSL

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class AwsConfigTest : public ClientTestSupport {
                };

                TEST_F (AwsConfigTest, testDefaultValues) {
                    client::config::ClientAwsConfig awsConfig;
                    ASSERT_EQ("", awsConfig.getAccessKey());
                    ASSERT_EQ("us-east-1", awsConfig.getRegion());
                    ASSERT_EQ("ec2.amazonaws.com", awsConfig.getHostHeader());
                    ASSERT_EQ("", awsConfig.getIamRole());
                    ASSERT_EQ("", awsConfig.getSecretKey());
                    ASSERT_EQ("", awsConfig.getSecurityGroupName());
                    ASSERT_EQ("", awsConfig.getTagKey());
                    ASSERT_EQ("", awsConfig.getTagValue());
                    ASSERT_FALSE(awsConfig.isInsideAws());
                    ASSERT_FALSE(awsConfig.isEnabled());
                }

                TEST_F (AwsConfigTest, testSetValues) {
                    client::config::ClientAwsConfig awsConfig;

                    awsConfig.setAccessKey("mykey");
                    awsConfig.setRegion("myregion");
                    awsConfig.setHostHeader("myheader");
                    awsConfig.setIamRole("myrole");
                    awsConfig.setSecretKey("mysecret");
                    awsConfig.setSecurityGroupName("mygroup");
                    awsConfig.setTagKey("mytagkey");
                    awsConfig.setTagValue("mytagvalue");
                    awsConfig.setInsideAws(true);
                    awsConfig.setEnabled(true);

                    ASSERT_EQ("mykey", awsConfig.getAccessKey());
                    ASSERT_EQ("myregion", awsConfig.getRegion());
                    ASSERT_EQ("myheader", awsConfig.getHostHeader());
                    ASSERT_EQ("myrole", awsConfig.getIamRole());
                    ASSERT_EQ("mysecret", awsConfig.getSecretKey());
                    ASSERT_EQ("mygroup", awsConfig.getSecurityGroupName());
                    ASSERT_EQ("mytagkey", awsConfig.getTagKey());
                    ASSERT_EQ("mytagvalue", awsConfig.getTagValue());
                    ASSERT_TRUE(awsConfig.isInsideAws());
                    ASSERT_TRUE(awsConfig.isEnabled()) << awsConfig;
                }

                TEST_F (AwsConfigTest, testSetEmptyValues) {
                    client::config::ClientAwsConfig awsConfig;

                    ASSERT_THROW(awsConfig.setAccessKey(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setRegion(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setHostHeader(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setSecretKey(""), exception::IllegalArgumentException);
                }

                TEST_F (AwsConfigTest, testClientConfigUsage) {
                    ClientConfig clientConfig;
                    client::config::ClientAwsConfig &awsConfig = clientConfig.getNetworkConfig().getAwsConfig();
                    awsConfig.setEnabled(true);

                    ASSERT_TRUE(clientConfig.getNetworkConfig().getAwsConfig().isEnabled());

                    client::config::ClientAwsConfig newConfig;

                    clientConfig.getNetworkConfig().setAwsConfig(newConfig);
                    // default constructor sets enabled to false
                    ASSERT_FALSE(clientConfig.getNetworkConfig().getAwsConfig().isEnabled());
                }

                TEST_F (AwsConfigTest, testInvalidAwsMemberPortConfig) {
                    ClientConfig clientConfig = getConfig();

                    clientConfig.setProperty(ClientProperties::PROP_AWS_MEMBER_PORT, "65536");
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    clientConfig.setProperty(ClientProperties::PROP_AWS_MEMBER_PORT, "-1");

                    ASSERT_THROW(HazelcastClient hazelcastClient(clientConfig),
                                 exception::InvalidConfigurationException);
                }
            }
        }
    }
}

#endif // HZ_BUILD_WITH_SSL


#ifdef HZ_BUILD_WITH_SSL

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class AwsClientTest : public ClientTestSupport {
                };

                TEST_F (AwsClientTest, testClientAwsMemberNonDefaultPortConfig) {
                    ClientConfig clientConfig;

                    clientConfig.setProperty(ClientProperties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(std::getenv("AWS_ACCESS_KEY_ID")).setSecretKey(std::getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
#else
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(false);
#endif
                    HazelcastClient hazelcastClient(clientConfig);
                    auto map = hazelcastClient.getMap("myMap");
                    map->put(5, 20).get();
                    auto val = map->get<int, int>(5).get();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ(20, val.value());
                }

                TEST_F (AwsClientTest, testClientAwsMemberWithSecurityGroupDefaultIamRole) {
                    ClientConfig clientConfig;
                    clientConfig.setProperty(ClientProperties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setSecurityGroupName("launch-wizard-147");

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                                                                                                                                            // The access key and secret will be retrieved from default IAM role at windows machine
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
#else
                    clientConfig.getNetworkConfig().getAwsConfig().setAccessKey(std::getenv("AWS_ACCESS_KEY_ID")).
                            setSecretKey(std::getenv("AWS_SECRET_ACCESS_KEY"));
#endif

                    HazelcastClient hazelcastClient(clientConfig);
                    auto map = hazelcastClient.getMap("myMap");
                    map->put(5, 20).get();
                    auto val = map->get<int, int>(5).get();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ(20, val.value());
                }

                // FIPS_mode_set is not available for Mac OS X built-in openssl library
#ifndef __APPLE__
                                                                                                                                        TEST_F (AwsClientTest, testFipsEnabledAwsDiscovery) {
                    ClientConfig clientConfig = getConfig();

                    clientConfig.setProperty(ClientProperties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");

                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
                    #else
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(false);
                    #endif

                    // Turn Fips mode on
                    FIPS_mode_set(1);

                    HazelcastClient hazelcastClient(clientConfig);
                    auto map = hazelcastClient.getMap("myMap");
                    map->put(5, 20);
                    auto val = map->get<int, int>(5).get();
                    ASSERT_TRUE(val);
                    ASSERT_EQ(20, *val);
                }
#endif // ifndef __APPLE__

                /**
                 * Following test can only run from inside the AWS network
                 */
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                                                                                                                                        TEST_F (AwsClientTest, testRetrieveCredentialsFromIamRoleAndConnect) {
                    ClientConfig clientConfig = getConfig();

                    clientConfig.setProperty(ClientProperties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).setIamRole("cloudbees-role").setTagKey(
                            "aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    HazelcastClient hazelcastClient(clientConfig);
                }

                TEST_F (AwsClientTest, testRetrieveCredentialsFromInstanceProfileDefaultIamRoleAndConnect) {
                    ClientConfig clientConfig = getConfig();

                    clientConfig.setProperty(ClientProperties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).setTagKey(
                            "aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    HazelcastClient hazelcastClient(clientConfig);
                }
#endif
            }
        }
    }
}

#endif // HZ_BUILD_WITH_SSL


#ifdef HZ_BUILD_WITH_SSL

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class DescribeInstancesTest : public ClientTestSupport {
                };

                TEST_F (DescribeInstancesTest, testDescribeInstancesTagAndValueSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesTagAndNonExistentValueSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag").setTagValue(
                            "non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSetToNonExistentTag) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("non-existent-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagValue("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSetToNonExistentValue) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagValue("non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesSecurityGroup) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setSecurityGroupName("launch-wizard-147");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesNonExistentSecurityGroup) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setSecurityGroupName("non-existent-group");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

            }
        }
    }
}

#endif //HZ_BUILD_WITH_SSL

#ifdef HZ_BUILD_WITH_SSL

namespace awsutil = hazelcast::client::aws::utility;

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class CloudUtilityTest : public ClientTestSupport {
                };

                TEST_F (CloudUtilityTest, testUnmarshallResponseXml) {
                    std::filebuf fb;
                    ASSERT_TRUE(fb.open("hazelcast/test/resources/sample_aws_response.xml", std::ios::in));
                    std::istream responseStream(&fb);

                    config::ClientAwsConfig awsConfig;
                    std::unordered_map<std::string, std::string> results = hazelcast::client::aws::utility::CloudUtility::unmarshalTheResponse(
                            responseStream, getLogger());
                    ASSERT_EQ(4U, results.size());
                    ASSERT_NE(results.end(), results.find("10.0.16.13"));
                    ASSERT_EQ("", results["10.0.16.13"]);
                    ASSERT_NE(results.end(), results.find("10.0.16.17"));
                    ASSERT_EQ("54.85.192.215", results["10.0.16.17"]);
                    ASSERT_NE(results.end(), results.find("10.0.16.25"));
                    ASSERT_EQ("", results["10.0.16.25"]);
                    ASSERT_NE(results.end(), results.find("172.30.4.118"));
                    ASSERT_EQ("54.85.192.213", results["172.30.4.118"]);
                }
            }
        }
    }
}

#endif //HZ_BUILD_WITH_SSL

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
