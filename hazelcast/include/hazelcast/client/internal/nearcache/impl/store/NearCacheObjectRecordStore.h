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

#include "hazelcast/client/internal/nearcache/impl/store/BaseHeapNearCacheRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/record/NearCacheObjectRecord.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"

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
template<typename K, typename V, typename KS>
class NearCacheObjectRecordStore
  : public BaseHeapNearCacheRecordStore<K,
                                        V,
                                        KS,
                                        record::NearCacheObjectRecord<V>>
{
public:
    typedef AbstractNearCacheRecordStore<
      K,
      V,
      KS,
      record::NearCacheObjectRecord<V>,
      HeapNearCacheRecordMap<K, V, KS, record::NearCacheObjectRecord<V>>>
      ANCRS;

    NearCacheObjectRecordStore(const client::config::near_cache_config& config,
                               serialization::pimpl::SerializationService& ss)
      : BaseHeapNearCacheRecordStore<K,
                                     V,
                                     serialization::pimpl::data,
                                     record::NearCacheObjectRecord<V>>(config,
                                                                       ss)
    {}

protected:
    int64_t get_key_storage_memory_cost(KS* /* key */) const override
    {
        // memory cost for "OBJECT" in memory format is totally not supported,
        // so just return zero
        return 0L;
    }

    int64_t get_record_storage_memory_cost(
      record::NearCacheObjectRecord<V>* /* record */) const override
    {
        // memory cost for "OBJECT" in memory format is totally not supported,
        // so just return zero
        return 0L;
    }

    //@Override
    /*
                                std::unique_ptr<record::NearCacheObjectRecord<V>
       > valueToRecord( const std::shared_ptr<serialization::pimpl::data>
       &value_data) { std::shared_ptr<serialization::pimpl::data> data =
       std::const_pointer_cast<serialization::pimpl::data>( value_data); const
       std::shared_ptr<V> value = ANCRS::toValue(data); return
       valueToRecordInternal(value);
                                }
    */

    std::unique_ptr<record::NearCacheObjectRecord<V>> value_to_record(
      const std::shared_ptr<V>& value) override
    {
        return value_to_record_internal(value);
    }

    std::shared_ptr<V> record_to_value(
      const record::NearCacheObjectRecord<V>* record) override
    {
        const std::shared_ptr<V> value = record->get_value();
        if (value.get() == NULL) {
            return std::static_pointer_cast<V>(NearCache<K, V>::NULL_OBJECT);
        }
        return value;
    }

    void put_to_record(
      std::shared_ptr<record::NearCacheObjectRecord<V>>& record,
      const std::shared_ptr<V>& value) override
    {
        record->set_value(value);
    }

private:
    std::unique_ptr<record::NearCacheObjectRecord<V>> value_to_record_internal(
      const std::shared_ptr<V>& value)
    {
        int64_t creationTime = util::current_time_millis();
        if (ANCRS::time_to_live_millis_ > 0) {
            return std::unique_ptr<record::NearCacheObjectRecord<V>>(
              new record::NearCacheObjectRecord<V>(
                value,
                creationTime,
                creationTime + ANCRS::time_to_live_millis_));
        } else {
            return std::unique_ptr<record::NearCacheObjectRecord<V>>(
              new record::NearCacheObjectRecord<V>(
                value, creationTime, NearCacheRecord<V>::TIME_NOT_SET));
        }
    }
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
