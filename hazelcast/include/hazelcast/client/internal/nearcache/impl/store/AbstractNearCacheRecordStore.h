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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_ABSTRACTNEARCACHERESCORDSTORE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_ABSTRACTNEARCACHERESCORDSTORE_H_

#include <stdint.h>
#include <memory>

#include "hazelcast/client/monitor/impl/NearCacheStatsImpl.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluator.h"
#include "hazelcast/client/internal/eviction/MaxSizeChecker.h"
#include "hazelcast/client/internal/eviction/EvictionChecker.h"
#include "hazelcast/client/internal/eviction/EvictionStrategy.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyType.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluatorProvider.h"
#include "hazelcast/client/internal/eviction/EvictionStrategyProvider.h"
#include "hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h"
#include "hazelcast/client/internal/eviction/EvictionListener.h"
#include "hazelcast/client/internal/nearcache/impl/store/BaseHeapNearCacheRecordStore.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/monitor/NearCacheStats.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace store {
                        template<typename K, typename V, typename KS, typename R, typename NCRM>
                        class AbstractNearCacheRecordStore
                                : public NearCacheRecordStore<KS, V>, public eviction::EvictionListener<KS, R> {
                        public:
                            AbstractNearCacheRecordStore(const config::NearCacheConfig<K, V> &cacheConfig,
                                                         serialization::pimpl::SerializationService &ss)
                                    : nearCacheConfig(cacheConfig),
                                      timeToLiveMillis(cacheConfig.getTimeToLiveSeconds() * MILLI_SECONDS_IN_A_SECOND),
                                      maxIdleMillis(cacheConfig.getMaxIdleSeconds() * MILLI_SECONDS_IN_A_SECOND),
                                      serializationService(ss) {
                                const boost::shared_ptr<config::EvictionConfig<K, V> > &evictionConfig = cacheConfig.getEvictionConfig();
                                if (NULL != evictionConfig.get()) {
                                    evictionPolicyType = evictionConfig->getEvictionPolicyType();
                                }
                            }

                            //@override
                            void initialize() {
                                const boost::shared_ptr<config::EvictionConfig<K, V> > &evictionConfig = nearCacheConfig.getEvictionConfig();
                                this->records = createNearCacheRecordMap(nearCacheConfig);
                                this->maxSizeChecker = createNearCacheMaxSizeChecker(evictionConfig, nearCacheConfig);
                                this->evictionPolicyEvaluator = createEvictionPolicyEvaluator(evictionConfig);
                                this->evictionChecker = createEvictionChecker(nearCacheConfig);
                                this->evictionStrategy = createEvictionStrategy(evictionConfig);
                                this->evictionPolicyType = evictionConfig->getEvictionPolicyType();
                            }

/*
                        public void setStaleReadDetector(StaleReadDetector staleReadDetector) {
                                this.staleReadDetector = staleReadDetector;
                            }

                        public StaleReadDetector getStaleReadDetector() {
                                return staleReadDetector;
                            }
*/

                            // public for tests.
                            virtual const boost::shared_ptr<R> getRecord(const boost::shared_ptr<KS> &key) {
                                assert(0);
                                return boost::shared_ptr<R>();
                            }

                            //@Override
                            virtual void onEvict(const boost::shared_ptr<KS> &key, const boost::shared_ptr<R> &record,
                                                 bool wasExpired) {
                                if (wasExpired) {
                                    nearCacheStats.incrementExpirations();
                                } else {
                                    nearCacheStats.incrementEvictions();
                                }
                                nearCacheStats.decrementOwnedEntryCount();
                            }

                            //@Override
                            boost::shared_ptr<V> get(const boost::shared_ptr<KS> &key) {
                                checkAvailable();

                                boost::shared_ptr<R> record;
                                boost::shared_ptr<V> value;
                                try {
                                    record = getRecord(key);
                                    if (record.get() != NULL) {
                                        if (isRecordExpired(record)) {
                                            remove(key);
                                            onExpire(key, record);
                                            return boost::shared_ptr<V>();
                                        }
                                        onRecordAccess(record);
                                        nearCacheStats.incrementHits();
                                        value = recordToValue(record.get());
                                        onGet(key, value, record);
                                        return value;
                                    } else {
                                        nearCacheStats.incrementMisses();
                                        return boost::shared_ptr<V>();
                                    }
                                } catch (exception::IException &error) {
                                    onGetError(key, value, record, error);
                                    throw;
                                }
                            }


                            //@Override
                            void put(const boost::shared_ptr<KS> &key, const boost::shared_ptr<V> &value) {
                                putInternal<V>(key, value);
                            }

                            //@Override
                            void put(const boost::shared_ptr<KS> &key,
                                     const boost::shared_ptr<serialization::pimpl::Data> &value) {
                                putInternal<serialization::pimpl::Data>(key, value);
                            }

                            //@Override
                            bool remove(const boost::shared_ptr<KS> &key) {
                                checkAvailable();

                                boost::shared_ptr<R> record;
                                bool removed = false;
                                try {
                                    record = removeRecord(key);
                                    if (record.get() != NULL) {
                                        removed = true;
                                        nearCacheStats.decrementOwnedEntryCount();
                                    }
                                    onRemove(key, record, removed);
                                    return record.get() != NULL;
                                } catch (exception::IException &error) {
                                    onRemoveError(key, record, removed, error);
                                    throw;
                                }
                            }

                            //@Override
                            void clear() {
                                checkAvailable();

                                clearRecords();
                                nearCacheStats.setOwnedEntryCount(0);
                                nearCacheStats.setOwnedEntryMemoryCost(0L);
                            }

                            //@Override
                            void destroy() {
                                checkAvailable();

                                destroyStore();
                                nearCacheStats.setOwnedEntryCount(0);
                                nearCacheStats.setOwnedEntryMemoryCost(0L);
                            }

                            //@Override
                            int size() const {
                                checkAvailable();

                                return (int) records->size();
                            }

                            //@Override
                            virtual monitor::NearCacheStats &getNearCacheStats() {
                                checkAvailable();

                                return nearCacheStats;
                            }

                            //@Override
                            void doEvictionIfRequired() {
                                checkAvailable();

                                if (isEvictionEnabled()) {
                                    evictionStrategy->evict(records.get(), evictionPolicyEvaluator.get(),
                                                            evictionChecker.get(), this);
                                }
                            }

                            //@Override
                            void doEviction() {
                                checkAvailable();

                                if (isEvictionEnabled()) {
                                    evictionStrategy->evict(records.get(), evictionPolicyEvaluator.get(), NULL, this);
                                }
                            }
                        protected:
                            virtual std::auto_ptr<eviction::MaxSizeChecker> createNearCacheMaxSizeChecker(
                                    const boost::shared_ptr<config::EvictionConfig<K, V> > &evictionConfig,
                                    const config::NearCacheConfig<K, V> &nearCacheConfig) {
                                assert(0);
                                return std::auto_ptr<eviction::MaxSizeChecker>();
                            }

                            virtual std::auto_ptr<NCRM> createNearCacheRecordMap(
                                    const config::NearCacheConfig<K, V> &nearCacheConfig) {
                                assert(0);
                                return std::auto_ptr<NCRM>();
                            }

                            virtual int64_t getKeyStorageMemoryCost(KS *key) const = 0;

                            virtual int64_t getRecordStorageMemoryCost(R *record) const = 0;

                            virtual std::auto_ptr<R> valueToRecord(const boost::shared_ptr<V> &value) {
                                assert(0);
                                return std::auto_ptr<R>();
                            }

                            virtual std::auto_ptr<R> valueToRecord(
                                    const boost::shared_ptr<serialization::pimpl::Data> &value) {
                                assert(0);
                                return std::auto_ptr<R>();
                            }

                            virtual boost::shared_ptr<V> recordToValue(const R *record) {
                                assert(0);
                                return boost::shared_ptr<V>();
                            }

                            virtual boost::shared_ptr<R> putRecord(const boost::shared_ptr<KS> &key,
                                                                   const boost::shared_ptr<R> &record) {
                                assert(0);
                                return boost::shared_ptr<R>();
                            }

                            virtual void putToRecord(boost::shared_ptr<R> &record,
                                                     const boost::shared_ptr<V> &value) {
                                assert(0);
                            }

                            virtual boost::shared_ptr<R> removeRecord(const boost::shared_ptr<KS> &key) {
                                assert(0);
                                return boost::shared_ptr<R>();
                            }

                            virtual bool containsRecordKey(const boost::shared_ptr<KS> &key) const {
                                assert(0);
                                return false;
                            }

                            void checkAvailable() const {
                                if (!isAvailable()) {
                                    throw exception::IllegalStateException(nearCacheConfig.getName() +
                                                                           " named Near Cache record store is not available");
                                }
                            }

                            std::auto_ptr<eviction::EvictionPolicyEvaluator<K, V, KS, R> > createEvictionPolicyEvaluator(
                                    const boost::shared_ptr<config::EvictionConfig<K, V> > &evictionConfig) {
                                return eviction::EvictionPolicyEvaluatorProvider::getEvictionPolicyEvaluator<K, V, KS, R>(
                                        evictionConfig);
                            }

                            boost::shared_ptr<eviction::EvictionStrategy<K, V, KS, R, NCRM> > createEvictionStrategy(
                                    const boost::shared_ptr<config::EvictionConfig<K, V> > &evictionConfig) {
                                return eviction::EvictionStrategyProvider<K, V, KS, R, NCRM>::getEvictionStrategy(
                                        evictionConfig);
                            }

                            std::auto_ptr<eviction::EvictionChecker> createEvictionChecker(
                                    const config::NearCacheConfig<K, V> &nearCacheConfig) {
                                return std::auto_ptr<eviction::EvictionChecker>(
                                        new MaxSizeEvictionChecker(maxSizeChecker.get()));
                            }

                            bool isAvailable() const {
                                return records.get() != NULL;
                            }

                            boost::shared_ptr<serialization::pimpl::Data> valueToData(
                                    const boost::shared_ptr<V> &value) {
                                if (value.get() != NULL) {
                                    return boost::shared_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(
                                            serializationService.toData<V>(value.get())));
                                } else {
                                    return boost::shared_ptr<serialization::pimpl::Data>();
                                }
                            }

                            boost::shared_ptr<serialization::pimpl::Data> valueToData(
                                    boost::shared_ptr<serialization::pimpl::Data> &value) {
                                return value;
                            }

                            boost::shared_ptr<V> dataToValue(
                                    const boost::shared_ptr<serialization::pimpl::Data> &data) {
                                if (data.get() != NULL) {
                                    std::auto_ptr<V> value = serializationService.toObject<V>(data.get());
                                    return boost::shared_ptr<V>(value);
                                } else {
                                    return boost::shared_ptr<V>();
                                }
                            }

                            const boost::shared_ptr<serialization::pimpl::Data> toData(
                                    const boost::shared_ptr<serialization::pimpl::Data> &obj) {
                                return obj;
                            }

                            const boost::shared_ptr<serialization::pimpl::Data> toData(
                                    const boost::shared_ptr<V> &obj) {
                                if (obj.get() == NULL) {
                                    return boost::shared_ptr<serialization::pimpl::Data>();
                                } else {
                                    return valueToData(obj);
                                }
                            }

                            boost::shared_ptr<V> toValue(boost::shared_ptr<serialization::pimpl::Data> &obj) {
                                if (obj.get() == NULL) {
                                    return boost::shared_ptr<V>();
                                } else {
                                    return dataToValue(obj);
                                }
                            }

                            boost::shared_ptr<V> toValue(boost::shared_ptr<V> &obj) {
                                return obj;
                            }

                            int64_t getTotalStorageMemoryCost(const boost::shared_ptr<KS> &key,
                                                              const boost::shared_ptr<R> &record) {
                                return getKeyStorageMemoryCost(key.get()) + getRecordStorageMemoryCost(record.get());
                            }

                            bool isRecordExpired(const boost::shared_ptr<R> &record) const {
                                int64_t now = util::currentTimeMillis();
                                if (record->isExpiredAt(now)) {
                                    return true;
                                } else {
                                    return record->isIdleAt(maxIdleMillis, now);
                                }
                            }

                            void onRecordCreate(const boost::shared_ptr<KS> &key, const boost::shared_ptr<R> &record) {
                                record->setCreationTime(util::currentTimeMillis());
                            }

                            void onRecordAccess(const boost::shared_ptr<R> &record) {
                                record->setAccessTime(util::currentTimeMillis());
                                record->incrementAccessHit();
                            }

                            void onGet(const boost::shared_ptr<KS> &key, const boost::shared_ptr<V> &value,
                                       const boost::shared_ptr<R> &record) {
                            }

                            void onGetError(const boost::shared_ptr<KS> &key, const boost::shared_ptr<V> &value,
                                            const boost::shared_ptr<R> &record, const exception::IException &error) {
                            }

                            void onPut(const boost::shared_ptr<KS> &key, const boost::shared_ptr<V> &value,
                                       const boost::shared_ptr<R> &record, const boost::shared_ptr<R> &oldRecord) {
                            }

                            void onPut(const boost::shared_ptr<KS> &key,
                                       const boost::shared_ptr<serialization::pimpl::Data> &value,
                                       const boost::shared_ptr<R> &record, const boost::shared_ptr<R> &oldRecord) {
                            }

                            void onPutError(const boost::shared_ptr<KS> &key, const boost::shared_ptr<V> &value,
                                            const boost::shared_ptr<R> &record, const boost::shared_ptr<R> &oldRecord,
                                            const exception::IException &error) {
                            }

                            void onPutError(const boost::shared_ptr<KS> &key,
                                            const boost::shared_ptr<serialization::pimpl::Data> &value,
                                            const boost::shared_ptr<R> &record, const boost::shared_ptr<R> &oldRecord,
                                            const exception::IException &error) {
                            }

                            void onRemove(const boost::shared_ptr<KS> &key, const boost::shared_ptr<R> &record,
                                          bool removed) {
                            }

                            void onRemoveError(const boost::shared_ptr<KS> &key, const boost::shared_ptr<R> &record,
                                               bool removed, const exception::IException &error) {
                            }

                            void onExpire(const boost::shared_ptr<KS> &key, const boost::shared_ptr<R> &record) {
                                nearCacheStats.incrementExpirations();
                            }

                            bool isEvictionEnabled() {
                                return evictionStrategy.get() != NULL
                                       && evictionPolicyEvaluator.get() != NULL
                                       && evictionPolicyType != eviction::NONE;
                            }

                            void clearRecords() {
                                records->clear();
                            }

                            void destroyStore() {
                                clearRecords();
                            }

                            /*
                        static const int REFERENCE_SIZE = MEM_AVAILABLE ? MEM.arrayIndexScale(Object[].class) : (Integer.SIZE / Byte.SIZE);
*/
                            const config::NearCacheConfig<K, V> &nearCacheConfig;
                            const int64_t timeToLiveMillis;
                            const int64_t maxIdleMillis;
                            serialization::pimpl::SerializationService &serializationService;
                            monitor::impl::NearCacheStatsImpl nearCacheStats;

                            std::auto_ptr<eviction::MaxSizeChecker> maxSizeChecker;
                            std::auto_ptr<eviction::EvictionPolicyEvaluator<K, V, KS, R> > evictionPolicyEvaluator;
                            std::auto_ptr<eviction::EvictionChecker> evictionChecker;
                            boost::shared_ptr<eviction::EvictionStrategy<K, V, KS, R, NCRM> > evictionStrategy;
                            eviction::EvictionPolicyType evictionPolicyType;
                            std::auto_ptr<NCRM> records;

/*
                            volatile StaleReadDetector staleReadDetector = ALWAYS_FRESH;
*/
                        private:
                            class MaxSizeEvictionChecker : public eviction::EvictionChecker {
                            public:

                                MaxSizeEvictionChecker(const eviction::MaxSizeChecker *maxSizeChecker) : maxSizeChecker(
                                        maxSizeChecker) { }

                                //@Override
                                bool isEvictionRequired() const {
                                    return maxSizeChecker != NULL && maxSizeChecker->isReachedToMaxSize();
                                }

                            private:
                                const eviction::MaxSizeChecker *maxSizeChecker;
                            };

                            template<typename VALUE>
                            void putInternal(const boost::shared_ptr<KS> &key, const boost::shared_ptr<VALUE> &value) {
                                checkAvailable();

                                // if there is no eviction configured we return if the Near Cache is full and it's a new key
                                // (we have to check the key, otherwise we might lose updates on existing keys)
                                if (!isEvictionEnabled() && evictionChecker->isEvictionRequired() &&
                                    !containsRecordKey(key)) {
                                    return;
                                }

                                boost::shared_ptr<R> record;
                                boost::shared_ptr<R> oldRecord;
                                try {
                                    record = valueToRecord(value);
                                    onRecordCreate(key, record);
                                    oldRecord = putRecord(key, record);
                                    if (oldRecord.get() == NULL) {
                                        nearCacheStats.incrementOwnedEntryCount();
                                    } else {
                                        int64_t oldRecordMemoryCost = getTotalStorageMemoryCost(key, oldRecord);
                                        nearCacheStats.decrementOwnedEntryMemoryCost(oldRecordMemoryCost);
                                    }
                                    onPut(key, value, record, oldRecord);
                                } catch (exception::IException &error) {
                                    onPutError(key, value, record, oldRecord, error);
                                    throw;
                                }
                            }

                            static const int MILLI_SECONDS_IN_A_SECOND = 1000;
                        };
                    }
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_ABSTRACTNEARCACHERESCORDSTORE_H_ */

