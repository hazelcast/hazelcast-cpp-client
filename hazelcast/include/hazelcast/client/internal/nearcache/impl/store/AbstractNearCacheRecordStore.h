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
                                      timeToLiveMillis_(cacheConfig.get_time_to_live_seconds() * MILLI_SECONDS_IN_A_SECOND),
                                      maxIdleMillis_(cacheConfig.get_max_idle_seconds() * MILLI_SECONDS_IN_A_SECOND),
                                      serializationService_(ss), nearCacheStats_(new monitor::impl::NearCacheStatsImpl) {
                                auto &evictionConfig = const_cast<client::config::NearCacheConfig &>(cacheConfig).get_eviction_config();
                                evictionPolicyType_ = evictionConfig.get_eviction_policy_type();
                            }

                            void initialize() override {
                                auto &evictionConfig = const_cast<client::config::NearCacheConfig &>(nearCacheConfig_).get_eviction_config();
                                this->records_ = create_near_cache_record_map(nearCacheConfig_);
                                this->maxSizeChecker_ = create_near_cache_max_size_checker(evictionConfig, nearCacheConfig_);
                                this->evictionPolicyEvaluator_ = create_eviction_policy_evaluator(evictionConfig);
                                this->evictionChecker_ = create_eviction_checker(nearCacheConfig_);
                                this->evictionStrategy_ = create_eviction_strategy(evictionConfig);
                                this->evictionPolicyType_ = evictionConfig.get_eviction_policy_type();
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
                            virtual const std::shared_ptr<R> get_record(const std::shared_ptr<KS> &key) {
                                assert(0);
                                return std::shared_ptr<R>();
                            }

                            void on_evict(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record,
                                                 bool wasExpired) override {
                                if (wasExpired) {
                                    nearCacheStats_->increment_expirations();
                                } else {
                                    nearCacheStats_->increment_evictions();
                                }
                                nearCacheStats_->decrement_owned_entry_count();
                            }

                            std::shared_ptr<V> get(const std::shared_ptr<KS> &key) override {
                                check_available();

                                std::shared_ptr<R> record;
                                std::shared_ptr<V> value;
                                try {
                                    record = get_record(key);
                                    if (record.get() != NULL) {
                                        if (is_record_expired(record)) {
                                            invalidate(key);
                                            on_expire(key, record);
                                            return std::shared_ptr<V>();
                                        }
                                        on_record_access(record);
                                        nearCacheStats_->increment_hits();
                                        value = record_to_value(record.get());
                                        on_get(key, value, record);
                                        return value;
                                    } else {
                                        nearCacheStats_->increment_misses();
                                        return std::shared_ptr<V>();
                                    }
                                } catch (exception::IException &error) {
                                    on_get_error(key, value, record, error);
                                    throw;
                                }
                            }


                            void put(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value) override {
                                put_internal<V>(key, value);
                            }

/*
                            //@Override
                            void put(const std::shared_ptr<KS> &key,
                                     const std::shared_ptr<serialization::pimpl::Data> &value) {
                                putInternal<serialization::pimpl::Data>(key, value);
                            }
*/

                            bool invalidate(const std::shared_ptr<KS> &key) override {
                                check_available();

                                std::shared_ptr<R> record;
                                bool removed = false;
                                try {
                                    record = remove_record(key);
                                    if (record.get() != NULL) {
                                        removed = true;
                                        nearCacheStats_->decrement_owned_entry_count();
                                        nearCacheStats_->decrement_owned_entry_memory_cost(
                                                get_total_storage_memory_cost(key.get(), record.get()));
                                        nearCacheStats_->increment_invalidations();
                                    }
                                    nearCacheStats_->increment_invalidation_requests();
                                    on_remove(key, record, removed);
                                    return record.get() != NULL;
                                } catch (exception::IException &error) {
                                    on_remove_error(key, record, removed, error);
                                    throw;
                                }
                            }

                            void clear() override {
                                check_available();

                                clear_records();
                                nearCacheStats_->set_owned_entry_count(0);
                                nearCacheStats_->set_owned_entry_memory_cost(0L);
                            }

                            void destroy() override {
                                check_available();

                                destroy_store();
                                nearCacheStats_->set_owned_entry_count(0);
                                nearCacheStats_->set_owned_entry_memory_cost(0L);
                            }

                            int size() const override {
                                check_available();
                                return (int) records_->size();
                            }

                            std::shared_ptr<monitor::NearCacheStats> get_near_cache_stats() const override {
                                check_available();
                                return nearCacheStats_;
                            }

                            void do_eviction_if_required() override {
                                check_available();
                                if (is_eviction_enabled()) {
                                    evictionStrategy_->evict(records_.get(), evictionPolicyEvaluator_.get(),
                                                            evictionChecker_.get(), this);
                                }
                            }

                            void do_eviction() override {
                                check_available();

                                if (is_eviction_enabled()) {
                                    evictionStrategy_->evict(records_.get(), evictionPolicyEvaluator_.get(), NULL, this);
                                }
                            }
                        protected:
                            virtual std::unique_ptr<eviction::MaxSizeChecker> create_near_cache_max_size_checker(
                                    const client::config::EvictionConfig &evictionConfig,
                                    const client::config::NearCacheConfig &) {
                                assert(0);
                                return std::unique_ptr<eviction::MaxSizeChecker>();
                            }

                            virtual std::unique_ptr<NCRM> create_near_cache_record_map(
                                    const client::config::NearCacheConfig &) {
                                assert(0);
                                return std::unique_ptr<NCRM>();
                            }

                            virtual int64_t get_key_storage_memory_cost(KS *key) const = 0;

                            virtual int64_t get_record_storage_memory_cost(R *record) const = 0;

                            int64_t get_total_storage_memory_cost(KS *key, R *record) const {
                                return get_key_storage_memory_cost(key) + get_record_storage_memory_cost(record);
                            }

                            virtual std::unique_ptr<R> value_to_record(const std::shared_ptr<V> &value) {
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

                            virtual std::shared_ptr<V> record_to_value(const R *record) {
                                assert(0);
                                return std::shared_ptr<V>();
                            }

                            virtual std::shared_ptr<R> put_record(const std::shared_ptr<KS> &key,
                                                                   const std::shared_ptr<R> &record) {
                                assert(0);
                                return std::shared_ptr<R>();
                            }

                            virtual void put_to_record(std::shared_ptr<R> &record,
                                                     const std::shared_ptr<V> &value) {
                                assert(0);
                            }

                            virtual std::shared_ptr<R> remove_record(const std::shared_ptr<KS> &key) {
                                assert(0);
                                return std::shared_ptr<R>();
                            }

                            virtual bool contains_record_key(const std::shared_ptr<KS> &key) const {
                                assert(0);
                                return false;
                            }

                            void check_available() const {
                                if (!is_available()) {
                                    BOOST_THROW_EXCEPTION(exception::IllegalStateException(nearCacheConfig_.get_name() +
                                                                                           " named Near Cache record store is not available"));
                                }
                            }

                            std::unique_ptr<eviction::EvictionPolicyEvaluator<K, V, KS, R> > create_eviction_policy_evaluator(
                                    const client::config::EvictionConfig &evictionConfig) {
                                return eviction::EvictionPolicyEvaluatorProvider::get_eviction_policy_evaluator<K, V, KS, R>(
                                        evictionConfig);
                            }

                            std::shared_ptr<eviction::EvictionStrategy<K, V, KS, R, NCRM> > create_eviction_strategy(
                                    const client::config::EvictionConfig &evictionConfig) {
                                return eviction::EvictionStrategyProvider<K, V, KS, R, NCRM>::get_eviction_strategy(
                                        evictionConfig);
                            }

                            std::unique_ptr<eviction::EvictionChecker> create_eviction_checker(
                                    const client::config::NearCacheConfig &cacheConfig) {
                                return std::unique_ptr<eviction::EvictionChecker>(
                                        new MaxSizeEvictionChecker(maxSizeChecker_.get()));
                            }

                            bool is_available() const {
                                return records_.get() != NULL;
                            }

                            std::shared_ptr<serialization::pimpl::Data> value_to_data(
                                    const std::shared_ptr<V> &value) {
                                if (value.get() != NULL) {
                                    return std::shared_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(
                                            serializationService_.to_data<V>(value.get())));
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

                            std::shared_ptr<V> data_to_value(
                                    const std::shared_ptr<serialization::pimpl::Data> &data, const TypedData *dummy) {
                                return std::shared_ptr<V>(new TypedData(*data, serializationService_));
                            }

                            std::shared_ptr<V> data_to_value(
                                    const std::shared_ptr<serialization::pimpl::Data> &data, void *dummy) {
                                return data;
/*
                                if (data.get() != NULL) {
                                    auto value = serializationService.to_object<V>(data.get());
                                    return std::shared_ptr<V>(new V(std::move(value).value()));
                                } else {
                                    return std::shared_ptr<V>();
                                }
*/
                            }

/*
                            const std::shared_ptr<serialization::pimpl::Data> to_data(
                                    const std::shared_ptr<serialization::pimpl::Data> &obj) {
                                return obj;
                            }
*/

                            const std::shared_ptr<serialization::pimpl::Data> to_data(
                                    const std::shared_ptr<V> &obj) {
                                if (obj.get() == NULL) {
                                    return std::shared_ptr<serialization::pimpl::Data>();
                                } else {
                                    return value_to_data(obj);
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

                            std::shared_ptr<V> to_value(std::shared_ptr<V> &obj) {
                                return obj;
                            }

                            int64_t get_total_storage_memory_cost(const std::shared_ptr<KS> &key,
                                                              const std::shared_ptr<R> &record) {
                                return get_key_storage_memory_cost(key.get()) + get_record_storage_memory_cost(record.get());
                            }

                            bool is_record_expired(const std::shared_ptr<R> &record) const {
                                int64_t now = util::current_time_millis();
                                if (record->is_expired_at(now)) {
                                    return true;
                                } else {
                                    return record->is_idle_at(maxIdleMillis_, now);
                                }
                            }

                            void on_record_create(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record) {
                                record->set_creation_time(util::current_time_millis());
                            }

                            void on_record_access(const std::shared_ptr<R> &record) {
                                record->set_access_time(util::current_time_millis());
                                record->increment_access_hit();
                            }

                            void on_get(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value,
                                       const std::shared_ptr<R> &record) {
                            }

                            void on_get_error(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value,
                                            const std::shared_ptr<R> &record, const exception::IException &error) {
                            }

                            void on_put(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value,
                                       const std::shared_ptr<R> &record, const std::shared_ptr<R> &oldRecord) {
                            }

/*
                            void onPut(const std::shared_ptr<KS> &key,
                                       const std::shared_ptr<serialization::pimpl::Data> &value,
                                       const std::shared_ptr<R> &record, const std::shared_ptr<R> &oldRecord) {
                            }
*/

                            void on_put_error(const std::shared_ptr<KS> &key, const std::shared_ptr<V> &value,
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

                            void on_remove(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record,
                                          bool removed) {
                            }

                            void on_remove_error(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record,
                                               bool removed, const exception::IException &error) {
                            }

                            void on_expire(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record) {
                                nearCacheStats_->increment_expirations();
                            }

                            bool is_eviction_enabled() {
                                return evictionStrategy_.get() != NULL
                                       && evictionPolicyEvaluator_.get() != NULL
                                       && evictionPolicyType_ != eviction::NONE;
                            }

                            void clear_records() {
                                records_->clear();
                            }

                            void destroy_store() {
                                clear_records();
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

                                bool is_eviction_required() const override {
                                    return maxSizeChecker_ != NULL && maxSizeChecker_->is_reached_to_max_size();
                                }

                            private:
                                const eviction::MaxSizeChecker *maxSizeChecker_;
                            };

                            template<typename VALUE>
                            void put_internal(const std::shared_ptr<KS> &key, const std::shared_ptr<VALUE> &value) {
                                check_available();

                                // if there is no eviction configured we return if the Near Cache is full and it's a new key
                                // (we have to check the key, otherwise we might lose updates on existing keys)
                                if (!is_eviction_enabled() && evictionChecker_->is_eviction_required() &&
                                    !contains_record_key(key)) {
                                    return;
                                }

                                std::shared_ptr<R> record;
                                std::shared_ptr<R> oldRecord;
                                try {
                                    record = value_to_record(value);
                                    on_record_create(key, record);
                                    oldRecord = put_record(key, record);
                                    if (oldRecord.get() == NULL) {
                                        nearCacheStats_->increment_owned_entry_count();
                                    }
                                    on_put(key, value, record, oldRecord);
                                } catch (exception::IException &error) {
                                    on_put_error(key, value, record, oldRecord, error);
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

