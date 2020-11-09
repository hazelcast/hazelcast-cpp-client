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

#include <memory>

#include "hazelcast/client/internal/nearcache/impl/store/HeapNearCacheRecordMap.h"
#include "hazelcast/client/internal/nearcache/impl/store/AbstractNearCacheRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/maxsize/EntryCountNearCacheMaxSizeChecker.h"

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
                        template<typename K, typename V, typename KS, typename R>
                        class BaseHeapNearCacheRecordStore
                                : public AbstractNearCacheRecordStore<K, V, KS, R, HeapNearCacheRecordMap<K, V, KS, R> > {
                        public:
                            typedef AbstractNearCacheRecordStore<K, V, KS, R, HeapNearCacheRecordMap<K, V, KS, R> > ANCRS;

                            BaseHeapNearCacheRecordStore(const std::string &name,
                                                         const client::config::NearCacheConfig &near_cache_config,
                                                         serialization::pimpl::SerializationService &serialization_service
                            ) : ANCRS(near_cache_config, serialization_service) {
                            }

                            const std::shared_ptr<R> get_record(const std::shared_ptr<KS> &key) override {
                                return ANCRS::records_->get(key);
                            }

                            void on_evict(const std::shared_ptr<KS> &key, const std::shared_ptr<R> &record,
                                         bool was_expired) override {
                                ANCRS::on_evict(key,
                                               record,
                                               was_expired);
                                ANCRS::near_cache_stats_->decrement_owned_entry_memory_cost(
                                        ANCRS::get_total_storage_memory_cost(key, record));
                            }

                            void do_expiration() override {
                                std::vector<std::pair<std::shared_ptr<KS>, std::shared_ptr<R> > > entries = ANCRS::records_->entry_set();
                                for (typename std::vector<std::pair<std::shared_ptr<KS>, std::shared_ptr<R> > >::const_iterator it = entries.begin();
                                     it != entries.end(); ++it) {
                                    const std::pair<std::shared_ptr<KS>, std::shared_ptr<R> > &entry = (*it);
                                    const std::shared_ptr<KS> &key = entry.first;
                                    const std::shared_ptr<R> &value = entry.second;
                                    if (ANCRS::is_record_expired(value)) {
                                        ANCRS::invalidate(key);
                                        ANCRS::on_expire(key, value);
                                    }
                                }
                            }
                        protected:
                            std::unique_ptr<eviction::MaxSizeChecker> create_near_cache_max_size_checker(
                                    const client::config::EvictionConfig &eviction_config,
                                    const client::config::NearCacheConfig &near_cache_config) override {
                                typename client::config::EvictionConfig::max_size_policy maxSizePolicy = eviction_config.get_maximum_size_policy();
                                if (maxSizePolicy == client::config::EvictionConfig::ENTRY_COUNT) {
                                    return std::unique_ptr<eviction::MaxSizeChecker>(
                                            new maxsize::EntryCountNearCacheMaxSizeChecker<K, V, KS, R>(
                                                    eviction_config.get_size(),
                                                    *ANCRS::records_));
                                }
                                std::ostringstream out;
                                out << "Invalid max-size policy " << '(' << (int) maxSizePolicy << ") for " <<
                                    near_cache_config.get_name() << "! Only " <<
                                    (int) client::config::EvictionConfig::ENTRY_COUNT << " is supported.";
                                BOOST_THROW_EXCEPTION(exception::illegal_argument(out.str()));
                            }

                            std::unique_ptr<HeapNearCacheRecordMap<K, V, KS, R> > create_near_cache_record_map(
                                    const client::config::NearCacheConfig &near_cache_config) override {
                                return std::unique_ptr<HeapNearCacheRecordMap<K, V, KS, R> >(
                                        new HeapNearCacheRecordMap<K, V, KS, R>(ANCRS::serialization_service_,
                                                                                DEFAULT_INITIAL_CAPACITY));
                            }

                            std::shared_ptr<R> put_record(const std::shared_ptr<KS> &key,
                                                           const std::shared_ptr<R> &record) override {
                                std::shared_ptr<R> oldRecord = ANCRS::records_->put(key, record);
                                ANCRS::near_cache_stats_->increment_owned_entry_memory_cost(
                                        ANCRS::get_total_storage_memory_cost(key, record));
                                if (oldRecord.get() != NULL) {
                                    ANCRS::near_cache_stats_->decrement_owned_entry_memory_cost(
                                            ANCRS::get_total_storage_memory_cost(key, oldRecord));
                                }
                                return oldRecord;
                            }

                            std::shared_ptr<R> remove_record(const std::shared_ptr<KS> &key) override {
                                return ANCRS::records_->remove(key);
                            }

                            bool contains_record_key(const std::shared_ptr<KS> &key) const override {
                                return ANCRS::records_->contains_key(key);
                            }

                            static const int32_t DEFAULT_INITIAL_CAPACITY = 1000;
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



