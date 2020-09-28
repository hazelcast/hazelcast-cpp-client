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
#include "serialization/Serializables.h"
#include <memory>
#include <utility>
#include <vector>
#include "ClientTestSupportBase.h"
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/impl/Partition.h>
#include <gtest/gtest.h>
#include <thread>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/connection/Connection.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/util/Util.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/LifecycleListener.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <unordered_set>
#include <cmath>
#include <iostream>
#include <string>
#include <cstdlib>
#include <boost/asio.hpp>

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
#include "hazelcast/util/ILogger.h"
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
#include "hazelcast/client/ItemListener.h"
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
            class BasicClientNearCacheTest
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
                    nearCacheConfig = NearCacheTestUtils::createNearCacheConfig<serialization::pimpl::Data, serialization::pimpl::Data>(
                            GetParam(),
                            getTestName());
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
                    template<typename K, typename V>
                    static std::shared_ptr<config::NearCacheConfig<K, V> > createNearCacheConfig(
                            config::InMemoryFormat inMemoryFormat, const std::string &mapName) {
                        std::shared_ptr<config::NearCacheConfig<K, V> > nearCacheConfig(
                                new config::NearCacheConfig<K, V>());

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
                    template<typename K, typename V = serialization::pimpl::Data>
                    static void setEvictionConfig(config::NearCacheConfig<K, V> &nearCacheConfig,
                                                  config::EvictionPolicy evictionPolicy,
                                                  typename config::EvictionConfig<K, V>::MaxSizePolicy maxSizePolicy,
                                                  int maxSize) {
                        nearCacheConfig.getEvictionConfig()
                                ->setEvictionPolicy(evictionPolicy)
                                .setMaximumSizePolicy(maxSizePolicy)
                                .setSize(maxSize);
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
                    noNearCacheMap = client->getMap(getTestName());
                }

                void createNearCacheContext() {
                    nearCachedClientConfig = getConfig();
                    nearCachedClientConfig.addNearCacheConfig(nearCacheConfig);
                    nearCachedClient = std::unique_ptr<HazelcastClient>(new HazelcastClient(nearCachedClientConfig));
                    nearCachedMap = nearCachedClient->getMap(getTestName());
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
                    auto adapter = useNearCachedMapForRemoval ? nearCachedMap : noNearCacheMap;
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
                        monitor::impl::NearCacheStatsImpl &nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl &) stat;
                        ASSERT_EQ_EVENTUALLY(invalidationRequests, nearCacheStatsImpl.getInvalidationRequests());
                        nearCacheStatsImpl.resetInvalidationEvents();
                    }
                }

                void populateMap() {
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
                        noNearCacheMap->put<int, std::string>(i, buf).get();
                    }

                    assertNearCacheInvalidationRequests(*stats, DEFAULT_RECORD_COUNT);
                }

                void populateNearCache() {
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        auto value = nearCachedMap->get<int, std::string>(i).get();
                        ASSERT_TRUE(value.has_value());
                        hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
                        ASSERT_EQ(buf, value.value());
                    }
                }

                std::shared_ptr<serialization::pimpl::Data> getNearCacheKey(int key) {
                    spi::ClientContext clientContext(*client);
                    return clientContext.getSerializationService().toSharedData<int>(&key);
                }

                int64_t getExpectedMissesWithLocalUpdatePolicy() {
                    if (nearCacheConfig->getLocalUpdatePolicy() ==
                        config::NearCacheConfig<serialization::pimpl::Data>::CACHE) {
                        // we expect the first and second get() to be hits, since the value should be already be cached
                        return stats->getMisses();
                    }
                    // we expect the first get() to be a miss, due to the replaced / invalidated value
                    return stats->getMisses() + 1;
                }

                int64_t getExpectedHitsWithLocalUpdatePolicy() {
                    if (nearCacheConfig->getLocalUpdatePolicy() ==
                        config::NearCacheConfig<serialization::pimpl::Data>::CACHE) {
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
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
                        invalidationMap[i] = buf;
                    }

                    // this should invalidate the Near Cache
                    auto adapter = useNearCacheAdapter ? nearCachedMap : noNearCacheMap;
                    adapter->putAll<int, std::string>(invalidationMap).get();

                    WAIT_EQ_EVENTUALLY(0, nearCache->size());
                    ASSERT_EQ(0, nearCache->size()) << "Invalidation is not working on putAll()";
                }

                ClientConfig clientConfig;
                ClientConfig nearCachedClientConfig;
                std::shared_ptr<config::NearCacheConfig<serialization::pimpl::Data>> nearCacheConfig;
                std::unique_ptr<HazelcastClient> client;
                std::unique_ptr<HazelcastClient> nearCachedClient;
                std::shared_ptr<IMap> noNearCacheMap;
                std::shared_ptr<IMap> nearCachedMap;
                hazelcast::client::internal::nearcache::NearCacheManager *nearCacheManager;
                std::shared_ptr<hazelcast::client::internal::nearcache::NearCache<serialization::pimpl::Data, serialization::pimpl::Data> > nearCache;
                std::shared_ptr<monitor::NearCacheStats> stats;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const int BasicClientNearCacheTest::DEFAULT_RECORD_COUNT = 1000;
            HazelcastServer *BasicClientNearCacheTest::instance = nullptr;
            HazelcastServer *BasicClientNearCacheTest::instance2 = nullptr;

            /**
             * Checks that the Near Cache keys are correctly checked when {@link DataStructureAdapter#containsKey(Object)} is used.
             *
             * This variant uses the {@link NearCacheTestContext#nearCacheAdapter}, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientNearCacheTest, testContainsKey_withUpdateOnNearCacheAdapter) {
                testContainsKey(true);
            }

            /**
             * Checks that the memory costs are calculated correctly.
             *
             * This variant uses the noNearCacheMap(client with no near cache), so we need to configure Near Cache
             * invalidation.
             */
            TEST_P(BasicClientNearCacheTest, testContainsKey_withUpdateOnDataAdapter) {
                nearCacheConfig->setInvalidateOnChange(true);
                testContainsKey(false);
            }

            /**
             * Checks that the Near Cache never returns its internal {@link NearCache#NULL_OBJECT} to the public API.
             */
            TEST_P(BasicClientNearCacheTest, whenEmptyMap_thenPopulatedNearCacheShouldReturnNull_neverNULLOBJECT) {
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
                    if (value) {
                        // the internal value should either be `null` or `NULL_OBJECT`
                        ASSERT_EQ(nearCache->NULL_OBJECT, nearCache->get(key)) << "Expected NULL_OBJECT in Near Cache for key " << i;
                    }
                }
            }

            /**
             * Checks that the Near Cache updates value for keys which are already in the Near Cache,
             * even if the Near Cache is full an the eviction is disabled (via {@link com.hazelcast.config.EvictionPolicy#NONE}.
             *
             * This variant uses the {@link NearCacheTestContext#nearCacheAdapter}, so there is no Near Cache invalidation necessary.
             */
            TEST_P(BasicClientNearCacheTest,
                   whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnNearCacheAdapter) {
                int size = DEFAULT_RECORD_COUNT / 2;
                NearCacheTestUtils::setEvictionConfig<serialization::pimpl::Data>(*nearCacheConfig, config::NONE,
                                                                                  config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data>::ENTRY_COUNT,
                                                                                  size);
                createNoNearCacheContext();

                createNearCacheContext();

                populateMap();

                populateNearCache();

                ASSERT_EQ(size, nearCache->size());
                auto value = nearCachedMap->get<int, std::string>(1).get();
                ASSERT_TRUE(value.has_value());
                ASSERT_EQ("value-1", value.value());

                nearCachedMap->put<int, std::string>(1, "newValue").get();

                // wait for the invalidation to be processed
                WAIT_EQ_EVENTUALLY(size - 1, nearCache->size());
                ASSERT_EQ(1, stats->getInvalidations());

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
            TEST_P(BasicClientNearCacheTest,
                   whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnDataAdapter) {
                int size = DEFAULT_RECORD_COUNT / 2;
                NearCacheTestUtils::setEvictionConfig<serialization::pimpl::Data>(*nearCacheConfig, config::NONE,
                                                                                  config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data>::ENTRY_COUNT,
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
            TEST_P(BasicClientNearCacheTest,
                   whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnNearCacheAdapter) {
                whenPutAllIsUsed_thenNearCacheShouldBeInvalidated(true);
            }

            /**
             * Checks that the Near Cache values are eventually invalidated when {@link DataStructureAdapter#putAll(Map)} is used.
             *
             * This variant uses the noNearCacheMap, so we need to configure Near Cache invalidation.
             */
            TEST_P(BasicClientNearCacheTest,
                   whenPutAllIsUsed_thenNearCacheShouldBeInvalidated_withUpdateOnDataAdapter) {
                nearCacheConfig->setInvalidateOnChange(true);
                whenPutAllIsUsed_thenNearCacheShouldBeInvalidated(false);
            }

            /**
             * Checks that the {@link com.hazelcast.monitor.NearCacheStats} are calculated correctly.
             */
            TEST_P(BasicClientNearCacheTest, testNearCacheStats) {
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

            TEST_P(BasicClientNearCacheTest, testNearCacheEviction) {
                NearCacheTestUtils::setEvictionConfig<serialization::pimpl::Data>(*nearCacheConfig, config::LRU,
                                                                                  config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data>::ENTRY_COUNT,
                                                                                  DEFAULT_RECORD_COUNT);
                createNoNearCacheContext();

                createNearCacheContext();

                // all Near Cache implementations use the same eviction algorithm, which evicts a single entry
                int64_t expectedEvictions = 1;

                // populate map with an extra entry
                populateMap();
                char buf[20];
                hazelcast::util::hz_snprintf(buf, 20, "value-%d", DEFAULT_RECORD_COUNT);
                noNearCacheMap->put<int, std::string>(DEFAULT_RECORD_COUNT, buf).get();

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

            INSTANTIATE_TEST_SUITE_P(ClientNearCacheTest, BasicClientNearCacheTest,
                                     ::testing::Values(config::BINARY, config::OBJECT));
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientMapNearCacheTest : public ClientTestSupport {
            protected:
                /**
                 * The default name used for the data structures which have a Near Cache.
                 */
                static constexpr const char *DEFAULT_NEAR_CACHE_NAME = "defaultNearCache";

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

                static std::shared_ptr<config::NearCacheConfig<serialization::pimpl::Data> >
                newNoInvalidationNearCacheConfig() {
                    std::shared_ptr<config::NearCacheConfig<serialization::pimpl::Data> > config(newNearCacheConfig());
                    config->setInMemoryFormat(config::OBJECT);
                    config->setInvalidateOnChange(false);
                    return config;
                }

                static std::shared_ptr<config::NearCacheConfig<serialization::pimpl::Data> > newNearCacheConfig() {
                    return std::shared_ptr<config::NearCacheConfig<serialization::pimpl::Data> >(
                            new config::NearCacheConfig<serialization::pimpl::Data>());
                }

                static std::unique_ptr<ClientConfig> newClientConfig() {
                    return std::unique_ptr<ClientConfig>(new ClientConfig(getConfig()));
                }

                std::shared_ptr<IMap> getNearCachedMapFromClient(
                        const std::shared_ptr<config::NearCacheConfig<serialization::pimpl::Data>> &config) {
                    std::string mapName = DEFAULT_NEAR_CACHE_NAME;

                    config->setName(mapName);

                    clientConfig = newClientConfig();
                    clientConfig->addNearCacheConfig(config);

                    client.reset(new HazelcastClient(*clientConfig));
                    map = client->getMap(mapName);
                    return map;
                }

                static std::shared_ptr<monitor::NearCacheStats> getNearCacheStats(IMap &m) {
                    return m.getLocalMapStats().getNearCacheStats();
                }

                static void assertThatOwnedEntryCountEquals(IMap &clientMap, int64_t expected) {
                    ASSERT_EQ(expected, getNearCacheStats(clientMap)->getOwnedEntryCount());
                }

                std::unique_ptr<ClientConfig> clientConfig;
                std::shared_ptr<config::NearCacheConfig<serialization::pimpl::Data> > nearCacheConfig;
                std::unique_ptr<HazelcastClient> client;
                std::shared_ptr<IMap> map;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            HazelcastServer *ClientMapNearCacheTest::instance = nullptr;
            HazelcastServer *ClientMapNearCacheTest::instance2 = nullptr;

            TEST_F(ClientMapNearCacheTest, testGetAllChecksNearCacheFirst) {
                auto map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

                std::unordered_set<int> keys;
                int size = 1003;
                for (int i = 0; i < size; i++) {
                    map->put(i, i).get();
                    keys.insert(i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get<int, int>(i).get();
                }
                // getAll() generates the Near Cache hits
                map->getAll<int, int>(keys).get();

                auto stats = getNearCacheStats(*map);
                ASSERT_EQ(size, stats->getOwnedEntryCount());
                ASSERT_EQ(size, stats->getHits());
            }

            TEST_F(ClientMapNearCacheTest, testGetAllPopulatesNearCache) {
                auto map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

                std::unordered_set<int> keys;
                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map->put(i, i).get();
                    keys.insert(i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get<int, int>(i).get();
                }
                // getAll() generates the Near Cache hits
                map->getAll<int, int>(keys).get();

                assertThatOwnedEntryCountEquals(*map, size);
            }

            TEST_F(ClientMapNearCacheTest, testRemoveAllNearCache) {
                auto map = getNearCachedMapFromClient(newNearCacheConfig());

                std::unordered_set<int> keys;
                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map->put(i, i).get();
                    keys.insert(i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map->get<int, int>(i).get();
                }

                map->removeAll(query::EqualPredicate(*client, query::QueryConstants::KEY_ATTRIBUTE_NAME, 20)).get();

                assertThatOwnedEntryCountEquals(*map, 0);
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {

            class ClientSetTest : public ClientTestSupport {
            protected:
                void addItems(int count) {
                    for (int i = 1; i <= count; ++i) {
                        ASSERT_TRUE(set->add(std::string("item") + std::to_string(i)).get());
                    }
                }

                void TearDown() override {
                    set->clear().get();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    set = client->getSet("MySet");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    set = nullptr;
                    client = nullptr;
                    instance = nullptr;
                }

                bool itemExists(const std::vector<std::string> &items, const std::string &item) const {
                    bool found = false;
                    for (std::vector<std::string>::const_iterator it = items.begin(); it != items.end(); ++it) {
                        if (item == *it) {
                            found = true;
                            break;
                        }
                    }
                    return found;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<ISet> set;
            };

            HazelcastServer *ClientSetTest::instance = nullptr;
            HazelcastClient *ClientSetTest::client = nullptr;
            std::shared_ptr<ISet> ClientSetTest::set;

            TEST_F(ClientSetTest, testAddAll) {
                std::vector<std::string> l;
                l.emplace_back("item1");
                l.emplace_back("item2");

                ASSERT_TRUE(set->addAll(l).get());
                ASSERT_EQ(2, set->size().get());

                ASSERT_FALSE(set->addAll(l).get());
                ASSERT_EQ(2, set->size().get());
            }

            TEST_F(ClientSetTest, testAddRemove) {
                addItems(3);
                ASSERT_EQ(3, set->size().get());

                ASSERT_FALSE(set->add("item3").get());
                ASSERT_EQ(3, set->size().get());


                ASSERT_FALSE(set->remove("item4").get());
                ASSERT_TRUE(set->remove("item3").get());
            }

            TEST_F(ClientSetTest, testContains) {
                addItems(4);

                ASSERT_FALSE(set->contains("item5").get());
                ASSERT_TRUE(set->contains("item2").get());

                std::vector<std::string> l;
                l.emplace_back("item6");
                l.emplace_back("item3");
                ASSERT_FALSE(set->containsAll(l).get());
                ASSERT_TRUE(set->add("item6").get());
                ASSERT_TRUE(set->containsAll(l).get());
            }

            TEST_F(ClientSetTest, testToArray) {
                addItems(4);
                ASSERT_FALSE(set->add("item4").get());

                std::vector<std::string> items = set->toArray<std::string>().get();

                ASSERT_EQ((size_t) 4, items.size());
                ASSERT_TRUE(itemExists(items, "item1"));
                ASSERT_TRUE(itemExists(items, "item2"));
                ASSERT_TRUE(itemExists(items, "item3"));
                ASSERT_TRUE(itemExists(items, "item4"));
            }

            TEST_F(ClientSetTest, testRemoveRetainAll) {
                addItems(4);
                std::vector<std::string> l;
                l.emplace_back("item4");
                l.emplace_back("item3");
                ASSERT_TRUE(set->removeAll(l).get());
                ASSERT_EQ(2, set->size().get());
                ASSERT_FALSE(set->removeAll(l).get());
                ASSERT_EQ(2, set->size().get());

                l.clear();
                l.emplace_back("item1");
                l.emplace_back("item2");
                ASSERT_FALSE(set->retainAll(l).get());
                ASSERT_EQ(2, set->size().get());

                l.clear();
                ASSERT_TRUE(set->retainAll(l).get());
                ASSERT_EQ(0, set->size().get());

            }

            TEST_F(ClientSetTest, testListener) {
                boost::latch latch1(6);

                auto registrationId = set->addItemListener(
                    ItemListener()
                        .on_added([&latch1](ItemEvent &&itemEvent) {
                            latch1.count_down();
                        })
                    , true).get();

                addItems(5);
                set->add("done").get();
                ASSERT_OPEN_EVENTUALLY(latch1);

                ASSERT_TRUE(set->removeItemListener(registrationId).get());
            }

            TEST_F(ClientSetTest, testIsEmpty) {
                ASSERT_TRUE(set->isEmpty().get());
                ASSERT_TRUE(set->add("item1").get());
                ASSERT_FALSE(set->isEmpty().get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ReliableTopicTest : public ClientTestSupport {
            protected:
                struct ListenerState {
                    ListenerState(int latchCount, int64_t startSequence) : latch1(latchCount),
                                                                           startSequence(startSequence),
                                                                           numberOfMessagesReceived(0) {}

                    explicit ListenerState(int latchCount) : ListenerState(latchCount, -1) {}

                    boost::latch latch1;
                    int64_t startSequence;
                    std::atomic<int> numberOfMessagesReceived;
                    hazelcast::util::ConcurrentQueue<topic::Message> messages;
                };

                class GenericListener : public topic::ReliableMessageListener {
                public:
                    explicit GenericListener(std::shared_ptr<ListenerState> state) : state_(std::move(state)) {}

                    ~GenericListener() override {
                        topic::Message *m = nullptr;
                        while ((m = state_->messages.poll()) != nullptr) {
                            delete (m);
                        }
                    }

                    void onMessage(topic::Message &&message) override {
                        ++state_->numberOfMessagesReceived;
                        state_->messages.offer(new topic::Message(std::move(message)));
                        state_->latch1.count_down();
                    }

                    int64_t retrieveInitialSequence() const override {
                        return state_->startSequence;
                    }

                    void storeSequence(int64_t sequence) override {
                    }

                    bool isLossTolerant() const override {
                        return false;
                    }

                    bool isTerminal(const exception::IException &failure) const override {
                        return false;
                    }

                    int getNumberOfMessagesReceived() {
                        int value = state_->numberOfMessagesReceived;
                        return value;
                    }

                    hazelcast::util::ConcurrentQueue<topic::Message> &getMessages() {
                        return state_->messages;
                    }

                private:
                    std::shared_ptr<ListenerState> state_;
                };

            protected:
                void TearDown() override {
                    if (topic) {
                        topic->destroy().get();
                    }
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                std::shared_ptr<ReliableTopic> topic;
                std::string listenerId;
            };

            HazelcastServer *ReliableTopicTest::instance = nullptr;
            HazelcastClient *ReliableTopicTest::client = nullptr;

            TEST_F(ReliableTopicTest, testBasics) {
                ASSERT_NO_THROW(topic = client->getReliableTopic("testBasics"));
                ASSERT_EQ("testBasics", topic->getName());

                auto state = std::make_shared<ListenerState>(1);
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(GenericListener(state)));

                Employee empl1("first", 20);
                ASSERT_NO_THROW(topic->publish(empl1).get());

                ASSERT_OPEN_EVENTUALLY(state->latch1);
                ASSERT_EQ(1, state->numberOfMessagesReceived);
                auto employee = state->messages.poll()->getMessageObject().template get<Employee>();
                ASSERT_TRUE(employee.has_value());
                ASSERT_EQ(empl1, employee.value());

                // remove listener
                ASSERT_TRUE(topic->removeMessageListener(listenerId));
                ASSERT_FALSE(topic->removeMessageListener(listenerId));
            }

            TEST_F(ReliableTopicTest, testListenerSequence) {
                ASSERT_NO_THROW(topic = client->getReliableTopic("testListenerSequence"));

                Employee empl1("first", 10);
                Employee empl2("second", 20);

                ASSERT_NO_THROW(topic->publish(empl1).get());
                ASSERT_NO_THROW(topic->publish(empl2).get());

                auto state = std::make_shared<ListenerState>(1, 1);
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(GenericListener(state)));

                ASSERT_OPEN_EVENTUALLY(state->latch1);
                ASSERT_EQ(1, state->numberOfMessagesReceived);
                auto employee = state->messages.poll()->getMessageObject().template get<Employee>();
                ASSERT_TRUE(employee.has_value());
                ASSERT_EQ(empl2, employee.value());

                // remove listener
                ASSERT_TRUE(topic->removeMessageListener(listenerId));
            }

            TEST_F(ReliableTopicTest, removeMessageListener_whenExisting) {
                ASSERT_NO_THROW(topic = client->getReliableTopic("removeMessageListener_whenExisting"));

                Employee empl1("first", 10);

                auto state = std::make_shared<ListenerState>(1);
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(GenericListener(state)));

                // remove listener
                ASSERT_TRUE(topic->removeMessageListener(listenerId));

                ASSERT_NO_THROW(topic->publish(empl1).get());

                ASSERT_EQ(boost::cv_status::timeout, state->latch1.wait_for(boost::chrono::seconds(2)));
                ASSERT_EQ(0, state->numberOfMessagesReceived);
            }

            TEST_F(ReliableTopicTest, removeMessageListener_whenNonExisting) {
                ASSERT_NO_THROW(topic = client->getReliableTopic("removeMessageListener_whenNonExisting"));

                // remove listener
                ASSERT_FALSE(topic->removeMessageListener("abc"));
            }

            TEST_F(ReliableTopicTest, publishMultiple) {
                ASSERT_NO_THROW(topic = client->getReliableTopic("publishMultiple"));

                auto state = std::make_shared<ListenerState>(5);
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(GenericListener(state)));

                std::vector<std::string> items;
                for (int k = 0; k < 5; k++) {
                    std::string item = std::to_string(k);
                    topic->publish(item).get();
                    items.push_back(item);
                }

                ASSERT_OPEN_EVENTUALLY(state->latch1);
                ASSERT_EQ(5, state->numberOfMessagesReceived);
                hazelcast::util::ConcurrentQueue<topic::Message> &queue = state->messages;
                for (int k = 0; k < 5; k++) {
                    auto val = queue.poll()->getMessageObject().get<std::string>();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ(items[k], val.value());
                }

                ASSERT_TRUE(topic->removeMessageListener(listenerId));
            }

            TEST_F(ReliableTopicTest, testConfig) {
                ClientConfig clientConfig;
                clientConfig.getNetworkConfig().addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                config::ReliableTopicConfig relConfig("testConfig");
                relConfig.setReadBatchSize(2);
                clientConfig.addReliableTopicConfig(relConfig);
                HazelcastClient configClient(clientConfig);

                ASSERT_NO_THROW(topic = configClient.getReliableTopic("testConfig"));

                auto state = std::make_shared<ListenerState>(5);
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(GenericListener(state)));

                std::vector<std::string> items;
                for (int k = 0; k < 5; k++) {
                    std::string item = std::to_string(k);
                    topic->publish(item).get();
                    items.push_back(item);
                }

                ASSERT_OPEN_EVENTUALLY(state->latch1);
                ASSERT_EQ(5, state->numberOfMessagesReceived);
                hazelcast::util::ConcurrentQueue<topic::Message> &queue = state->messages;
                for (int k = 0; k < 5; k++) {
                    auto val = queue.poll()->getMessageObject().get<std::string>();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ(items[k], val.value());
                }
                ASSERT_TRUE(topic->removeMessageListener(listenerId));
                topic.reset();
            }

            TEST_F(ReliableTopicTest, testMessageFieldSetCorrectly) {
                ASSERT_NO_THROW(topic = client->getReliableTopic("testMessageFieldSetCorrectly"));

                auto state = std::make_shared<ListenerState>(1);
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(GenericListener(state)));

                auto timeBeforePublish = std::chrono::system_clock::now();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                topic->publish<int>(3).get();
                auto timeAfterPublish = std::chrono::system_clock::now();
                ASSERT_OPEN_EVENTUALLY(state->latch1);
                ASSERT_EQ(1, state->numberOfMessagesReceived);
                auto message = state->messages.poll();
                auto val = message->getMessageObject().get<int>();
                ASSERT_TRUE(val.has_value());
                ASSERT_EQ(3, val.value());

                ASSERT_LE(timeBeforePublish, message->getPublishTime());
                ASSERT_GE(timeAfterPublish, message->getPublishTime());
                ASSERT_EQ(topic->getName(), message->getSource());
                ASSERT_EQ(nullptr, message->getPublishingMember());

                ASSERT_TRUE(topic->removeMessageListener(listenerId));
            }

            TEST_F(ReliableTopicTest, testAlwaysStartAfterTail) {
                ASSERT_NO_THROW(topic = client->getReliableTopic("testAlwaysStartAfterTail"));
                ASSERT_NO_THROW(topic->publish(1).get());
                ASSERT_NO_THROW(topic->publish(2).get());
                ASSERT_NO_THROW(topic->publish(3).get());

                auto state = std::make_shared<ListenerState>(3);
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(GenericListener(state)));

                std::vector<int> expectedValues = {4, 5, 6};
                // spawn a thread for publishing new data
                std::thread([=]() {
                    for (auto val : expectedValues) {
                        topic->publish(val).get();
                    }
                }).detach();

                ASSERT_OPEN_EVENTUALLY(state->latch1);
                ASSERT_EQ((int) expectedValues.size(), state->numberOfMessagesReceived);
                auto &objects = state->messages;

                for (auto &val : expectedValues) {
                    auto receivedValue = objects.poll()->getMessageObject().get<int>();
                    ASSERT_TRUE(receivedValue.has_value());
                    ASSERT_EQ(val, receivedValue.value());
                }

                ASSERT_TRUE(topic->removeMessageListener(listenerId));
            }
        }
    }
}

using namespace hazelcast::client;

namespace hazelcast {
    namespace client {
        namespace test {
            namespace performance {
                class SimpleMapTest : public ClientTestSupport {
                protected:
                    static const int THREAD_COUNT = 40;
                    static const int ENTRY_COUNT = 10000;
                    static const int VALUE_SIZE = 10;
                    static const int STATS_SECONDS = 10;
                    static const int GET_PERCENTAGE = 40;
                    static const int PUT_PERCENTAGE = 40;

                    class Stats {
                    public:
                        Stats() : getCount(0), putCount(0), removeCount(0) {
                        }

                        Stats(const Stats &rhs) : getCount(const_cast<Stats &>(rhs).getCount.load()),
                                                  putCount(const_cast<Stats &>(rhs).putCount.load()),
                                                  removeCount(const_cast<Stats &>(rhs).removeCount.load()) {
                        }

                        Stats getAndReset() {
                            Stats newOne(*this);
                            getCount = 0;
                            putCount = 0;
                            removeCount = 0;
                            return newOne;
                        }

                        mutable std::atomic<int64_t> getCount;
                        mutable std::atomic<int64_t> putCount;
                        mutable std::atomic<int64_t> removeCount;

                        void print() const {
                            std::cerr << "Total = " << total() << ", puts = " << putCount << " , gets = " << getCount
                                      << " , removes = "
                                      << removeCount << std::endl;
                        }

                        int total() const {
                            return (int) getCount + (int) putCount + (int) removeCount;
                        }
                    };

                    class StatsPrinterTask {
                    public:
                        explicit StatsPrinterTask(Stats &stats) : stats(stats) {}

                        void run()  {
                            while (true) {
                                try {
                                    hazelcast::util::sleep((unsigned int) STATS_SECONDS);
                                    const Stats statsNow = stats.getAndReset();
                                    statsNow.print();
                                    std::cerr << "Operations per Second : " << statsNow.total() / STATS_SECONDS
                                              << std::endl;
                                } catch (std::exception &e) {
                                    std::cerr << e.what() << std::endl;
                                }
                            }
                        }

                        const std::string getName() const {
                            return "StatPrinterTask";
                        }

                    private:
                        Stats &stats;
                    };

                    class Task {
                    public:
                        Task(Stats &stats, std::shared_ptr<IMap> map,
                             std::shared_ptr<hazelcast::util::ILogger> logger) : stats(stats), map(map),
                                                                                 logger(std::move(logger)) {
                        }

                        void run() {
                            std::vector<char> value(VALUE_SIZE);
                            bool running = true;
                            int getCount = 0;
                            int putCount = 0;
                            int removeCount = 0;

                            int updateIntervalCount = 1000;
                            while (running) {
                                int key = rand() % ENTRY_COUNT;
                                int operation = (rand() % 100);
                                try {
                                    if (operation < GET_PERCENTAGE) {
                                        map->get<int, std::vector<char>>(key).get();
                                        ++getCount;
                                    } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                                        map->put<int, std::vector<char>>(key, value).get();
                                        ++putCount;
                                    } else {
                                        map->remove<int, std::string>(key).get();
                                        ++removeCount;
                                    }
                                    updateStats(updateIntervalCount, getCount, putCount, removeCount);
                                } catch (hazelcast::client::exception::IOException &e) {
                                    logger->warning(
                                            std::string("[SimpleMapTest IOException] ") + e.what());
                                } catch (hazelcast::client::exception::HazelcastClientNotActiveException &e) {
                                    logger->warning(
                                            std::string("[SimpleMapTest::run] ") + e.what());
                                } catch (hazelcast::client::exception::IException &e) {
                                    logger->warning(
                                            std::string("[SimpleMapTest:run] ") + e.what());
                                } catch (...) {
                                    logger->warning("[SimpleMapTest:run] unknown exception!");
                                    running = false;
                                    throw;
                                }
                            }
                        }

                        const std::string getName() {
                            return "SimpleMapTest Task";
                        }

                    private:
                        void
                        updateStats(int updateIntervalCount, int &getCount, int &putCount, int &removeCount) const {
                            if ((getCount + putCount + removeCount) % updateIntervalCount == 0) {
                                int64_t current = stats.getCount;
                                stats.getCount = current + getCount;
                                getCount = 0;

                                current = stats.putCount;
                                stats.putCount = current + putCount;
                                putCount = 0;

                                current = stats.removeCount;
                                stats.removeCount = current + removeCount;
                                removeCount = 0;
                            }
                        }

                        Stats &stats;
                        std::shared_ptr<IMap> map;
                        std::shared_ptr<hazelcast::util::ILogger> logger;
                    };


                    static void start(const HazelcastServer &server) {
                        std::cerr << "Starting Test with  " << std::endl;
                        std::cerr << "      Thread Count: " << THREAD_COUNT << std::endl;
                        std::cerr << "       Entry Count: " << ENTRY_COUNT << std::endl;
                        std::cerr << "        Value Size: " << VALUE_SIZE << std::endl;
                        std::cerr << "    Get Percentage: " << GET_PERCENTAGE << std::endl;
                        std::cerr << "    Put Percentage: " << PUT_PERCENTAGE << std::endl;
                        std::cerr << " Remove Percentage: " << (100 - (PUT_PERCENTAGE + GET_PERCENTAGE)) << std::endl;
                        ClientConfig clientConfig;
                        clientConfig.setProperty(ClientProperties::PROP_HEARTBEAT_TIMEOUT, "10");
                        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
                        auto member = server.getMember();
                        clientConfig.getNetworkConfig().addAddress(Address(member.host, member.port)).setConnectionAttemptPeriod(10 * 1000);
                        clientConfig.setLogLevel(FINEST);

                        Stats stats;
                        std::shared_ptr<hazelcast::util::ILogger> logger(
                                new hazelcast::util::ILogger("SimpleMapTest", "SimpleMapTest", "testversion",
                                                             config::LoggerConfig()));
                        if (!logger->start()) {
                            BOOST_THROW_EXCEPTION(
                                    (client::exception::ExceptionBuilder<client::exception::IllegalStateException>(
                                            "SimpleMapTest::start") << "Could not start logger "
                                                                    << logger->getInstanceName()).build());
                        }

                        auto monitor = std::async([&]() {
                            StatsPrinterTask(stats).run();
                        });

                        HazelcastClient hazelcastClient(clientConfig);

                        auto map = hazelcastClient.getMap("cppDefault");

                        std::vector<std::future<void>> futures;
                        for (int i = 0; i < THREAD_COUNT; i++) {
                            futures.push_back(std::async([&]() { Task(stats, map, logger).run(); }));
                        }

                        monitor.wait();
                    }
                };

                TEST_F(SimpleMapTest, DISABLED_testThroughput) {
                    HazelcastServer server(*g_srvFactory);

                    start(server);
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class IssueTest : public ClientTestSupport {
            public:
                IssueTest();
            protected:

                boost::latch latch1;
                boost::latch latch2;
                EntryListener issue864MapListener;
            };

            IssueTest::IssueTest() : latch1(1), latch2(1), issue864MapListener() {
                issue864MapListener.
                    on_added([this](EntryEvent &&event) {
                        auto key = event.getKey().get<int>().value();
                        ASSERT_TRUE(1 == key || 2 == key);
                        if (key == 1) {
                            // The received event should be the addition of key value: 1, 10
                            ASSERT_EQ(10, event.getValue().get<int>().value());
                            this->latch1.count_down();
                        } else {
                            // The received event should be the addition of key value: 2, 20
                            ASSERT_EQ(20, event.getValue().get<int>().value());
                            this->latch2.count_down();
                        }
                    }).
                    on_updated([this](EntryEvent &&event) {
                        ASSERT_EQ(2, event.getKey().get<int>().value());
                        ASSERT_EQ(20, event.getValue().get<int>().value());
                        this->latch1.count_down();
                    });
            }

            TEST_F(IssueTest, testOperationRedo_smartRoutingDisabled) {
                HazelcastServer hz1(*g_srvFactory);
                HazelcastServer hz2(*g_srvFactory);

                ClientConfig clientConfig(getConfig());
                clientConfig.setRedoOperation(true);
                clientConfig.getNetworkConfig().setSmartRouting(false);

                HazelcastClient client(clientConfig);

                auto map = client.getMap("m");
                int expected = 1000;
                std::thread t;
                for (int i = 0; i < expected; i++) {
                    if (i == 5) {
                        t = std::thread([&] () {
                            hz1.shutdown();
                        });
                    }
                    map->put(i, i).get();
                }
                t.join();
                ASSERT_EQ(expected, map->size().get());
            }

            TEST_F(IssueTest, testListenerSubscriptionOnSingleServerRestart) {
                HazelcastServer server(*g_srvFactory);

                // 2. Start a client
                ClientConfig clientConfig = getConfig();
                clientConfig.getNetworkConfig().setConnectionAttemptLimit(10);

                HazelcastClient client(clientConfig);

                // 3. Get a map
                auto map = client.getMap("IssueTest_map");

                // 4. Subscribe client to entry added event
                map->addEntryListener(std::move(issue864MapListener), true).get();

                // Put a key, value to the map
                ASSERT_FALSE(map->put(1, 10).get().has_value());

                ASSERT_OPEN_EVENTUALLY(latch1);

                // 5. Restart the server
                ASSERT_TRUE(server.shutdown());
                HazelcastServer server2(*g_srvFactory);

                std::thread([=] () {
                    // 7. Put a 2nd entry to the map
                    ASSERT_NO_THROW(map->put(2, 20).get());
                }).detach();

                // 6. Verify that the 2nd entry is received by the listener
                ASSERT_OPEN_EVENTUALLY(latch2);

                // 7. Shut down the server
                ASSERT_TRUE(server2.shutdown());
            }

            TEST_F(IssueTest, testIssue221) {
                // start a server
                HazelcastServer server(*g_srvFactory);

                // start a client
                HazelcastClient client(getConfig());

                auto map = client.getMap("Issue221_test_map");

                server.shutdown();

                ASSERT_THROW((map->get<int, int>(1).get()), exception::HazelcastClientNotActiveException);
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            HazelcastServer::HazelcastServer(HazelcastServerFactory &factory) : factory(factory), isStarted(false),
                                                                                isShutdown(false),
                                                                                logger(new hazelcast::util::ILogger(
                                                                                        "HazelcastServer",
                                                                                        "HazelcastServer",
                                                                                        "testversion",
                                                                                        config::LoggerConfig())) {
                start();
            }

            bool HazelcastServer::start() {
                if (!logger->start()) {
                    BOOST_THROW_EXCEPTION(
                            (client::exception::ExceptionBuilder<client::exception::IllegalStateException>(
                                    "HazelcastServer::start") << "Could not start logger "
                                                              << logger->getInstanceName()).build());
                }

                bool expected = false;
                if (!isStarted.compare_exchange_strong(expected, true)) {
                    return true;
                }

                try {
                    member = factory.startServer();
                    isStarted = true;
                    return true;
                } catch (exception::IllegalStateException &illegalStateException) {
                    std::ostringstream out;
                    out << "Could not start new member!!! " << illegalStateException.what();
                    logger->severe(out.str());
                    isStarted = false;
                    return false;
                }
            }

            bool HazelcastServer::shutdown() {
                bool expected = false;
                if (!isShutdown.compare_exchange_strong(expected, true)) {
                    return false;
                }

                if (!isStarted) {
                    return true;
                }

                if (!factory.shutdownServer(member)) {
                    return false;
                }

                isStarted = false;
                return true;
            }

            bool HazelcastServer::terminate() {
                bool expected = false;
                if (!isShutdown.compare_exchange_strong(expected, true)) {
                    return false;
                }

                if (!isStarted) {
                    return true;
                }

                if (!factory.terminateServer(member)) {
                    return false;
                }

                isStarted = false;
                return true;
            }

            HazelcastServer::~HazelcastServer() {
                shutdown();
            }

            const remote::Member &HazelcastServer::getMember() const {
                return member;
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            ClientTestSupport::ClientTestSupport() {
                const testing::TestInfo *testInfo = testing::UnitTest::GetInstance()->current_test_info();
                std::ostringstream out;
                out << testInfo->test_case_name() << "_" << testInfo->name();
                testName = out.str();
                logger = std::make_shared<hazelcast::util::ILogger>("Test", testName, "TestVersion",
                                                                    config::LoggerConfig());
                if (!logger->start()) {
                    BOOST_THROW_EXCEPTION((exception::ExceptionBuilder<exception::IllegalStateException>(
                            "ClientTestSupport::ClientTestSupport()") << "Could not start logger "
                                                                      << testInfo->name()).build());
                }
            }

            hazelcast::util::ILogger &ClientTestSupport::getLogger() {
                return *logger;
            }

            const std::string &ClientTestSupport::getTestName() const {
                return testName;
            }

            CountDownLatchWaiter &CountDownLatchWaiter::add(boost::latch &latch1) {
                latches.push_back(&latch1);
                return *this;
            }

            boost::cv_status CountDownLatchWaiter::wait_for(boost::chrono::steady_clock::duration duration) {
                if (latches.empty()) {
                    return boost::cv_status::no_timeout;
                }

                auto end = boost::chrono::steady_clock::now() + duration;
                for (auto &l : latches) {
                    auto waitDuration = end - boost::chrono::steady_clock::now();
                    auto status = l->wait_for(waitDuration);
                    if (boost::cv_status::timeout == status) {
                        return boost::cv_status::timeout;
                    }
                }
                return boost::cv_status::no_timeout;
            }

            void CountDownLatchWaiter::reset() {
                latches.clear();
            }

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientMessageTest: public ClientTestSupport {};
            TEST_F(ClientMessageTest, testOperationNameGetSet) {
                protocol::ClientMessage message(8);
                constexpr const char* operation_name = "OPERATION_NAME";
                message.setOperationName(operation_name);
                ASSERT_EQ(message.getOperationName(), operation_name);
            }
            TEST_F(ClientMessageTest, testOperationNameAfterRequestEncoding) {
                auto request = protocol::codec::map_size_encode("map_name");
                ASSERT_EQ(request.getOperationName(), "Map.Size");
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
