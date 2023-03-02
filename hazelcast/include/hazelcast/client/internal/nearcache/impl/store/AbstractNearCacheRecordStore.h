/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/config/near_cache_config.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluator.h"
#include "hazelcast/client/internal/eviction/MaxSizeChecker.h"
#include "hazelcast/client/internal/eviction/EvictionChecker.h"
#include "hazelcast/client/internal/eviction/EvictionStrategy.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/internal/eviction/EvictionPolicyEvaluatorProvider.h"
#include "hazelcast/client/internal/eviction/EvictionStrategyProvider.h"
#include "hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h"
#include "hazelcast/client/internal/eviction/EvictionListener.h"
#include "hazelcast/client/internal/nearcache/impl/store/BaseHeapNearCacheRecordStore.h"
#include "hazelcast/client/serialization/pimpl/data.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace nearcache {
namespace impl {
namespace store {
template<typename K, typename V, typename KS, typename R, typename NCRM>
class AbstractNearCacheRecordStore
  : public NearCacheRecordStore<KS, V>
  , public eviction::EvictionListener<KS, R>
{
public:
    AbstractNearCacheRecordStore(
      const client::config::near_cache_config& cache_config,
      serialization::pimpl::SerializationService& ss)
      : near_cache_config_(cache_config)
      , time_to_live_millis_(cache_config.get_time_to_live_seconds() *
                             MILLI_SECONDS_IN_A_SECOND)
      , max_idle_millis_(cache_config.get_max_idle_seconds() *
                         MILLI_SECONDS_IN_A_SECOND)
      , serialization_service_(ss)
      , near_cache_stats_(new monitor::impl::NearCacheStatsImpl)
    {
        auto& evictionConfig =
          const_cast<client::config::near_cache_config&>(cache_config)
            .get_eviction_config();
        eviction_policy_ = evictionConfig.get_eviction_policy();
    }

    void initialize() override
    {
        auto& evictionConfig =
          const_cast<client::config::near_cache_config&>(near_cache_config_)
            .get_eviction_config();
        this->records_ = create_near_cache_record_map(near_cache_config_);
        this->max_size_checker_ = create_near_cache_max_size_checker(
          evictionConfig, near_cache_config_);
        this->eviction_policy_evaluator_ =
          create_eviction_policy_evaluator(evictionConfig);
        this->eviction_checker_ = create_eviction_checker(near_cache_config_);
        this->eviction_strategy_ = create_eviction_strategy(evictionConfig);
        this->eviction_policy_ = evictionConfig.get_eviction_policy();
    }

    /*
                            public void setStaleReadDetector(StaleReadDetector
       staleReadDetector) { this.staleReadDetector = staleReadDetector;
                                }

                            public StaleReadDetector getStaleReadDetector() {
                                    return staleReadDetector;
                                }
    */

    // public for tests.
    virtual const std::shared_ptr<R> get_record(const std::shared_ptr<KS>& key)
    {
        assert(0);
        return std::shared_ptr<R>();
    }

    void on_evict(const std::shared_ptr<KS>& key,
                  const std::shared_ptr<R>& record,
                  bool was_expired) override
    {
        if (was_expired) {
            near_cache_stats_->increment_expirations();
        } else {
            near_cache_stats_->increment_evictions();
        }
        near_cache_stats_->decrement_owned_entry_count();
    }

    std::shared_ptr<V> get(const std::shared_ptr<KS>& key) override
    {
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
                near_cache_stats_->increment_hits();
                value = record_to_value(record.get());
                on_get(key, value, record);
                return value;
            } else {
                near_cache_stats_->increment_misses();
                return std::shared_ptr<V>();
            }
        } catch (exception::iexception& error) {
            on_get_error(key, value, record, error);
            throw;
        }
    }

    void put(const std::shared_ptr<KS>& key,
             const std::shared_ptr<V>& value) override
    {
        put_internal<V>(key, value);
    }

    /*
                                //@Override
                                void put(const std::shared_ptr<KS> &key,
                                         const
       std::shared_ptr<serialization::pimpl::data> &value) {
                                    putInternal<serialization::pimpl::data>(key,
       value);
                                }
    */

    bool invalidate(const std::shared_ptr<KS>& key) override
    {
        check_available();

        std::shared_ptr<R> record;
        bool removed = false;
        try {
            record = remove_record(key);
            if (record.get() != NULL) {
                removed = true;
                near_cache_stats_->decrement_owned_entry_count();
                near_cache_stats_->decrement_owned_entry_memory_cost(
                  get_total_storage_memory_cost(key.get(), record.get()));
                near_cache_stats_->increment_invalidations();
            }
            near_cache_stats_->increment_invalidation_requests();
            on_remove(key, record, removed);
            return record.get() != NULL;
        } catch (exception::iexception& error) {
            on_remove_error(key, record, removed, error);
            throw;
        }
    }

    void clear() override
    {
        check_available();

        clear_records();
        near_cache_stats_->set_owned_entry_count(0);
        near_cache_stats_->set_owned_entry_memory_cost(0L);
    }

    void destroy() override
    {
        check_available();

        destroy_store();
        near_cache_stats_->set_owned_entry_count(0);
        near_cache_stats_->set_owned_entry_memory_cost(0L);
    }

    int size() const override
    {
        check_available();
        return (int)records_->size();
    }

    std::shared_ptr<monitor::near_cache_stats> get_near_cache_stats()
      const override
    {
        check_available();
        return near_cache_stats_;
    }

    void do_eviction_if_required() override
    {
        check_available();
        if (is_eviction_enabled()) {
            eviction_strategy_->evict(records_.get(),
                                      eviction_policy_evaluator_.get(),
                                      eviction_checker_.get(),
                                      this);
        }
    }

    void do_eviction() override
    {
        check_available();

        if (is_eviction_enabled()) {
            eviction_strategy_->evict(
              records_.get(), eviction_policy_evaluator_.get(), NULL, this);
        }
    }

protected:
    virtual std::unique_ptr<eviction::MaxSizeChecker>
    create_near_cache_max_size_checker(
      const client::config::eviction_config& eviction_config,
      const client::config::near_cache_config&)
    {
        assert(0);
        return std::unique_ptr<eviction::MaxSizeChecker>();
    }

    virtual std::unique_ptr<NCRM> create_near_cache_record_map(
      const client::config::near_cache_config&)
    {
        assert(0);
        return std::unique_ptr<NCRM>();
    }

    virtual int64_t get_key_storage_memory_cost(KS* key) const = 0;

    virtual int64_t get_record_storage_memory_cost(R* record) const = 0;

    int64_t get_total_storage_memory_cost(KS* key, R* record) const
    {
        return get_key_storage_memory_cost(key) +
               get_record_storage_memory_cost(record);
    }

    virtual std::unique_ptr<R> value_to_record(const std::shared_ptr<V>& value)
    {
        assert(0);
        return std::unique_ptr<R>();
    }

    /*
                                virtual std::unique_ptr<R> valueToRecord(
                                        const
       std::shared_ptr<serialization::pimpl::data> &value) { assert(0); return
       std::unique_ptr<R>();
                                }
    */

    virtual std::shared_ptr<V> record_to_value(const R* record)
    {
        assert(0);
        return std::shared_ptr<V>();
    }

    virtual std::shared_ptr<R> put_record(const std::shared_ptr<KS>& key,
                                          const std::shared_ptr<R>& record)
    {
        assert(0);
        return std::shared_ptr<R>();
    }

    virtual void put_to_record(std::shared_ptr<R>& record,
                               const std::shared_ptr<V>& value)
    {
        assert(0);
    }

    virtual std::shared_ptr<R> remove_record(const std::shared_ptr<KS>& key)
    {
        assert(0);
        return std::shared_ptr<R>();
    }

    virtual bool contains_record_key(const std::shared_ptr<KS>& key) const
    {
        assert(0);
        return false;
    }

    void check_available() const
    {
        if (!is_available()) {
            BOOST_THROW_EXCEPTION(exception::illegal_state(
              near_cache_config_.get_name() +
              " named Near Cache record store is not available"));
        }
    }

    std::unique_ptr<eviction::EvictionPolicyEvaluator<K, V, KS, R>>
    create_eviction_policy_evaluator(
      const client::config::eviction_config& eviction_config)
    {
        return eviction::EvictionPolicyEvaluatorProvider::
          get_eviction_policy_evaluator<K, V, KS, R>(eviction_config);
    }

    std::shared_ptr<eviction::EvictionStrategy<K, V, KS, R, NCRM>>
    create_eviction_strategy(
      const client::config::eviction_config& eviction_config)
    {
        return eviction::EvictionStrategyProvider<K, V, KS, R, NCRM>::
          get_eviction_strategy(eviction_config);
    }

    std::unique_ptr<eviction::EvictionChecker> create_eviction_checker(
      const client::config::near_cache_config& cache_config)
    {
        return std::unique_ptr<eviction::EvictionChecker>(
          new MaxSizeEvictionChecker(max_size_checker_.get()));
    }

    bool is_available() const { return records_.get() != NULL; }

    std::shared_ptr<serialization::pimpl::data> value_to_data(
      const std::shared_ptr<V>& value)
    {
        if (value.get() != NULL) {
            return std::shared_ptr<serialization::pimpl::data>(
              new serialization::pimpl::data(
                serialization_service_.to_data<V>(value.get())));
        } else {
            return std::shared_ptr<serialization::pimpl::data>();
        }
    }

    /*
                                std::shared_ptr<serialization::pimpl::data>
       valueToData( std::shared_ptr<serialization::pimpl::data> &value) { return
       value;
                                }
    */

    std::shared_ptr<V> data_to_value(
      const std::shared_ptr<serialization::pimpl::data>& data,
      const typed_data* dummy)
    {
        return std::shared_ptr<V>(
          new typed_data(*data, serialization_service_));
    }

    std::shared_ptr<V> data_to_value(
      const std::shared_ptr<serialization::pimpl::data>& data,
      void* dummy)
    {
        return data;
        /*
                                        if (data.get() != NULL) {
                                            auto value =
           serializationService.to_object<V>(data.get()); return
           std::shared_ptr<V>(new V(std::move(value).value())); } else { return
           std::shared_ptr<V>();
                                        }
        */
    }

    /*
                                const
       std::shared_ptr<serialization::pimpl::data> to_data( const
       std::shared_ptr<serialization::pimpl::data> &obj) { return obj;
                                }
    */

    const std::shared_ptr<serialization::pimpl::data> to_data(
      const std::shared_ptr<V>& obj)
    {
        if (obj.get() == NULL) {
            return std::shared_ptr<serialization::pimpl::data>();
        } else {
            return value_to_data(obj);
        }
    }

    /*
                                std::shared_ptr<V>
       toValue(std::shared_ptr<serialization::pimpl::data> &obj) { if (obj.get()
       == NULL) { return std::shared_ptr<V>(); } else { return dataToValue(obj,
       (V *)NULL);
                                    }
                                }
    */

    std::shared_ptr<V> to_value(std::shared_ptr<V>& obj) { return obj; }

    int64_t get_total_storage_memory_cost(const std::shared_ptr<KS>& key,
                                          const std::shared_ptr<R>& record)
    {
        return get_key_storage_memory_cost(key.get()) +
               get_record_storage_memory_cost(record.get());
    }

    bool is_record_expired(const std::shared_ptr<R>& record) const
    {
        int64_t now = util::current_time_millis();
        if (record->is_expired_at(now)) {
            return true;
        } else {
            return record->is_idle_at(max_idle_millis_, now);
        }
    }

    void on_record_create(const std::shared_ptr<KS>& key,
                          const std::shared_ptr<R>& record)
    {
        record->set_creation_time(util::current_time_millis());
    }

    void on_record_access(const std::shared_ptr<R>& record)
    {
        record->set_access_time(util::current_time_millis());
        record->increment_access_hit();
    }

    void on_get(const std::shared_ptr<KS>& key,
                const std::shared_ptr<V>& value,
                const std::shared_ptr<R>& record)
    {}

    void on_get_error(const std::shared_ptr<KS>& key,
                      const std::shared_ptr<V>& value,
                      const std::shared_ptr<R>& record,
                      const exception::iexception& error)
    {}

    void on_put(const std::shared_ptr<KS>& key,
                const std::shared_ptr<V>& value,
                const std::shared_ptr<R>& record,
                const std::shared_ptr<R>& old_record)
    {}

    /*
                                void onPut(const std::shared_ptr<KS> &key,
                                           const
       std::shared_ptr<serialization::pimpl::data> &value, const
       std::shared_ptr<R> &record, const std::shared_ptr<R> &oldRecord) {
                                }
    */

    void on_put_error(const std::shared_ptr<KS>& key,
                      const std::shared_ptr<V>& value,
                      const std::shared_ptr<R>& record,
                      const std::shared_ptr<R>& old_record,
                      const exception::iexception& error)
    {}

    /*
                                void onPutError(const std::shared_ptr<KS> &key,
                                                const
       std::shared_ptr<serialization::pimpl::data> &value, const
       std::shared_ptr<R> &record, const std::shared_ptr<R> &oldRecord, const
       exception::IException &error) {
                                }
    */

    void on_remove(const std::shared_ptr<KS>& key,
                   const std::shared_ptr<R>& record,
                   bool removed)
    {}

    void on_remove_error(const std::shared_ptr<KS>& key,
                         const std::shared_ptr<R>& record,
                         bool removed,
                         const exception::iexception& error)
    {}

    void on_expire(const std::shared_ptr<KS>& key,
                   const std::shared_ptr<R>& record)
    {
        near_cache_stats_->increment_expirations();
    }

    bool is_eviction_enabled()
    {
        return eviction_strategy_.get() != NULL &&
               eviction_policy_evaluator_.get() != NULL &&
               eviction_policy_ !=
                 hazelcast::client::config::eviction_policy::NONE;
    }

    void clear_records() { records_->clear(); }

    void destroy_store() { clear_records(); }

    /*
static const int REFERENCE_SIZE = MEM_AVAILABLE ?
MEM.arrayIndexScale(Object[].class) : (Integer.SIZE / Byte.SIZE);
*/
    const client::config::near_cache_config& near_cache_config_;
    const int64_t time_to_live_millis_;
    const int64_t max_idle_millis_;
    serialization::pimpl::SerializationService& serialization_service_;
    std::shared_ptr<monitor::impl::NearCacheStatsImpl> near_cache_stats_;

    std::unique_ptr<eviction::MaxSizeChecker> max_size_checker_;
    std::unique_ptr<eviction::EvictionPolicyEvaluator<K, V, KS, R>>
      eviction_policy_evaluator_;
    std::unique_ptr<eviction::EvictionChecker> eviction_checker_;
    std::shared_ptr<eviction::EvictionStrategy<K, V, KS, R, NCRM>>
      eviction_strategy_;
    ::hazelcast::client::config::eviction_policy eviction_policy_;
    std::unique_ptr<NCRM> records_;

    /*
                                volatile StaleReadDetector staleReadDetector =
       ALWAYS_FRESH;
    */
private:
    class MaxSizeEvictionChecker : public eviction::EvictionChecker
    {
    public:
        MaxSizeEvictionChecker(const eviction::MaxSizeChecker* max_size_checker)
          : max_size_checker_(max_size_checker)
        {}

        bool is_eviction_required() const override
        {
            return max_size_checker_ != NULL &&
                   max_size_checker_->is_reached_to_max_size();
        }

    private:
        const eviction::MaxSizeChecker* max_size_checker_;
    };

    template<typename VALUE>
    void put_internal(const std::shared_ptr<KS>& key,
                      const std::shared_ptr<VALUE>& value)
    {
        check_available();

        // if there is no eviction configured we return if the Near Cache is
        // full and it's a new key (we have to check the key, otherwise we might
        // lose updates on existing keys)
        if (!is_eviction_enabled() &&
            eviction_checker_->is_eviction_required() &&
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
                near_cache_stats_->increment_owned_entry_count();
            }
            on_put(key, value, record, oldRecord);
        } catch (exception::iexception& error) {
            on_put_error(key, value, record, oldRecord, error);
            throw;
        }
    }

    static const int MILLI_SECONDS_IN_A_SECOND = 1000;
};
} // namespace store
} // namespace impl
} // namespace nearcache
} // namespace internal
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
