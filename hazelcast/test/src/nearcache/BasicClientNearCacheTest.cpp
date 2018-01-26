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

#include <TestHelperFunctions.h>
#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/util/Util.h"
#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class BasicClientNearCacheTest
                    : public ClientTestSupport, public ::testing::WithParamInterface<config::InMemoryFormat> {
            public:
                virtual void SetUp() {
                    nearCacheConfig = NearCacheTestUtils::createNearCacheConfig<int, std::string>(GetParam());
                }

                virtual void TearDown() {
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
                            config::InMemoryFormat inMemoryFormat) {
                        boost::shared_ptr<config::NearCacheConfig<K, V> > nearCacheConfig(
                                new config::NearCacheConfig<K, V>());

                        nearCacheConfig->setName(BasicClientNearCacheTest::DEFAULT_NEAR_CACHE_NAME)
                                .setInMemoryFormat(inMemoryFormat)
                                .setInvalidateOnChange(true);

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
                        util::snprintf(buf, 300, messageFormat, expected, actual);
                        ASSERT_EQ(expected, actual) << buf << "(" << stats.toString() << ")";
                    }
                private:
                    NearCacheTestUtils();

                    NearCacheTestUtils(const NearCacheTestUtils &);
                };

                /**
                 * The default name used for the data structures which have a Near Cache.
                 */
                static const std::string DEFAULT_NEAR_CACHE_NAME;
                /**
                 * The default count to be inserted into the Near Caches.
                 */
                static const int DEFAULT_RECORD_COUNT;

                void createContext() {
                    createNoNearCacheContext();
                    createNearCacheContext();
                }

                void createNoNearCacheContext() {
                    servers.push_back(boost::shared_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory)));
                    servers.push_back(boost::shared_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory)));
                    clientConfig = getConfig();
                    client = std::auto_ptr<HazelcastClient>(new HazelcastClient(*clientConfig));
                    noNearCacheMap = std::auto_ptr<IMap<int, std::string> >(
                            new IMap<int, std::string>(client->getMap<int, std::string>(DEFAULT_NEAR_CACHE_NAME)));
                }

                void createNearCacheContext() {
                    nearCachedClientConfig = getConfig();
                    nearCachedClientConfig->addNearCacheConfig(nearCacheConfig);
                    nearCachedClient = std::auto_ptr<HazelcastClient>(new HazelcastClient(*nearCachedClientConfig));
                    nearCachedMap = std::auto_ptr<IMap<int, std::string> >(new IMap<int, std::string>(
                            nearCachedClient->getMap<int, std::string>(DEFAULT_NEAR_CACHE_NAME)));
                    nearCacheManager = &nearCachedClient->getNearCacheManager();
                    nearCache = nearCacheManager->
                            getNearCache<int, std::string, serialization::pimpl::Data>(DEFAULT_NEAR_CACHE_NAME);
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
                    IMap<int, std::string> &adapter = useNearCachedMapForRemoval ? *nearCachedMap
                                                                                 : *noNearCacheMap;
                    adapter.remove(1);

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

                void populateMap() {
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        util::snprintf(buf, 30, "value-%d", i);
                        noNearCacheMap->put(i, buf);
                    }
                    // let enough time to finish invalidation event propagation
                    util::sleep(3);
                }

                void populateNearCache() {
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        nearCachedMap->get(i);
                    }
                }

                boost::shared_ptr<serialization::pimpl::Data> getNearCacheKey(int key) {
                    return client->getSerializationService().toSharedData<int>(&key);
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

                bool checkMissesAndHits(int64_t &expectedMisses, int64_t &expectedHits, boost::shared_ptr<std::string> &value) {
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

                    populateMap();

                    createNearCacheContext();
                    populateNearCache();

                    std::map<int, std::string> invalidationMap;
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        util::snprintf(buf, 30, "value-%d", i);
                        invalidationMap[i] = buf;
                    }

                    // this should invalidate the Near Cache
                    IMap<int, std::string> &adapter = useNearCacheAdapter ? *nearCachedMap
                                                                                 : *noNearCacheMap;
                    adapter.putAll(invalidationMap);

                    WAIT_EQ_EVENTUALLY(0, nearCache->size());
                    ASSERT_EQ(0, nearCache->size()) << "Invalidation is not working on putAll()";
                }

                std::auto_ptr<ClientConfig> clientConfig;
                std::auto_ptr<ClientConfig> nearCachedClientConfig;
                boost::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<HazelcastClient> nearCachedClient;
                std::auto_ptr<IMap<int, std::string> > noNearCacheMap;
                std::auto_ptr<IMap<int, std::string> > nearCachedMap;
                internal::nearcache::NearCacheManager *nearCacheManager;
                boost::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::Data, std::string> > nearCache;
                monitor::NearCacheStats *stats;
                std::vector<boost::shared_ptr<HazelcastServer> > servers;
            };

            const std::string BasicClientNearCacheTest::DEFAULT_NEAR_CACHE_NAME = "defaultNearCache";
            const int BasicClientNearCacheTest::DEFAULT_RECORD_COUNT = 1000;

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
                        boost::shared_ptr<std::string> &nullObj = internal::nearcache::NearCache<int, std::string>::NULL_OBJECT;
                        ASSERT_EQ(nullObj, nearCache->get(key)) << "Expected NULL_OBJECT in Near Cache for key " << i;
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
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::NONE,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        size);
                createNoNearCacheContext();
                populateMap();

                createNearCacheContext();
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
            TEST_P(BasicClientNearCacheTest,
                   whenCacheIsFull_thenPutOnSameKeyShouldUpdateValue_withUpdateOnDataAdapter) {
                int size = DEFAULT_RECORD_COUNT / 2;
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::NONE,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        size);
                nearCacheConfig->setInvalidateOnChange(true);

                createNoNearCacheContext();
                populateMap();

                createNearCacheContext();
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

                // populate map
                populateMap();

                createNearCacheContext();

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
                    NearCacheTestUtils::assertNearCacheStats(*stats, DEFAULT_RECORD_COUNT, DEFAULT_RECORD_COUNT, DEFAULT_RECORD_COUNT);
                }
            }

            TEST_P(BasicClientNearCacheTest, testNearCacheEviction) {
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::LRU,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        DEFAULT_RECORD_COUNT);
                createNoNearCacheContext();

                // all Near Cache implementations use the same eviction algorithm, which evicts a single entry
                int64_t expectedEvictions = 1;

                // populate map with an extra entry
                populateMap();
                char buf[20];
                util::snprintf(buf, 20, "value-%d", DEFAULT_RECORD_COUNT);
                noNearCacheMap->put(DEFAULT_RECORD_COUNT, buf);

                createNearCacheContext();

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

            INSTANTIATE_TEST_CASE_P(ClientNearCacheTest, BasicClientNearCacheTest,
                                    ::testing::Values(config::BINARY, config::OBJECT));
        }
    }
}
