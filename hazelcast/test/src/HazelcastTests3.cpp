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
#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include "ClientTestSupport.h"
#include <regex>
#include <vector>
#include "ringbuffer/StartsWithStringFilter.h"
#include "ClientTestSupportBase.h"
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/exception/IllegalStateException.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/impl/Partition.h>
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
#include <ctime>
#include <errno.h>
#include <hazelcast/client/LifecycleListener.h>
#include "serialization/Serializables.h"
#include <hazelcast/client/SerializationConfig.h>
#include <hazelcast/client/HazelcastJsonValue.h>
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
#include <hazelcast/client/serialization/serialization.h>
#include <stdlib.h>
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
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/UTFUtil.h"
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/Pipelining.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/HazelcastJsonValue.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/client/ISet.h"
#include "hazelcast/client/ReliableTopic.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientReplicatedMapTest : public ClientTestSupport {
            public:
                struct SamplePortable {
                    int32_t a;
                };
            protected:
                static constexpr size_t OPERATION_COUNT = 100;
                typedef std::vector<std::pair<int, int>> TEST_VALUES_TYPE;

                template<typename Functor>
                void executeForEach(Functor f) {
                    for (size_t i = 0; i < OPERATION_COUNT; i++) {
                        f(i);
                    }
                }

                void verifyEntriesInMap(const std::shared_ptr<ReplicatedMap>& map) {
                    auto entries = map->entrySet<TypedData, TypedData>().get();
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
                
                void getAndVerifyEntriesInMap(const std::shared_ptr<ReplicatedMap> &map) {
                    getAndVerifyEntriesInMap(map, "bar");
                }

                void
                getAndVerifyEntriesInMap(const std::shared_ptr<ReplicatedMap>& map, const std::string &expectedValue) {
                    executeForEach([=] (size_t i) {
                        auto key = std::string("foo-") + std::to_string(i);
                        boost::optional<std::string> val = map->get<std::string, std::string>(key).get();
                        ASSERT_TRUE((val));
                        ASSERT_EQ(expectedValue, (val.value()));
                    });
                }

                void putAllEntriesIntoMap(std::shared_ptr<ReplicatedMap> map) {
                    std::unordered_map<std::string, std::string> mapTest;
                    executeForEach([=, &mapTest] (size_t i) {
                        mapTest[std::string("foo-") + std::to_string(i)] = "bar";
                    });
                    map->putAll(mapTest).get();
                    ASSERT_EQ((int) OPERATION_COUNT, map->size().get());
                }

                void putEntriesIntoMap(const std::shared_ptr<ReplicatedMap>& map) {
                    executeForEach([=] (size_t i) {
                        auto oldEntry = map->put<std::string, std::string>(std::string("foo-") + std::to_string(i),
                                                                           "bar").get();
                        ASSERT_FALSE(oldEntry);
                    });
                }

                void putEntriesIntoMap(const std::shared_ptr<ReplicatedMap> &map, const std::string value) {
                    executeForEach([&] (size_t i) {
                        map->put<std::string, std::string>(std::string("foo-") + std::to_string(i),
                                                                           value).get();
                    });
                }

                static bool findValueForKey(int key, TEST_VALUES_TYPE &testValues, int &value) {
                    for (const TEST_VALUES_TYPE::value_type &entry : testValues) {
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

                static TEST_VALUES_TYPE buildTestValues() {
                    TEST_VALUES_TYPE testValues;
                    for (int i = 0; i < 100; ++i) {
                        testValues.emplace_back(i, i * i);
                    }
                    return testValues;
                }

                virtual void SetUp() {
                    ASSERT_TRUE(factory);
                    ASSERT_TRUE(instance1);
                    ASSERT_TRUE(client);
                    ASSERT_TRUE(client2);
                }

                static void SetUpTestCase() {
                    factory = new HazelcastServerFactory(g_srvFactory->getServerAddress(), 
                            "hazelcast/test/resources/replicated-map-binary-in-memory-config-hazelcast.xml");
                    instance1 = new HazelcastServer(*factory);
                    auto config = getConfig().setClusterName("replicated-map-binary-test");
                    client = new HazelcastClient(config);
                    client2 = new HazelcastClient(config);
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

                static ClientConfig getClientConfigWithNearCacheInvalidationEnabled() {
                    auto nearCacheConfig = std::make_shared<config::NearCacheConfig>();
                    nearCacheConfig->setInvalidateOnChange(true).setInMemoryFormat(config::BINARY);
                    return getConfig().setClusterName("replicated-map-binary-test").addNearCacheConfig(nearCacheConfig);
                }

                static HazelcastServer *instance1;
                static HazelcastClient *client;
                static HazelcastClient *client2;
                static HazelcastServerFactory *factory;
            };

            HazelcastServer *ClientReplicatedMapTest::instance1 = nullptr;
            HazelcastClient *ClientReplicatedMapTest::client = nullptr;
            HazelcastClient *ClientReplicatedMapTest::client2 = nullptr;
            HazelcastServerFactory *ClientReplicatedMapTest::factory = nullptr;
            constexpr size_t ClientReplicatedMapTest::OPERATION_COUNT;

            TEST_F(ClientReplicatedMapTest, testEmptyMapIsEmpty) {
                std::shared_ptr<ReplicatedMap> map = client->getReplicatedMap(getTestName());
                ASSERT_TRUE(map->isEmpty().get()) << "map should be empty";
            }

            TEST_F(ClientReplicatedMapTest, testNonEmptyMapIsNotEmpty) {
                auto map = client->getReplicatedMap(getTestName());
                map->put(1, 1).get();
                ASSERT_FALSE(map->isEmpty().get()) << "map should not be empty";
            }

            TEST_F(ClientReplicatedMapTest, testPutAll) {
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                putAllEntriesIntoMap(map1);
                verifyEntriesInMap(map1);
                verifyEntriesInMap(map2);

// TODO add server side data check using remote controller scripting
            }

            TEST_F(ClientReplicatedMapTest, testGet) {
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());
                putEntriesIntoMap(map1);
                getAndVerifyEntriesInMap(map1);
                getAndVerifyEntriesInMap(map2);
            }

            TEST_F(ClientReplicatedMapTest, testPutNullReturnValueDeserialization) {
                auto map = client->getReplicatedMap(getTestName());
                ASSERT_FALSE(map->put(1, 2).get().has_value()) << "Put should return null";
            }

            TEST_F(ClientReplicatedMapTest, testPutReturnValueDeserialization) {
                auto map = client->getReplicatedMap(getTestName());
                map->put(1, 2).get();
                auto value = map->put(1, 3).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ(2, value.value());
            }

            TEST_F(ClientReplicatedMapTest, testAdd) {
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                putEntriesIntoMap(map1);
                ASSERT_EQ(OPERATION_COUNT, map2->size().get());

                verifyEntriesInMap(map2);
                verifyEntriesInMap(map1);
            }

            TEST_F(ClientReplicatedMapTest, testClear) {
                std::shared_ptr<ReplicatedMap> map1 =client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 =client2->getReplicatedMap(getTestName());

                putEntriesIntoMap(map1);
                ASSERT_EQ(OPERATION_COUNT, map2->size().get());

                verifyEntriesInMap(map1);
                verifyEntriesInMap(map2);

                map1->clear().get();
                ASSERT_EQ(0, map1->size().get());
                ASSERT_EQ(0, map2->size().get());
            }

            TEST_F(ClientReplicatedMapTest, testUpdate) {
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                putEntriesIntoMap(map1);
                ASSERT_EQ(OPERATION_COUNT, map2->size().get());

                verifyEntriesInMap(map1);
                verifyEntriesInMap(map2);

                putEntriesIntoMap(map1, "bar2");

                getAndVerifyEntriesInMap(map2, "bar2");
                getAndVerifyEntriesInMap(map1, "bar2");
            }

            TEST_F(ClientReplicatedMapTest, testRemove) {
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                putEntriesIntoMap(map1);
                ASSERT_EQ(OPERATION_COUNT, map2->size().get());

                verifyEntriesInMap(map2);
                verifyEntriesInMap(map1);

                executeForEach([=] (size_t index) {
                    auto val = map2->remove<std::string, std::string>(std::string("foo-") + std::to_string(index)).get();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ("bar", val.value());
                });

                executeForEach([=](size_t index) {
                    auto key = std::string("foo-") + std::to_string(index);
                    ASSERT_FALSE((map1->get<std::string, std::string>(key).get().has_value()))
                                                << "Removed value should not exist for map1";
                    ASSERT_FALSE((map2->get<std::string, std::string>(key).get().has_value()))
                                                << "Removed value should not exist for map2";
                });
            }

            TEST_F(ClientReplicatedMapTest, testSize) {
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
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
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                putEntriesIntoMap(map1);

                executeForEach([=] (size_t i) {
                    ASSERT_TRUE(map2->containsKey(std::string("foo-") + std::to_string(i)).get());
                });

                executeForEach([=] (size_t i) {
                    ASSERT_TRUE(map2->containsKey(std::string("foo-") + std::to_string(i)).get());
                });
            }

            TEST_F(ClientReplicatedMapTest, testContainsValue) {
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    auto map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second).get();
                }

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(map2->containsValue(entry.second).get());
                }

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(map1->containsValue(entry.second).get());
                }
            }

            TEST_F(ClientReplicatedMapTest, testValues) {
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
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
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    auto map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second).get();
                }

                auto keys1 = map1->keySet<int>().get();
                auto keys2 = map2->keySet<int>().get();

                for (TEST_VALUES_TYPE::value_type &entry : testValues) {
                    ASSERT_TRUE(contains(keys1, entry.first));
                    ASSERT_TRUE(contains(keys2, entry.first));
                }
            }

            TEST_F(ClientReplicatedMapTest, testEntrySet) {
                std::shared_ptr<ReplicatedMap> map1 = client->getReplicatedMap(getTestName());
                std::shared_ptr<ReplicatedMap> map2 = client2->getReplicatedMap(getTestName());

                TEST_VALUES_TYPE testValues = buildTestValues();
                size_t half = testValues.size() / 2;
                for (size_t i = 0; i < testValues.size(); i++) {
                    auto map = i < half ? map1 : map2;
                    std::pair<int, int> &entry = testValues[i];
                    map->put(entry.first, entry.second).get();
                }

                auto entrySet1 = map1->entrySet<TypedData, TypedData>().get();
                auto entrySet2 = map2->entrySet<TypedData, TypedData>().get();

                for (auto &entry : entrySet2) {
                    int value;
                    ASSERT_TRUE(findValueForKey(entry.first.get<int>().value(), testValues, value));
                    ASSERT_EQ(value, entry.second.get<int>().value());
                }

                for (auto &entry : entrySet1) {
                    int value;
                    ASSERT_TRUE(findValueForKey(entry.first.get<int>().value(), testValues, value));
                    ASSERT_EQ(value, entry.second.get<int>().value());
                }
            }

            TEST_F(ClientReplicatedMapTest, testRetrieveUnknownValue) {
                std::shared_ptr<ReplicatedMap> map = client->getReplicatedMap(getTestName());
                auto value = map->get<std::string, std::string>("foo").get();
                ASSERT_FALSE(value.has_value()) << "No entry with key foo should exist";
            }

            TEST_F(ClientReplicatedMapTest, testNearCacheInvalidation) {
                std::string mapName = randomString();

                ClientConfig clientConfig = getClientConfigWithNearCacheInvalidationEnabled();
                HazelcastClient client1(clientConfig);
                HazelcastClient client2(clientConfig);

                auto replicatedMap1 = client1.getReplicatedMap(mapName);

                replicatedMap1->put(1, 1).get();
// puts key 1 to Near Cache
                replicatedMap1->get<int, int>(1).get();

                auto replicatedMap2 = client2.getReplicatedMap(mapName);
// this should invalidate Near Cache of replicatedMap1
                replicatedMap2->clear().get();

                ASSERT_FALSE_EVENTUALLY((replicatedMap1->get<int, int>(1).get().has_value()));
            }

            TEST_F(ClientReplicatedMapTest, testClientPortableWithoutRegisteringToNode) {
                auto sampleMap = client->getReplicatedMap(getTestName());
                sampleMap->put(1, SamplePortable{666});
                auto samplePortable = sampleMap->get<int, SamplePortable>(1).get();
                ASSERT_TRUE(samplePortable.has_value());
                ASSERT_EQ(666, samplePortable->a);
            }
        }

        namespace serialization {
            template<>
            struct hz_serializer<test::ClientReplicatedMapTest::SamplePortable> : public portable_serializer {
                static int32_t getFactoryId() {
                    return 5;
                }

                static int32_t getClassId() {
                    return 6;
                }

                static void writePortable(test::ClientReplicatedMapTest::SamplePortable object, serialization::PortableWriter &out) {
                    out.write<int32_t>("a", object.a);
                }

                static test::ClientReplicatedMapTest::SamplePortable readPortable(serialization::PortableReader &reader) {
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
                    std::atomic<int> addCount{ 0 }, removeCount{ 0 }, updateCount{ 0 }, evictCount{ 0 },
                    mapClearCount{ 0 }, mapEvictCount{ 0 };
                };

                EntryListener makeEventCountingListener(ListenerState &state) {
                    const auto pushKey = [&state](const EntryEvent &event) {
                        state.keys.push(event.getKey().get<int>().value());
                    };

                    return EntryListener().
                            on_added([&state, pushKey](EntryEvent &&event) {
                                pushKey(event);
                                ++state.addCount;
                            }).
                            on_removed([&state, pushKey](EntryEvent &&event) {
                                pushKey(event);
                                ++state.removeCount;
                            }).
                            on_updated([&state, pushKey](EntryEvent &&event) {
                                pushKey(event);
                                ++state.updateCount;
                            }).
                            on_evicted([&state, pushKey](EntryEvent &&event) {
                                pushKey(event);
                                ++state.evictCount;
                            }).
                            on_map_evicted([&state](MapEvent &&) {
                                ++state.mapEvictCount;
                            }).
                            on_map_cleared([&state](MapEvent &&) {
                                ++state.mapClearCount;
                            });
                }

                static void SetUpTestCase() {
                    instance1 = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    client2 = new HazelcastClient(getConfig());
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
                static HazelcastClient *client;
                static HazelcastClient *client2;
                ListenerState state;
            };

            HazelcastServer *ClientReplicatedMapListenerTest::instance1 = nullptr;
            HazelcastServer *ClientReplicatedMapListenerTest::instance2 = nullptr;
            HazelcastClient *ClientReplicatedMapListenerTest::client = nullptr;
            HazelcastClient *ClientReplicatedMapListenerTest::client2 = nullptr;

            TEST_F(ClientReplicatedMapListenerTest, testEntryAdded) {
                auto replicatedMap = client->getReplicatedMap(getTestName());
                replicatedMap->addEntryListener(makeEventCountingListener(state)).get();
                replicatedMap->put(1, 1).get();
                ASSERT_EQ_EVENTUALLY(1, state.addCount.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testEntryUpdated) {
                auto replicatedMap = client->getReplicatedMap(getTestName());
                replicatedMap->addEntryListener(makeEventCountingListener(state)).get();
                replicatedMap->put(1, 1).get();
                replicatedMap->put(1, 2).get();
                ASSERT_EQ_EVENTUALLY(1, state.updateCount.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testEntryRemoved) {
                auto replicatedMap = client->getReplicatedMap(getTestName());
                replicatedMap->addEntryListener(makeEventCountingListener(state)).get();
                replicatedMap->put(1, 1).get();
                replicatedMap->remove<int, int>(1).get();
                ASSERT_EQ_EVENTUALLY(1, state.removeCount.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testMapClear) {
                auto replicatedMap = client->getReplicatedMap(getTestName());
                replicatedMap->addEntryListener(makeEventCountingListener(state)).get();
                replicatedMap->put(1, 1).get();
                replicatedMap->clear().get();
                ASSERT_EQ_EVENTUALLY(1, state.mapClearCount.load());
            }

            TEST_F(ClientReplicatedMapListenerTest, testListenToKeyForEntryAdded) {
                auto replicatedMap = client->getReplicatedMap(getTestName());
                replicatedMap->addEntryListener(makeEventCountingListener(state), 1).get();
                replicatedMap->put(1, 1).get();
                replicatedMap->put(2, 2).get();
                ASSERT_TRUE_EVENTUALLY(
                        state.keys.size() == 1U && state.keys.pop() == 1 && state.addCount.load() == 1);
            }

            TEST_F(ClientReplicatedMapListenerTest, testListenWithPredicate) {
                auto replicatedMap = client->getReplicatedMap(getTestName());
                replicatedMap->addEntryListener(makeEventCountingListener(state), query::FalsePredicate(*client)).get();
                replicatedMap->put(2, 2).get();
                ASSERT_TRUE_ALL_THE_TIME((state.addCount.load() == 0), 1);
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class BasicClientReplicatedMapNearCacheTest
                    : public ClientTestSupport, public ::testing::WithParamInterface<config::InMemoryFormat> {
            public:
                static void SetUpTestSuite() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestSuite() {
                    delete instance2;
                    delete instance;
                    instance2 = nullptr;
                    instance = nullptr;
                }

                void SetUp() override {
                    nearCacheConfig = NearCacheTestUtils::createNearCacheConfig(GetParam(), getTestName());
                }

                void TearDown() override {
                    if (nearCachedMap) {
                        nearCachedMap->destroy().get();
                    }
                    if (noNearCacheMap) {
                        noNearCacheMap->destroy().get();
                    }
                    if (client) {
                        client->shutdown();
                    }
                    if (nearCachedClient) {
                        nearCachedClient->shutdown();
                    }
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
                    static std::shared_ptr<config::NearCacheConfig> createNearCacheConfig(
                            config::InMemoryFormat inMemoryFormat, const std::string &mapName) {
                        std::shared_ptr<config::NearCacheConfig> nearCacheConfig(
                                new config::NearCacheConfig());

                        nearCacheConfig->setName(mapName).setInMemoryFormat(inMemoryFormat).setInvalidateOnChange(true);

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
                    static void setEvictionConfig(config::NearCacheConfig &nearCacheConfig,
                                                  config::EvictionPolicy evictionPolicy,
                                                  typename config::EvictionConfig::MaxSizePolicy maxSizePolicy,
                                                  int maxSize) {
                        nearCacheConfig.getEvictionConfig().setEvictionPolicy(evictionPolicy)
                                .setMaximumSizePolicy(maxSizePolicy).setSize(maxSize);
                    }

                    /**
                     * Asserts the {@link NearCacheStats} for expected values.
                     *
                     * @param stats                   stats of the near cache
                     * @param expectedOwnedEntryCount the expected owned entry count
                     * @param expectedHits            the expected Near Cache hits
                     * @param expectedMisses          the expected Near Cache misses
                     */
                    static void assertNearCacheStats(monitor::NearCacheStats &stats,
                                                     int64_t expectedOwnedEntryCount, int64_t expectedHits,
                                                     int64_t expectedMisses) {
                        assertNearCacheStats(stats, expectedOwnedEntryCount, expectedHits, expectedMisses, 0, 0);
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
                    static void assertNearCacheStats(monitor::NearCacheStats &stats,
                                                     int64_t expectedOwnedEntryCount, int64_t expectedHits,
                                                     int64_t expectedMisses,
                                                     int64_t expectedEvictions, int64_t expectedExpirations) {
                        assertEqualsFormat("Near Cache entry count should be %ld, but was %ld ",
                                           expectedOwnedEntryCount, stats.getOwnedEntryCount(), stats);
                        assertEqualsFormat("Near Cache hits should be %ld, but were %ld ",
                                           expectedHits, stats.getHits(), stats);
                        assertEqualsFormat("Near Cache misses should be %ld, but were %ld ",
                                           expectedMisses, stats.getMisses(), stats);
                        assertEqualsFormat("Near Cache evictions should be %ld, but were %ld ",
                                           expectedEvictions, stats.getEvictions(), stats);
                        assertEqualsFormat("Near Cache expirations should be %ld, but were %ld ",
                                           expectedExpirations, stats.getExpirations(), stats);
                    }

                    static void assertEqualsFormat(const char *messageFormat, int64_t expected, int64_t actual,
                                                   monitor::NearCacheStats &stats) {
                        char buf[300];
                        hazelcast::util::hz_snprintf(buf, 300, messageFormat, expected, actual);
                        ASSERT_EQ(expected, actual) << buf << "(" << stats.toString() << ")";
                    }

                private:
                    NearCacheTestUtils() = delete;
                    NearCacheTestUtils(const NearCacheTestUtils &) = delete;
                };

                /**
                 * The default count to be inserted into the Near Caches.
                 */
                static const int DEFAULT_RECORD_COUNT;

                void createContext() {
                    createNoNearCacheContext();
                    createNearCacheContext();
                }

                void createNoNearCacheContext() {
                    client = std::unique_ptr<HazelcastClient>(new HazelcastClient(getConfig()));
                    noNearCacheMap = client->getReplicatedMap(getTestName());
                }

                void createNearCacheContext() {
                    ClientConfig nearCachedClientConfig = getConfig();
                    nearCachedClientConfig.addNearCacheConfig(nearCacheConfig);
                    nearCachedClient = std::unique_ptr<HazelcastClient>(new HazelcastClient(nearCachedClientConfig));
                    nearCachedMap = nearCachedClient->getReplicatedMap(getTestName());
                    spi::ClientContext clientContext(*nearCachedClient);
                    nearCacheManager = &clientContext.getNearCacheManager();
                    nearCache = nearCacheManager->
                            getNearCache<serialization::pimpl::Data, serialization::pimpl::Data, serialization::pimpl::Data>(getTestName());
                    this->stats = nearCache ? nearCache->getNearCacheStats() : nullptr;
                }

                void testContainsKey(bool useNearCachedMapForRemoval) {
                    createNoNearCacheContext();

                    // populate map
                    noNearCacheMap->put<int, std::string>(1, "value1").get();
                    noNearCacheMap->put<int, std::string>(2, "value2").get();
                    noNearCacheMap->put<int, std::string>(3, "value3").get();

                    createNearCacheContext();

                    // populate Near Cache
                    nearCachedMap->get<int, std::string>(1).get();
                    nearCachedMap->get<int, std::string>(2).get();
                    nearCachedMap->get<int, std::string>(3).get();

                    ASSERT_TRUE(nearCachedMap->containsKey(1).get());
                    ASSERT_TRUE(nearCachedMap->containsKey(2).get());
                    ASSERT_TRUE(nearCachedMap->containsKey(3).get());
                    ASSERT_FALSE(nearCachedMap->containsKey(5).get());

                    // remove a key which is in the Near Cache
                    std::shared_ptr<ReplicatedMap> &adapter = useNearCachedMapForRemoval
                                                                                 ? nearCachedMap
                                                                                 : noNearCacheMap;
                    adapter->remove<int, std::string>(1).get();

                    WAIT_TRUE_EVENTUALLY(checkContainKeys());
                    ASSERT_FALSE(nearCachedMap->containsKey(1).get());
                    ASSERT_TRUE(nearCachedMap->containsKey(2).get());
                    ASSERT_TRUE(nearCachedMap->containsKey(3).get());
                    ASSERT_FALSE(nearCachedMap->containsKey(5).get());
                }

                bool checkContainKeys() {
                    return !nearCachedMap->containsKey(1).get() && nearCachedMap->containsKey(2).get() &&
                           nearCachedMap->containsKey(3).get() && !nearCachedMap->containsKey(5).get();
                }

                void
                assertNearCacheInvalidationRequests(monitor::NearCacheStats &stat, int64_t invalidationRequests) {
                    if (nearCacheConfig->isInvalidateOnChange() && invalidationRequests > 0) {
                        auto &nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl &) stat;
                        ASSERT_EQ_EVENTUALLY(invalidationRequests, nearCacheStatsImpl.getInvalidationRequests());
                        nearCacheStatsImpl.resetInvalidationEvents();
                    }
                }

                void populateMap() {
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        noNearCacheMap->put<int, std::string>(i, std::string("value-") + std::to_string(i)).get();
                    }

                    assertNearCacheInvalidationRequests(*stats, DEFAULT_RECORD_COUNT);
                }

                void populateNearCache() {
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        auto value = nearCachedMap->get<int, std::string>(i).get();
                        ASSERT_TRUE(value.has_value());
                        ASSERT_EQ(std::string("value-" + std::to_string(i)), value.value());
                    }
                }

                std::shared_ptr<serialization::pimpl::Data> getNearCacheKey(int key) {
                    spi::ClientContext clientContext(*client);
                    return clientContext.getSerializationService().toSharedData<int>(&key);
                }

                int64_t getExpectedMissesWithLocalUpdatePolicy() {
                    if (nearCacheConfig->getLocalUpdatePolicy() ==
                        config::NearCacheConfig::CACHE) {
                        // we expect the first and second get() to be hits, since the value should be already be cached
                        return stats->getMisses();
                    }
                    // we expect the first get() to be a miss, due to the replaced / invalidated value
                    return stats->getMisses() + 1;
                }

                int64_t getExpectedHitsWithLocalUpdatePolicy() {
                    if (nearCacheConfig->getLocalUpdatePolicy() == config::NearCacheConfig::CACHE) {
                        // we expect the first and second get() to be hits, since the value should be already be cached
                        return stats->getHits() + 2;
                    }
                    // we expect the second get() to be a hit, since it should be served from the Near Cache
                    return stats->getHits() + 1;
                }

                bool checkMissesAndHits(int64_t &expectedMisses, int64_t &expectedHits,
                                        boost::optional<std::string> &value) {
                    expectedMisses = getExpectedMissesWithLocalUpdatePolicy();
                    expectedHits = getExpectedHitsWithLocalUpdatePolicy();

                    value = nearCachedMap->get<int, std::string>(1).get();
                    if (!value.has_value() || value.value() != "newValue") {
                        return false;
                    }
                    value = nearCachedMap->get<int, std::string>(1).get();
                    if (!value.has_value() || value.value() != "newValue") {
                        return false;
                    }

                    return expectedHits == stats->getHits() && expectedMisses == stats->getMisses();
                }

                void whenPutAllIsUsed_thenNearCacheShouldBeInvalidated(bool useNearCacheAdapter) {
                    createNoNearCacheContext();

                    createNearCacheContext();

                    populateMap();

                    populateNearCache();

                    std::unordered_map<int, std::string> invalidationMap;
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        invalidationMap[i] = std::string("value-" + std::to_string(i));
                    }

                    // this should invalidate the Near Cache
                    std::shared_ptr<ReplicatedMap> &adapter = useNearCacheAdapter ? nearCachedMap : noNearCacheMap;
                    adapter->putAll(invalidationMap).get();

                    WAIT_EQ_EVENTUALLY(0, nearCache->size());
                    ASSERT_EQ(0, nearCache->size()) << "Invalidation is not working on putAll()";
                }

                std::shared_ptr<config::NearCacheConfig> nearCacheConfig;
                std::unique_ptr<HazelcastClient> client;
                std::unique_ptr<HazelcastClient> nearCachedClient;
                std::shared_ptr<ReplicatedMap> noNearCacheMap;
                std::shared_ptr<ReplicatedMap> nearCachedMap;
                hazelcast::client::internal::nearcache::NearCacheManager *nearCacheManager{};
                std::shared_ptr<hazelcast::client::internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>> nearCache;
                std::shared_ptr<monitor::NearCacheStats> stats;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const int BasicClientReplicatedMapNearCacheTest::DEFAULT_RECORD_COUNT = 1000;
            HazelcastServer *BasicClientReplicatedMapNearCacheTest::instance = nullptr;
            HazelcastServer *BasicClientReplicatedMapNearCacheTest::instance2 = nullptr;

            /**
             * Checks that the Near Cache keys are correctly checked when {@link DataStructureAdapter#containsKey(Object)} is used.
             *
             * This variant uses the {@link NearCacheTestContext#nearCacheAdapter}, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest, testContainsKey_withUpdateOnNearCacheAdapter) {
                testContainsKey(true);
            }

            /**
             * Checks that the memory costs are calculated correctly.
             *
             * This variant uses the noNearCacheMap(client with no near cache), so we need to configure Near Cache
             * invalidation.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest, testContainsKey_withUpdateOnDataAdapter) {
                nearCacheConfig->setInvalidateOnChange(true);
                testContainsKey(false);
            }

            /**
             * Checks that the Near Cache never returns its internal {@link NearCache#NULL_OBJECT} to the public API.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenEmptyMap_thenPopulatedNearCacheShouldReturnNull_neverNULLOBJECT) {
                createContext();

                for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                    // populate Near Cache
                    ASSERT_FALSE((nearCachedMap->get<int, std::string>(i).get().has_value()))
                                                << "Expected null from original data structure for key " << i;
                    // fetch value from Near Cache
                    ASSERT_FALSE((nearCachedMap->get<int, std::string>(i).get().has_value()))
                                                << "Expected null from Near cached data structure for key " << i;

                    // fetch internal value directly from Near Cache
                    std::shared_ptr<serialization::pimpl::Data> key = getNearCacheKey(i);
                    auto value = nearCache->get(key);
                    if (value.get() != NULL) {
                        // the internal value should either be `null` or `NULL_OBJECT`
                        ASSERT_EQ(
                        (hazelcast::client::internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data>::NULL_OBJECT),
                                nearCache->get(key)) << "Expected NULL_OBJECT in Near Cache for key " << i;
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
                NearCacheTestUtils::setEvictionConfig(*nearCacheConfig, config::NONE,
                                                                        config::EvictionConfig::ENTRY_COUNT,
                                                                        size);

                nearCacheConfig->setInvalidateOnChange(false);

                createNoNearCacheContext();

                createNearCacheContext();

                populateMap();

                populateNearCache();

                ASSERT_EQ(size, nearCache->size());
                auto value = nearCachedMap->get<int, std::string>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("value-1", value.value());

                nearCachedMap->put<int, std::string>(1, "newValue").get();

                int64_t expectedMisses = getExpectedMissesWithLocalUpdatePolicy();
                int64_t expectedHits = getExpectedHitsWithLocalUpdatePolicy();

                value = nearCachedMap->get<int, std::string>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("newValue", value.value());
                value = nearCachedMap->get<int, std::string>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("newValue", value.value());

                NearCacheTestUtils::assertNearCacheStats(*stats, size, expectedHits, expectedMisses);
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
                NearCacheTestUtils::setEvictionConfig(*nearCacheConfig, config::NONE,
                                                                        config::EvictionConfig::ENTRY_COUNT,
                                                                        size);
                nearCacheConfig->setInvalidateOnChange(true);

                createNoNearCacheContext();

                createNearCacheContext();

                populateMap();

                populateNearCache();

                ASSERT_EQ(size, nearCache->size());
                auto value = nearCachedMap->get<int, std::string>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("value-1", value.value());

                noNearCacheMap->put<int, std::string>(1, "newValue").get();

                // we have to use assertTrueEventually since the invalidation is done asynchronously
                int64_t expectedMisses = 0;
                int64_t expectedHits = 0;
                WAIT_TRUE_EVENTUALLY(checkMissesAndHits(expectedMisses, expectedHits, value));
                SCOPED_TRACE("whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnDataAdapter");
                NearCacheTestUtils::assertNearCacheStats(*stats, size, expectedHits, expectedMisses);
            }

            /**
             * Checks that the Near Cache values are eventually invalidated when {@link DataStructureAdapter#putAll(Map)} is used.
             *
             * This variant uses the nearCacheMap, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnNearCacheAdapter) {
                whenPutAllIsUsed_thenNearCacheShouldBeInvalidated(true);
            }

            /**
             * Checks that the Near Cache values are eventually invalidated when {@link DataStructureAdapter#putAll(Map)} is used.
             *
             * This variant uses the noNearCacheMap, so we need to configure Near Cache invalidation.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest,
                   whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnDataAdapter) {
                nearCacheConfig->setInvalidateOnChange(true);
                whenPutAllIsUsed_thenNearCacheShouldBeInvalidated(false);
            }

            /**
             * Checks that the {@link com.hazelcast.monitor.NearCacheStats} are calculated correctly.
             */
            TEST_P(BasicClientReplicatedMapNearCacheTest, testNearCacheStats) {
                createNoNearCacheContext();

                createNearCacheContext();

                // populate map
                populateMap();

                {
                    SCOPED_TRACE("testNearCacheStats when near cache is empty");
                    NearCacheTestUtils::assertNearCacheStats(*stats, 0, 0, 0);
                }

                // populate Near Cache. Will cause misses and will increment the owned entry count
                populateNearCache();
                {
                    SCOPED_TRACE("testNearCacheStats when near cache is initially populated");
                    NearCacheTestUtils::assertNearCacheStats(*stats, DEFAULT_RECORD_COUNT, 0, DEFAULT_RECORD_COUNT);
                }

                // make some hits
                populateNearCache();
                {
                    SCOPED_TRACE("testNearCacheStats when near cache is hit after being populated.");
                    NearCacheTestUtils::assertNearCacheStats(*stats, DEFAULT_RECORD_COUNT, DEFAULT_RECORD_COUNT,
                                                             DEFAULT_RECORD_COUNT);
                }
            }

            TEST_P(BasicClientReplicatedMapNearCacheTest, testNearCacheEviction) {
                NearCacheTestUtils::setEvictionConfig(*nearCacheConfig, config::LRU,
                                                                        config::EvictionConfig::ENTRY_COUNT,
                                                                        DEFAULT_RECORD_COUNT);
                createNoNearCacheContext();

                // all Near Cache implementations use the same eviction algorithm, which evicts a single entry
                int64_t expectedEvictions = 1;

                createNearCacheContext();

                // populate map with an extra entry
                populateMap();
                noNearCacheMap->put(DEFAULT_RECORD_COUNT, std::string("value-") + std::to_string(DEFAULT_RECORD_COUNT)).get();

                // populate Near Caches
                populateNearCache();

                // we expect (size + the extra entry - the expectedEvictions) entries in the Near Cache
                int64_t expectedOwnedEntryCount = DEFAULT_RECORD_COUNT + 1 - expectedEvictions;
                int64_t expectedHits = stats->getHits();
                int64_t expectedMisses = stats->getMisses() + 1;

                // trigger eviction via fetching the extra entry
                nearCachedMap->get<int, std::string>(DEFAULT_RECORD_COUNT).get();

                int64_t evictions = stats->getEvictions();
                ASSERT_GE(evictions, expectedEvictions)
                                            << "Near Cache eviction count didn't reach the desired value ("
                                            << expectedEvictions << " vs. " << evictions << ") ("
                                            << stats->toString();

                SCOPED_TRACE("testNearCacheEviction");

                NearCacheTestUtils::assertNearCacheStats(*stats, expectedOwnedEntryCount, expectedHits, expectedMisses,
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
                    if (map) {
                        map->destroy().get();
                    }
                }

                std::shared_ptr<config::NearCacheConfig> newNoInvalidationNearCacheConfig() {
                    std::shared_ptr<config::NearCacheConfig> config(newNearCacheConfig());
                    config->setInMemoryFormat(config::OBJECT);
                    config->setInvalidateOnChange(false);
                    return config;
                }

                static std::shared_ptr<config::NearCacheConfig> newNearCacheConfig() {
                    return std::shared_ptr<config::NearCacheConfig>(
                            new config::NearCacheConfig());
                }

                static std::unique_ptr<ClientConfig> newClientConfig() {
                    return std::unique_ptr<ClientConfig>(new ClientConfig(getConfig()));
                }

                std::shared_ptr<ReplicatedMap > getNearCachedMapFromClient(
                        std::shared_ptr<config::NearCacheConfig> config) {
                    std::string mapName = DEFAULT_NEAR_CACHE_NAME;

                    config->setName(mapName);

                    clientConfig = newClientConfig();
                    clientConfig->addNearCacheConfig(config);

                    client = std::unique_ptr<HazelcastClient>(new HazelcastClient(*clientConfig));
                    map = client->getReplicatedMap(mapName);
                    return map;
                }

                std::shared_ptr<monitor::NearCacheStats> getNearCacheStats(ReplicatedMap &repMap) {
                    spi::ClientContext clientContext(*client);
                    auto nearCacheManager = &clientContext.getNearCacheManager();
                    auto nearCache = nearCacheManager->
                            getNearCache<serialization::pimpl::Data, serialization::pimpl::Data, serialization::pimpl::Data>(repMap.getName());
                    return nearCache->getNearCacheStats();
                }

                void assertThatOwnedEntryCountEquals(ReplicatedMap &clientMap, int64_t expected) {
                    ASSERT_EQ(expected, getNearCacheStats(clientMap)->getOwnedEntryCount());
                }

                std::unique_ptr<ClientConfig> clientConfig;
                std::shared_ptr<config::NearCacheConfig> nearCacheConfig;
                std::unique_ptr<HazelcastClient> client;
                std::shared_ptr<ReplicatedMap> map;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const std::string ClientReplicatedMapNearCacheTest::DEFAULT_NEAR_CACHE_NAME = "defaultNearCache";
            HazelcastServer *ClientReplicatedMapNearCacheTest::instance = nullptr;
            HazelcastServer *ClientReplicatedMapNearCacheTest::instance2 = nullptr;

            TEST_F(ClientReplicatedMapNearCacheTest, testGetAllChecksNearCacheFirst) {
                auto map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

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

                auto stats = getNearCacheStats(*map);
                ASSERT_EQ(size, stats->getOwnedEntryCount());
                ASSERT_EQ(size, stats->getHits());
            }

            TEST_F(ClientReplicatedMapNearCacheTest, testGetAllPopulatesNearCache) {
                auto map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

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

                assertThatOwnedEntryCountEquals(*map, size);
            }

            TEST_F(ClientReplicatedMapNearCacheTest, testRemoveAllNearCache) {
                auto map = getNearCachedMapFromClient(newNearCacheConfig());

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

                assertThatOwnedEntryCountEquals(*map, 0);
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
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
                std::shared_ptr<ITopic> topic;
            };

            ClientTopicTest::ClientTopicTest() : instance(*g_srvFactory), client(getNewClient()),
                                                 topic(client.getTopic("ClientTopicTest")) {}

            TEST_F(ClientTopicTest, testTopicListeners) {
                boost::latch latch1(10);
                auto id = topic->addMessageListener(
                    topic::Listener().
                        on_received([&latch1](topic::Message &&) {
                            latch1.count_down();
                        })
                ).get();

                for (int i = 0; i < 10; i++) {
                    topic->publish(std::string("naber") + std::to_string(i)).get();
                }

                ASSERT_OPEN_EVENTUALLY(latch1);
                topic->removeMessageListener(id).get();
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


