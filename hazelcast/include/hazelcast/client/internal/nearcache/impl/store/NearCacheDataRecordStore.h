/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <atomic>

#include "hazelcast/client/internal/nearcache/impl/store/BaseHeapNearCacheRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/record/NearCacheDataRecord.h"
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
class NearCacheDataRecordStore
  : public BaseHeapNearCacheRecordStore<K, V, KS, record::NearCacheDataRecord>
{
public:
    typedef AbstractNearCacheRecordStore<
      K,
      V,
      KS,
      record::NearCacheDataRecord,
      HeapNearCacheRecordMap<K, V, KS, record::NearCacheDataRecord>>
      ANCRS;

    static const int64_t REFERENCE_SIZE =
      sizeof(std::shared_ptr<serialization::pimpl::data>);

    NearCacheDataRecordStore(const std::string& name,
                             const client::config::near_cache_config& config,
                             serialization::pimpl::SerializationService& ss)
      : BaseHeapNearCacheRecordStore<K,
                                     V,
                                     serialization::pimpl::data,
                                     record::NearCacheDataRecord>(name,
                                                                  config,
                                                                  ss)
    {}

protected:
    int64_t get_key_storage_memory_cost(KS* key) const override
    {
        return
          // reference to this key data inside map ("store" field)
          REFERENCE_SIZE
          // cost of this key data
          + (key != NULL ? key->total_size() : 0);
    }

    int64_t get_record_storage_memory_cost(
      record::NearCacheDataRecord* record) const override
    {
        if (record == NULL) {
            return 0L;
        }
        std::shared_ptr<serialization::pimpl::data> value = record->get_value();
        return
          // reference to this record inside map ("store" field)
          REFERENCE_SIZE
          // reference to "value" field
          + REFERENCE_SIZE
          // heap cost of this value data
          + (value.get() != NULL ? (int64_t)value->total_size() : 0)
          // 3 primitive int64_t typed fields: "creationTime", "expirationTime"
          // and "accessTime"
          + (3 * (sizeof(std::atomic<int64_t>)))
          // reference to "accessHit" field
          + REFERENCE_SIZE
          // primitive int typed "value" field in "AtomicInteger" typed
          // "accessHit" field
          + (sizeof(std::atomic<int32_t>));
    }

    std::unique_ptr<record::NearCacheDataRecord> value_to_record(
      const std::shared_ptr<serialization::pimpl::data>& value) override
    {
        return value_to_record_internal(value);
    }

    //@Override
    /*
                                std::unique_ptr<record::NearCacheDataRecord>
       valueToRecord( const std::shared_ptr<V> &value) { const
       std::shared_ptr<serialization::pimpl::data> data = ANCRS::to_data(value);
                                    return valueToRecordInternal(data);
                                }
    */

    std::shared_ptr<V> record_to_value(
      const record::NearCacheDataRecord* record) override
    {
        const std::shared_ptr<serialization::pimpl::data> value =
          record->get_value();
        if (value.get() == NULL) {
            ANCRS::near_cache_stats_->increment_misses();
            return std::static_pointer_cast<V>(NearCache<K, V>::NULL_OBJECT);
        }
        return ANCRS::data_to_value(value, (V*)NULL);
    }

    void put_to_record(std::shared_ptr<record::NearCacheDataRecord>& record,
                       const std::shared_ptr<V>& value) override
    {
        record->set_value(ANCRS::to_data(value));
    }

private:
    std::unique_ptr<record::NearCacheDataRecord> value_to_record_internal(
      const std::shared_ptr<serialization::pimpl::data>& data)
    {
        int64_t creationTime = util::current_time_millis();
        if (ANCRS::time_to_live_millis_ > 0) {
            return std::unique_ptr<record::NearCacheDataRecord>(
              new record::NearCacheDataRecord(data,
                                              creationTime,
                                              creationTime +
                                                ANCRS::time_to_live_millis_));
        } else {
            return std::unique_ptr<record::NearCacheDataRecord>(
              new record::NearCacheDataRecord(
                data, creationTime, NearCacheRecord<V>::TIME_NOT_SET));
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
