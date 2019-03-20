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
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include <TestHelperFunctions.h>
#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class BasicClientReplicatedMapNearCacheTest
                    : public ClientTestSupport, public ::testing::WithParamInterface<config::InMemoryFormat> {
            public:
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    instance2 = new HazelcastServer(*g_srvFactory);
                }

                static void TearDownTestCase() {
                    delete instance2;
                    delete instance;
                    instance2 = NULL;
                    instance = NULL;
                }

                virtual void SetUp() {
                    nearCacheConfig = NearCacheTestUtils::createNearCacheConfig<int, std::string>(GetParam(),
                                                                                                  getTestName());
                }

                virtual void TearDown() {
                    if (nearCachedMap.get()) {
                        nearCachedMap->destroy();
                    }
                    if (noNearCacheMap.get()) {
                        noNearCacheMap->destroy();
                    }
                    if (NULL != client.get()) {
                        client->shutdown();
                    }
                    if (NULL != nearCachedClient.get()) {
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
                    static boost::shared_ptr<config::NearCacheConfig<K, V> > createNearCacheConfig(
                            config::InMemoryFormat inMemoryFormat, const std::string &mapName) {
                        boost::shared_ptr<config::NearCacheConfig<K, V> > nearCacheConfig(
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
                    template<typename K, typename V>
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
                        util::hz_snprintf(buf, 300, messageFormat, expected, actual);
                        ASSERT_EQ(expected, actual) << buf << "(" << stats.toString() << ")";
                    }

                private:
                    NearCacheTestUtils();

                    NearCacheTestUtils(const NearCacheTestUtils &);
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
                    clientConfig = getConfig();
                    client = std::auto_ptr<HazelcastClient>(new HazelcastClient(*clientConfig));
                    noNearCacheMap = client->getReplicatedMap<int, std::string>(getTestName());
                }

                void createNearCacheContext() {
                    nearCachedClientConfig = getConfig();
                    nearCachedClientConfig->addNearCacheConfig(nearCacheConfig);
                    nearCachedClient = std::auto_ptr<HazelcastClient>(new HazelcastClient(*nearCachedClientConfig));
                    nearCachedMap = nearCachedClient->getReplicatedMap<int, std::string>(getTestName());
                    spi::ClientContext clientContext(*nearCachedClient);
                    nearCacheManager = &clientContext.getNearCacheManager();
                    nearCache = nearCacheManager->
                            getNearCache<int, std::string, serialization::pimpl::Data>(getTestName());
                    this->stats = (nearCache.get() == NULL) ? NULL : &nearCache->getNearCacheStats();
                }

                void testContainsKey(bool useNearCachedMapForRemoval) {
                    createNoNearCacheContext();

                    // populate map
                    noNearCacheMap->put(1, "value1");
                    noNearCacheMap->put(2, "value2");
                    noNearCacheMap->put(3, "value3");

                    createNearCacheContext();

                    // populate Near Cache
                    nearCachedMap->get(1);
                    nearCachedMap->get(2);
                    nearCachedMap->get(3);

                    ASSERT_TRUE(nearCachedMap->containsKey(1));
                    ASSERT_TRUE(nearCachedMap->containsKey(2));
                    ASSERT_TRUE(nearCachedMap->containsKey(3));
                    ASSERT_FALSE(nearCachedMap->containsKey(5));

                    // remove a key which is in the Near Cache
                    boost::shared_ptr<ReplicatedMap<int, std::string> > &adapter = useNearCachedMapForRemoval
                                                                                   ? nearCachedMap
                                                                                   : noNearCacheMap;
                    adapter->remove(1);

                    WAIT_TRUE_EVENTUALLY(checkContainKeys());
                    ASSERT_FALSE(nearCachedMap->containsKey(1));
                    ASSERT_TRUE(nearCachedMap->containsKey(2));
                    ASSERT_TRUE(nearCachedMap->containsKey(3));
                    ASSERT_FALSE(nearCachedMap->containsKey(5));
                }

                bool checkContainKeys() {
                    return !nearCachedMap->containsKey(1) && nearCachedMap->containsKey(2) &&
                           nearCachedMap->containsKey(3) && !nearCachedMap->containsKey(5);
                }

                void
                assertNearCacheInvalidationRequests(monitor::NearCacheStats &stats, int64_t invalidationRequests) {
                    if (nearCacheConfig->isInvalidateOnChange() && invalidationRequests > 0) {
                        monitor::impl::NearCacheStatsImpl &nearCacheStatsImpl = (monitor::impl::NearCacheStatsImpl &) stats;
                        ASSERT_EQ_EVENTUALLY(invalidationRequests, nearCacheStatsImpl.getInvalidationRequests());
                        nearCacheStatsImpl.resetInvalidationEvents();
                    }
                }

                void populateMap() {
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        util::hz_snprintf(buf, 30, "value-%d", i);
                        noNearCacheMap->put(i, buf);
                    }

                    assertNearCacheInvalidationRequests(*stats, DEFAULT_RECORD_COUNT);
                }

                void populateNearCache() {
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        boost::shared_ptr<string> value = nearCachedMap->get(i);
                        ASSERT_NOTNULL(value.get(), std::string);
                        util::hz_snprintf(buf, 30, "value-%d", i);
                        ASSERT_EQ(buf, *value);
                    }
                }

                boost::shared_ptr<serialization::pimpl::Data> getNearCacheKey(int key) {
                    spi::ClientContext clientContext(*client);
                    return clientContext.getSerializationService().toSharedData<int>(&key);
                }

                int64_t getExpectedMissesWithLocalUpdatePolicy() {
                    if (nearCacheConfig->getLocalUpdatePolicy() == config::NearCacheConfig<int, std::string>::CACHE) {
                        // we expect the first and second get() to be hits, since the value should be already be cached
                        return stats->getMisses();
                    }
                    // we expect the first get() to be a miss, due to the replaced / invalidated value
                    return stats->getMisses() + 1;
                }

                int64_t getExpectedHitsWithLocalUpdatePolicy() {
                    if (nearCacheConfig->getLocalUpdatePolicy() == config::NearCacheConfig<int, std::string>::CACHE) {
                        // we expect the first and second get() to be hits, since the value should be already be cached
                        return stats->getHits() + 2;
                    }
                    // we expect the second get() to be a hit, since it should be served from the Near Cache
                    return stats->getHits() + 1;
                }

                bool checkMissesAndHits(int64_t &expectedMisses, int64_t &expectedHits,
                                        boost::shared_ptr<std::string> &value) {
                    expectedMisses = getExpectedMissesWithLocalUpdatePolicy();
                    expectedHits = getExpectedHitsWithLocalUpdatePolicy();

                    value = nearCachedMap->get(1);
                    if (NULL == value.get() || *value != "newValue") {
                        return false;
                    }
                    value = nearCachedMap->get(1);
                    if (NULL == value.get() || *value != "newValue") {
                        return false;
                    }

                    return expectedHits == stats->getHits() && expectedMisses == stats->getMisses();
                }

                void whenPutAllIsUsed_thenNearCacheShouldBeInvalidated(bool useNearCacheAdapter) {
                    createNoNearCacheContext();

                    createNearCacheContext();

                    populateMap();

                    populateNearCache();

                    std::map<int, std::string> invalidationMap;
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        util::hz_snprintf(buf, 30, "value-%d", i);
                        invalidationMap[i] = buf;
                    }

                    // this should invalidate the Near Cache
                    boost::shared_ptr<ReplicatedMap<int, std::string> > &adapter = useNearCacheAdapter ? nearCachedMap
                                                                                                       : noNearCacheMap;
                    adapter->putAll(invalidationMap);

                    WAIT_EQ_EVENTUALLY(0, nearCache->size());
                    ASSERT_EQ(0, nearCache->size()) << "Invalidation is not working on putAll()";
                }

                std::auto_ptr<ClientConfig> clientConfig;
                std::auto_ptr<ClientConfig> nearCachedClientConfig;
                boost::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<HazelcastClient> nearCachedClient;
                boost::shared_ptr<ReplicatedMap<int, std::string> > noNearCacheMap;
                boost::shared_ptr<ReplicatedMap<int, std::string> > nearCachedMap;
                internal::nearcache::NearCacheManager *nearCacheManager;
                boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, std::string> > nearCache;
                monitor::NearCacheStats *stats;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const int BasicClientReplicatedMapNearCacheTest::DEFAULT_RECORD_COUNT = 1000;
            HazelcastServer *BasicClientReplicatedMapNearCacheTest::instance = NULL;
            HazelcastServer *BasicClientReplicatedMapNearCacheTest::instance2 = NULL;

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
                    ASSERT_NULL("Expected null from original data structure for key " << i,
                                nearCachedMap->get(i).get(), std::string);
                    // fetch value from Near Cache
                    ASSERT_NULL("Expected null from Near cached data structure for key " << i,
                                nearCachedMap->get(i).get(), std::string);

                    // fetch internal value directly from Near Cache
                    boost::shared_ptr<serialization::pimpl::Data> key = getNearCacheKey(i);
                    boost::shared_ptr<std::string> value = nearCache->get(key);
                    if (value.get() != NULL) {
                        // the internal value should either be `null` or `NULL_OBJECT`
                        boost::shared_ptr<std::string> nullObj = boost::static_pointer_cast<std::string>(
                                internal::nearcache::NearCache<int, std::string>::NULL_OBJECT);
                        ASSERT_EQ(nullObj, nearCache->get(key)) << "Expected NULL_OBJECT in Near Cache for key " << i;
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
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::NONE,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        size);

                nearCacheConfig->setInvalidateOnChange(false);

                createNoNearCacheContext();

                createNearCacheContext();

                populateMap();

                populateNearCache();

                ASSERT_EQ(size, nearCache->size());
                boost::shared_ptr<std::string> value = nearCachedMap->get(1);
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value-1", *value);

                nearCachedMap->put(1, "newValue");

                int64_t expectedMisses = getExpectedMissesWithLocalUpdatePolicy();
                int64_t expectedHits = getExpectedHitsWithLocalUpdatePolicy();

                value = nearCachedMap->get(1);
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("newValue", *value);
                value = nearCachedMap->get(1);
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("newValue", *value);

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
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::NONE,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        size);
                nearCacheConfig->setInvalidateOnChange(true);

                createNoNearCacheContext();

                createNearCacheContext();

                populateMap();

                populateNearCache();

                ASSERT_EQ(size, nearCache->size());
                boost::shared_ptr<std::string> value = nearCachedMap->get(1);
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ("value-1", *value);

                noNearCacheMap->put(1, "newValue");

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
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::LRU,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        DEFAULT_RECORD_COUNT);
                createNoNearCacheContext();

                // all Near Cache implementations use the same eviction algorithm, which evicts a single entry
                int64_t expectedEvictions = 1;

                createNearCacheContext();

                // populate map with an extra entry
                populateMap();
                char buf[20];
                util::hz_snprintf(buf, 20, "value-%d", DEFAULT_RECORD_COUNT);
                noNearCacheMap->put(DEFAULT_RECORD_COUNT, buf);

                // populate Near Caches
                populateNearCache();

                // we expect (size + the extra entry - the expectedEvictions) entries in the Near Cache
                int64_t expectedOwnedEntryCount = DEFAULT_RECORD_COUNT + 1 - expectedEvictions;
                int64_t expectedHits = stats->getHits();
                int64_t expectedMisses = stats->getMisses() + 1;

                // trigger eviction via fetching the extra entry
                nearCachedMap->get(DEFAULT_RECORD_COUNT);

                int64_t evictions = stats->getEvictions();
                ASSERT_GE(evictions, expectedEvictions)
                                            << "Near Cache eviction count didn't reach the desired value ("
                                            << expectedEvictions << " vs. " << evictions << ") ("
                                            << stats->toString();

                SCOPED_TRACE("testNearCacheEviction");

                NearCacheTestUtils::assertNearCacheStats(*stats, expectedOwnedEntryCount, expectedHits, expectedMisses,
                                                         expectedEvictions, 0);
            }

            INSTANTIATE_TEST_CASE_P(ReplicatedMapNearCacheTest, BasicClientReplicatedMapNearCacheTest,
                                    ::testing::Values(config::BINARY, config::OBJECT));
        }
    }
}
