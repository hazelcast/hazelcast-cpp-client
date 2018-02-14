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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_BASEHEAPNEARCACHERESCORDSTORE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_BASEHEAPNEARCACHERESCORDSTORE_H_

#include <memory>

#include "hazelcast/client/internal/nearcache/impl/store/HeapNearCacheRecordMap.h"
#include "hazelcast/client/internal/nearcache/impl/store/AbstractNearCacheRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/maxsize/EntryCountNearCacheMaxSizeChecker.h"
#include "hazelcast/client/internal/adapter/DataStructureAdapter.h"

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
                                                         const config::NearCacheConfig<K, V> &nearCacheConfig,
                                                         serialization::pimpl::SerializationService &serializationService
                            ) : ANCRS(nearCacheConfig, serializationService) {
                            }

                            //@Override
                            const boost::shared_ptr<R> getRecord(const boost::shared_ptr<KS> &key) {
                                return ANCRS::records->get(key);
                            }

                            //@Override
                            void onEvict(const boost::shared_ptr<KS> &key, const boost::shared_ptr<R> &record,
                                         bool wasExpired) {
                                ANCRS::onEvict(key,
                                               record,
                                               wasExpired);
                                ANCRS::nearCacheStats.decrementOwnedEntryMemoryCost(
                                        ANCRS::getTotalStorageMemoryCost(key, record));
                            }

                            //@Override
                            void doExpiration() {
                                std::vector<std::pair<boost::shared_ptr<KS>, boost::shared_ptr<R> > > entries = ANCRS::records->entrySet();
                                for (typename std::vector<std::pair<boost::shared_ptr<KS>, boost::shared_ptr<R> > >::const_iterator it = entries.begin();
                                     it != entries.end(); ++it) {
                                    const std::pair<boost::shared_ptr<KS>, boost::shared_ptr<R> > &entry = (*it);
                                    const boost::shared_ptr<KS> &key = entry.first;
                                    const boost::shared_ptr<R> &value = entry.second;
                                    if (ANCRS::isRecordExpired(value)) {
                                        ANCRS::remove(key);
                                        ANCRS::onExpire(key, value);
                                    }
                                }
                            }
                        protected:
                            //@Override
                            std::auto_ptr<eviction::MaxSizeChecker> createNearCacheMaxSizeChecker(
                                    const boost::shared_ptr<config::EvictionConfig<K, V> > &evictionConfig,
                                    const config::NearCacheConfig<K, V> &nearCacheConfig) {
                                typename config::EvictionConfig<K, V>::MaxSizePolicy maxSizePolicy = evictionConfig->getMaximumSizePolicy();
                                if (maxSizePolicy == config::EvictionConfig<K, V>::ENTRY_COUNT) {
                                    return std::auto_ptr<eviction::MaxSizeChecker>(
                                            new maxsize::EntryCountNearCacheMaxSizeChecker<K, V, KS, R>(
                                                    evictionConfig->getSize(),
                                                    *ANCRS::records));
                                }
                                std::ostringstream out;
                                out << "Invalid max-size policy " << '(' << (int) maxSizePolicy << ") for " <<
                                nearCacheConfig.getName() << "! Only " <<
                                        (int) config::EvictionConfig<K, V>::ENTRY_COUNT << " is supported.";
                                throw exception::IllegalArgumentException(out.str());
                            }

                            //@Override
                            std::auto_ptr<HeapNearCacheRecordMap<K, V, KS, R> > createNearCacheRecordMap(
                                    const config::NearCacheConfig<K, V> &nearCacheConfig) {
                                return std::auto_ptr<HeapNearCacheRecordMap<K, V, KS, R> >(
                                        new HeapNearCacheRecordMap<K, V, KS, R>(ANCRS::serializationService,
                                                                                DEFAULT_INITIAL_CAPACITY));
                            }

                            //@Override
                            boost::shared_ptr<R> putRecord(const boost::shared_ptr<KS> &key,
                                                           const boost::shared_ptr<R> &record) {
                                boost::shared_ptr<R> oldRecord = ANCRS::records->put(key, record);
                                ANCRS::nearCacheStats.incrementOwnedEntryMemoryCost(
                                        ANCRS::getTotalStorageMemoryCost(key, record));
                                return oldRecord;
                            }

                            //@OverrideR
                            boost::shared_ptr<R> removeRecord(const boost::shared_ptr<KS> &key) {
                                boost::shared_ptr<R> removedRecord = ANCRS::records->remove(key);
                                if (removedRecord.get() != NULL) {
                                    ANCRS::nearCacheStats.decrementOwnedEntryMemoryCost(
                                            ANCRS::getTotalStorageMemoryCost(key, removedRecord));
                                }
                                return removedRecord;
                            }

                            //@Override
                            bool containsRecordKey(const boost::shared_ptr<KS> &key) const {
                                return ANCRS::records->containsKey(key);
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_BASEHEAPNEARCACHERESCORDSTORE_H_ */

