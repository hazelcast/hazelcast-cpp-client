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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEDATARESCORDSTORE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEDATARESCORDSTORE_H_

#include <stdint.h>

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

                            NearCacheDataRecordStore(const std::string &name,
                                                     const client::config::NearCacheConfig<K, V> &config,
                                                     serialization::pimpl::SerializationService &ss)
                                    : BaseHeapNearCacheRecordStore<K, V, serialization::pimpl::Data, record::NearCacheDataRecord>(name, config, ss) {
                            }
                        protected:
                            //@Override
                        virtual int64_t getKeyStorageMemoryCost(KS *key) const {
/*TODO
                                if (key instanceof Data) {
                                    return
                                        // reference to this key data inside map ("store" field)
                                            REFERENCE_SIZE
                                            // heap cost of this key data
                                            + ((Data) key).getHeapCost();
                                } else {
                                    // memory cost for non-data typed instance is not supported
                                    return 0L;
                                }
*/
                                return 1L;
                            }

                            //@Override
                            virtual int64_t getRecordStorageMemoryCost(record::NearCacheDataRecord *record) const {
                                return 1L;
/*TODO
                                if (record == NULL) {
                                    return 0L;
                                }
                                boost::shared_ptr<serialization::pimpl::Data> value = record->getValue();
                                return
                                    // reference to this record inside map ("store" field)
                                        REFERENCE_SIZE
                                        // reference to "value" field
                                        + REFERENCE_SIZE
                                        // heap cost of this value data
                                        + (value != null ? value.getHeapCost() : 0)
                                        // 3 primitive int64_t typed fields: "creationTime", "expirationTime" and "accessTime"
                                        + (3 * (Long.SIZE / Byte.SIZE))
                                        // reference to "accessHit" field
                                        + REFERENCE_SIZE
                                        // primitive int typed "value" field in "AtomicInteger" typed "accessHit" field
                                        + (Integer.SIZE / Byte.SIZE);
*/
                            }

                            //@Override
                            std::auto_ptr<record::NearCacheDataRecord> valueToRecord(
                                    const boost::shared_ptr<serialization::pimpl::Data> &value) {
                                return valueToRecordInternal(value);
                            }

                            //@Override
                            std::auto_ptr<record::NearCacheDataRecord> valueToRecord(
                                    const boost::shared_ptr<V> &value) {
                                const boost::shared_ptr<serialization::pimpl::Data> data = ANCRS::toData(value);
                                return valueToRecordInternal(data);
                            }

                            //@Override
                            boost::shared_ptr<V> recordToValue(const record::NearCacheDataRecord *record) {
                                const boost::shared_ptr<serialization::pimpl::Data> value = record->getValue();
                                if (value.get() == NULL) {
                                    ANCRS::nearCacheStats.incrementMisses();
                                    return NearCache<K, V>::NULL_OBJECT;
                                }
                                return ANCRS::dataToValue(value, (V *)NULL);
                            }

                            //@Override
                            void putToRecord(boost::shared_ptr<record::NearCacheDataRecord> &record,
                                             const boost::shared_ptr<V> &value) {
                                record->setValue(ANCRS::toData(value));
                            }
                        private:
                            std::auto_ptr<record::NearCacheDataRecord> valueToRecordInternal(
                                    const boost::shared_ptr<serialization::pimpl::Data> &data) {
                                int64_t creationTime = util::currentTimeMillis();
                                if (ANCRS::timeToLiveMillis > 0) {
                                    return std::auto_ptr<record::NearCacheDataRecord>(
                                            new record::NearCacheDataRecord(data, creationTime,
                                                                            creationTime + ANCRS::timeToLiveMillis));
                                } else {
                                    return std::auto_ptr<record::NearCacheDataRecord>(
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

