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
#include "HazelcastServer.h"
#include "ClientTestSupport.h"
#include <regex>
#include <vector>
#include "ringbuffer/StartsWithStringFilter.h"
#include "serialization/Employee.h"
#include "ClientTestSupportBase.h"
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/exception/IllegalStateException.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/pimpl/SerializationService.h>
#include <hazelcast/util/UuidUtil.h>
#include <hazelcast/client/impl/Partition.h>
#include <gtest/gtest.h>
#include <thread>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/protocol/Principal.h>
#include <hazelcast/client/connection/Connection.h>
#include <ClientTestSupport.h>
#include <memory>
#include <hazelcast/client/proxy/ClientPNCounterProxy.h>
#include <hazelcast/client/serialization/pimpl/DataInput.h>
#include <hazelcast/util/AddressUtil.h>
#include <hazelcast/util/RuntimeAvailableProcessors.h>
#include <hazelcast/client/serialization/pimpl/DataOutput.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/client/exception/IOException.h>
#include <hazelcast/client/protocol/ClientExceptionFactory.h>
#include <hazelcast/util/IOUtil.h>
#include <hazelcast/util/CountDownLatch.h>
#include <ClientTestSupportBase.h>
#include <hazelcast/util/Executor.h>
#include <hazelcast/util/Util.h>
#include <hazelcast/util/impl/SimpleExecutorService.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <hazelcast/util/ILogger.h>
#include <ctime>
#include <errno.h>
#include <hazelcast/client/LifecycleListener.h>
#include "serialization/TestRawDataPortable.h"
#include "serialization/TestSerializationConstants.h"
#include "serialization/TestMainPortable.h"
#include "serialization/TestNamedPortable.h"
#include "serialization/TestInvalidReadPortable.h"
#include "serialization/TestInvalidWritePortable.h"
#include "serialization/TestInnerPortable.h"
#include "serialization/TestNamedPortableV2.h"
#include "serialization/TestNamedPortableV3.h"
#include <hazelcast/client/SerializationConfig.h>
#include <hazelcast/client/HazelcastJsonValue.h>
#include <stdint.h>
#include "customSerialization/TestCustomSerializerX.h"
#include "customSerialization/TestCustomXSerializable.h"
#include "customSerialization/TestCustomPersonSerializer.h"
#include "serialization/ChildTemplatedPortable2.h"
#include "serialization/ParentTemplatedPortable.h"
#include "serialization/ChildTemplatedPortable1.h"
#include "serialization/ObjectCarryingPortable.h"
#include "serialization/TestDataSerializable.h"
#include <hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <hazelcast/client/query/FalsePredicate.h>
#include <set>
#include <hazelcast/client/query/EqualPredicate.h>
#include <hazelcast/client/query/QueryConstants.h>
#include <HazelcastServer.h>
#include "TestHelperFunctions.h"
#include <cmath>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h>
#include <hazelcast/util/Thread.h>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h>
#include <iostream>
#include <string>
#include "executor/tasks/SelectAllMembers.h"
#include "executor/tasks/IdentifiedFactory.h"
#include <hazelcast/client/serialization/ObjectDataOutput.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>
#include "executor/tasks/CancellationAwareTask.h"
#include "executor/tasks/NullCallable.h"
#include "executor/tasks/SerializedCounterCallable.h"
#include "executor/tasks/MapPutPartitionAwareCallable.h"
#include "executor/tasks/SelectNoMembers.h"
#include "executor/tasks/GetMemberUuidTask.h"
#include "executor/tasks/FailingCallable.h"
#include "executor/tasks/AppendCallable.h"
#include "executor/tasks/TaskWithUnserializableResponse.h"
#include <executor/tasks/CancellationAwareTask.h>
#include <executor/tasks/FailingCallable.h>
#include <executor/tasks/SelectNoMembers.h>
#include <executor/tasks/SerializedCounterCallable.h>
#include <executor/tasks/TaskWithUnserializableResponse.h>
#include <executor/tasks/GetMemberUuidTask.h>
#include <executor/tasks/AppendCallable.h>
#include <executor/tasks/SelectAllMembers.h>
#include <executor/tasks/MapPutPartitionAwareCallable.h>
#include <executor/tasks/NullCallable.h>
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
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/query/SqlPredicate.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/UTFUtil.h"
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/util/Future.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/Pipelining.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/adaptor/RawPointerSet.h"
#include "hazelcast/client/query/OrPredicate.h"
#include "hazelcast/client/query/RegexPredicate.h"
#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/query/QueryConstants.h"
#include "hazelcast/client/query/NotPredicate.h"
#include "hazelcast/client/query/InstanceOfPredicate.h"
#include "hazelcast/client/query/NotEqualPredicate.h"
#include "hazelcast/client/query/InPredicate.h"
#include "hazelcast/client/query/ILikePredicate.h"
#include "hazelcast/client/query/LikePredicate.h"
#include "hazelcast/client/query/GreaterLessPredicate.h"
#include "hazelcast/client/query/AndPredicate.h"
#include "hazelcast/client/query/BetweenPredicate.h"
#include "hazelcast/client/query/EqualPredicate.h"
#include "hazelcast/client/query/TruePredicate.h"
#include "hazelcast/client/query/FalsePredicate.h"
#include "hazelcast/client/adaptor/RawPointerMap.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/adaptor/RawPointerList.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalQueue.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/adaptor/RawPointerQueue.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalMap.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/adaptor/RawPointerMultiMap.h"
#include "hazelcast/client/adaptor/RawPointerTransactionalMultiMap.h"
#include "hazelcast/util/LittleEndianBufferWrapper.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/HazelcastJsonValue.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/IAtomicLong.h"
#include "hazelcast/client/mixedtype/MultiMap.h"
#include "hazelcast/client/mixedtype/IList.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/mixedtype/IQueue.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/client/ISet.h"
#include "hazelcast/client/mixedtype/ISet.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ReliableTopic.h"
#include "hazelcast/client/IdGenerator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
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
                        hazelcast::util::hz_snprintf(buf, 300, messageFormat, expected, actual);
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
                    client = std::unique_ptr<HazelcastClient>(new HazelcastClient(getConfig()));
                    noNearCacheMap = std::unique_ptr<IMap<int, std::string> >(
                            new IMap<int, std::string>(client->getMap<int, std::string>(getTestName())));
                }

                void createNearCacheContext() {
                    nearCachedClientConfig = getConfig();
                    nearCachedClientConfig.addNearCacheConfig(nearCacheConfig);
                    nearCachedClient = std::unique_ptr<HazelcastClient>(new HazelcastClient(nearCachedClientConfig));
                    nearCachedMap = std::unique_ptr<IMap<int, std::string> >(new IMap<int, std::string>(
                            nearCachedClient->getMap<int, std::string>(getTestName())));
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
                        hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
                        noNearCacheMap->put(i, buf);
                    }

                    assertNearCacheInvalidationRequests(*stats, DEFAULT_RECORD_COUNT);
                }

                void populateNearCache() {
                    char buf[30];
                    for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                        std::shared_ptr<string> value = nearCachedMap->get(i);
                        ASSERT_NOTNULL(value.get(), std::string);
                        hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
                        ASSERT_EQ(buf, *value);
                    }
                }

                std::shared_ptr<serialization::pimpl::Data> getNearCacheKey(int key) {
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
                                        std::shared_ptr<std::string> &value) {
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
                        hazelcast::util::hz_snprintf(buf, 30, "value-%d", i);
                        invalidationMap[i] = buf;
                    }

                    // this should invalidate the Near Cache
                    IMap<int, std::string> &adapter = useNearCacheAdapter ? *nearCachedMap
                                                                          : *noNearCacheMap;
                    adapter.putAll(invalidationMap);

                    WAIT_EQ_EVENTUALLY(0, nearCache->size());
                    ASSERT_EQ(0, nearCache->size()) << "Invalidation is not working on putAll()";
                }

                ClientConfig clientConfig;
                ClientConfig nearCachedClientConfig;
                std::shared_ptr<config::NearCacheConfig<int, std::string> > nearCacheConfig;
                std::unique_ptr<HazelcastClient> client;
                std::unique_ptr<HazelcastClient> nearCachedClient;
                std::unique_ptr<IMap<int, std::string> > noNearCacheMap;
                std::unique_ptr<IMap<int, std::string> > nearCachedMap;
                hazelcast::client::internal::nearcache::NearCacheManager *nearCacheManager;
                std::shared_ptr<hazelcast::client::internal::nearcache::NearCache<serialization::pimpl::Data, std::string> > nearCache;
                monitor::NearCacheStats *stats;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const int BasicClientNearCacheTest::DEFAULT_RECORD_COUNT = 1000;
            HazelcastServer *BasicClientNearCacheTest::instance = NULL;
            HazelcastServer *BasicClientNearCacheTest::instance2 = NULL;

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
                    std::shared_ptr<serialization::pimpl::Data> key = getNearCacheKey(i);
                    std::shared_ptr<std::string> value = nearCache->get(key);
                    if (value.get() != NULL) {
                        // the internal value should either be `null` or `NULL_OBJECT`
                        std::shared_ptr<std::string> nullObj = std::static_pointer_cast<std::string>(
                                hazelcast::client::internal::nearcache::NearCache<int, std::string>::NULL_OBJECT);
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

                createNearCacheContext();

                populateMap();

                populateNearCache();

                ASSERT_EQ(size, nearCache->size());
                std::shared_ptr<std::string> value = nearCachedMap->get(1);
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

                createNearCacheContext();

                populateMap();

                populateNearCache();

                ASSERT_EQ(size, nearCache->size());
                std::shared_ptr<std::string> value = nearCachedMap->get(1);
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
                NearCacheTestUtils::setEvictionConfig<int, std::string>(*nearCacheConfig, config::LRU,
                                                                        config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                                        DEFAULT_RECORD_COUNT);
                createNoNearCacheContext();

                createNearCacheContext();

// all Near Cache implementations use the same eviction algorithm, which evicts a single entry
                int64_t expectedEvictions = 1;

// populate map with an extra entry
                populateMap();
                char buf[20];
                hazelcast::util::hz_snprintf(buf, 20, "value-%d", DEFAULT_RECORD_COUNT);
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
                static const std::string DEFAULT_NEAR_CACHE_NAME;

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


                virtual void TearDown() {
                    if (map.get()) {
                        map->destroy();
                    }
                }

                std::shared_ptr<config::NearCacheConfig<int, int> > newNoInvalidationNearCacheConfig() {
                    std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig(newNearCacheConfig());
                    nearCacheConfig->setInMemoryFormat(config::OBJECT);
                    nearCacheConfig->setInvalidateOnChange(false);
                    return nearCacheConfig;
                }

                std::shared_ptr<config::NearCacheConfig<int, int> > newNearCacheConfig() {
                    return std::shared_ptr<config::NearCacheConfig<int, int> >(
                            new config::NearCacheConfig<int, int>());
                }

                std::unique_ptr<ClientConfig> newClientConfig() {
                    return std::unique_ptr<ClientConfig>(new ClientConfig());
                }

                IMap<int, int> &getNearCachedMapFromClient(
                        std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig) {
                    std::string mapName = DEFAULT_NEAR_CACHE_NAME;

                    nearCacheConfig->setName(mapName);

                    clientConfig = newClientConfig();
                    clientConfig->addNearCacheConfig(nearCacheConfig);

                    client = std::unique_ptr<HazelcastClient>(new HazelcastClient(*clientConfig));
                    map.reset(new IMap<int, int>(client->getMap<int, int>(mapName)));
                    return *map;
                }

                monitor::NearCacheStats *getNearCacheStats(IMap<int, int> &map) {
                    return map.getLocalMapStats().getNearCacheStats();
                }

                void assertThatOwnedEntryCountEquals(IMap<int, int> &clientMap, int64_t expected) {
                    ASSERT_EQ(expected, getNearCacheStats(clientMap)->getOwnedEntryCount());
                }

                std::unique_ptr<ClientConfig> clientConfig;
                std::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig;
                std::unique_ptr<HazelcastClient> client;
                std::shared_ptr<IMap<int, int> > map;
                static HazelcastServer *instance;
                static HazelcastServer *instance2;
            };

            const std::string ClientMapNearCacheTest::DEFAULT_NEAR_CACHE_NAME = "defaultNearCache";
            HazelcastServer *ClientMapNearCacheTest::instance = NULL;
            HazelcastServer *ClientMapNearCacheTest::instance2 = NULL;

            TEST_F(ClientMapNearCacheTest, testGetAllChecksNearCacheFirst) {
                IMap<int, int> &map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

                std::set<int> keys;

                int size = 1003;
                for (int i = 0; i < size; i++) {
                    map.put(i, i);
                    keys.insert(i);
                }
                // populate Near Cache
                for (int i = 0; i < size; i++) {
                    map.get(i);
                }
                // getAll() generates the Near Cache hits
                map.getAll(keys);

                monitor::NearCacheStats *stats = getNearCacheStats(map);
                ASSERT_EQ(size, stats->getOwnedEntryCount());
                ASSERT_EQ(size, stats->getHits());
            }

            TEST_F(ClientMapNearCacheTest, testGetAllPopulatesNearCache) {
                IMap<int, int> &map = getNearCachedMapFromClient(newNoInvalidationNearCacheConfig());

                std::set<int> keys;

                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map.put(i, i);
                    keys.insert(i);
                }
// populate Near Cache
                for (int i = 0; i < size; i++) {
                    map.get(i);
                }
// getAll() generates the Near Cache hits
                map.getAll(keys);

                assertThatOwnedEntryCountEquals(map, size);
            }

            TEST_F(ClientMapNearCacheTest, testRemoveAllNearCache) {
                IMap<int, int> &map = getNearCachedMapFromClient(newNearCacheConfig());

                std::set<int> keys;

                int size = 1214;
                for (int i = 0; i < size; i++) {
                    map.put(i, i);
                    keys.insert(i);
                }
// populate Near Cache
                for (int i = 0; i < size; i++) {
                    map.get(i);
                }

                map.removeAll(query::EqualPredicate<int>(query::QueryConstants::getKeyAttributeName(), 20));

                assertThatOwnedEntryCountEquals(map, 0);
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            class MySetItemListener : public ItemListener<std::string> {
            public:
                MySetItemListener(hazelcast::util::CountDownLatch &latch)
                        :latch(latch) {

                }

                void itemAdded(const ItemEvent<std::string>& itemEvent) {
                    latch.countDown();
                }

                void itemRemoved(const ItemEvent<std::string>& item) {
                }

            private:
                hazelcast::util::CountDownLatch &latch;
            };

            class ClientSetTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    set->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    set = new ISet<std::string>(client->getSet<std::string>("MySet"));
                }

                static void TearDownTestCase() {
                    delete set;
                    delete client;
                    delete instance;

                    set = NULL;
                    client = NULL;
                    instance = NULL;
                }

                bool itemExists(const std::vector<std::string> &items, const std::string &item) const {
                    bool found = false;
                    for (std::vector<std::string>::const_iterator it = items.begin();it != items.end();++it) {
                        if (item == *it) {
                            found = true;
                            break;
                        }
                    }
                    return found;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static ISet<std::string> *set;
            };

            HazelcastServer *ClientSetTest::instance = NULL;
            HazelcastClient *ClientSetTest::client = NULL;
            ISet<std::string> *ClientSetTest::set = NULL;

            TEST_F(ClientSetTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");

                ASSERT_TRUE(set->addAll(l));
                ASSERT_EQ(2, set->size());

                ASSERT_FALSE(set->addAll(l));
                ASSERT_EQ(2, set->size());
            }

            TEST_F(ClientSetTest, testAddRemove) {
                ASSERT_TRUE(set->add("item1"));
                ASSERT_TRUE(set->add("item2"));
                ASSERT_TRUE(set->add("item3"));
                ASSERT_EQ(3, set->size());

                ASSERT_FALSE(set->add("item3"));
                ASSERT_EQ(3, set->size());


                ASSERT_FALSE(set->remove("item4"));
                ASSERT_TRUE(set->remove("item3"));
            }

            TEST_F(ClientSetTest, testContains) {
                ASSERT_TRUE(set->add("item1"));
                ASSERT_TRUE(set->add("item2"));
                ASSERT_TRUE(set->add("item3"));
                ASSERT_TRUE(set->add("item4"));

                ASSERT_FALSE(set->contains("item5"));
                ASSERT_TRUE(set->contains("item2"));

                std::vector<std::string> l;
                l.push_back("item6");
                l.push_back("item3");

                ASSERT_FALSE(set->containsAll(l));
                ASSERT_TRUE(set->add("item6"));
                ASSERT_TRUE(set->containsAll(l));
            }

            TEST_F(ClientSetTest, testToArray) {
                ASSERT_TRUE(set->add("item1"));
                ASSERT_TRUE(set->add("item2"));
                ASSERT_TRUE(set->add("item3"));
                ASSERT_TRUE(set->add("item4"));
                ASSERT_FALSE(set->add("item4"));

                std::vector<std::string> items = set->toArray();

                ASSERT_EQ((size_t) 4, items.size());
                ASSERT_TRUE(itemExists(items, "item1"));
                ASSERT_TRUE(itemExists(items, "item2"));
                ASSERT_TRUE(itemExists(items, "item3"));
                ASSERT_TRUE(itemExists(items, "item4"));
            }

            TEST_F(ClientSetTest, testRemoveRetainAll) {
                ASSERT_TRUE(set->add("item1"));
                ASSERT_TRUE(set->add("item2"));
                ASSERT_TRUE(set->add("item3"));
                ASSERT_TRUE(set->add("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(set->removeAll(l));
                ASSERT_EQ(2, set->size());
                ASSERT_FALSE(set->removeAll(l));
                ASSERT_EQ(2, set->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(set->retainAll(l));
                ASSERT_EQ(2, set->size());

                l.clear();
                ASSERT_TRUE(set->retainAll(l));
                ASSERT_EQ(0, set->size());

            }

            TEST_F(ClientSetTest, testListener) {
                hazelcast::util::CountDownLatch latch(6);

                MySetItemListener listener(latch);
                std::string registrationId = set->addItemListener(listener, true);

                for (int i = 0; i < 5; i++) {
                    set->add(std::string("item") + hazelcast::util::IOUtil::to_string(i));
                }
                set->add("done");

                ASSERT_TRUE(latch.await(20));

                ASSERT_TRUE(set->removeItemListener(registrationId));
            }

            TEST_F(ClientSetTest, testIsEmpty) {
                ASSERT_TRUE(set->isEmpty());
                ASSERT_TRUE(set->add("item1"));
                ASSERT_FALSE(set->isEmpty());
            }
        }
    }
}





using namespace hazelcast::client::mixedtype;

namespace hazelcast {
    namespace client {
        namespace test {
            class MixedSetItemListener : public MixedItemListener {
            public:
                MixedSetItemListener(hazelcast::util::CountDownLatch &latch)
                        :latch(latch) {
                }

                virtual void itemAdded(const ItemEvent<TypedData> &item) {
                    latch.countDown();
                }

                virtual void itemRemoved(const ItemEvent<TypedData> &item) {
                }

            private:
                hazelcast::util::CountDownLatch &latch;
            };

            class MixedSetTest : public ClientTestSupport {
            protected:
                virtual void TearDown() {
                    set->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient;
                    set = new mixedtype::ISet(client->toMixedType().getSet("MySet"));
                }

                static void TearDownTestCase() {
                    delete set;
                    delete client;
                    delete instance;

                    set = NULL;
                    client = NULL;
                    instance = NULL;
                }

                bool itemExists(const std::vector<TypedData> &items, const std::string &item) const {
                    bool found = false;
                    for (std::vector<TypedData>::const_iterator it = items.begin();it != items.end();++it) {
                        if (item == *((*it).get<std::string>())) {
                            found = true;
                            break;
                        }
                    }
                    return found;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static  mixedtype::ISet *set;
            };

            HazelcastServer *MixedSetTest::instance = NULL;
            HazelcastClient *MixedSetTest::client = NULL;
            mixedtype::ISet *MixedSetTest::set = NULL;

            TEST_F(MixedSetTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");

                ASSERT_TRUE(set->addAll<std::string>(l));
                ASSERT_EQ(2, set->size());

                ASSERT_FALSE(set->addAll<std::string>(l));
                ASSERT_EQ(2, set->size());
            }

            TEST_F(MixedSetTest, testAddRemove) {
                ASSERT_TRUE(set->add<std::string>("item1"));
                ASSERT_TRUE(set->add<std::string>("item2"));
                ASSERT_TRUE(set->add<std::string>("item3"));
                ASSERT_EQ(3, set->size());

                ASSERT_FALSE(set->add<std::string>("item3"));
                ASSERT_EQ(3, set->size());


                ASSERT_FALSE(set->remove<std::string>("item4"));
                ASSERT_TRUE(set->remove<std::string>("item3"));
            }

            TEST_F(MixedSetTest, testContains) {
                ASSERT_TRUE(set->add<std::string>("item1"));
                ASSERT_TRUE(set->add<std::string>("item2"));
                ASSERT_TRUE(set->add<std::string>("item3"));
                ASSERT_TRUE(set->add<std::string>("item4"));

                ASSERT_FALSE(set->contains<std::string>("item5"));
                ASSERT_TRUE(set->contains<std::string>("item2"));

                std::vector<std::string> l;
                l.push_back("item6");
                l.push_back("item3");

                ASSERT_FALSE(set->containsAll<std::string>(l));
                ASSERT_TRUE(set->add<std::string>("item6"));
                ASSERT_TRUE(set->containsAll<std::string>(l));
            }

            TEST_F(MixedSetTest, testToArray) {
                ASSERT_TRUE(set->add<std::string>("item1"));
                ASSERT_TRUE(set->add<std::string>("item2"));
                ASSERT_TRUE(set->add<std::string>("item3"));
                ASSERT_TRUE(set->add<std::string>("item4"));
                ASSERT_FALSE(set->add<std::string>("item4"));

                std::vector<TypedData> items = set->toArray();

                ASSERT_EQ((size_t) 4, items.size());
                ASSERT_TRUE(itemExists(items, "item1"));
                ASSERT_TRUE(itemExists(items, "item2"));
                ASSERT_TRUE(itemExists(items, "item3"));
                ASSERT_TRUE(itemExists(items, "item4"));
            }

            TEST_F(MixedSetTest, testRemoveRetainAll) {
                ASSERT_TRUE(set->add<std::string>("item1"));
                ASSERT_TRUE(set->add<std::string>("item2"));
                ASSERT_TRUE(set->add<std::string>("item3"));
                ASSERT_TRUE(set->add<std::string>("item4"));

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(set->removeAll<std::string>(l));
                ASSERT_EQ(2, set->size());
                ASSERT_FALSE(set->removeAll<std::string>(l));
                ASSERT_EQ(2, set->size());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(set->retainAll<std::string>(l));
                ASSERT_EQ(2, set->size());

                l.clear();
                ASSERT_TRUE(set->retainAll<std::string>(l));
                ASSERT_EQ(0, set->size());

            }

            TEST_F(MixedSetTest, testListener) {
                hazelcast::util::CountDownLatch latch(6);

                MixedSetItemListener listener(latch);
                std::string registrationId = set->addItemListener(listener, true);

                for (int i = 0; i < 5; i++) {
                    set->add<std::string>(std::string("item") + hazelcast::util::IOUtil::to_string(i));
                }
                set->add<std::string>("done");

                ASSERT_TRUE(latch.await(20));

                ASSERT_TRUE(set->removeItemListener(registrationId));
            }
        }
    }
}




namespace hazelcast {
    namespace client {
        namespace test {
            class ICountDownLatchTest : public ClientTestSupport {
            public:
                ICountDownLatchTest();

                ~ICountDownLatchTest();
            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
                std::unique_ptr<ICountDownLatch> l;
            };

            ICountDownLatchTest::ICountDownLatchTest()
                    : instance(*g_srvFactory), client(getNewClient()),
                      l(new ICountDownLatch(client.getICountDownLatch("ICountDownLatchTest"))) {
            }

            ICountDownLatchTest::~ICountDownLatchTest() {
            }

            void testLatchThread(hazelcast::util::ThreadArgs &args) {
                ICountDownLatch *l = (ICountDownLatch *) args.arg0;
                for (int i = 0; i < 20; i++) {
                    l->countDown();
                }
            }

            TEST_F(ICountDownLatchTest, testLatch) {
                ASSERT_TRUE(l->trySetCount(20));
                ASSERT_FALSE(l->trySetCount(10));
                ASSERT_EQ(20, l->getCount());

                hazelcast::util::StartedThread t(testLatchThread, l.get());

                ASSERT_TRUE(l->await(10 * 1000));

                t.join();
            }

        }
    }
}






namespace hazelcast {
    namespace client {
        namespace test {
            class ReliableTopicTest : public ClientTestSupport {
            public:
                static void publishTopics(hazelcast::util::ThreadArgs &args) {
                    ReliableTopic<int> *topic = (ReliableTopic<int> *)args.arg0;
                    std::vector<int> *publishValues = (std::vector<int> *)args.arg1;

                    hazelcast::util::sleep(5);

                    for (std::vector<int>::const_iterator it = publishValues->begin();it != publishValues->end(); ++it) {
                        topic->publish(&(*it));
                    }
                }
            protected:
                template <typename T>
                class GenericListener : public topic::ReliableMessageListener<T> {
                public:
                    GenericListener(hazelcast::util::CountDownLatch &countDownLatch) : latch(countDownLatch),
                                                                                       startSequence(-1),
                                                                                       numberOfMessagesReceived(0) {
                    }

                    GenericListener(hazelcast::util::CountDownLatch &countDownLatch, int64_t sequence) : latch(
                            countDownLatch),
                                                                                                         startSequence(
                                                                                                                 sequence),
                                                                                                         numberOfMessagesReceived(
                                                                                                                 0) {
                    }

                    virtual ~GenericListener() {
                        topic::Message<T> *m = NULL;
                        while ((m = messages.poll()) != NULL) {
                            delete (m);
                        }
                    }

                    virtual void onMessage(std::unique_ptr<topic::Message<T> > &&message) {
                        ++numberOfMessagesReceived;

                        messages.offer(message.release());

                        latch.countDown();
                    }

                    virtual int64_t retrieveInitialSequence() const {
                        return startSequence;
                    }

                    virtual void storeSequence(int64_t sequence) {
                    }

                    virtual bool isLossTolerant() const {
                        return false;
                    }

                    virtual bool isTerminal(const exception::IException &failure) const {
                        return false;
                    }

                    int getNumberOfMessagesReceived() {
                        int value = numberOfMessagesReceived;
                        return value;
                    }

                    hazelcast::util::ConcurrentQueue<topic::Message<T> > &getMessages() {
                        return messages;
                    }
                private:
                    hazelcast::util::CountDownLatch &latch;
                    int64_t startSequence;
                    hazelcast::util::AtomicInt numberOfMessagesReceived;
                    hazelcast::util::ConcurrentQueue<topic::Message<T> > messages;
                };

                class IntListener : public GenericListener<int> {
                public:
                    IntListener(hazelcast::util::CountDownLatch &countDownLatch) : GenericListener<int>(countDownLatch) { }

                    IntListener(hazelcast::util::CountDownLatch &countDownLatch, int64_t sequence) : GenericListener<int>(
                            countDownLatch, sequence) { }
                };

                class MyReliableListener : public GenericListener<Employee> {
                public:
                    MyReliableListener(hazelcast::util::CountDownLatch &countDownLatch, int64_t sequence) : GenericListener<Employee>(
                            countDownLatch, sequence) { }

                    MyReliableListener(hazelcast::util::CountDownLatch &countDownLatch) : GenericListener<Employee>(countDownLatch) { }
                };

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    client = NULL;
                    instance = NULL;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
            };

            HazelcastServer *ReliableTopicTest::instance = NULL;
            HazelcastClient *ReliableTopicTest::client = NULL;

            TEST_F(ReliableTopicTest, testBasics) {
                std::shared_ptr<ReliableTopic<Employee> > rt;
                ASSERT_NO_THROW(rt = client->getReliableTopic<Employee>("testBasics"));

                ASSERT_EQ("testBasics", rt->getName());

                hazelcast::util::CountDownLatch latch(1);

                MyReliableListener listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = rt->addMessageListener(listener));

                Employee empl1("first", 20);

                ASSERT_NO_THROW(rt->publish(&empl1));

                ASSERT_TRUE(latch.await(2));
                ASSERT_EQ(1, listener.getNumberOfMessagesReceived());
                const Employee *employee = listener.getMessages().poll()->getMessageObject();
                ASSERT_NE((Employee *) NULL, employee);
                ASSERT_EQ(empl1, *employee);

                // remove listener
                ASSERT_TRUE(rt->removeMessageListener(listenerId));
                ASSERT_FALSE(rt->removeMessageListener(listenerId));
            }

            TEST_F(ReliableTopicTest, testListenerSequence) {
                std::shared_ptr<ReliableTopic<Employee> > rt;
                ASSERT_NO_THROW(rt = client->getReliableTopic<Employee>("testListenerSequence"));

                Employee empl1("first", 10);
                Employee empl2("second", 20);

                ASSERT_NO_THROW(rt->publish(&empl1));
                ASSERT_NO_THROW(rt->publish(&empl2));

                hazelcast::util::CountDownLatch latch(1);

                MyReliableListener listener(latch, 1);
                std::string listenerId;
                ASSERT_NO_THROW(listenerId = rt->addMessageListener(listener));

                ASSERT_TRUE(latch.await(1));
                ASSERT_EQ(1, listener.getNumberOfMessagesReceived());
                const Employee *employee = listener.getMessages().poll()->getMessageObject();
                ASSERT_NE((Employee *) NULL, employee);
                ASSERT_EQ(empl2, *employee);

                // remove listener
                ASSERT_TRUE(rt->removeMessageListener(listenerId));
            }

            TEST_F(ReliableTopicTest, removeMessageListener_whenExisting) {
                std::shared_ptr<ReliableTopic<Employee> > rt;
                ASSERT_NO_THROW(rt = client->getReliableTopic<Employee>("removeMessageListener_whenExisting"));

                Employee empl1("first", 10);

                hazelcast::util::CountDownLatch latch(1);

                MyReliableListener listener(latch);
                std::string listenerId;
                ASSERT_NO_THROW(listenerId = rt->addMessageListener(listener));

                // remove listener
                ASSERT_TRUE(rt->removeMessageListener(listenerId));

                ASSERT_NO_THROW(rt->publish(&empl1));

                ASSERT_FALSE(latch.await(2));
                ASSERT_EQ(0, listener.getNumberOfMessagesReceived());
            }

            TEST_F(ReliableTopicTest, removeMessageListener_whenNonExisting) {
                std::shared_ptr<ReliableTopic<Employee> > rt;
                ASSERT_NO_THROW(rt = client->getReliableTopic<Employee>("removeMessageListener_whenNonExisting"));

// remove listener
                ASSERT_FALSE(rt->removeMessageListener("abc"));
            }

            TEST_F(ReliableTopicTest, publishNull) {
                std::shared_ptr<ReliableTopic<int> > intTopic;
                ASSERT_NO_THROW(intTopic = client->getReliableTopic<int>("publishNull"));

                hazelcast::util::CountDownLatch latch(1);
                IntListener listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = intTopic->addMessageListener(listener));

                intTopic->publish((int *) NULL);

                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ(1, listener.getNumberOfMessagesReceived());
                const int *val = listener.getMessages().poll()->getMessageObject();
                ASSERT_EQ((const int *) NULL, val);
            }

            TEST_F(ReliableTopicTest, publishMultiple) {
                std::shared_ptr<ReliableTopic<std::string> > topic;
                ASSERT_NO_THROW(topic = client->getReliableTopic<std::string>("publishMultiple"));

                hazelcast::util::CountDownLatch latch(5);
                GenericListener <std::string> listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(listener));

                std::vector<std::string> items;
                for (int k = 0; k < 5; k++) {
                    std::string item = hazelcast::util::IOUtil::to_string<int>(k);
                    topic->publish(&item);
                    items.push_back(item);
                }

                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ(5, listener.getNumberOfMessagesReceived());
                hazelcast::util::ConcurrentQueue<topic::Message<std::string> > &queue = listener.getMessages();
                for (int k = 0; k < 5; k++) {
                    const std::string *val = queue.poll()->getMessageObject();
                    ASSERT_NE((const std::string *) NULL, val);
                    ASSERT_EQ(items[k], *val);
                }
            }

            TEST_F(ReliableTopicTest, testConfig) {
                ClientConfig clientConfig;
                clientConfig.addAddress(Address(g_srvFactory->getServerAddress(), 5701));
                config::ReliableTopicConfig relConfig("testConfig");
                relConfig.setReadBatchSize(2);
                clientConfig.addReliableTopicConfig(relConfig);
                HazelcastClient configClient(clientConfig);

                std::shared_ptr<ReliableTopic<std::string> > topic;
                ASSERT_NO_THROW(topic = configClient.getReliableTopic<std::string>("testConfig"));

                hazelcast::util::CountDownLatch latch(5);
                GenericListener <std::string> listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = topic->addMessageListener(listener));

                std::vector<std::string> items;
                for (int k = 0; k < 5; k++) {
                    std::string item = hazelcast::util::IOUtil::to_string<int>(k);
                    topic->publish(&item);
                    items.push_back(item);
                }

                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ(5, listener.getNumberOfMessagesReceived());
                hazelcast::util::ConcurrentQueue<topic::Message<std::string> > &queue = listener.getMessages();
                for (int k = 0; k < 5; k++) {
                    const std::string *val = queue.poll()->getMessageObject();
                    ASSERT_NE((const std::string *) NULL, val);
                    ASSERT_EQ(items[k], *val);
                }
            }

            TEST_F(ReliableTopicTest, testMessageFieldSetCorrectly) {
                std::shared_ptr<ReliableTopic<int> > intTopic;
                ASSERT_NO_THROW(intTopic = client->getReliableTopic<int>("testMessageFieldSetCorrectly"));

                hazelcast::util::CountDownLatch latch(1);
                IntListener listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = intTopic->addMessageListener(listener));

                int64_t timeBeforePublish = hazelcast::util::currentTimeMillis();
                int publishedValue = 3;
                intTopic->publish(&publishedValue);
                int64_t timeAfterPublish = hazelcast::util::currentTimeMillis();

                ASSERT_TRUE(latch.await(5));
                ASSERT_EQ(1, listener.getNumberOfMessagesReceived());
                topic::Message<int> *message = listener.getMessages().poll();
                const int *val = message->getMessageObject();
                ASSERT_EQ(publishedValue, *val);

                ASSERT_LE(timeBeforePublish, message->getPublishTime());
                ASSERT_GE(timeAfterPublish, message->getPublishTime());
                ASSERT_EQ(intTopic->getName(), message->getSource());
                ASSERT_EQ((Member *) NULL, message->getPublishingMember());
            }

// makes sure that when a listener is register, we don't see any messages being published before
// it got registered. We'll only see the messages after it got registered.
            TEST_F(ReliableTopicTest, testAlwaysStartAfterTail) {
                std::shared_ptr<ReliableTopic<int> > intTopic;
                ASSERT_NO_THROW(intTopic = client->getReliableTopic<int>("testAlwaysStartAfterTail"));

                int publishedValue = 1;
                ASSERT_NO_THROW(intTopic->publish(&publishedValue));
                publishedValue = 2;
                ASSERT_NO_THROW(intTopic->publish(&publishedValue));
                publishedValue = 3;
                ASSERT_NO_THROW(intTopic->publish(&publishedValue));

                std::vector<int> expectedValues;
                expectedValues.push_back(4);
                expectedValues.push_back(5);
                expectedValues.push_back(6);

// spawn a thread for publishing new data
                hazelcast::util::StartedThread t(publishTopics, intTopic.get(), &expectedValues);

                hazelcast::util::CountDownLatch latch(3);
                IntListener listener(latch);

                std::string listenerId;
                ASSERT_NO_THROW(listenerId = intTopic->addMessageListener(listener));

                ASSERT_TRUE(latch.await(10));
                ASSERT_EQ((int) expectedValues.size(), listener.getNumberOfMessagesReceived());
                hazelcast::util::ConcurrentQueue<topic::Message<int> > &objects = listener.getMessages();

                for (std::vector<int>::const_iterator it = expectedValues.begin(); it != expectedValues.end(); ++it) {
                    std::unique_ptr<int> val = objects.poll()->releaseMessageObject();
                    ASSERT_NE((int *) NULL, val.get());
                    ASSERT_EQ(*it, *val);
                }
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

                    class StatsPrinterTask : public hazelcast::util::Runnable {
                    public:
                        StatsPrinterTask(Stats &stats) : stats(stats) {}

                        virtual void run() {
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

                        virtual const std::string getName() const {
                            return "StatPrinterTask";
                        }

                    private:
                        Stats &stats;
                    };

                    class Task : public hazelcast::util::Runnable {
                    public:
                        Task(Stats &stats, IMap<int, std::vector<char> > &map,
                             const std::shared_ptr<hazelcast::util::ILogger> &logger) : stats(stats), map(map),
                                                                                        logger(logger) {
                        }

                        virtual void run() {
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
                                        map.get(key);
                                        ++getCount;
                                    } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                                        std::shared_ptr<std::vector<char> > vector = map.put(key, value);
                                        ++putCount;
                                    } else {
                                        map.remove(key);
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

                        virtual const std::string getName() const {
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
                        IMap<int, std::vector<char> > &map;
                        std::shared_ptr<hazelcast::util::ILogger> logger;
                    };


                    void start(const HazelcastServer &server) {
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
                        clientConfig.addAddress(Address(member.host, member.port)).setAttemptPeriod(10 * 1000);
                        clientConfig.setLogLevel(FINEST);

                        Stats stats;
                        std::shared_ptr<hazelcast::util::ILogger> logger(
                                new hazelcast::util::ILogger("SimpleMapTest", "SimpleMapTest", "testversion",
                                                             config::LoggerConfig()));
                        if (!logger->start()) {
                            throw (client::exception::ExceptionBuilder<client::exception::IllegalStateException>(
                                    "SimpleMapTest::start") << "Could not start logger "
                                                            << logger->getInstanceName()).build();
                        }

                        hazelcast::util::Thread monitor(
                                std::shared_ptr<hazelcast::util::Runnable>(new StatsPrinterTask(stats)),
                                *logger);

                        HazelcastClient hazelcastClient(clientConfig);

                        IMap<int, std::vector<char> > map = hazelcastClient.getMap<int, std::vector<char> >(
                                "cppDefault");

                        std::vector<std::shared_ptr<hazelcast::util::Thread> > threads;

                        for (int i = 0; i < THREAD_COUNT; i++) {
                            std::shared_ptr<hazelcast::util::Thread> thread = std::shared_ptr<hazelcast::util::Thread>(
                                    new hazelcast::util::Thread(
                                            std::shared_ptr<hazelcast::util::Runnable>(new Task(stats, map, logger)),
                                            *logger));
                            thread->start();
                            threads.push_back(thread);
                        }

                        monitor.start();
                        monitor.join();
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
            class IdGeneratorTest : public ClientTestSupport {
            public:
                IdGeneratorTest();

            protected:
                HazelcastServer instance;
                ClientConfig clientConfig;
                HazelcastClient client;
                std::unique_ptr<IdGenerator> generator;

            };

            IdGeneratorTest::IdGeneratorTest()
                    : instance(*g_srvFactory), client(getNewClient()),
                      generator(new IdGenerator(client.getIdGenerator("clientIdGenerator"))) {
            }

            TEST_F (IdGeneratorTest, testGenerator) {
                int initValue = 3569;
                ASSERT_TRUE(generator->init(initValue));
                ASSERT_FALSE(generator->init(4569));
                for (int i = 0; i < 2000; i++) {
                    ASSERT_EQ(++initValue, generator->newId());
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

                ~IssueTest();

            protected:
                class Issue864MapListener : public hazelcast::client::EntryAdapter<int, int> {
                public:
                    Issue864MapListener(hazelcast::util::CountDownLatch &l);

                    virtual void entryAdded(const EntryEvent<int, int> &event);

                    virtual void entryUpdated(const EntryEvent<int, int> &event);

                private:
                    hazelcast::util::CountDownLatch &latch;
                };

                hazelcast::util::CountDownLatch latch;
                Issue864MapListener listener;
            };

            IssueTest::IssueTest()
                    : latch(2), listener(latch) {
            }

            IssueTest::~IssueTest() {
            }

            void threadTerminateNode(hazelcast::util::ThreadArgs &args) {
                HazelcastServer *node = (HazelcastServer *) args.arg0;
                node->shutdown();
            }

            void putMapMessage(hazelcast::util::ThreadArgs &args) {
                IMap<int, int> *map = (IMap<int, int> *) args.arg0;
                hazelcast::util::CountDownLatch *latch = (hazelcast::util::CountDownLatch *) args.arg1;

                do {
                    // 7. Put a 2nd entry to the map
                    try {
                        map->put(2, 20);
                    } catch (std::exception &e) {
                        // suppress the error
                        (void) e; // suppress the unused variable warning
                    }
                    hazelcast::util::sleep(1);
                } while (latch->get() > 0);

            }

            TEST_F(IssueTest, testOperationRedo_smartRoutingDisabled) {
                HazelcastServer hz1(*g_srvFactory);
                HazelcastServer hz2(*g_srvFactory);

                ClientConfig clientConfig(getConfig());
                clientConfig.setRedoOperation(true);
                clientConfig.setSmart(false);

                HazelcastClient client(clientConfig);

                client::IMap<int, int> map = client.getMap<int, int>("m");
                hazelcast::util::StartedThread *thread = NULL;
                int expected = 1000;
                for (int i = 0; i < expected; i++) {
                    if (i == 5) {
                        thread = new hazelcast::util::StartedThread(threadTerminateNode, &hz1);
                    }
                    map.put(i, i);
                }
                thread->join();
                delete thread;
                ASSERT_EQ(expected, map.size());
            }

            TEST_F(IssueTest, testListenerSubscriptionOnSingleServerRestart) {
                HazelcastServer server(*g_srvFactory);

// 2. Start a client
                ClientConfig clientConfig(getConfig());
                clientConfig.setConnectionAttemptLimit(10);

                HazelcastClient client(clientConfig);

// 3. Get a map
                IMap<int, int> map = client.getMap<int, int>("IssueTest_map");

// 4. Subscribe client to entry added event
                map.addEntryListener(listener, true);

// Put a key, value to the map
                ASSERT_EQ((int *) NULL, map.put(1, 10).get());

                ASSERT_TRUE(latch.await(20, 1)); // timeout of 20 seconds

// 5. Verify that the listener got the entry added event
                ASSERT_EQ(1, latch.get());

// 6. Restart the server
                ASSERT_TRUE(server.shutdown());
                HazelcastServer server2(*g_srvFactory);

                std::string putThreadName("Map Put Thread");
                hazelcast::util::StartedThread t(putThreadName, putMapMessage, &map, &latch);

// 8. Verify that the 2nd entry is received by the listener
                ASSERT_TRUE(latch.await(20, 0)); // timeout of 20 seconds

                t.cancel();
                t.join();

// 9. Shut down the server
                ASSERT_TRUE(server2.shutdown());
            }

            TEST_F(IssueTest, testIssue221) {
// start a server
                HazelcastServer server(*g_srvFactory);

// start a client
                HazelcastClient client(getConfig());

                IMap<int, int> map = client.getMap<int, int>("Issue221_test_map");

                server.shutdown();

                ASSERT_THROW(map.get(1), exception::HazelcastClientNotActiveException);
            }

            void IssueTest::Issue864MapListener::entryAdded(const EntryEvent<int, int> &event) {
                int count = latch.get();
                if (2 == count) {
                    // The received event should be the addition of key value: 1, 10
                    ASSERT_EQ(1, event.getKey());
                    ASSERT_EQ(10, event.getValue());
                } else if (1 == count) {
                    // The received event should be the addition of key value: 2, 20
                    ASSERT_EQ(2, event.getKey());
                    ASSERT_EQ(20, event.getValue());
                }

                latch.countDown();
            }

            void IssueTest::Issue864MapListener::entryUpdated(const EntryEvent<int, int> &event) {
                ASSERT_EQ(2, event.getKey());
                ASSERT_EQ(20, event.getValue());
                latch.countDown();
            }

            IssueTest::Issue864MapListener::Issue864MapListener(hazelcast::util::CountDownLatch &l) : latch(l) {

            }
        }
    }
}

//
//  hazelcastInstance.cpp
//  hazelcast
//
//  Created by Sancar on 14.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//



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
                    throw (client::exception::ExceptionBuilder<client::exception::IllegalStateException>(
                            "HazelcastServer::start") << "Could not start logger " << logger->getInstanceName()).build();
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

            bool HazelcastServer::setAttributes(int memberStartOrder) {
                if (!isStarted) {
                    return false;
                }
                return factory.setAttributes(memberStartOrder);
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
                logger.reset(new hazelcast::util::ILogger("Test", testName, "TestVersion", config::LoggerConfig()));
                if (!logger->start()) {
                    throw (exception::ExceptionBuilder<exception::IllegalStateException>(
                            "ClientTestSupport::ClientTestSupport()") << "Could not start logger "
                                                                      << testInfo->name()).build();
                }
            }

            hazelcast::util::ILogger &ClientTestSupport::getLogger() {
                return *logger;
            }

            const std::string &ClientTestSupport::getTestName() const {
                return testName;
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
