/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEOBJECTRESCORDSTORE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEOBJECTRESCORDSTORE_H_

#include "hazelcast/client/internal/nearcache/impl/store/BaseHeapNearCacheRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/record/NearCacheObjectRecord.h"

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
                        class NearCacheObjectRecordStore
                                : public BaseHeapNearCacheRecordStore<K, V, KS, record::NearCacheObjectRecord<V> > {
                        public:
                            typedef AbstractNearCacheRecordStore <K, V, KS, record::NearCacheObjectRecord<V>, HeapNearCacheRecordMap<K, V, KS, record::NearCacheObjectRecord<V> > > ANCRS;

                            NearCacheObjectRecordStore(const std::string &name,
                                                       const config::NearCacheConfig<K, V> &config,
                                                       serialization::pimpl::SerializationService &ss)
                                    : BaseHeapNearCacheRecordStore<K, V, serialization::pimpl::Data, record::NearCacheObjectRecord<V> >(
                                    name, config, ss) {
                            }

                        protected:
                            //@Override
/*
                        int64_t getKeyStorageMemoryCost(K key) const {
                            // memory cost for "OBJECT" in memory format is totally not supported, so just return zero
                            return 0L;
                            }

                            //@Override
                        int64_t getRecordStorageMemoryCost(NearCacheObjectRecord<V> record) {
                            // memory cost for "OBJECT" in memory format is totally not supported, so just return zero
                            return 0L;
                            }
*/
                            //@Override
                            std::auto_ptr<record::NearCacheObjectRecord<V> > valueToRecord(
                                    const boost::shared_ptr<serialization::pimpl::Data> &valueData) {
                                boost::shared_ptr<serialization::pimpl::Data> data = boost::const_pointer_cast<serialization::pimpl::Data>(
                                        valueData);
                                const boost::shared_ptr<V> value = ANCRS::toValue(data);
                                return valueToRecordInternal(value);
                            }

                            //@Override
                            std::auto_ptr<record::NearCacheObjectRecord<V> > valueToRecord(
                                    const boost::shared_ptr<V> &value) {
                                return valueToRecordInternal(value);
                            }

                            //@Override
                            boost::shared_ptr<V> recordToValue(const record::NearCacheObjectRecord<V> *record) {
                                const boost::shared_ptr<V> value = record->getValue();
                                if (value.get() == NULL) {
/*
                                    nearCacheStats.incrementMisses();
*/
                                    return NearCache<K, V>::NULL_OBJECT;
                                }
                                return value;
                            }

                            //@Override
                            void putToRecord(boost::shared_ptr<record::NearCacheObjectRecord<V> > &record,
                                             const boost::shared_ptr<V> &value) {
                                record->setValue(value);
                            }

                        private:
                            std::auto_ptr<record::NearCacheObjectRecord<V> > valueToRecordInternal(
                                    const boost::shared_ptr<V> &value) {
                                int64_t creationTime = util::currentTimeMillis();
                                if (ANCRS::timeToLiveMillis > 0) {
                                    return std::auto_ptr<record::NearCacheObjectRecord<V> >(
                                            new record::NearCacheObjectRecord<V>(value, creationTime,
                                                                                 creationTime +
                                                                                 ANCRS::timeToLiveMillis));
                                } else {
                                    return std::auto_ptr<record::NearCacheObjectRecord<V> >(
                                            new record::NearCacheObjectRecord<V>(value, creationTime,
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEOBJECTRESCORDSTORE_H_ */

