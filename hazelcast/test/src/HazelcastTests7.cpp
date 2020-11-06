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
#include "hazelcast/client/entry_listener.h"
#include "ringbuffer/StartsWithStringFilter.h"
#include "ClientTestSupportBase.h"
#include <hazelcast/client/client_config.h>
#include <hazelcast/client/exception/IllegalStateException.h>
#include <hazelcast/client/hazelcast_client.h>
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
#include <hazelcast/client/serialization/pimpl/data_input.h>
#include <hazelcast/util/AddressUtil.h>
#include <hazelcast/client/serialization/pimpl/data_output.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/client/exception/IOException.h>
#include <hazelcast/client/protocol/ClientExceptionFactory.h>
#include <hazelcast/util/IOUtil.h>
#include <ClientTestSupportBase.h>
#include <hazelcast/util/Util.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <ctime>
#include <cerrno>
#include <hazelcast/client/lifecycle_listener.h>
#include "serialization/Serializables.h"
#include <hazelcast/client/serialization_config.h>
#include <hazelcast/client/hazelcast_json_value.h>
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

#include "hazelcast/client/config/client_aws_config.h"
#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/hazelcast_client.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/membership_listener.h"
#include "hazelcast/client/initial_membership_event.h"
#include "hazelcast/client/lifecycle_listener.h"
#include "hazelcast/client/socket_interceptor.h"
#include "hazelcast/client/hz_socket.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/execution_callback.h"
#include "hazelcast/client/pipelining.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/item_listener.h"
#include "hazelcast/client/multi_map.h"
#include "hazelcast/client/entry_event.h"
#include "hazelcast/client/ilist.h"
#include "hazelcast/client/iqueue.h"
#include "hazelcast/client/client_properties.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientMultiMapTest : public ClientTestSupport {
            protected:

                entry_listener make_add_remove_listener(boost::latch &added_latch, boost::latch &removed_latch) {
                    return entry_listener().
                        on_added([&added_latch](entry_event &&) {
                            added_latch.count_down();
                        }).
                        on_removed([&removed_latch](entry_event &&) {
                            removed_latch.count_down();
                        });
                }

                static void fill_data() {
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
                    client = new hazelcast_client(get_config());
                    mm = client->get_multi_map("MyMultiMap");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    mm = nullptr;
                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static hazelcast_client *client;
                static std::shared_ptr<multi_map> mm;
            };

            HazelcastServer *ClientMultiMapTest::instance = nullptr;
            hazelcast_client *ClientMultiMapTest::client = nullptr;
            std::shared_ptr<multi_map> ClientMultiMapTest::mm;

            TEST_F(ClientMultiMapTest, testPutGetRemove) {
                fill_data();
                ASSERT_EQ(3, mm->value_count("key1").get());
                ASSERT_EQ(2, mm->value_count("key2").get());
                ASSERT_EQ(5, mm->size().get());

                auto coll = mm->get<std::string, std::string>("key1").get();
                ASSERT_EQ(3, (int) coll.size());

                coll = mm->remove<std::string, std::string>("key2").get();
                ASSERT_EQ(2, (int) coll.size());
                ASSERT_EQ(0, mm->value_count("key2").get());
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
                fill_data();
                ASSERT_EQ(2, (int) mm->key_set<std::string>().get().size());
                ASSERT_EQ(5, (int) mm->values<std::string>().get().size());
                ASSERT_EQ(5, ((int) mm->entry_set<std::string, std::string>().get().size()));
            }


            TEST_F(ClientMultiMapTest, testContains) {
                fill_data();
                ASSERT_FALSE(mm->contains_key<std::string>("key3").get());
                ASSERT_TRUE(mm->contains_key<std::string>("key1").get());

                ASSERT_FALSE(mm->contains_value<std::string>("value6").get());
                ASSERT_TRUE(mm->contains_value<std::string>("value4").get());

                ASSERT_FALSE(mm->contains_entry("key1", "value4").get());
                ASSERT_FALSE(mm->contains_entry("key2", "value3").get());
                ASSERT_TRUE(mm->contains_entry("key1", "value1").get());
                ASSERT_TRUE(mm->contains_entry("key2", "value5").get());
            }

            TEST_F(ClientMultiMapTest, testListener) {
                boost::latch latch1Add(8);
                boost::latch latch1Remove(4);
                boost::latch latch2Add(3);
                boost::latch latch2Remove(3);
                auto listener1 = make_add_remove_listener(latch1Add, latch1Remove);
                auto listener2 = make_add_remove_listener(latch2Add, latch2Remove);

                auto id1 = mm->add_entry_listener(std::move(listener1), true).get();
                auto id2 = mm->add_entry_listener(std::move(listener2), "key3", true).get();

                fill_data();

                mm->remove("key1", "value2").get();

                mm->put("key3", "value6").get();
                mm->put("key3", "value7").get();
                mm->put("key3", "value8").get();

                mm->remove<std::string, std::string>("key3").get();

                ASSERT_OPEN_EVENTUALLY(latch1Add);
                ASSERT_OPEN_EVENTUALLY(latch1Remove);
                ASSERT_OPEN_EVENTUALLY(latch2Add);
                ASSERT_OPEN_EVENTUALLY(latch2Remove);

                ASSERT_TRUE(mm->remove_entry_listener(id1).get());
                ASSERT_TRUE(mm->remove_entry_listener(id2).get());
            }

            TEST_F(ClientMultiMapTest, testLock) {
                mm->lock("key1").get();
                boost::latch latch1(1);
                std::thread([&]() {
                    if (!mm->try_lock("key1").get()) {
                        latch1.count_down();
                    }
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                mm->force_unlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testLockTtl) {
                mm->lock("key1", std::chrono::seconds(2)).get();
                boost::latch latch1(2);
                std::thread([&]() {
                    if (!mm->try_lock("key1").get()) {
                        latch1.count_down();
                    }
                    if (mm->try_lock("key1", std::chrono::seconds(5)).get()) {
                        latch1.count_down();
                    }
                }).detach();

                ASSERT_OPEN_EVENTUALLY(latch1);
                mm->force_unlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testTryLock) {
                ASSERT_TRUE(mm->try_lock("key1", std::chrono::seconds(2)).get());
                boost::latch latch1(1);
                std::thread([&]() {
                    if (!mm->try_lock("key1", std::chrono::milliseconds(500)).get()) {
                        latch1.count_down();
                    }
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_TRUE(mm->is_locked("key1").get());

                boost::latch latch2(1);
                boost::barrier b(2);
                std::thread([&]() {
                    b.count_down_and_wait();
                    if (mm->try_lock("key1", std::chrono::seconds(20)).get()) {
                        latch2.count_down();
                    }
                }).detach();
                b.count_down_and_wait();
                mm->unlock("key1").get();
                ASSERT_OPEN_EVENTUALLY(latch2);
                ASSERT_TRUE(mm->is_locked("key1").get());
                mm->force_unlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testForceUnlock) {
                mm->lock("key1").get();
                boost::latch latch1(1);
                std::thread([&]() {
                    mm->force_unlock("key1").get();
                    latch1.count_down();
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_FALSE(mm->is_locked("key1").get());
            }

            TEST_F(ClientMultiMapTest, testTryLockTtl) {
                ASSERT_TRUE(mm->try_lock("key1", std::chrono::seconds(2), std::chrono::seconds(1)).get());
                boost::latch latch1(1);
                std::thread([&]() {
                    if (!mm->try_lock("key1", std::chrono::milliseconds(500)).get()) {
                        latch1.count_down();
                    }
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_TRUE(mm->is_locked("key1").get());

                boost::latch latch2(1);
                boost::barrier b(2);
                std::thread([&]() {
                    b.count_down_and_wait();
                    if (mm->try_lock("key1", std::chrono::seconds(20)).get()) {
                        latch2.count_down();
                    }
                }).detach();
                b.count_down_and_wait();
                mm->unlock("key1").get();
                ASSERT_OPEN_EVENTUALLY(latch2);
                ASSERT_TRUE(mm->is_locked("key1").get());
                mm->force_unlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testTryLockTtlTimeout) {
                ASSERT_TRUE(mm->try_lock("key1", std::chrono::seconds(1), std::chrono::seconds(200)).get());
                boost::latch latch1(1);
                std::thread([&]() {
                    if (!mm->try_lock("key1", std::chrono::seconds(2)).get()) {
                        latch1.count_down();
                    }
                }).detach();
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_TRUE(mm->is_locked("key1").get());
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
                    sslFactory = new HazelcastServerFactory(g_srvFactory->get_server_address(), get_ssl_file_path());
                    instance = new HazelcastServer(*sslFactory);
#else
                    instance = new HazelcastServer(*g_srvFactory);
#endif

#ifdef HZ_BUILD_WITH_SSL
                    client_config clientConfig = get_config(true);
                    clientConfig.get_network_config().get_ssl_config().set_cipher_list("HIGH");
#else
                    client_config clientConfig = get_config();
#endif // HZ_BUILD_WITH_SSL
                    client = new hazelcast_client(clientConfig);
                    list = client->get_list("MyList");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;
                    delete sslFactory;

                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static hazelcast_client *client;
                static std::shared_ptr<ilist> list;
                static HazelcastServerFactory *sslFactory;
            };

            std::shared_ptr<ilist> ClientListTest::list;
            HazelcastServer *ClientListTest::instance = nullptr;
            hazelcast_client *ClientListTest::client = nullptr;
            HazelcastServerFactory *ClientListTest::sslFactory = nullptr;

            TEST_F(ClientListTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_TRUE(list->add_all(l).get());

                ASSERT_TRUE(list->add_all(1, l).get());
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

                ASSERT_EQ(-1, list->index_of("item5").get());
                ASSERT_EQ(0, list->index_of("item1").get());

                ASSERT_EQ(-1, list->last_index_of("item6").get());
                ASSERT_EQ(2, list->last_index_of("item1").get());
            }

            TEST_F(ClientListTest, testToArray) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                std::vector<std::string> ar = list->to_array<std::string>().get();

                ASSERT_EQ("item1", ar[0]);
                ASSERT_EQ("item2", ar[1]);
                ASSERT_EQ("item1", ar[2]);
                ASSERT_EQ("item4", ar[3]);

                std::vector<std::string> arr2 = list->sub_list<std::string>(1, 3).get();

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

                ASSERT_FALSE(list->contains_all(l).get());
                ASSERT_TRUE(list->add("item3").get());
                ASSERT_TRUE(list->contains_all(l).get());
            }

            TEST_F(ClientListTest, testRemoveRetainAll) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(list->remove_all(l).get());
                ASSERT_EQ(3, (int) list->size().get());
                ASSERT_FALSE(list->remove_all(l).get());
                ASSERT_EQ(3, (int) list->size().get());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(list->retain_all(l).get());
                ASSERT_EQ(3, (int) list->size().get());

                l.clear();
                ASSERT_TRUE(list->retain_all(l).get());
                ASSERT_EQ(0, (int) list->size().get());
            }

            TEST_F(ClientListTest, testListener) {
                boost::latch latch1(1);

                item_listener listener;

                listener.on_added([&latch1](item_event &&item_event) {
                    auto type = item_event.get_event_type();
                    ASSERT_EQ(item_event_type::ADDED, type);
                    ASSERT_EQ("MyList", item_event.get_name());
                    std::string host = item_event.get_member().get_address().get_host();
                    ASSERT_TRUE(host == "localhost" || host == "127.0.0.1");
                    ASSERT_EQ(5701, item_event.get_member().get_address().get_port());
                    ASSERT_EQ("item-1", item_event.get_item().get<std::string>().value());
                    latch1.count_down();
                });

                auto registrationId = list->add_item_listener(std::move(listener), true).get();

                list->add("item-1").get();

                ASSERT_OPEN_EVENTUALLY(latch1);

                ASSERT_TRUE(list->remove_item_listener(registrationId).get());
            }

            TEST_F(ClientListTest, testIsEmpty) {
                ASSERT_TRUE(list->is_empty().get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_FALSE(list->is_empty().get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientQueueTest : public ClientTestSupport {
            protected:
                void offer(int number_of_items) {
                    for (int i = 1; i <= number_of_items; ++i) {
                        ASSERT_TRUE(q->offer(std::string("item") + std::to_string(i)).get());
                    }
                }
                
                void TearDown() override {
                    q->clear().get();
                }
                
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    client = new hazelcast_client(get_config().backup_acks_enabled(false));
                    q = client->get_queue("MyQueue");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;
                    delete instance2;

                    q = nullptr;
                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastServer *instance2;
                static hazelcast_client *client;
                static std::shared_ptr<iqueue> q;
            };

            HazelcastServer *ClientQueueTest::instance = nullptr;
            HazelcastServer *ClientQueueTest::instance2 = nullptr;
            hazelcast_client *ClientQueueTest::client = nullptr;
            std::shared_ptr<iqueue> ClientQueueTest::q;

            TEST_F(ClientQueueTest, testListener) {
                ASSERT_EQ(0, q->size().get());

                boost::latch latch1(5);

                auto listener = item_listener()
                    .on_added([&latch1](item_event &&item_event) {
                        latch1.count_down();
                    });

                auto id = q->add_item_listener(std::move(listener), true).get();
                
                for (int i = 0; i < 5; i++) {
                    ASSERT_TRUE(q->offer(std::string("event_item") + std::to_string(i)).get());
                }

                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_TRUE(q->remove_item_listener(id).get());

                // added for test coverage
                ASSERT_NO_THROW(q->destroy().get());
            }

            void test_offer_poll_thread2(hazelcast::util::ThreadArgs &args) {
                auto *q = (iqueue *) args.arg0;
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

                hazelcast::util::StartedThread t2(test_offer_poll_thread2, q.get());

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

                ASSERT_TRUE(q->is_empty().get());

                // start a thread to insert an item
                hazelcast::util::StartedThread t2(test_offer_poll_thread2, q.get());

                item = q->take<std::string>().get();  //  should block till it gets an item
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());

                t2.join();
            }

            TEST_F(ClientQueueTest, testRemainingCapacity) {
                int capacity = q->remaining_capacity().get();
                ASSERT_TRUE(capacity > 10000);
                q->offer("item");
                ASSERT_EQ(capacity - 1, q->remaining_capacity().get());
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

                ASSERT_TRUE(q->contains_all(list).get());

                list.emplace_back("item");
                ASSERT_FALSE(q->contains_all(list).get());
            }

            TEST_F(ClientQueueTest, testDrain) {
                offer(5);
                std::vector<std::string> list;
                size_t result = q->drain_to(list, 2).get();
                ASSERT_EQ(2U, result);
                ASSERT_EQ("item1", list[0]);
                ASSERT_EQ("item2", list[1]);

                std::vector<std::string> list2;
                result = q->drain_to(list2).get();
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item3", list2[0]);
                ASSERT_EQ("item4", list2[1]);
                ASSERT_EQ("item5", list2[2]);

                offer(3);
                list2.clear();
                result = q->drain_to(list2, 5).get();
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item1", list2[0]);
                ASSERT_EQ("item2", list2[1]);
                ASSERT_EQ("item3", list2[2]);
            }

            TEST_F(ClientQueueTest, testToArray) {
                offer(5);
                std::vector<std::string> array = q->to_array<std::string>().get();
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

                ASSERT_TRUE(q->add_all(coll).get());
                int size = q->size().get();
                ASSERT_EQ(size, (int) coll.size());
            }

            TEST_F(ClientQueueTest, testRemoveRetain) {
                offer(5);
                std::vector<std::string> list{"item8", "item9"};
                ASSERT_FALSE(q->remove_all(list).get());
                ASSERT_EQ(5, q->size().get());

                list.emplace_back("item3");
                list.emplace_back("item4");
                list.emplace_back("item1");
                ASSERT_TRUE(q->remove_all(list).get());
                ASSERT_EQ(2, q->size().get());

                list.clear();
                list.emplace_back("item2");
                list.emplace_back("item5");
                ASSERT_FALSE(q->retain_all(list).get());
                ASSERT_EQ(2, q->size().get());

                list.clear();
                ASSERT_TRUE(q->retain_all(list).get());
                ASSERT_EQ(0, q->size().get());
            }

            TEST_F(ClientQueueTest, testClear) {
                offer(5);
                q->clear().get();
                ASSERT_EQ(0, q->size().get());
                ASSERT_FALSE(q->poll<std::string>().get().has_value());
            }

            TEST_F(ClientQueueTest, testIsEmpty) {
                ASSERT_TRUE(q->is_empty().get());
                ASSERT_TRUE(q->offer("item1").get());
                ASSERT_FALSE(q->is_empty().get());
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
                    bool SelectAllMembers::select(const hazelcast::client::member &member) const {
                        return true;
                    }

                    void SelectAllMembers::to_string(std::ostream &os) const {
                        os << "SelectAllMembers";
                    }

                    bool SelectNoMembers::select(const hazelcast::client::member &member) const {
                        return false;
                    }

                    void SelectNoMembers::to_string(std::ostream &os) const {
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
                    factory = new HazelcastServerFactory(g_srvFactory->get_server_address(),
                                                         "hazelcast/test/resources/hazelcast-test-executor.xml");
                    for (size_t i = 0; i < numberOfMembers; ++i) {
                        instances.push_back(new HazelcastServer(*factory));
                    }
                    client = new hazelcast_client(client_config().set_cluster_name("executor-test"));
                }

                static void TearDownTestCase() {
                    delete client;
                    for (HazelcastServer *server : instances) {
                        server->shutdown();
                        delete server;
                    }

                    client = nullptr;
                }

                class FailingExecutionCallback : public execution_callback<std::string> {
                public:
                    FailingExecutionCallback(const std::shared_ptr<boost::latch> &latch1) : latch1_(
                            latch1) {}

                    void on_response(const boost::optional<std::string> &response) override {
                    }

                    void on_failure(std::exception_ptr e) override {
                        exception_ = e;
                        latch1_->count_down();
                    }

                    std::exception_ptr get_exception() {
                        return exception_;
                    }

                private:
                    const std::shared_ptr<boost::latch> latch1_;
                    hazelcast::util::Sync<std::exception_ptr> exception_;
                };

                class SuccessfullExecutionCallback : public execution_callback<boost::uuids::uuid> {
                public:
                    SuccessfullExecutionCallback(const std::shared_ptr<boost::latch> &latch1) : latch1_(latch1) {}

                    void on_response(const boost::optional<boost::uuids::uuid> &response) override {
                        latch1_->count_down();
                    }

                    void on_failure(std::exception_ptr e) override {
                    }

                private:
                    const std::shared_ptr<boost::latch> latch1_;
                };

                template<typename T>
                class ResultSettingExecutionCallback : public execution_callback<T> {
                public:
                    explicit ResultSettingExecutionCallback(const std::shared_ptr<boost::latch> &latch1) : latch1_(latch1) {}

                    void on_response(const boost::optional<T> &response) override {
                        result_.set(response);
                        latch1_->count_down();
                    }

                    void on_failure(std::exception_ptr e) override {
                    }

                    boost::optional<T> get_result() {
                        return result_.get();
                    }

                private:
                    const std::shared_ptr<boost::latch> latch1_;
                    hazelcast::util::Sync<boost::optional<T>> result_;
                };

                class MultiExecutionCompletionCallback : public multi_execution_callback<std::string> {
                public:
                    MultiExecutionCompletionCallback(std::string msg,
                                                     std::shared_ptr<boost::latch> response_latch,
                                                     const std::shared_ptr<boost::latch> &complete_latch) : msg_(std::move(msg)),
                                                                                                           response_latch_(std::move(
                                                                                                                   response_latch)),
                                                                                                           complete_latch_(
                                                                                                                   complete_latch) {}

                    void on_response(const member &member, const boost::optional<std::string> &response) override {
                        if (response && *response == msg_ + APPENDAGE) {
                            response_latch_->count_down();
                        }
                    }

                    void
                    on_failure(const member &member, std::exception_ptr exception) override {
                    }

                    void on_complete(const std::unordered_map<member, boost::optional<std::string> > &values,
                                            const std::unordered_map<member, std::exception_ptr> &exceptions) override {
                        typedef std::unordered_map<member, boost::optional<std::string> > VALUE_MAP;
                        std::string expectedValue(msg_ + APPENDAGE);
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (entry.second && *entry.second == expectedValue) {
                                complete_latch_->count_down();
                            }
                        }
                    }

                private:
                    std::string msg_;
                    const std::shared_ptr<boost::latch> response_latch_;
                    const std::shared_ptr<boost::latch> complete_latch_;
                };

                class MultiExecutionNullCallback : public multi_execution_callback<std::string> {
                public:
                    MultiExecutionNullCallback(std::shared_ptr<boost::latch> response_latch,
                                               std::shared_ptr<boost::latch> complete_latch)
                            : response_latch_(std::move(response_latch)), complete_latch_(std::move(complete_latch)) {}

                    void on_response(const member &member, const boost::optional<std::string> &response) override {
                        if (!response) {
                            response_latch_->count_down();
                        }
                    }

                    void
                    on_failure(const member &member, std::exception_ptr exception) override {
                    }

                    void on_complete(const std::unordered_map<member, boost::optional<std::string> > &values,
                                            const std::unordered_map<member, std::exception_ptr> &exceptions) override {
                        typedef std::unordered_map<member, boost::optional<std::string> > VALUE_MAP;
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (!entry.second) {
                                complete_latch_->count_down();
                            }
                        }
                    }

                private:
                    const std::shared_ptr<boost::latch> response_latch_;
                    const std::shared_ptr<boost::latch> complete_latch_;
                };

                static std::vector<HazelcastServer *> instances;
                static hazelcast_client *client;
                static HazelcastServerFactory *factory;
            };

            std::vector<HazelcastServer *>ClientExecutorServiceTest::instances;
            hazelcast_client *ClientExecutorServiceTest::client = nullptr;
            HazelcastServerFactory *ClientExecutorServiceTest::factory = nullptr;
            const size_t ClientExecutorServiceTest::numberOfMembers = 4;

            TEST_F(ClientExecutorServiceTest, testIsTerminated) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                ASSERT_FALSE(service->is_terminated().get());
            }

            TEST_F(ClientExecutorServiceTest, testIsShutdown) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                ASSERT_FALSE(service->is_shutdown().get());
            }

            TEST_F(ClientExecutorServiceTest, testShutdown) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->is_shutdown().get());
            }

            TEST_F(ClientExecutorServiceTest, testShutdownMultipleTimes) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                service->shutdown();
                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->is_shutdown().get());
            }

            TEST_F(ClientExecutorServiceTest, testCancellationAwareTask_whenTimeOut) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                executor::tasks::CancellationAwareTask task{INT64_MAX};

                auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);

                ASSERT_EQ(boost::future_status::timeout, promise.get_future().wait_for(boost::chrono::seconds(1)));
            }

            TEST_F(ClientExecutorServiceTest, testFutureAfterCancellationAwareTaskTimeOut) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                executor::tasks::CancellationAwareTask task{INT64_MAX};

                auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);
                auto future = promise.get_future();

                ASSERT_EQ(boost::future_status::timeout, future.wait_for(boost::chrono::seconds(1)));

                ASSERT_FALSE(future.is_ready());
            }

            TEST_F(ClientExecutorServiceTest, testGetFutureAfterCancel) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                executor::tasks::CancellationAwareTask task{INT64_MAX};

                auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);

                auto future = promise.get_future();
                ASSERT_EQ(boost::future_status::timeout, future.wait_for(boost::chrono::seconds(1)));

                ASSERT_TRUE(promise.cancel(true));

                ASSERT_THROW(future.get(), exception::CancellationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                executor::tasks::FailingCallable task;

                auto future = service->submit<executor::tasks::FailingCallable, std::string>(task).get_future();

                ASSERT_THROW(future.get(), exception::IllegalStateException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException_withExecutionCallback) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));

                executor::tasks::FailingCallable task;
                std::shared_ptr<execution_callback<std::string> > callback(new FailingExecutionCallback(latch1));

                service->submit<executor::tasks::FailingCallable, std::string>(task, callback);

                ASSERT_OPEN_EVENTUALLY(*latch1);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableReasonExceptionCause) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                auto failingFuture = service->submit<executor::tasks::FailingCallable, std::string>(
                        executor::tasks::FailingCallable()).get_future();

                ASSERT_THROW(failingFuture.get(), exception::IllegalStateException);
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withNoMemberSelected) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::string mapName = random_map_name();

                executor::tasks::SelectNoMembers selector;

                ASSERT_THROW(service->execute<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>>(
                        executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>{mapName, spi::ClientContext(
                                *client).random_uuid()}, selector),
                             exception::RejectedExecutionException);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce) {
                std::string name = get_test_name();

                std::shared_ptr<iexecutor_service> service = client->get_executor_service(name);

                executor::tasks::SerializedCounterCallable counterCallable{0};

                auto future = service->submit_to_key_owner<executor::tasks::SerializedCounterCallable, int, std::string>(
                        counterCallable, name).get_future();
                auto value = future.get();
                ASSERT_TRUE(value);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce_submitToAddress) {
                std::string name = get_test_name();

                std::shared_ptr<iexecutor_service> service = client->get_executor_service(name);

                executor::tasks::SerializedCounterCallable counterCallable{0};

                std::vector<member> members = client->get_cluster().get_members();
                ASSERT_FALSE(members.empty());
                auto future = service->submit_to_member<executor::tasks::SerializedCounterCallable, int>(
                        counterCallable, members[0]).get_future();
                auto value = future.get();
                ASSERT_TRUE(value);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClient) {
                std::string name = get_test_name();

                std::shared_ptr<iexecutor_service> service = client->get_executor_service(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                auto future = service->submit<executor::tasks::TaskWithUnserializableResponse, bool>(
                        taskWithUnserializableResponse).get_future();

                ASSERT_THROW(future.get(), exception::HazelcastSerializationException);
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClientCallback) {
                std::string name = get_test_name();

                std::shared_ptr<iexecutor_service> service = client->get_executor_service(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));

                std::shared_ptr<FailingExecutionCallback> callback(new FailingExecutionCallback(latch1));

                service->submit<executor::tasks::TaskWithUnserializableResponse, std::string>(
                        taskWithUnserializableResponse, callback);

                ASSERT_OPEN_EVENTUALLY(*latch1);

                auto exception = callback->get_exception();
                ASSERT_THROW(std::rethrow_exception(exception), exception::HazelcastSerializationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMember) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                executor::tasks::GetMemberUuidTask task;

                std::vector<member> members = client->get_cluster().get_members();
                ASSERT_EQ(numberOfMembers, members.size());

                auto future = service->submit_to_member<executor::tasks::GetMemberUuidTask, boost::uuids::uuid>(
                        task, members[0]).get_future();

                auto uuid = future.get();
                ASSERT_TRUE(uuid);
                ASSERT_EQ(members[0].get_uuid(), uuid);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                executor::tasks::GetMemberUuidTask task;

                std::vector<member> members = client->get_cluster().get_members();
                ASSERT_EQ(numberOfMembers, members.size());

                auto futuresMap = service->submit_to_members<executor::tasks::GetMemberUuidTask, boost::uuids::uuid>(task,
                                                                                                            members);

                for (const member &member : members) {
                    ASSERT_EQ(1U, futuresMap.count(member));
                    auto it = futuresMap.find(member);
                    ASSERT_NE(futuresMap.end(), it);
                    auto uuid = (*it).second.get_future().get();
                    ASSERT_TRUE(uuid);
                    ASSERT_EQ(member.get_uuid(), uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withMemberSelector) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};
                executor::tasks::SelectAllMembers selectAll;

                auto f = service->submit<executor::tasks::AppendCallable, std::string>(callable, selectAll).get_future();

                auto result = f.get();
                ASSERT_TRUE(result);
                ASSERT_EQ(msg + APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers_withMemberSelector) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                executor::tasks::GetMemberUuidTask task;
                executor::tasks::SelectAllMembers selectAll;

                auto futuresMap = service->submit_to_members<executor::tasks::GetMemberUuidTask, boost::uuids::uuid>(
                        task, selectAll);

                for (auto &pair : futuresMap) {
                    const member &member = pair.first;
                    auto future = pair.second.get_future();

                    auto uuid = future.get();
                    ASSERT_TRUE(uuid);
                    ASSERT_EQ(member.get_uuid(), uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};

                auto futuresMap = service->submit_to_all_members<executor::tasks::AppendCallable, std::string>(callable);

                for (auto &pair : futuresMap) {
                    auto future = pair.second.get_future();

                    auto result = future.get();
                    ASSERT_TRUE(result);
                    ASSERT_EQ(msg + APPENDAGE, *result);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withExecutionCallback) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

                executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid> callable(testName, spi::ClientContext(*client).random_uuid());

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                std::shared_ptr<SuccessfullExecutionCallback> callback(new SuccessfullExecutionCallback(latch1));

                std::vector<member> members = client->get_cluster().get_members();
                ASSERT_EQ(numberOfMembers, members.size());

                service->submit_to_member<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>, boost::uuids::uuid>(callable, members[0],
                                                                                                    callback);

                auto map = client->get_map(testName);

                ASSERT_OPEN_EVENTUALLY(*latch1);
                ASSERT_EQ(1, map->size().get());
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withMultiExecutionCallback) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::shared_ptr<boost::latch> responseLatch(new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(new boost::latch(numberOfMembers));

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};
                std::vector<member> members = client->get_cluster().get_members();
                ASSERT_EQ(numberOfMembers, members.size());

                std::shared_ptr<multi_execution_callback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submit_to_members<executor::tasks::AppendCallable, std::string>(callable, members, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallable_withExecutionCallback) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};
                executor::tasks::SelectAllMembers selector;
                std::shared_ptr<boost::latch> responseLatch(new boost::latch(1));
                auto callback = std::make_shared<ResultSettingExecutionCallback<std::string>>(responseLatch);

                service->submit<executor::tasks::AppendCallable, std::string>(callable, selector,
                                                                              std::static_pointer_cast<execution_callback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                auto message = callback->get_result();
                ASSERT_TRUE(message.has_value());
                ASSERT_EQ(msg + APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMembers_withExecutionCallback) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::shared_ptr<boost::latch> responseLatch(
                        new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(
                        new boost::latch(numberOfMembers));

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};
                executor::tasks::SelectAllMembers selector;

                std::shared_ptr<multi_execution_callback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submit_to_members<executor::tasks::AppendCallable, std::string>(callable, selector, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::shared_ptr<boost::latch> responseLatch(
                        new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(
                        new boost::latch(numberOfMembers));

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};

                std::shared_ptr<multi_execution_callback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submit_to_all_members<executor::tasks::AppendCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*completeLatch);
                ASSERT_OPEN_EVENTUALLY(*responseLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableWithNullResultToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::shared_ptr<boost::latch> responseLatch(new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(new boost::latch(numberOfMembers));

                executor::tasks::NullCallable callable;

                std::shared_ptr<multi_execution_callback<std::string> > callback(
                        new MultiExecutionNullCallback(responseLatch, completeLatch));

                service->submit_to_all_members<executor::tasks::NullCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};

                auto result = service->submit<executor::tasks::AppendCallable, std::string>(callable).get_future();

                auto message = result.get();
                ASSERT_TRUE(message);
                ASSERT_EQ(msg + APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withExecutionCallback) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                auto callback = std::make_shared<ResultSettingExecutionCallback<std::string>>(latch1);

                service->submit<executor::tasks::AppendCallable, std::string>(callable, std::static_pointer_cast<execution_callback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*latch1);
                auto value = callback->get_result();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(msg + APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};

                auto f = service->submit_to_key_owner<executor::tasks::AppendCallable, std::string, std::string>(callable, "key").get_future();

                auto result = f.get();
                ASSERT_TRUE(result);
                ASSERT_EQ(msg + APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner_withExecutionCallback) {
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(get_test_name());

                std::string msg = random_string();
                executor::tasks::AppendCallable callable{msg};

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                auto callback = std::make_shared<ResultSettingExecutionCallback<std::string>>(latch1);

                service->submit_to_key_owner<executor::tasks::AppendCallable, std::string, std::string>(callable, "key",
                                                                                                     std::static_pointer_cast<execution_callback<std::string>>(
                                                                                                             callback));

                ASSERT_OPEN_EVENTUALLY(*latch1);
                auto value = callback->get_result();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(msg + APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

                auto map = client->get_map(testName);

                std::vector<member> members = client->get_cluster().get_members();
                spi::ClientContext clientContext(*client);
                member &member = members[0];
                auto key = generate_key_owned_by(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid> callable{testName, key};

                auto f = service->submit<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>, boost::uuids::uuid>(
                        callable).get_future();

                auto result = f.get();
                ASSERT_TRUE(result);
                ASSERT_EQ(member.get_uuid(), *result);
                ASSERT_TRUE(map->contains_key(member.get_uuid()).get());
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware_WithExecutionCallback) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

                auto map = client->get_map(testName);

                std::vector<member> members = client->get_cluster().get_members();
                spi::ClientContext clientContext(*client);
                member &member = members[0];
                auto key = generate_key_owned_by(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid> callable(testName, key);

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                auto callback = std::make_shared<ResultSettingExecutionCallback<boost::uuids::uuid>>(latch1);

                service->submit<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>, boost::uuids::uuid>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*latch1);
                auto value = std::static_pointer_cast<ResultSettingExecutionCallback<boost::uuids::uuid>>(callback)->get_result();
                ASSERT_TRUE(value);
                ASSERT_EQ(member.get_uuid(), *value);
                ASSERT_TRUE(map->contains_key(member.get_uuid()).get());
            }

            TEST_F(ClientExecutorServiceTest, testExecute) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

                service->execute(
                        executor::tasks::MapPutPartitionAwareCallable<std::string>(testName, "key"));

                auto map = client->get_map(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withMemberSelector) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);
                executor::tasks::SelectAllMembers selector;

                service->execute(
                        executor::tasks::MapPutPartitionAwareCallable<std::string>(testName, "key"), selector);
                auto map = client->get_map(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnKeyOwner) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

                auto map = client->get_map(testName);
                map->put(1, 1).get();

                std::vector<member> members = client->get_cluster().get_members();
                spi::ClientContext clientContext(*client);
                member &member = members[0];
                auto targetUuid = member.get_uuid();
                auto key = generate_key_owned_by(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid> callable(testName, key);

                service->execute_on_key_owner<executor::tasks::MapPutPartitionAwareCallable<boost::uuids::uuid>, boost::uuids::uuid>(callable, key);

                ASSERT_TRUE_EVENTUALLY(map->contains_key(targetUuid).get());
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMember) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

                auto map = client->get_map(testName);

                std::vector<member> members = client->get_cluster().get_members();
                member &member = members[0];
                auto targetUuid = member.get_uuid();

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                service->execute_on_member(callable, member);

                ASSERT_TRUE_EVENTUALLY(map->contains_key(targetUuid).get());
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

                auto map = client->get_map(testName);

                std::vector<member> allMembers = client->get_cluster().get_members();
                std::vector<member> members(allMembers.begin(), allMembers.begin() + 2);

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                service->execute_on_members(callable, members);

                ASSERT_TRUE_EVENTUALLY(map->contains_key(members[0].get_uuid()).get() && map->contains_key(members[1].get_uuid()).get());
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withEmptyCollection) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

               auto map = client->get_map(testName);

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                service->execute_on_members(callable, std::vector<member>());

                assertSizeEventually(0, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withSelector) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

                auto map = client->get_map(testName);

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                executor::tasks::SelectAllMembers selector;

                service->execute_on_members(callable, selector);

                assertSizeEventually((int) numberOfMembers, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnAllMembers) {
                std::string testName = get_test_name();
                std::shared_ptr<iexecutor_service> service = client->get_executor_service(testName);

                auto map = client->get_map(testName);

                executor::tasks::MapPutPartitionAwareCallable<std::string> callable(testName, "key");

                service->execute_on_all_members(callable);

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
                    client::config::client_aws_config awsConfig;
                    ASSERT_EQ("", awsConfig.get_access_key());
                    ASSERT_EQ("us-east-1", awsConfig.get_region());
                    ASSERT_EQ("ec2.amazonaws.com", awsConfig.get_host_header());
                    ASSERT_EQ("", awsConfig.get_iam_role());
                    ASSERT_EQ("", awsConfig.get_secret_key());
                    ASSERT_EQ("", awsConfig.get_security_group_name());
                    ASSERT_EQ("", awsConfig.get_tag_key());
                    ASSERT_EQ("", awsConfig.get_tag_value());
                    ASSERT_FALSE(awsConfig.is_inside_aws());
                    ASSERT_FALSE(awsConfig.is_enabled());
                }

                TEST_F (AwsConfigTest, testSetValues) {
                    client::config::client_aws_config awsConfig;

                    awsConfig.set_access_key("mykey");
                    awsConfig.set_region("myregion");
                    awsConfig.set_host_header("myheader");
                    awsConfig.set_iam_role("myrole");
                    awsConfig.set_secret_key("mysecret");
                    awsConfig.set_security_group_name("mygroup");
                    awsConfig.set_tag_key("mytagkey");
                    awsConfig.set_tag_value("mytagvalue");
                    awsConfig.set_inside_aws(true);
                    awsConfig.set_enabled(true);

                    ASSERT_EQ("mykey", awsConfig.get_access_key());
                    ASSERT_EQ("myregion", awsConfig.get_region());
                    ASSERT_EQ("myheader", awsConfig.get_host_header());
                    ASSERT_EQ("myrole", awsConfig.get_iam_role());
                    ASSERT_EQ("mysecret", awsConfig.get_secret_key());
                    ASSERT_EQ("mygroup", awsConfig.get_security_group_name());
                    ASSERT_EQ("mytagkey", awsConfig.get_tag_key());
                    ASSERT_EQ("mytagvalue", awsConfig.get_tag_value());
                    ASSERT_TRUE(awsConfig.is_inside_aws());
                    ASSERT_TRUE(awsConfig.is_enabled()) << awsConfig;
                }

                TEST_F (AwsConfigTest, testSetEmptyValues) {
                    client::config::client_aws_config awsConfig;

                    ASSERT_THROW(awsConfig.set_access_key(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.set_region(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.set_host_header(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.set_secret_key(""), exception::IllegalArgumentException);
                }

                TEST_F (AwsConfigTest, testClientConfigUsage) {
                    client_config clientConfig;
                    client::config::client_aws_config &awsConfig = clientConfig.get_network_config().get_aws_config();
                    awsConfig.set_enabled(true);

                    ASSERT_TRUE(clientConfig.get_network_config().get_aws_config().is_enabled());

                    client::config::client_aws_config newConfig;

                    clientConfig.get_network_config().set_aws_config(newConfig);
                    // default constructor sets enabled to false
                    ASSERT_FALSE(clientConfig.get_network_config().get_aws_config().is_enabled());
                }

                TEST_F (AwsConfigTest, testInvalidAwsMemberPortConfig) {
                    client_config clientConfig = get_config();

                    clientConfig.set_property(client_properties::PROP_AWS_MEMBER_PORT, "65536");
                    clientConfig.get_network_config().get_aws_config().set_enabled(true).
                            set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(getenv("AWS_SECRET_ACCESS_KEY")).
                            set_tag_key("aws-test-tag").set_tag_value("aws-tag-value-1").set_inside_aws(true);

                    clientConfig.set_property(client_properties::PROP_AWS_MEMBER_PORT, "-1");

                    ASSERT_THROW(hazelcast_client hazelcastClient(clientConfig),
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
                    client_config clientConfig;

                    clientConfig.set_property(client_properties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.get_network_config().get_aws_config().set_enabled(true).
                            set_access_key(std::getenv("AWS_ACCESS_KEY_ID")).set_secret_key(std::getenv("AWS_SECRET_ACCESS_KEY")).
                            set_tag_key("aws-test-tag").set_tag_value("aws-tag-value-1");

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    clientConfig.get_network_config().get_aws_config().set_inside_aws(true);
#else
                    clientConfig.get_network_config().get_aws_config().set_inside_aws(false);
#endif
                    hazelcast_client hazelcastClient(clientConfig);
                    auto map = hazelcastClient.get_map("myMap");
                    map->put(5, 20).get();
                    auto val = map->get<int, int>(5).get();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ(20, val.value());
                }

                TEST_F (AwsClientTest, testClientAwsMemberWithSecurityGroupDefaultIamRole) {
                    client_config clientConfig;
                    clientConfig.set_property(client_properties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.get_network_config().get_aws_config().set_enabled(true).
                            set_security_group_name("launch-wizard-147");

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                                                                                                                                            // The access key and secret will be retrieved from default IAM role at windows machine
                    clientConfig.get_network_config().get_aws_config().set_inside_aws(true);
#else
                    clientConfig.get_network_config().get_aws_config().set_access_key(std::getenv("AWS_ACCESS_KEY_ID")).
                            set_secret_key(std::getenv("AWS_SECRET_ACCESS_KEY"));
#endif

                    hazelcast_client hazelcastClient(clientConfig);
                    auto map = hazelcastClient.get_map("myMap");
                    map->put(5, 20).get();
                    auto val = map->get<int, int>(5).get();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ(20, val.value());
                }

                // FIPS_mode_set is not available for Mac OS X built-in openssl library
#ifndef __APPLE__
                    TEST_F (AwsClientTest, testFipsEnabledAwsDiscovery) {
                    client_config clientConfig = get_config();

                    clientConfig.set_property(client_properties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.get_network_config().get_aws_config().set_enabled(true).
                            set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(getenv("AWS_SECRET_ACCESS_KEY")).
                            set_tag_key("aws-test-tag").set_tag_value("aws-tag-value-1");

                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    clientConfig.get_network_config().get_aws_config().set_inside_aws(true);
                    #else
                    clientConfig.get_network_config().get_aws_config().set_inside_aws(false);
                    #endif

                    // Turn Fips mode on
                    FIPS_mode_set(1);

                    hazelcast_client hazelcastClient(clientConfig);
                    auto map = hazelcastClient.get_map("myMap");
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
                    client_config clientConfig = get_config();

                    clientConfig.set_property(client_properties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.get_network_config().get_aws_config().set_enabled(true).set_iam_role("cloudbees-role").set_tag_key(
                            "aws-test-tag").set_tag_value("aws-tag-value-1").set_inside_aws(true);

                    hazelcast_client hazelcastClient(clientConfig);
                }

                TEST_F (AwsClientTest, testRetrieveCredentialsFromInstanceProfileDefaultIamRoleAndConnect) {
                    client_config clientConfig = get_config();

                    clientConfig.set_property(client_properties::PROP_AWS_MEMBER_PORT, "60000");
                    clientConfig.get_network_config().get_aws_config().set_enabled(true).set_tag_key(
                            "aws-test-tag").set_tag_value("aws-tag-value-1").set_inside_aws(true);

                    hazelcast_client hazelcastClient(clientConfig);
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
                    client::config::client_aws_config awsConfig;
                    awsConfig.set_enabled(true).set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(
                            getenv("AWS_SECRET_ACCESS_KEY")).set_tag_key("aws-test-tag").set_tag_value("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.get_host_header(), get_logger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesTagAndNonExistentValueSet) {
                    client::config::client_aws_config awsConfig;
                    awsConfig.set_enabled(true).set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(
                            getenv("AWS_SECRET_ACCESS_KEY")).set_tag_key("aws-test-tag").set_tag_value(
                            "non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.get_host_header(), get_logger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSet) {
                    client::config::client_aws_config awsConfig;
                    awsConfig.set_enabled(true).set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(
                            getenv("AWS_SECRET_ACCESS_KEY")).set_tag_key("aws-test-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.get_host_header(), get_logger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSetToNonExistentTag) {
                    client::config::client_aws_config awsConfig;
                    awsConfig.set_enabled(true).set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(
                            getenv("AWS_SECRET_ACCESS_KEY")).set_tag_key("non-existent-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.get_host_header(), get_logger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSet) {
                    client::config::client_aws_config awsConfig;
                    awsConfig.set_enabled(true).set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(
                            getenv("AWS_SECRET_ACCESS_KEY")).set_tag_value("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.get_host_header(), get_logger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSetToNonExistentValue) {
                    client::config::client_aws_config awsConfig;
                    awsConfig.set_enabled(true).set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(
                            getenv("AWS_SECRET_ACCESS_KEY")).set_tag_value("non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.get_host_header(), get_logger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesSecurityGroup) {
                    client::config::client_aws_config awsConfig;
                    awsConfig.set_enabled(true).set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(
                            getenv("AWS_SECRET_ACCESS_KEY")).set_security_group_name("launch-wizard-147");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.get_host_header(), get_logger());
                    std::unordered_map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesNonExistentSecurityGroup) {
                    client::config::client_aws_config awsConfig;
                    awsConfig.set_enabled(true).set_access_key(getenv("AWS_ACCESS_KEY_ID")).set_secret_key(
                            getenv("AWS_SECRET_ACCESS_KEY")).set_security_group_name("non-existent-group");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.get_host_header(), get_logger());
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

                    config::client_aws_config awsConfig;
                    std::unordered_map<std::string, std::string> results = hazelcast::client::aws::utility::CloudUtility::unmarshal_the_response(
                            responseStream, get_logger());
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
