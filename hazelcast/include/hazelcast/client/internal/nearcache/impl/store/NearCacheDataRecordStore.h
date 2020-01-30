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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEDATARESCORDSTORE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEDATARESCORDSTORE_H_

#include <stdint.h>
#include <atomic>

#include "hazelcast/client/internal/nearcache/impl/store/BaseHeapNearCacheRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/record/NearCacheDataRecord.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"

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
                        template<typename K, typename V, typename KS>
                        class NearCacheDataRecordStore
                                : public BaseHeapNearCacheRecordStore<K, V, KS, record::NearCacheDataRecord> {
                        public:
                            typedef AbstractNearCacheRecordStore<K, V, KS, record::NearCacheDataRecord, HeapNearCacheRecordMap<K, V, KS, record::NearCacheDataRecord> > ANCRS;

                            static const int64_t REFERENCE_SIZE = sizeof(std::shared_ptr<serialization::pimpl::Data>);

                            NearCacheDataRecordStore(const std::string &name,
                                                     const client::config::NearCacheConfig<K, V> &config,
                                                     serialization::pimpl::SerializationService &ss)
                                    : BaseHeapNearCacheRecordStore<K, V, serialization::pimpl::Data, record::NearCacheDataRecord>(name, config, ss) {
                            }
                        protected:
                            //@Override
                        virtual int64_t getKeyStorageMemoryCost(KS *key) const {
                                return
                                    // reference to this key data inside map ("store" field)
                                        REFERENCE_SIZE
                                        // cost of this key data
                                        + (key != NULL ? key->totalSize() : 0);
                            }

                            //@Override
                            virtual int64_t getRecordStorageMemoryCost(record::NearCacheDataRecord *record) const {
                                if (record == NULL) {
                                    return 0L;
                                }
                                std::shared_ptr<serialization::pimpl::Data> value = record->getValue();
                                return
                                    // reference to this record inside map ("store" field)
                                        REFERENCE_SIZE
                                        // reference to "value" field
                                        + REFERENCE_SIZE
                                        // heap cost of this value data
                                        + (value.get() != NULL ? (int64_t) value->totalSize() : 0)
                                        // 3 primitive int64_t typed fields: "creationTime", "expirationTime" and "accessTime"
                                        + (3 * (sizeof(std::atomic<int64_t>)))
                                        // reference to "accessHit" field
                                        + REFERENCE_SIZE
                                        // primitive int typed "value" field in "AtomicInteger" typed "accessHit" field
                                        + (sizeof(std::atomic<int32_t>));
                            }

                            //@Override
                            std::unique_ptr<record::NearCacheDataRecord> valueToRecord(
                                    const std::shared_ptr<serialization::pimpl::Data> &value) {
                                return valueToRecordInternal(value);
                            }

                            //@Override
                            std::unique_ptr<record::NearCacheDataRecord> valueToRecord(
                                    const std::shared_ptr<V> &value) {
                                const std::shared_ptr<serialization::pimpl::Data> data = ANCRS::toData(value);
                                return valueToRecordInternal(data);
                            }

                            //@Override
                            std::shared_ptr<V> recordToValue(const record::NearCacheDataRecord *record) {
                                const std::shared_ptr<serialization::pimpl::Data> value = record->getValue();
                                if (value.get() == NULL) {
                                    ANCRS::nearCacheStats.incrementMisses();
                                    return std::static_pointer_cast<V>(NearCache<K, V>::NULL_OBJECT);
                                }
                                return ANCRS::dataToValue(value, (V *)NULL);
                            }

                            //@Override
                            void putToRecord(std::shared_ptr<record::NearCacheDataRecord> &record,
                                             const std::shared_ptr<V> &value) {
                                record->setValue(ANCRS::toData(value));
                            }
                        private:
                            std::unique_ptr<record::NearCacheDataRecord> valueToRecordInternal(
                                    const std::shared_ptr<serialization::pimpl::Data> &data) {
                                int64_t creationTime = util::currentTimeMillis();
                                if (ANCRS::timeToLiveMillis > 0) {
                                    return std::unique_ptr<record::NearCacheDataRecord>(
                                            new record::NearCacheDataRecord(data, creationTime,
                                                                            creationTime + ANCRS::timeToLiveMillis));
                                } else {
                                    return std::unique_ptr<record::NearCacheDataRecord>(
                                            new record::NearCacheDataRecord(data, creationTime,
                                                                            NearCacheRecord<V>::TIME_NOT_SET));
                                }
                            }
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEDATARESCORDSTORE_H_ */

