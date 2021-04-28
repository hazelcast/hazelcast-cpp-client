/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <cmath>
#include <ctime>
#include <fstream>
#include <memory>
#include <regex>
#include <thread>
#include <vector>
#include <iostream>

#include <gtest/gtest.h>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include <hazelcast/client/client_config.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/entry_event.h>
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/impl/Partition.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <hazelcast/client/internal/socket/SSLSocket.h>
#include <hazelcast/client/itopic.h>
#include <hazelcast/client/multi_map.h>
#include <hazelcast/client/pipelining.h>
#include <hazelcast/client/protocol/ClientExceptionFactory.h>
#include <hazelcast/client/protocol/ClientMessage.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/util/BlockingConcurrentQueue.h>
#include <hazelcast/util/MurmurHash3.h>
#include <hazelcast/util/Util.h>

#include "ClientTestSupport.h"
#include "HazelcastServer.h"
#include "HazelcastServerFactory.h"
#include "TestHelperFunctions.h"


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapTestBase : public ClientTestSupport {
            public:
                struct SamplePortable {
                    int32_t a;
                };
            protected:
                static constexpr size_t OPERATION_COUNT = 100;
                typedef std::vector<std::pair<int, int>> TEST_VALUES_TYPE;

                template<typename Functor>
                void execute_for_each(Functor f) {
                    for (size_t i = 0; i < OPERATION_COUNT; i++) {
                        f(i);
                    }
                }

                void verify_entries_in_map(const std::shared_ptr<replicated_map>& map) {
                    auto entries = map->entry_set<typed_data, typed_data>().get();
                    ASSERT_EQ(OPERATION_COUNT, entries.size());
                    for (auto &entry : entries) {
                        auto key = entry.first.get<std::string>();
                        ASSERT_TRUE(key.has_value());
                        ASSERT_EQ(0U, key.value().find("foo-"));
                        auto val = entry.second.get<std::string>();
                        ASSERT_TRUE(val.has_value());
                        ASSERT_EQ("bar", val.value());
                    }
                }

                void get_and_verify_entries_in_map(const std::shared_ptr<replicated_map> &map) {
                    get_and_verify_entries_in_map(map, "bar");
                }

                void
                get_and_verify_entries_in_map(const std::shared_ptr<replicated_map>& map, const std::string &expected_value) {
                    execute_for_each([=] (size_t i) {
                        auto key = std::string("foo-") + std::to_string(i);
                        boost::optional<std::string> val = map->get<std::string, std::string>(key).get();
                        ASSERT_TRUE((val));
                        ASSERT_EQ(expected_value, (val.value()));
                    });
                }

                void put_all_entries_into_map(std::shared_ptr<replicated_map> map) {
                    std::unordered_map<std::string, std::string> mapTest;
                    execute_for_each([=, &mapTest] (size_t i) {
                        mapTest[std::string("foo-") + std::to_string(i)] = "bar";
                    });
                    map->put_all(mapTest).get();
                    ASSERT_EQ((int) OPERATION_COUNT, map->size().get());
                }

                void put_entries_into_map(const std::shared_ptr<replicated_map>& map) {
                    execute_for_each([=] (size_t i) {
                        auto oldEntry = map->put<std::string, std::string>(std::string("foo-") + std::to_string(i),
                                                                           "bar").get();
                        ASSERT_FALSE(oldEntry);
                    });
                }

                void put_entries_into_map(const std::shared_ptr<replicated_map> &map, const std::string value) {
                    execute_for_each([&] (size_t i) {
                        map->put<std::string, std::string>(std::string("foo-") + std::to_string(i),
                                                           value).get();
                    });
                }

                static bool find_value_for_key(int key, TEST_VALUES_TYPE &test_values, int &value) {
                    for (const TEST_VALUES_TYPE::value_type &entry : test_values) {
                        if (key == entry.first) {
                            value = entry.second;
                            return true;
                        }
                    }
                    return false;
                }

                template<typename T>
                bool contains(std::vector<T> &values, const T &value) {
                    return std::find(values.begin(), values.end(), value) != values.end();
                }

                static TEST_VALUES_TYPE build_test_values() {
                    TEST_VALUES_TYPE testValues;
                    for (int i = 0; i < 100; ++i) {
                        testValues.emplace_back(i, i * i);
                    }
                    return testValues;
                }

                static hazelcast_client* create_client() {
                    auto config = get_config();
                    config.set_cluster_name("replicated-map-binary-test");
                    return new hazelcast_client(new_client(std::move(config)).get());
                }

                static client_config get_client_config_with_near_cache_invalidation_enabled() {
                    config::near_cache_config nearCacheConfig;
                    nearCacheConfig.set_invalidate_on_change(true).set_in_memory_format(config::BINARY);
                    return std::move(get_config().set_cluster_name("replicated-map-binary-test").add_near_cache_config(nearCacheConfig));
                }
            };

            constexpr size_t ClientReplicatedMapTestBase::OPERATION_COUNT;

            class ClientReplicatedMapTest : public ClientReplicatedMapTestBase {
            protected:
                virtual void SetUp() {
                    ASSERT_TRUE(factory);
                    ASSERT_TRUE(instance1);
                    ASSERT_TRUE(client);
                    ASSERT_TRUE(client2);
                }

                static void SetUpTestCase() {
                    factory = new HazelcastServerFactory(g_srvFactory->get_server_address(),
                            "hazelcast/test/resources/replicated-map-binary-in-memory-config-hazelcast.xml");
                    instance1 = new HazelcastServer(*factory);

                    client = create_client();
                    client2 = create_client();
                }

                static void TearDownTestCase() {
                    delete client;
                    delete client2;
                    delete instance1;
                    delete factory;

                    client = nullptr;
                    client2 = nullptr;
                    instance1 = nullptr;
                    factory = nullptr;
                }

                static HazelcastServer *instance1;
                static hazelcast_client *client;
                static hazelcast_client *client2;
                static HazelcastServerFactory *factory;
            };

            HazelcastServer *ClientReplicatedMapTest::instance1 = nullptr;
            hazelcast_client *ClientReplicatedMapTest::client = nullptr;
            hazelcast_client *ClientReplicatedMapTest::client2 = nullptr;
            HazelcastServerFactory *ClientReplicatedMapTest::factory = nullptr;

            TEST_F(ClientReplicatedMapTest, testEmptyMapIsEmpty) {
                std::shared_ptr<replicated_map> map = client->get_replicated_map(get_test_name()).get();
                ASSERT_TRUE(map->is_empty().get()) << "map should be empty";
            }

            TEST_F(ClientReplicatedMapTest, testNonEmptyMapIsNotEmpty) {
                auto map = client->get_replicated_map(get_test_name()).get();
                map->put(1, 1).get();
                ASSERT_FALSE(map->is_empty().get()) << "map should not be empty";
            }

            TEST_F(ClientReplicatedMapTest, testPutAll) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                put_all_entries_into_map(map1);
                verify_entries_in_map(map1);
                verify_entries_in_map(map2);

// TODO add server side data check using remote controller scripting
            }

            TEST_F(ClientReplicatedMapTest, testGet) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();
                put_entries_into_map(map1);
                get_and_verify_entries_in_map(map1);
                get_and_verify_entries_in_map(map2);
            }

            TEST_F(ClientReplicatedMapTest, testPutNullReturnValueDeserialization) {
                auto map = client->get_replicated_map(get_test_name()).get();
                ASSERT_FALSE(map->put(1, 2).get().has_value()) << "Put should return null";
            }

            TEST_F(ClientReplicatedMapTest, testPutReturnValueDeserialization) {
                auto map = client->get_replicated_map(get_test_name()).get();
                map->put(1, 2).get();
                auto value = map->put(1, 3).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(2, value.value());
            }

            TEST_F(ClientReplicatedMapTest, testAdd) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                put_entries_into_map(map1);
                ASSERT_EQ(OPERATION_COUNT, map2->size().get());

                verify_entries_in_map(map2);
                verify_entries_in_map(map1);
            }

            TEST_F(ClientReplicatedMapTest, testClear) {
                std::shared_ptr<replicated_map> map1 =client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 =client2->get_replicated_map(get_test_name()).get();

                put_entries_into_map(map1);
                ASSERT_EQ(OPERATION_COUNT, map2->size().get());

                verify_entries_in_map(map1);
                verify_entries_in_map(map2);

                map1->clear().get();
                ASSERT_EQ(0, map1->size().get());
                ASSERT_EQ(0, map2->size().get());
            }

            TEST_F(ClientReplicatedMapTest, testUpdate) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                put_entries_into_map(map1);
                ASSERT_EQ(OPERATION_COUNT, map2->size().get());

                verify_entries_in_map(map1);
                verify_entries_in_map(map2);

                put_entries_into_map(map1, "bar2");

                get_and_verify_entries_in_map(map2, "bar2");
                get_and_verify_entries_in_map(map1, "bar2");
            }

            TEST_F(ClientReplicatedMapTest, testRemove) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                put_entries_into_map(map1);
                ASSERT_EQ(OPERATION_COUNT, map2->size().get());

                verify_entries_in_map(map2);
                verify_entries_in_map(map1);

                execute_for_each([=] (size_t index) {
                    auto val = map2->remove<std::string, std::string>(std::string("foo-") + std::to_string(index)).get();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ("bar", val.value());
                });

                execute_for_each([=](size_t index) {
                    auto key = std::string("foo-") + std::to_string(index);
                    ASSERT_FALSE((map1->get<std::string, std::string>(key).get().has_value()))
                                                << "Removed value should not exist for map1";
                    ASSERT_FALSE((map2->get<std::string, std::string>(key).get().has_value()))
                                                << "Removed value should not exist for map2";
                });
            }

            TEST_F(ClientReplicatedMapTest, testSize) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                TEST_VALUES_TYPE testValues = build_test_values();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    auto map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second).get();
                }

                ASSERT_EQ((int32_t) testValues.size(), map1->size().get());
                ASSERT_EQ((int32_t) testValues.size(), map2->size().get());
            }

            TEST_F(ClientReplicatedMapTest, testContainsKey) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                put_entries_into_map(map1);

                execute_for_each([=] (size_t i) {
                    ASSERT_TRUE(map2->contains_key(std::string("foo-") + std::to_string(i)).get());
                });

                execute_for_each([=] (size_t i) {
                    ASSERT_TRUE(map2->contains_key(std::string("foo-") + std::to_string(i)).get());
                });
            }

            TEST_F(ClientReplicatedMapTest, testContainsValue) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                TEST_VALUES_TYPE testValues = build_test_values();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    auto map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second).get();
                }

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(map2->contains_value(entry.second).get());
                }

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(map1->contains_value(entry.second).get());
                }
            }

            TEST_F(ClientReplicatedMapTest, testValues) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                TEST_VALUES_TYPE testValues = build_test_values();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    auto map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second);
                }

                auto values1 = map1->values<int>().get();
                auto values2 = map2->values<int>().get();
                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    contains(values1, entry.second);
                    contains(values2, entry.second);
                }
            }

            TEST_F(ClientReplicatedMapTest, testKeySet) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                TEST_VALUES_TYPE testValues = build_test_values();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    auto map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second).get();
                }

                auto keys1 = map1->key_set<int>().get();
                auto keys2 = map2->key_set<int>().get();

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(contains(keys1, entry.first));
                    ASSERT_TRUE(contains(keys2, entry.first));
                }
            }

            TEST_F(ClientReplicatedMapTest, testEntrySet) {
                std::shared_ptr<replicated_map> map1 = client->get_replicated_map(get_test_name()).get();
                std::shared_ptr<replicated_map> map2 = client2->get_replicated_map(get_test_name()).get();

                TEST_VALUES_TYPE testValues = build_test_values();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    auto map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second).get();
                }

                auto entrySet1 = map1->entry_set<typed_data, typed_data>().get();
                auto entrySet2 = map2->entry_set<typed_data, typed_data>().get();

                for (auto &entry : entrySet2) {
                    int value;
                    ASSERT_TRUE(find_value_for_key(entry.first.get<int>().value(), testValues, value));
                    ASSERT_EQ(value, entry.second.get<int>().value());
                }

                for (auto &entry : entrySet1) {
                    int value;
                    ASSERT_TRUE(find_value_for_key(entry.first.get<int>().value(), testValues, value));
                    ASSERT_EQ(value, entry.second.get<int>().value());
                }
            }

            TEST_F(ClientReplicatedMapTest, testRetrieveUnknownValue) {
                std::shared_ptr<replicated_map> map = client->get_replicated_map(get_test_name()).get();
                auto value = map->get<std::string, std::string>("foo").get();
                ASSERT_FALSE(value.has_value()) << "No entry with key foo should exist";
            }

            TEST_F(ClientReplicatedMapTest, testClientPortableWithoutRegisteringToNode) {
                auto sampleMap = client->get_replicated_map(get_test_name()).get();
                sampleMap->put(1, SamplePortable{666});
                auto samplePortable = sampleMap->get<int, SamplePortable>(1).get();
                ASSERT_TRUE(samplePortable.has_value());
                ASSERT_EQ(666, samplePortable->a);
            }

            class ClientReplicatedMapInvalidation : public ClientReplicatedMapTestBase {
            };

            TEST_F(ClientReplicatedMapInvalidation, testNearCacheInvalidation) {
                HazelcastServerFactory factory(g_srvFactory->get_server_address(),
                                               "hazelcast/test/resources/replicated-map-binary-in-memory-config-hazelcast.xml");
                HazelcastServer server(factory);

                std::string mapName = random_string();

                hazelcast_client client1(new_client(get_client_config_with_near_cache_invalidation_enabled()).get());
                hazelcast_client client2(new_client(get_client_config_with_near_cache_invalidation_enabled()).get());

                auto replicatedMap1 = client1.get_replicated_map(mapName).get();

                replicatedMap1->put(1, 1).get();
                // puts key 1 to Near Cache
                replicatedMap1->get<int, int>(1).get();

                auto replicatedMap2 = client2.get_replicated_map(mapName).get();
                // this should invalidate Near Cache of replicatedMap1
                replicatedMap2->clear().get();

                ASSERT_FALSE_EVENTUALLY((replicatedMap1->get<int, int>(1).get().has_value()));
            }

        }

        namespace serialization {
            template<>
            struct hz_serializer<test::ClientReplicatedMapTest::SamplePortable> : public portable_serializer {
                static int32_t get_factory_id() {
                    return 5;
                }

                static int32_t get_class_id() {
                    return 6;
                }

                static void write_portable(test::ClientReplicatedMapTest::SamplePortable object, serialization::portable_writer &out) {
                    out.write<int32_t>("a", object.a);
                }

                static test::ClientReplicatedMapTest::SamplePortable read_portable(serialization::portable_reader &reader) {
                    return {reader.read<int32_t>("a")};
                }
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapListenerTest : public ClientTestSupport {
            protected:
                
                struct ListenerState {
                    ListenerState() : keys(UINT_MAX) {}
                    hazelcast::util::BlockingConcurrentQueue<int> keys;
                    std::atomic<int> add_count{ 0 }, remove_count{ 0 }, update_count{ 0 }, evict_count{ 0 },
                    map_clear_count{ 0 }, map_evict_count{ 0 };
                };

                entry_listener make_event_counting_listener(ListenerState &state) {
                    const auto pushKey = [&state](const entry_event &event) {
                        state.keys.push(event.get_key().get<int>().value());
                    };

                    return entry_listener().
                            on_added([&state, pushKey](entry_event &&event) {
                                pushKey(event);
                                ++state.add_count;
                            }).
                            on_removed([&state, pushKey](entry_event &&event) {
                                pushKey(event);
                                ++state.remove_count;
                            }).
                            on_updated([&state, pushKey](entry_event &&event) {
                                pushKey(event);
                                ++state.update_count;
                            }).
                            on_evicted([&state, pushKey](entry_event &&event) {
                                pushKey(event);
                                ++state.evict_count;
                            }).
                            on_map_evicted([&state](map_event &&) {
                                ++state.map_evict_count;
                            }).
                            on_map_cleared([&state](map_event &&) {
                                ++state.map_clear_count;
                            });
                }

                static void SetUpTestCase() {
                    instance1 = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    client = new hazelcast_client{new_client(get_config()).get()};
                    client2 = new hazelcast_client{new_client(get_config()).get()};
                }

                static void TearDownTestCase() {
                    delete client;
                    delete client2;
                    delete instance1;
                    delete instance2;

                    client = nullptr;
                    client2 = nullptr;
                    instance1 = nullptr;
                    instance2 = nullptr;
                }

                static HazelcastServer *instance1;
                static HazelcastServer *instance2;
                static hazelcast_client *client;
                static hazelcast_client *client2;
                ListenerState state_;
            };

            HazelcastServer *ClientReplicatedMapListenerTest::instance1 = nullptr;
            HazelcastServer *ClientReplicatedMapListenerTest::instance2 = nullptr;
            hazelcast_client *ClientReplicatedMapListenerTest::client = nullptr;
            hazelcast_client *ClientReplicatedMapListenerTest::client2 = nullptr;

            TEST_F(ClientReplicatedMapListenerTest, testEntryAdded) {
                auto replicatedMap = client->get_replicated_map(get_test_name()).get();
                replicatedMap->add_entry_listener(make_event_counting_listener(state_)).get();
                replicatedMap->put(1, 1).get();
                ASSERT_EQ_EVENTUALLY(1, state_.add_count.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testEntryUpdated) {
                auto replicatedMap = client->get_replicated_map(get_test_name()).get();
                replicatedMap->add_entry_listener(make_event_counting_listener(state_)).get();
                replicatedMap->put(1, 1).get();
                replicatedMap->put(1, 2).get();
                ASSERT_EQ_EVENTUALLY(1, state_.update_count.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testEntryRemoved) {
                auto replicatedMap = client->get_replicated_map(get_test_name()).get();
                replicatedMap->add_entry_listener(make_event_counting_listener(state_)).get();
                replicatedMap->put(1, 1).get();
                replicatedMap->remove<int, int>(1).get();
                ASSERT_EQ_EVENTUALLY(1, state_.remove_count.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testMapClear) {
                auto replicatedMap = client->get_replicated_map(get_test_name()).get();
                replicatedMap->add_entry_listener(make_event_counting_listener(state_)).get();
                replicatedMap->put(1, 1).get();
                replicatedMap->clear().get();
                ASSERT_EQ_EVENTUALLY(1, state_.map_clear_count.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testListenToKeyForEntryAdded) {
                auto replicatedMap = client->get_replicated_map(get_test_name()).get();
                replicatedMap->add_entry_listener(make_event_counting_listener(state_), 1).get();
                replicatedMap->put(1, 1).get();
                replicatedMap->put(2, 2).get();
                ASSERT_TRUE_EVENTUALLY(
                        state_.keys.size() == 1U && state_.keys.pop() == 1 && state_.add_count.load() == 1);
            }

            TEST_F(ClientReplicatedMapListenerTest, testListenWithPredicate) {
                auto replicatedMap = client->get_replicated_map(get_test_name()).get();
                replicatedMap->add_entry_listener(make_event_counting_listener(state_), query::false_predicate(*client)).get();
                replicatedMap->put(2, 2).get();
                ASSERT_TRUE_ALL_THE_TIME((state_.add_count.load() == 0), 1);
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class BasicClientReplicatedMapNearCacheTest
                    : public ClientTestSupport, public ::testing::WithParamInterface<config::in_memory_format> {
            public:
                static void SetUpTestSuite() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestSuite() {
                    std::cout << "TearDownTestSuite() starting" << std::endl;
                    if (instance2) {
                        std::cout << "deleting instance2" << std::endl;
                        delete instance2;
                        std::cout << "deleted instance2" << std::endl;
                    }
                    if (instance) {
                        std::cout << "deleting instance" << std::endl;
                        delete instance;
                        std::cout << "deleted instance" << std::endl;
                    }
                    instance2 = nullptr;
                    instance = nullptr;
                    std::cout << "TearDownTestSuite() finished" << std::endl;
                }

                void SetUp() override {
                    near_cache_config_ = NearCacheTestUtils::create_near_cache_config(GetParam(), get_test_name());
                }

                void TearDown() override {
                    std::cout << "TearDown() started" << std::endl;
                    if (near_cached_map_) {
                        near_cached_map_->destroy().get();
                    }
                    if (no_near_cache_map_) {
                        no_near_cache_map_->destroy().get();
                    }
                    if (client_) {
                        client_->shutdown().get();
                    }
                    if (near_cached_client_) {
                        near_cached_client_->shutdown().get();
                    }
                    std::cout << "TearDown() finished" << std::endl;
                }

            protected:
                /**
                 * Provides utility methods for unified Near Cache tests.
                 */
                class NearCacheTestUtils : public ClientTestSupport {
                public:
                    /**
                     * Creates a {@link NearCacheConfig} with a given {@link InMemoryFormat}.
                     *
                     * @param inMemoryFormat the {@link InMemoryFormat} to set
                     * @return the {@link NearCacheConfig}
                     */
                    static config::near_cache_config create_near_cache_config(
                            config::in_memory_format in_memory_format, const std::string &map_name) {
                        config::near_cache_config nearCacheConfig;
                        nearCacheConfig.set_name(map_name).set_in_memory_format(in_memory_format).set_invalidate_on_change(true);

                        return nearCacheConfig;
                    }

                    /**
                     * Configures the {@link EvictionConfig} of the given {@link NearCacheConfig}.
                     *
                     * @param nearCacheConfig the {@link NearCacheConfig} to configure
                     * @param evictionPolicy  the {@link EvictionPolicy} to set
                     * @param maxSizePolicy   the {@link MaxSizePolicy} to set
                     * @param maxSize         the max size to set
                     */
                    static void set_eviction_config(config::near_cache_config &near_cache_config,
                                                  config::eviction_policy eviction_policy,
                                                  typename config::eviction_config::max_size_policy max_size_policy,
                                                  int max_size) {
                        near_cache_config.get_eviction_config().set_eviction_policy(eviction_policy)
                                .set_maximum_size_policy(max_size_policy).set_size(max_size);
                    }

                    /**
                     * Asserts the {@link NearCacheStats} for expected values.
                     *
                     * @param stats                   stats of the near cache
                     * @param expectedOwnedEntryCount the expected owned entry count
                     * @param expectedHits            the expected Near Cache hits
                     * @param expectedMisses          the expected Near Cache misses
                     */
                    static void assert_near_cache_stats(monitor::near_cache_stats &stats,
                                                     int64_t expected_owned_entry_count, int64_t expected_hits,
                                                     int64_t expected_misses) {
                        assert_near_cache_stats(stats, expected_owned_entry_count, expected_hits, expected_misses, 0, 0);
                    }

                    /**
                     * Asserts the {@link NearCacheStats} for expected values.
                     *
                     * @param stats                   stats of the near cache
                     * @param expectedOwnedEntryCount the expected owned entry count
                     * @param expectedHits            the expected Near Cache hits
                     * @param expectedMisses          the expected Near Cache misses
                     * @param expectedEvictions       the expected Near Cache evictions
                     * @param expectedExpirations     the expected Near Cache expirations
                     */
                    static void assert_near_cache_stats(monitor::near_cache_stats &stats,
                                                     int64_t expected_owned_entry_count, int64_t expected_hits,
                                                     int64_t expected_misses,
                                                     int64_t expected_evictions, int64_t expected_expirations) {
                        assert_equals_format("Near Cache entry count should be %ld, but was %ld ",
                                           expected_owned_entry_count, stats.get_owned_entry_count(), stats);
                        assert_equals_format("Near Cache hits should be %ld, but were %ld ",
                                           expected_hits, stats.get_hits(), stats);
                        assert_equals_format("Near Cache misses should be %ld, but were %ld ",
                                           expected_misses, stats.get_misses(), stats);
                        assert_equals_format("Near Cache evictions should be %ld, but were %ld ",
                                           expected_evictions, stats.get_evictions(), stats);
                        assert_equals_format("Near Cache expirations should be %ld, but were %ld ",
                                           expected_expirations, stats.get_expirations(), stats);
                    }

                    static void assert_equals_format(const char *message_format, int64_t expected, int64_t actual,
                                                   monitor::near_cache_stats &stats) {
                        char buf[300];
                        hazelcast::util::hz_snprintf(buf, 300, message_format, expected, actual);
                        ASSERT_EQ(expected, actual) << buf << "(" << stats.to_string() << ")";
                    }

                private:
                    NearCacheTestUtils() = delete;
                    NearCacheTestUtils(const NearCacheTestUtils &) = delete;
                };

                /**
                 * The default count to be inserted into the Near Caches.
                 */
                static const int DEFAULT_RECORD_COUNT;

                void create_context() {
                    create_no_near_cache_context();
                    create_near_cache_context();
                }

                void create_no_near_cache_context() {
                    client_ = std::unique_ptr<hazelcast_client>(
                            new hazelcast_client{new_client(get_config()).get()});
                    no_near_cache_map_ = client_->get_replicated_map(get_test_name()).get();
                }

                void create_near_cache_context() {
                    client_config nearCachedClientConfig = get_config();
                    nearCachedClientConfig.add_near_cache_config(near_cache_config_);
                    near_cached_client_ = std::unique_ptr<hazelcast_client>(
                            new hazelcast_client(
                                    new_client(std::move(nearCachedClientConfig)).get()));
                    near_cached_map_ = near_cached_client_->get_replicated_map(get_test_name()).get();
                    spi::ClientContext clientContext(*near_cached_client_);
                    near_cache_manager_ = &clientContext.get_near_cache_manager();
                    near_cache_ = near_cache_manager_->
                            get_near_cache<serialization::pimpl::data, serialization::pimpl::data, serialization::pimpl::data>(get_test_name());
                    this->stats_ = near_cache_ ? near_cache_->get_near_cache_stats() : nullptr;
                }

                void test_contains_key(bool use_near_cached_map_for_removal) {
                    create_no_near_cache_context();

                    // populate map
                    no_near_cache_map_->put<int, std::string>(1, "value1").get();
                    no_near_cache_map_->put<int, std::string>(2, "value2").get();
                    no_near_cache_map_->put<int, std::string>(3, "value3").get();

                    create_near_cache_context();

                    // populate Near Cache
                    near_cached_map_->get<int, std::string>(1).get();
                    near_cached_map_->get<int, std::string>(2).get();
                    near_cached_map_->get<int, std::string>(3).get();

                    ASSERT_TRUE(near_cached_map_->contains_key(1).get());
                    ASSERT_TRUE(near_cached_map_->contains_key(2).get());
                    ASSERT_TRUE(near_cached_map_->contains_key(3).get());
                    ASSERT_FALSE(near_cached_map_->contains_key(5).get());

                    // remove a key which is in the Near Cache
                    std::shared_ptr<replicated_map> &adapter = use_near_cached_map_for_removal
                                                                                 ? near_cached_map_
                                                                                 : no_near_cache_map_;
                    adapter->remove<int, std::string>(1).get();

                    WAIT_TRUE_EVENTUALLY(check_contain_keys());
                    ASSERT_FALSE(near_cached_map_->contains_key(1).get());
                    ASSERT_TRUE(near_cached_map_->contains_key(2).get());
                    ASSERT_TRUE(near_cached_map_->contains_key(3).get());
                    ASSERT_FALSE(near_cached_map_->contains_key(5).get());
                }

                bool check_contain_keys() {
                    return !near_cached_map_->contains_key(1).get() && near_cached_map_->contains_key(2).get() &&
                           near_cached_map_->contains_key(3).get() && !near_cached_map_->contains_key(5).get();
                }

                void
                assert_near_cache_invalidation_requests(monitor::near_cache_stats &stat, int64_t invalidation_requests) {
                    if (near_cache_config_.is_invalidate_on_change() && invalidation_requests > 0) {
                        auto &nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl &) stat;
                        ASSERT_EQ_EVENTUALLY(invalidation_requests, nearCacheStatsImpl.get_invalidation_requests());
                        nearCacheStatsImpl.reset_invalidation_events();
                    }
                }

                void populate_map() {
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        no_near_cache_map_->put<int, std::string>(i, std::string("value-") + std::to_string(i)).get();
                    }

                    assert_near_cache_invalidation_requests(*stats_, DEFAULT_RECORD_COUNT);
                }

                void populate_near_cache() {
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        auto value = near_cached_map_->get<int, std::string>(i).get();
                        ASSERT_TRUE(value.has_value());
                        ASSERT_EQ(std::string("value-" + std::to_string(i)), value.value());
                    }
                }

                std::shared_ptr<serialization::pimpl::data> get_near_cache_key(int key) {
                    spi::ClientContext clientContext(*client_);
                    return clientContext.get_serialization_service().to_shared_data<int>(&key);
                }

                int64_t get_expected_misses_with_local_update_policy() {
                    if (near_cache_config_.get_local_update_policy() ==
                        config::near_cache_config::CACHE) {
                        // we expect the first and second get() to be hits, since the value should be already be cached
                        return stats_->get_misses();
                    }
                    // we expect the first get() to be a miss, due to the replaced / invalidated value
                    return stats_->get_misses() + 1;
                }

                int64_t get_expected_hits_with_local_update_policy() {
                    if (near_cache_config_.get_local_update_policy() == config::near_cache_config::CACHE) {
                        // we expect the first and second get() to be hits, since the value should be already be cached
                        return stats_->get_hits() + 2;
                    }
                    // we expect the second get() to be a hit, since it should be served from the Near Cache
                    return stats_->get_hits() + 1;
                }

                bool check_misses_and_hits(int64_t &expected_misses, int64_t &expected_hits,
                                        boost::optional<std::string> &value) {
                    expected_misses = get_expected_misses_with_local_update_policy();
                    expected_hits = get_expected_hits_with_local_update_policy();

                    value = near_cached_map_->get<int, std::string>(1).get();
                    if (!value.has_value() || value.value() != "newValue") {
                        return false;
                    }
                    value = near_cached_map_->get<int, std::string>(1).get();
                    if (!value.has_value() || value.value() != "newValue") {
                        return false;
                    }

                    return expected_hits == stats_->get_hits() && expected_misses == stats_->get_misses();
                }

                void when_put_all_is_used_then_near_cache_should_be_invalidated(bool use_near_cache_adapter) {
                    create_no_near_cache_context();

                    create_near_cache_context();

                    populate_map();

                    populate_near_cache();

                    std::unordered_map<int, std::string> invalidationMap;
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        invalidationMap[i] = std::string("value-" + std::to_string(i));
                    }

                    // this should invalidate the Near Cache
                    std::shared_ptr<replicated_map> &adapter = use_near_cache_adapter ? near_cached_map_ : no_near_cache_map_;
                    adapter->put_all(invalidationMap).get();

                    WAIT_EQ_EVENTUALLY(0, near_cache_->size());
                    ASSERT_EQ(0, near_cache_->size()) << "Invalidation is not working on putAll()";
                }

                config::near_cache_config near_cache_config_;
                std::unique_ptr<hazelcast_client> client_;
                std::unique_ptr<hazelcast_client> near_cached_client_;
                std::shared_ptr<replicated_map> no_near_cache_map_;
                std::shared_ptr<replicated_map> near_cached_map_;
                hazelcast::client::internal::nearcache::NearCacheManager *near_cache_manager_{};
                std::shared_ptr<hazelcast::client::internal::nearcache::NearCache<serialization::pimpl::data, serialization::pimpl::data>> near_cache_;
                std::shared_ptr<monitor::near_cache_stats> stats_;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const int BasicClientReplicatedMapNearCacheTest::DEFAULT_RECORD_COUNT = 1000;
            HazelcastServer *BasicClientReplicatedMapNearCacheTest::instance = nullptr;
            HazelcastServer *BasicClientReplicatedMapNearCacheTest::instance2 = nullptr;

            /**
             * Checks that the Near Cache keys are correctly checked when {@link DataStructureAdapter#contains_key(Object)} is used.
             *
             * This variant uses the {@link NearCacheTestContext#nearCacheAdapter}, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest, testContainsKey_withUpdateOnNearCacheAdapter) {
                test_contains_key(true);
            }

            /**
             * Checks that the memory costs are calculated correctly.
             *
             * This variant uses the noNearCacheMap(client with no near cache), so we need to configure Near Cache
             * invalidation.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest, testContainsKey_withUpdateOnDataAdapter) {
                near_cache_config_.set_invalidate_on_change(true);
                test_contains_key(false);
            }

            /**
             * Checks that the Near Cache never returns its internal {@link NearCache#NULL_OBJECT} to the public API.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenEmptyMap_thenPopulatedNearCacheShouldReturnNull_neverNULLOBJECT) {
                create_context();

                for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                    // populate Near Cache
                    ASSERT_FALSE((near_cached_map_->get<int, std::string>(i).get().has_value()))
                                                << "Expected null from original data structure for key " << i;
                    // fetch value from Near Cache
                    ASSERT_FALSE((near_cached_map_->get<int, std::string>(i).get().has_value()))
                                                << "Expected null from Near cached data structure for key " << i;

                    // fetch internal value directly from Near Cache
                    std::shared_ptr<serialization::pimpl::data> key = get_near_cache_key(i);
                    auto value = near_cache_->get(key);
                    if (value.get() != NULL) {
                        // the internal value should either be `null` or `NULL_OBJECT`
                        ASSERT_EQ(
                        (hazelcast::client::internal::nearcache::NearCache<serialization::pimpl::data, serialization::pimpl::data>::NULL_OBJECT),
                                near_cache_->get(key)) << "Expected NULL_OBJECT in Near Cache for key " << i;
                    }
                }
            }

            /**
             * Checks that the Near Cache updates value for keys which are already in the Near Cache,
             * even if the Near Cache is full and the eviction is disabled (via {@link com.hazelcast.config.EvictionPolicy#NONE}.
             *
             * This variant uses the {@link NearCacheTestContext#nearCacheAdapter}, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnNearCacheAdapter) {
                int size = DEFAULT_RECORD_COUNT / 2;
                NearCacheTestUtils::set_eviction_config(near_cache_config_, config::NONE,
                                                                        config::eviction_config::ENTRY_COUNT,
                                                                        size);

                near_cache_config_.set_invalidate_on_change(false);

                create_no_near_cache_context();

                create_near_cache_context();

                populate_map();

                populate_near_cache();

                ASSERT_EQ(size, near_cache_->size());
                auto value = near_cached_map_->get<int, std::string>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("value-1", value.value());

                near_cached_map_->put<int, std::string>(1, "newValue").get();

                int64_t expectedMisses = get_expected_misses_with_local_update_policy();
                int64_t expectedHits = get_expected_hits_with_local_update_policy();

                value = near_cached_map_->get<int, std::string>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("newValue", value.value());
                value = near_cached_map_->get<int, std::string>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("newValue", value.value());

                NearCacheTestUtils::assert_near_cache_stats(*stats_, size, expectedHits, expectedMisses);
            }

            /**
             * Checks that the Near Cache updates value for keys which are already in the Near Cache,
             * even if the Near Cache is full an the eviction is disabled (via {@link com.hazelcast.config.EvictionPolicy#NONE}.
             *
             * This variant uses the {@link NearCacheTestContext#dataAdapter}, so we need to configure Near Cache invalidation.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnDataAdapter) {
                int size = DEFAULT_RECORD_COUNT / 2;
                NearCacheTestUtils::set_eviction_config(near_cache_config_, config::NONE,
                                                                        config::eviction_config::ENTRY_COUNT,
                                                                        size);
                near_cache_config_.set_invalidate_on_change(true);

                create_no_near_cache_context();

                create_near_cache_context();

                populate_map();

                populate_near_cache();

                ASSERT_EQ(size, near_cache_->size());
                auto value = near_cached_map_->get<int, std::string>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("value-1", value.value());

                no_near_cache_map_->put<int, std::string>(1, "newValue").get();

                // we have to use assertTrueEventually since the invalidation is done asynchronously
                int64_t expectedMisses = 0;
                int64_t expectedHits = 0;
                WAIT_TRUE_EVENTUALLY(check_misses_and_hits(expectedMisses, expectedHits, value));
                SCOPED_TRACE("whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnDataAdapter");
                NearCacheTestUtils::assert_near_cache_stats(*stats_, size, expectedHits, expectedMisses);
            }

            /**
             * Checks that the Near Cache values are eventually invalidated when {@link DataStructureAdapter#putAll(Map)} is used.
             *
             * This variant uses the nearCacheMap, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnNearCacheAdapter) {
                when_put_all_is_used_then_near_cache_should_be_invalidated(true);
            }

            /**
             * Checks that the Near Cache values are eventually invalidated when {@link DataStructureAdapter#putAll(Map)} is used.
             *
             * This variant uses the noNearCacheMap, so we need to configure Near Cache invalidation.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnDataAdapter) {
                near_cache_config_.set_invalidate_on_change(true);
                when_put_all_is_used_then_near_cache_should_be_invalidated(false);
            }

            /**
             * Checks that the {@link com.hazelcast.monitor.NearCacheStats} are calculated correctly.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest, testNearCacheStats) {
                create_no_near_cache_context();

                create_near_cache_context();

                // populate map
                populate_map();

                {
                    SCOPED_TRACE("testNearCacheStats when near cache is empty");
                    NearCacheTestUtils::assert_near_cache_stats(*stats_, 0, 0, 0);
                }

                // populate Near Cache. Will cause misses and will increment the owned entry count
                populate_near_cache();
                {
                    SCOPED_TRACE("testNearCacheStats when near cache is initially populated");
                    NearCacheTestUtils::assert_near_cache_stats(*stats_, DEFAULT_RECORD_COUNT, 0, DEFAULT_RECORD_COUNT);
                }

                // make some hits
                populate_near_cache();
                {
                    SCOPED_TRACE("testNearCacheStats when near cache is hit after being populated.");
                    NearCacheTestUtils::assert_near_cache_stats(*stats_, DEFAULT_RECORD_COUNT, DEFAULT_RECORD_COUNT,
                                                             DEFAULT_RECORD_COUNT);
                }
            }

            TEST_P(BasicClientReplicatedMapNearCacheTest, testNearCacheEviction) {
                NearCacheTestUtils::set_eviction_config(near_cache_config_, config::LRU,
                                                                        config::eviction_config::ENTRY_COUNT,
                                                                        DEFAULT_RECORD_COUNT);
                create_no_near_cache_context();

                // all Near Cache implementations use the same eviction algorithm, which evicts a single entry
                int64_t expectedEvictions = 1;

                create_near_cache_context();

                // populate map with an extra entry
                populate_map();
                no_near_cache_map_->put(DEFAULT_RECORD_COUNT, std::string("value-") + std::to_string(DEFAULT_RECORD_COUNT)).get();

                // populate Near Caches
                populate_near_cache();

                // we expect (size + the extra entry - the expectedEvictions) entries in the Near Cache
                int64_t expectedOwnedEntryCount = DEFAULT_RECORD_COUNT + 1 - expectedEvictions;
                int64_t expectedHits = stats_->get_hits();
                int64_t expectedMisses = stats_->get_misses() + 1;

                // trigger eviction via fetching the extra entry
                near_cached_map_->get<int, std::string>(DEFAULT_RECORD_COUNT).get();

                int64_t evictions = stats_->get_evictions();
                ASSERT_GE(evictions, expectedEvictions)
                                            << "Near Cache eviction count didn't reach the desired value ("
                                            << expectedEvictions << " vs. " << evictions << ") ("
                                            << stats_->to_string();

                SCOPED_TRACE("testNearCacheEviction");

                NearCacheTestUtils::assert_near_cache_stats(*stats_, expectedOwnedEntryCount, expectedHits, expectedMisses,
                                                         expectedEvictions, 0);
            }

            INSTANTIATE_TEST_SUITE_P(ReplicatedMapNearCacheTest, BasicClientReplicatedMapNearCacheTest,
                                     ::testing::Values(config::BINARY, config::OBJECT));
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapNearCacheTest : public ClientTestSupport {
            protected:
                /**
                 * The default name used for the data structures which have a Near Cache.
                 */
                static const std::string DEFAULT_NEAR_CACHE_NAME;

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestCase() {
                    delete instance2;
                    delete instance;
                    instance2 = nullptr;
                    instance = nullptr;
                }


                void TearDown() override {
                    if (map_) {
                        map_->destroy().get();
                    }
                }

                config::near_cache_config new_no_invalidation_near_cache_config() {
                    config::near_cache_config config(new_near_cache_config());
                    config.set_in_memory_format(config::OBJECT);
                    config.set_invalidate_on_change(false);
                    return config;
                }

                static config::near_cache_config new_near_cache_config() {
                    return config::near_cache_config();
                }

                static std::unique_ptr<client_config> new_client_config() {
                    return std::unique_ptr<client_config>(new client_config(get_config()));
                }

                std::shared_ptr<replicated_map> get_near_cached_map_from_client(
                        config::near_cache_config config) {
                    std::string mapName = DEFAULT_NEAR_CACHE_NAME;

                    config.set_name(mapName);

                    client_config_ = new_client_config();
                    client_config_->add_near_cache_config(config);

                    client_ = std::unique_ptr<hazelcast_client>(
                            new hazelcast_client(new_client(std::move(*client_config_)).get()));
                    map_ = client_->get_replicated_map(mapName).get();
                    return map_;
                }

                std::shared_ptr<monitor::near_cache_stats> get_near_cache_stats(replicated_map &rep_map) {
                    spi::ClientContext clientContext(*client_);
                    auto nearCacheManager = &clientContext.get_near_cache_manager();
                    auto nearCache = nearCacheManager->
                            get_near_cache<serialization::pimpl::data, serialization::pimpl::data, serialization::pimpl::data>(rep_map.get_name());
                    return nearCache->get_near_cache_stats();
                }

                void assert_that_owned_entry_count_equals(replicated_map &client_map, int64_t expected) {
                    ASSERT_EQ(expected, get_near_cache_stats(client_map)->get_owned_entry_count());
                }

                std::unique_ptr<client_config> client_config_;
                config::near_cache_config near_cache_config_;
                std::unique_ptr<hazelcast_client> client_;
                std::shared_ptr<replicated_map> map_;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const std::string ClientReplicatedMapNearCacheTest::DEFAULT_NEAR_CACHE_NAME = "defaultNearCache";
            HazelcastServer *ClientReplicatedMapNearCacheTest::instance = nullptr;
            HazelcastServer *ClientReplicatedMapNearCacheTest::instance2 = nullptr;

            TEST_F(ClientReplicatedMapNearCacheTest, testGetAllChecksNearCacheFirst) {
                auto map = get_near_cached_map_from_client(new_no_invalidation_near_cache_config());

                int size = 1003;
                for (int i = 0; i < size; i++) {
                    map->put(i, i).get();
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get<int, int>(i).get();
                }
                // getAll() generates the Near Cache hits
                for (int i = 0; i < size; i++) {
                    map->get<int, int>(i).get();
                }

                auto stats = get_near_cache_stats(*map);
                ASSERT_EQ(size, stats->get_owned_entry_count());
                ASSERT_EQ(size, stats->get_hits());
            }

            TEST_F(ClientReplicatedMapNearCacheTest, testGetAllPopulatesNearCache) {
                auto map = get_near_cached_map_from_client(new_no_invalidation_near_cache_config());

                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map->put(i, i).get();
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get<int, int>(i).get();
                }
                // getAll() generates the Near Cache hits
                for (int i = 0; i < size; i++) {
                    map->get<int, int>(i).get();
                }

                assert_that_owned_entry_count_equals(*map, size);
            }

            TEST_F(ClientReplicatedMapNearCacheTest, testRemoveAllNearCache) {
                auto map = get_near_cached_map_from_client(new_near_cache_config());

                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map->put(i, i).get();
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get<int, int>(i).get();
                }

                for (int i = 0; i < size; i++) {
                    map->remove<int, int>(i).get();
                }

                assert_that_owned_entry_count_equals(*map, 0);
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTopicTest : public ClientTestSupport {
            public:
                ClientTopicTest();
            protected:
                HazelcastServer instance_;
                client_config client_config_;
                hazelcast_client client_;
                std::shared_ptr<itopic> topic_;
            };

            ClientTopicTest::ClientTopicTest() : instance_(*g_srvFactory), client_(get_new_client()),
                                                 topic_(client_.get_topic("ClientTopicTest").get()) {}

            TEST_F(ClientTopicTest, testTopicListeners) {
                boost::latch latch1(10);
                auto id = topic_->add_message_listener(
                    topic::listener().
                        on_received([&latch1](topic::message &&) {
                            latch1.count_down();
                        })
                ).get();

                for (int i = 0; i < 10; i++) {
                    topic_->publish(std::string("naber") + std::to_string(i)).get();
                }

                ASSERT_OPEN_EVENTUALLY(latch1);
                topic_->remove_message_listener(id).get();
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


