/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
// Created by İhsan Demir on Jan 10 2017.
//

#include <hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include "hazelcast/client/ClientConfig.h"

#include <TestHelperFunctions.h>
#include "ClientTestSupport.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace internal {
                namespace nearcache {
                    class NearCacheRecordStoreTest
                            : public ClientTestSupport, public ::testing::WithParamInterface<config::InMemoryFormat> {
                    public:
                        NearCacheRecordStoreTest() {
                            ss = std::auto_ptr<serialization::pimpl::SerializationService>(
                                    new serialization::pimpl::SerializationService(serializationConfig));
                        }

                    protected:
                        static const int DEFAULT_RECORD_COUNT;
                        static const char *DEFAULT_NEAR_CACHE_NAME;

                        void putAndGetRecord(config::InMemoryFormat inMemoryFormat) {
                            config::NearCacheConfig<int, std::string> nearCacheConfig = createNearCacheConfig<int, std::string>(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, std::string> >
                                    nearCacheRecordStore = createNearCacheRecordStore<int, std::string, serialization::pimpl::Data>(
                                    nearCacheConfig,
                                    inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                            }

                            ASSERT_EQ(DEFAULT_RECORD_COUNT, nearCacheRecordStore->size());

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                boost::shared_ptr<std::string> value = nearCacheRecordStore->get(getSharedKey(i));
                                ASSERT_NOTNULL(value.get(), std::string);
                                ASSERT_EQ(*getSharedValue(i), *value);
                            }
                        }

                        void putAndRemoveRecord(config::InMemoryFormat inMemoryFormat) {
                            config::NearCacheConfig<int, std::string> nearCacheConfig = createNearCacheConfig<int, std::string>(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, std::string> >
                                    nearCacheRecordStore = createNearCacheRecordStore<int, std::string, serialization::pimpl::Data>(
                                    nearCacheConfig,
                                    inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                boost::shared_ptr<serialization::pimpl::Data> key = getSharedKey(i);
                                nearCacheRecordStore->put(key, getSharedValue(i));

                                // ensure that they are stored
                                ASSERT_NOTNULL(nearCacheRecordStore->get(key).get(), std::string);
                            }

                            ASSERT_EQ(DEFAULT_RECORD_COUNT, nearCacheRecordStore->size());

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                boost::shared_ptr<serialization::pimpl::Data> key = getSharedKey(i);
                                ASSERT_TRUE(nearCacheRecordStore->invalidate(key));
                                ASSERT_NULL("Should not exist", nearCacheRecordStore->get(key).get(), std::string);
                            }

                            ASSERT_EQ(0, nearCacheRecordStore->size());
                        }

                        void clearRecordsOrDestroyStore(config::InMemoryFormat inMemoryFormat, bool destroy) {
                            config::NearCacheConfig<int, std::string> nearCacheConfig = createNearCacheConfig<int, std::string>(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, std::string> >
                                    nearCacheRecordStore = createNearCacheRecordStore<int, std::string, serialization::pimpl::Data>(
                                    nearCacheConfig,
                                    inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                boost::shared_ptr<serialization::pimpl::Data> key = getSharedKey(i);
                                nearCacheRecordStore->put(key, getSharedValue(i));

                                // ensure that they are stored
                                ASSERT_NOTNULL(nearCacheRecordStore->get(key).get(), std::string);
                            }

                            if (destroy) {
                                nearCacheRecordStore->destroy();
                            } else {
                                nearCacheRecordStore->clear();
                            }

                            ASSERT_EQ(0, nearCacheRecordStore->size());
                        }

                        void statsCalculated(config::InMemoryFormat inMemoryFormat) {
                            int64_t creationStartTime = util::currentTimeMillis();
                            config::NearCacheConfig<int, std::string> nearCacheConfig = createNearCacheConfig<int, std::string>(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, std::string> >
                                    nearCacheRecordStore = createNearCacheRecordStore<int, std::string, serialization::pimpl::Data>(
                                    nearCacheConfig,
                                    inMemoryFormat);
                            int64_t creationEndTime = util::currentTimeMillis();

                            int64_t expectedEntryCount = 0;
                            int64_t expectedHits = 0;
                            int64_t expectedMisses = 0;

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));

                                expectedEntryCount++;
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                int selectedKey = i * 3;
                                if (nearCacheRecordStore->get(getSharedKey(selectedKey)) != NULL) {
                                    expectedHits++;
                                } else {
                                    expectedMisses++;
                                }
                            }

                            monitor::NearCacheStats &nearCacheStats = nearCacheRecordStore->getNearCacheStats();

                            int64_t memoryCostWhenFull = nearCacheStats.getOwnedEntryMemoryCost();
                            ASSERT_TRUE(nearCacheStats.getCreationTime() >= creationStartTime);
                            ASSERT_TRUE(nearCacheStats.getCreationTime() <= creationEndTime);
                            ASSERT_EQ(expectedHits, nearCacheStats.getHits());
                            ASSERT_EQ(expectedMisses, nearCacheStats.getMisses());
                            ASSERT_EQ(expectedEntryCount, nearCacheStats.getOwnedEntryCount());
                            switch (inMemoryFormat) {
                                case config::BINARY:
                                    ASSERT_TRUE(memoryCostWhenFull > 0);
                                    break;
                                case config::OBJECT:
                                    ASSERT_EQ(0, memoryCostWhenFull);
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                int selectedKey = i * 3;
                                if (nearCacheRecordStore->invalidate(getSharedKey(selectedKey))) {
                                    expectedEntryCount--;
                                }
                            }

                            ASSERT_EQ(expectedEntryCount, nearCacheStats.getOwnedEntryCount());
                            switch (inMemoryFormat) {
                                case config::BINARY:
                                    ASSERT_TRUE(nearCacheStats.getOwnedEntryMemoryCost() > 0);
                                    ASSERT_TRUE(nearCacheStats.getOwnedEntryMemoryCost() < memoryCostWhenFull);
                                    break;
                                case config::OBJECT:
                                    ASSERT_EQ(0, nearCacheStats.getOwnedEntryMemoryCost());
                                    break;
                            }

                            nearCacheRecordStore->clear();

                            switch (inMemoryFormat) {
                                case config::BINARY:
                                case config::OBJECT:
                                    ASSERT_EQ(0, nearCacheStats.getOwnedEntryMemoryCost());
                                    break;
                            }
                        }

                        void ttlEvaluated(config::InMemoryFormat inMemoryFormat) {
                            int ttlSeconds = 3;

                            config::NearCacheConfig<int, std::string> nearCacheConfig = createNearCacheConfig<int, std::string>(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            nearCacheConfig.setTimeToLiveSeconds(ttlSeconds);

                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, std::string> >
                                    nearCacheRecordStore = createNearCacheRecordStore<int, std::string, serialization::pimpl::Data>(
                                    nearCacheConfig,
                                    inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_NOTNULL(nearCacheRecordStore->get(getSharedKey(i)).get(), std::string);
                            }

                            util::sleep(ttlSeconds + 1);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_NULL("", nearCacheRecordStore->get(getSharedKey(i)).get(), std::string);
                            }
                        }

                        void maxIdleTimeEvaluatedSuccessfully(config::InMemoryFormat inMemoryFormat) {
                            int maxIdleSeconds = 3;

                            config::NearCacheConfig<int, std::string> nearCacheConfig = createNearCacheConfig<int, std::string>(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            nearCacheConfig.setMaxIdleSeconds(maxIdleSeconds);

                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, std::string> >
                                    nearCacheRecordStore = createNearCacheRecordStore<int, std::string, serialization::pimpl::Data>(
                                    nearCacheConfig,
                                    inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_NOTNULL(nearCacheRecordStore->get(getSharedKey(i)).get(), std::string);
                            }

                            util::sleep(maxIdleSeconds + 1);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_NULL("", nearCacheRecordStore->get(getSharedKey(i)).get(), std::string);
                            }
                        }

                        void expiredRecordsCleanedUpSuccessfully(config::InMemoryFormat inMemoryFormat,
                                                                 bool useIdleTime) {
                            int cleanUpThresholdSeconds = 3;

                            config::NearCacheConfig<int, std::string> nearCacheConfig = createNearCacheConfig<int, std::string>(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            if (useIdleTime) {
                                nearCacheConfig.setMaxIdleSeconds(cleanUpThresholdSeconds);
                            } else {
                                nearCacheConfig.setTimeToLiveSeconds(cleanUpThresholdSeconds);
                            }

                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, std::string> >
                                    nearCacheRecordStore = createNearCacheRecordStore<int, std::string, serialization::pimpl::Data>(
                                    nearCacheConfig,
                                    inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                            }

                            util::sleep(cleanUpThresholdSeconds + 1);

                            nearCacheRecordStore->doExpiration();

                            ASSERT_EQ(0, nearCacheRecordStore->size());

                            monitor::NearCacheStats &nearCacheStats = nearCacheRecordStore->getNearCacheStats();
                            ASSERT_EQ(0, nearCacheStats.getOwnedEntryCount());
                            ASSERT_EQ(0, nearCacheStats.getOwnedEntryMemoryCost());
                        }

                        void createNearCacheWithMaxSizePolicy(config::InMemoryFormat inMemoryFormat,
                                                              config::EvictionConfig<int, std::string>::MaxSizePolicy maxSizePolicy,
                                                              int32_t size) {
                            config::NearCacheConfig<int, std::string> nearCacheConfig = createNearCacheConfig<int, std::string>(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            boost::shared_ptr<config::EvictionConfig<int, std::string> > evictionConfig(
                                    new config::EvictionConfig<int, std::string>());
                            evictionConfig->setMaximumSizePolicy(maxSizePolicy);
                            evictionConfig->setSize(size);
                            nearCacheConfig.setEvictionConfig(evictionConfig);

                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, std::string> >
                                    nearCacheRecordStore = createNearCacheRecordStore<int, std::string, serialization::pimpl::Data>(
                                    nearCacheConfig,
                                    inMemoryFormat);
                        }

                        void doEvictionWithEntryCountMaxSizePolicy(config::InMemoryFormat inMemoryFormat,
                                                                   config::EvictionPolicy evictionPolicy) {
                            int32_t maxSize = DEFAULT_RECORD_COUNT / 2;

                            config::NearCacheConfig<int, std::string> nearCacheConfig = createNearCacheConfig<int, std::string>(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);


                            boost::shared_ptr<config::EvictionConfig<int, std::string> > evictionConfig(
                                    new config::EvictionConfig<int, std::string>());

                            evictionConfig->setMaximumSizePolicy(config::EvictionConfig<int, std::string>::ENTRY_COUNT);
                            evictionConfig->setSize(maxSize);
                            evictionConfig->setEvictionPolicy(evictionPolicy);
                            nearCacheConfig.setEvictionConfig(evictionConfig);

                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, std::string> >
                                    nearCacheRecordStore = createNearCacheRecordStore<int, std::string, serialization::pimpl::Data>(
                                    nearCacheConfig,
                                    inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                                nearCacheRecordStore->doEvictionIfRequired();
                                ASSERT_TRUE(maxSize >= nearCacheRecordStore->size());
                            }
                        }

                        template<typename K, typename V, typename KS>
                        std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<KS, V> > createNearCacheRecordStore(
                                config::NearCacheConfig<K, V> &nearCacheConfig,
                                config::InMemoryFormat inMemoryFormat) {
                            std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<KS, V> > recordStore;
                            switch (inMemoryFormat) {
                                case config::BINARY:
                                    recordStore = std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<KS, V> >(
                                            new client::internal::nearcache::impl::store::NearCacheDataRecordStore<K, V, KS>(
                                                    DEFAULT_NEAR_CACHE_NAME, nearCacheConfig, *ss));
                                    break;
                                case config::OBJECT:
                                    recordStore = std::auto_ptr<client::internal::nearcache::impl::NearCacheRecordStore<KS, V> >(
                                            new client::internal::nearcache::impl::store::NearCacheObjectRecordStore<K, V, KS>(
                                                    DEFAULT_NEAR_CACHE_NAME,
                                                    nearCacheConfig, *ss));
                                    break;
                                default:
                                    std::ostringstream out;
                                    out << "Unsupported in-memory format: " << inMemoryFormat;
                                    throw exception::IllegalArgumentException("NearCacheRecordStoreTest", out.str());
                            }
                            recordStore->initialize();

                            return recordStore;
                        }

                        template<typename K, typename V>
                        config::NearCacheConfig<K, V> createNearCacheConfig(const char *name,
                                                                            config::InMemoryFormat inMemoryFormat) {
                            config::NearCacheConfig<K, V> config;
                            config.setName(name).setInMemoryFormat(inMemoryFormat);
                            return config;
                        }

                        boost::shared_ptr<std::string> getSharedValue(int value) const {
                            char buf[30];
                            util::hz_snprintf(buf, 30, "Record-%ld", value);
                            return boost::shared_ptr<std::string>(new std::string(buf));
                        }

                        boost::shared_ptr<serialization::pimpl::Data> getSharedKey(int value) {
                            return ss->toSharedData<int>(&value);
                        }

                        std::auto_ptr<serialization::pimpl::SerializationService> ss;
                        SerializationConfig serializationConfig;
                    };

                    const int NearCacheRecordStoreTest::DEFAULT_RECORD_COUNT = 100;
                    const char *NearCacheRecordStoreTest::DEFAULT_NEAR_CACHE_NAME = "TestNearCache";

                    TEST_P(NearCacheRecordStoreTest, putAndGetRecord) {
                        putAndGetRecord(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, putAndRemoveRecord) {
                        putAndRemoveRecord(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, clearRecords) {
                        clearRecordsOrDestroyStore(GetParam(), false);
                    }

                    TEST_P(NearCacheRecordStoreTest, destroyStore) {
                        clearRecordsOrDestroyStore(GetParam(), true);
                    }

                    TEST_P(NearCacheRecordStoreTest, statsCalculated) {
                        statsCalculated(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, ttlEvaluated) {
                        ttlEvaluated(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, maxIdleTimeEvaluatedSuccessfully) {
                        maxIdleTimeEvaluatedSuccessfully(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, expiredRecordsCleanedUpSuccessfullyBecauseOfTTL) {
                        expiredRecordsCleanedUpSuccessfully(GetParam(), false);
                    }

                    TEST_P(NearCacheRecordStoreTest, expiredRecordsCleanedUpSuccessfullyBecauseOfIdleTime) {
                        expiredRecordsCleanedUpSuccessfully(GetParam(), true);
                    }

                    TEST_P(NearCacheRecordStoreTest, canCreateWithEntryCountMaxSizePolicy) {
                        createNearCacheWithMaxSizePolicy(GetParam(),
                                                         config::EvictionConfig<int, std::string>::ENTRY_COUNT,
                                                         1000);
                    }

                    TEST_P(NearCacheRecordStoreTest,
                           evictionTriggeredAndHandledSuccessfullyWithEntryCountMaxSizePolicyAndLRUEvictionPolicy) {
                        doEvictionWithEntryCountMaxSizePolicy(GetParam(), config::LRU);
                    }

                    TEST_P(NearCacheRecordStoreTest,
                           evictionTriggeredAndHandledSuccessfullyWithEntryCountMaxSizePolicyAndLFUEvictionPolicy) {
                        doEvictionWithEntryCountMaxSizePolicy(GetParam(), config::LFU);
                    }

                    TEST_P(NearCacheRecordStoreTest,
                           evictionTriggeredAndHandledSuccessfullyWithEntryCountMaxSizePolicyAndRandomEvictionPolicy) {
                        doEvictionWithEntryCountMaxSizePolicy(GetParam(), config::RANDOM);
                    }

                    INSTANTIATE_TEST_CASE_P(BasicStoreTest, NearCacheRecordStoreTest,
                                            ::testing::Values(config::BINARY, config::OBJECT));

                }
            }
        }
    }
}
