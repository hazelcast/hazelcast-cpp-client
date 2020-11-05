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
#pragma once

#include <stdint.h>
#include <memory>
#include <cassert>

#include "hazelcast/client/monitor/impl/NearCacheStatsImpl.h"
#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluator.h"
#include "hazelcast/client/internal/eviction/MaxSizeChecker.h"
#include "hazelcast/client/internal/eviction/EvictionChecker.h"
#include "hazelcast/client/internal/eviction/EvictionStrategy.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyType.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluatorProvider.h"
#include "hazelcast/client/internal/eviction/EvictionStrategyProvider.h"
#include "hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h"
#include "hazelcast/client/internal/eviction/EvictionListener.h"
#include "hazelcast/client/internal/nearcache/impl/store/BaseHeapNearCacheRecordStore.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

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
                            AbstractNearCacheRecordStore(const client::config::NearCacheConfig &cacheConfig,
                                                         serialization::pimpl::SerializationService &ss)
                                    : nearCacheConfig_(cacheConfig),
                                      timeToLiveMillis_(cacheConfig.getTimeToLiveSeconds() * MILLI_SECONDS_IN_A_SECOND),
                                      maxIdleMillis_(cacheConfig.getMaxIdleSeconds() * MILLI_SECONDS_IN_A_SECOND),
                                      serializationService_(ss), nearCacheStats_(new monitor::impl::NearCacheStatsImpl) {
                                auto &evictionConfig = const_cast<client::config::NearCacheConfig &>(cacheConfig).getEvictionConfig();
                                evictionPolicyType_ = evictionConfig.getEvictionPolicyType();
                            }

                            void initialize() override {
                                auto &evictionConfig = const_cast<client::config::NearCacheConfig &>(nearCacheConfig_).getEvictionConfig();
                                this->records_ = createNearCacheRecordMap(nearCacheConfig_);
                                this->maxSizeChecker_ = createNearCacheMaxSizeChecker(evictionConfig, nearCacheConfig_);
                                this->evictionPolicyEvaluator_ = createEvictionPolicyEvaluator(evictionConfig);
                                this->evictionChecker_ = createEvictionChecker(nearCacheConfig_);
                                this->evictionStrategy_ = createEvictionStrategy(evictionConfig);
                                this->evictionPolicyType_ = evictionConfig.getEvictionPolicyType();
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
                            virtual const std::shared_ptr<R> getRecord(const std::shared_ptr<KS> &key) {
                                assert(0);
                                return std::shared_ptr<R>();
                            }

                            void onEvict(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record,
                                                 bool wasExpired) override {
                                if (wasExpired) {
                                    nearCacheStats_->incrementExpirations();
                                } else {
                                    nearCacheStats_->incrementEvictions();
                                }
                                nearCacheStats_->decrementOwnedEntryCount();
                            }

                            std::shared_ptr<V> get(const std::shared_ptr<KS> &key) override {
                                checkAvailable();

                                std::shared_ptr<R> record;
                                std::shared_ptr<V> value;
                                try {
                                    record = getRecord(key);
                                    if (record.get() != NULL) {
                                        if (isRecordExpired(record)) {
                                            invalidate(key);
                                            onExpire(key, record);
                                            return std::shared_ptr<V>();
                                        }
                                        onRecordAccess(record);
                                        nearCacheStats_->incrementHits();
                                        value = recordToValue(record.get());
                                        onGet(key, value, record);
                                        return value;
                                    } else {
                                        nearCacheStats_->incrementMisses();
                                        return std::shared_ptr<V>();
                                    }
                                } catch (exception::IException &error) {
                                    onGetError(key, value, record, error);
                                    throw;
                                }
                            }


                            void put(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value) override {
                                putInternal<V>(key, value);
                            }

/*
                            //@Override
                            void put(const std::shared_ptr<KS> &key,
                                     const std::shared_ptr<serialization::pimpl::Data> &value) {
                                putInternal<serialization::pimpl::Data>(key, value);
                            }
*/

                            bool invalidate(const std::shared_ptr<KS> &key) override {
                                checkAvailable();

                                std::shared_ptr<R> record;
                                bool removed = false;
                                try {
                                    record = removeRecord(key);
                                    if (record.get() != NULL) {
                                        removed = true;
                                        nearCacheStats_->decrementOwnedEntryCount();
                                        nearCacheStats_->decrementOwnedEntryMemoryCost(
                                                getTotalStorageMemoryCost(key.get(), record.get()));
                                        nearCacheStats_->incrementInvalidations();
                                    }
                                    nearCacheStats_->incrementInvalidationRequests();
                                    onRemove(key, record, removed);
                                    return record.get() != NULL;
                                } catch (exception::IException &error) {
                                    onRemoveError(key, record, removed, error);
                                    throw;
                                }
                            }

                            void clear() override {
                                checkAvailable();

                                clearRecords();
                                nearCacheStats_->setOwnedEntryCount(0);
                                nearCacheStats_->setOwnedEntryMemoryCost(0L);
                            }

                            void destroy() override {
                                checkAvailable();

                                destroyStore();
                                nearCacheStats_->setOwnedEntryCount(0);
                                nearCacheStats_->setOwnedEntryMemoryCost(0L);
                            }

                            int size() const override {
                                checkAvailable();
                                return (int) records_->size();
                            }

                            std::shared_ptr<monitor::NearCacheStats> getNearCacheStats() const override {
                                checkAvailable();
                                return nearCacheStats_;
                            }

                            void doEvictionIfRequired() override {
                                checkAvailable();
                                if (isEvictionEnabled()) {
                                    evictionStrategy_->evict(records_.get(), evictionPolicyEvaluator_.get(),
                                                            evictionChecker_.get(), this);
                                }
                            }

                            void doEviction() override {
                                checkAvailable();

                                if (isEvictionEnabled()) {
                                    evictionStrategy_->evict(records_.get(), evictionPolicyEvaluator_.get(), NULL, this);
                                }
                            }
                        protected:
                            virtual std::unique_ptr<eviction::MaxSizeChecker> createNearCacheMaxSizeChecker(
                                    const client::config::EvictionConfig &evictionConfig,
                                    const client::config::NearCacheConfig &) {
                                assert(0);
                                return std::unique_ptr<eviction::MaxSizeChecker>();
                            }

                            virtual std::unique_ptr<NCRM> createNearCacheRecordMap(
                                    const client::config::NearCacheConfig &) {
                                assert(0);
                                return std::unique_ptr<NCRM>();
                            }

                            virtual int64_t getKeyStorageMemoryCost(KS *key) const = 0;

                            virtual int64_t getRecordStorageMemoryCost(R *record) const = 0;

                            int64_t getTotalStorageMemoryCost(KS *key, R *record) const {
                                return getKeyStorageMemoryCost(key) + getRecordStorageMemoryCost(record);
                            }

                            virtual std::unique_ptr<R> valueToRecord(const std::shared_ptr<V> &value) {
                                assert(0);
                                return std::unique_ptr<R>();
                            }

/*
                            virtual std::unique_ptr<R> valueToRecord(
                                    const std::shared_ptr<serialization::pimpl::Data> &value) {
                                assert(0);
                                return std::unique_ptr<R>();
                            }
*/

                            virtual std::shared_ptr<V> recordToValue(const R *record) {
                                assert(0);
                                return std::shared_ptr<V>();
                            }

                            virtual std::shared_ptr<R> putRecord(const std::shared_ptr<KS> &key,
                                                                   const std::shared_ptr<R> &record) {
                                assert(0);
                                return std::shared_ptr<R>();
                            }

                            virtual void putToRecord(std::shared_ptr<R> &record,
                                                     const std::shared_ptr<V> &value) {
                                assert(0);
                            }

                            virtual std::shared_ptr<R> removeRecord(const std::shared_ptr<KS> &key) {
                                assert(0);
                                return std::shared_ptr<R>();
                            }

                            virtual bool containsRecordKey(const std::shared_ptr<KS> &key) const {
                                assert(0);
                                return false;
                            }

                            void checkAvailable() const {
                                if (!isAvailable()) {
                                    BOOST_THROW_EXCEPTION(exception::IllegalStateException(nearCacheConfig_.getName() +
                                                                                           " named Near Cache record store is not available"));
                                }
                            }

                            std::unique_ptr<eviction::EvictionPolicyEvaluator<K, V, KS, R> > createEvictionPolicyEvaluator(
                                    const client::config::EvictionConfig &evictionConfig) {
                                return eviction::EvictionPolicyEvaluatorProvider::getEvictionPolicyEvaluator<K, V, KS, R>(
                                        evictionConfig);
                            }

                            std::shared_ptr<eviction::EvictionStrategy<K, V, KS, R, NCRM> > createEvictionStrategy(
                                    const client::config::EvictionConfig &evictionConfig) {
                                return eviction::EvictionStrategyProvider<K, V, KS, R, NCRM>::getEvictionStrategy(
                                        evictionConfig);
                            }

                            std::unique_ptr<eviction::EvictionChecker> createEvictionChecker(
                                    const client::config::NearCacheConfig &cacheConfig) {
                                return std::unique_ptr<eviction::EvictionChecker>(
                                        new MaxSizeEvictionChecker(maxSizeChecker_.get()));
                            }

                            bool isAvailable() const {
                                return records_.get() != NULL;
                            }

                            std::shared_ptr<serialization::pimpl::Data> valueToData(
                                    const std::shared_ptr<V> &value) {
                                if (value.get() != NULL) {
                                    return std::shared_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(
                                            serializationService_.toData<V>(value.get())));
                                } else {
                                    return std::shared_ptr<serialization::pimpl::Data>();
                                }
                            }

/*
                            std::shared_ptr<serialization::pimpl::Data> valueToData(
                                    std::shared_ptr<serialization::pimpl::Data> &value) {
                                return value;
                            }
*/

                            std::shared_ptr<V> dataToValue(
                                    const std::shared_ptr<serialization::pimpl::Data> &data, const TypedData *dummy) {
                                return std::shared_ptr<V>(new TypedData(*data, serializationService_));
                            }

                            std::shared_ptr<V> dataToValue(
                                    const std::shared_ptr<serialization::pimpl::Data> &data, void *dummy) {
                                return data;
/*
                                if (data.get() != NULL) {
                                    auto value = serializationService.toObject<V>(data.get());
                                    return std::shared_ptr<V>(new V(std::move(value).value()));
                                } else {
                                    return std::shared_ptr<V>();
                                }
*/
                            }

/*
                            const std::shared_ptr<serialization::pimpl::Data> toData(
                                    const std::shared_ptr<serialization::pimpl::Data> &obj) {
                                return obj;
                            }
*/

                            const std::shared_ptr<serialization::pimpl::Data> toData(
                                    const std::shared_ptr<V> &obj) {
                                if (obj.get() == NULL) {
                                    return std::shared_ptr<serialization::pimpl::Data>();
                                } else {
                                    return valueToData(obj);
                                }
                            }

/*
                            std::shared_ptr<V> toValue(std::shared_ptr<serialization::pimpl::Data> &obj) {
                                if (obj.get() == NULL) {
                                    return std::shared_ptr<V>();
                                } else {
                                    return dataToValue(obj, (V *)NULL);
                                }
                            }
*/

                            std::shared_ptr<V> toValue(std::shared_ptr<V> &obj) {
                                return obj;
                            }

                            int64_t getTotalStorageMemoryCost(const std::shared_ptr<KS> &key,
                                                              const std::shared_ptr<R> &record) {
                                return getKeyStorageMemoryCost(key.get()) + getRecordStorageMemoryCost(record.get());
                            }

                            bool isRecordExpired(const std::shared_ptr<R> &record) const {
                                int64_t now = util::currentTimeMillis();
                                if (record->isExpiredAt(now)) {
                                    return true;
                                } else {
                                    return record->isIdleAt(maxIdleMillis_, now);
                                }
                            }

                            void onRecordCreate(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record) {
                                record->setCreationTime(util::currentTimeMillis());
                            }

                            void onRecordAccess(const std::shared_ptr<R> &record) {
                                record->setAccessTime(util::currentTimeMillis());
                                record->incrementAccessHit();
                            }

                            void onGet(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value,
                                       const std::shared_ptr<R> &record) {
                            }

                            void onGetError(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value,
                                            const std::shared_ptr<R> &record, const exception::IException &error) {
                            }

                            void onPut(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value,
                                       const std::shared_ptr<R> &record, const std::shared_ptr<R> &oldRecord) {
                            }

/*
                            void onPut(const std::shared_ptr<KS> &key,
                                       const std::shared_ptr<serialization::pimpl::Data> &value,
                                       const std::shared_ptr<R> &record, const std::shared_ptr<R> &oldRecord) {
                            }
*/

                            void onPutError(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value,
                                            const std::shared_ptr<R> &record, const std::shared_ptr<R> &oldRecord,
                                            const exception::IException &error) {
                            }

/*
                            void onPutError(const std::shared_ptr<KS> &key,
                                            const std::shared_ptr<serialization::pimpl::Data> &value,
                                            const std::shared_ptr<R> &record, const std::shared_ptr<R> &oldRecord,
                                            const exception::IException &error) {
                            }
*/

                            void onRemove(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record,
                                          bool removed) {
                            }

                            void onRemoveError(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record,
                                               bool removed, const exception::IException &error) {
                            }

                            void onExpire(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record) {
                                nearCacheStats_->incrementExpirations();
                            }

                            bool isEvictionEnabled() {
                                return evictionStrategy_.get() != NULL
                                       && evictionPolicyEvaluator_.get() != NULL
                                       && evictionPolicyType_ != eviction::NONE;
                            }

                            void clearRecords() {
                                records_->clear();
                            }

                            void destroyStore() {
                                clearRecords();
                            }

                            /*
                        static const int REFERENCE_SIZE = MEM_AVAILABLE ? MEM.arrayIndexScale(Object[].class) : (Integer.SIZE / Byte.SIZE);
*/
                            const client::config::NearCacheConfig &nearCacheConfig_;
                            const int64_t timeToLiveMillis_;
                            const int64_t maxIdleMillis_;
                            serialization::pimpl::SerializationService &serializationService_;
                            std::shared_ptr<monitor::impl::NearCacheStatsImpl> nearCacheStats_;

                            std::unique_ptr<eviction::MaxSizeChecker> maxSizeChecker_;
                            std::unique_ptr<eviction::EvictionPolicyEvaluator<K, V, KS, R> > evictionPolicyEvaluator_;
                            std::unique_ptr<eviction::EvictionChecker> evictionChecker_;
                            std::shared_ptr<eviction::EvictionStrategy<K, V, KS, R, NCRM> > evictionStrategy_;
                            eviction::EvictionPolicyType evictionPolicyType_;
                            std::unique_ptr<NCRM> records_;

/*
                            volatile StaleReadDetector staleReadDetector = ALWAYS_FRESH;
*/
                        private:
                            class MaxSizeEvictionChecker : public eviction::EvictionChecker {
                            public:

                                MaxSizeEvictionChecker(const eviction::MaxSizeChecker *maxSizeChecker) : maxSizeChecker_(
                                        maxSizeChecker) { }

                                bool isEvictionRequired() const override {
                                    return maxSizeChecker_ != NULL && maxSizeChecker_->isReachedToMaxSize();
                                }

                            private:
                                const eviction::MaxSizeChecker *maxSizeChecker_;
                            };

                            template<typename VALUE>
                            void putInternal(const std::shared_ptr<KS> &key, const std::shared_ptr<VALUE> &value) {
                                checkAvailable();

                                // if there is no eviction configured we return if the Near Cache is full and it's a new key
                                // (we have to check the key, otherwise we might lose updates on existing keys)
                                if (!isEvictionEnabled() && evictionChecker_->isEvictionRequired() &&
                                    !containsRecordKey(key)) {
                                    return;
                                }

                                std::shared_ptr<R> record;
                                std::shared_ptr<R> oldRecord;
                                try {
                                    record = valueToRecord(value);
                                    onRecordCreate(key, record);
                                    oldRecord = putRecord(key, record);
                                    if (oldRecord.get() == NULL) {
                                        nearCacheStats_->incrementOwnedEntryCount();
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

