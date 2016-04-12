/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_IMAP_IMPL
#define HAZELCAST_IMAP_IMPL

#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/query/Predicate.h"
#include "hazelcast/client/protocol/codec/MapExecuteWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnKeyCodec.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnAllKeysCodec.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/map/DataEntryView.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class IdentifiedDataSerializable;
        }
        namespace proxy {
            class HAZELCAST_API IMapImpl : public ProxyImpl {
            protected:
                IMapImpl(const std::string& instanceName, spi::ClientContext *context);

                bool containsKey(const serialization::pimpl::Data& key);

                bool containsValue(const serialization::pimpl::Data& value);

                std::auto_ptr<serialization::pimpl::Data> getData(const serialization::pimpl::Data& key);

                std::auto_ptr<serialization::pimpl::Data> putData(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                std::auto_ptr<serialization::pimpl::Data> removeData(const serialization::pimpl::Data& key);

                bool remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                void deleteEntry(const serialization::pimpl::Data& key);

                void flush();

                bool tryRemove(const serialization::pimpl::Data& key, long timeoutInMillis);

                bool tryPut(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long timeoutInMillis);

                std::auto_ptr<serialization::pimpl::Data> putData(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis);

                void putTransient(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis);

                std::auto_ptr<serialization::pimpl::Data> putIfAbsentData(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis);

                bool replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& oldValue, const serialization::pimpl::Data& newValue);

                std::auto_ptr<serialization::pimpl::Data> replaceData(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                void set(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttl);

                void lock(const serialization::pimpl::Data& key);

                void lock(const serialization::pimpl::Data& key, long leaseTime);

                bool isLocked(const serialization::pimpl::Data& key);

                bool tryLock(const serialization::pimpl::Data& key, long timeInMillis);

                void unlock(const serialization::pimpl::Data& key);

                void forceUnlock(const serialization::pimpl::Data& key);

                std::string addInterceptor(serialization::Portable &interceptor);

                std::string addInterceptor(serialization::IdentifiedDataSerializable &interceptor);

                void removeInterceptor(const std::string& id);

                std::string addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue);

                std::string addEntryListener(impl::BaseEventHandler *entryEventHandler, const query::Predicate &predicate, bool includeValue);

                bool removeEntryListener(const std::string& registrationId);

                std::string addEntryListener(impl::BaseEventHandler *entryEventHandler, const serialization::pimpl::Data& key, bool includeValue);

                std::auto_ptr<map::DataEntryView> getEntryViewData(const serialization::pimpl::Data& key);

                bool evict(const serialization::pimpl::Data& key);

                void evictAll();

                EntryVector getAllData(const std::vector<serialization::pimpl::Data>& keys);

                std::vector<serialization::pimpl::Data> keySetData();

                std::vector<serialization::pimpl::Data> keySetData(
                        const serialization::IdentifiedDataSerializable &predicate);

                std::vector<serialization::pimpl::Data> keySetForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate);

                EntryVector entrySetData();

                EntryVector entrySetData(const serialization::IdentifiedDataSerializable &predicate);

                EntryVector entrySetForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate);

                std::vector<serialization::pimpl::Data> valuesData();

                std::vector<serialization::pimpl::Data> valuesData(
                        const serialization::IdentifiedDataSerializable &predicate);

                EntryVector valuesForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate);

                void addIndex(const std::string& attribute, bool ordered);

                int size();

                bool isEmpty();

                void putAll(const EntryVector& entries);

                void clear();

                template<typename KEY, typename ENTRYPROCESSOR>
                std::auto_ptr<serialization::pimpl::Data> executeOnKeyData(const KEY& key, ENTRYPROCESSOR &entryProcessor) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data processor = toData(entryProcessor); 
                    int partitionId = getPartitionId(keyData);

                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapExecuteOnKeyCodec::RequestParameters::encode(getName(), processor, keyData, util::getThreadId());

                    return invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapExecuteOnKeyCodec::ResponseParameters>(request, partitionId);
                }

                template<typename ENTRYPROCESSOR>
                EntryVector executeOnEntriesData(ENTRYPROCESSOR &entryProcessor) {
                    serialization::pimpl::Data processor = toData<ENTRYPROCESSOR>(entryProcessor);

                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapExecuteOnAllKeysCodec::RequestParameters::encode(getName(), processor);

                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response =
                            invokeAndGetResult<std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >,
                                    protocol::codec::MapExecuteOnAllKeysCodec::ResponseParameters>(request);

                    return response;
                }

                template<typename ENTRYPROCESSOR>
                EntryVector executeOnEntriesData(ENTRYPROCESSOR &entryProcessor, const query::Predicate &predicate) {
                    serialization::pimpl::Data processor = toData<ENTRYPROCESSOR>(entryProcessor);
                    serialization::pimpl::Data predData = toData<serialization::IdentifiedDataSerializable>(predicate);
                    std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapExecuteWithPredicateCodec::RequestParameters::encode(getName(), processor, predData);

                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > response =
                            invokeAndGetResult<std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >,
                                    protocol::codec::MapExecuteWithPredicateCodec::ResponseParameters>(request);

                    return response;
                }

                template <typename K, typename V, typename Compare>
                std::pair<int, int> getSortedQueryResultSet(std::vector<std::pair<K, V> > &entries, query::PagingPredicate<K, V, Compare> &predicate, query::IterationType iterationType) {
                    if (entries.empty()) {
                        return std::pair<int, int>(0, 0);
                    }

                    sort<K, V, Compare>(entries, predicate, iterationType);

                    const std::pair<int, std::pair<K, V> > *nearestAnchorEntry = predicate.getNearestAnchorEntry();
                    int nearestPage = (NULL == nearestAnchorEntry ? -1 : nearestAnchorEntry->first);
                    int page = predicate.getPage();
                    int pageSize = predicate.getPageSize();
                    int begin = pageSize * (page - nearestPage - 1);
                    int size = (int)entries.size();
                    if (begin > size) {
                        return std::pair<int, int>(0, 0);
                    }
                    int end = begin + pageSize;
                    if (end > size) {
                        end = size;
                    }

                    setAnchor(entries, predicate, nearestPage);

                    return std::pair<int, int>(begin, end);
                }

                template <typename K, typename V, typename Compare>
                void sort(std::vector<std::pair<K, V> > &entries, const query::PagingPredicate<K, V, Compare> &predicate, query::IterationType iterationType) const {
                    if (NULL != predicate.getComparator()) {
                        std::sort<typename std::vector<std::pair<K, V> >::iterator, Compare>(entries.begin(), entries.end(), *predicate.getComparator());
                    } else {
                        switch (iterationType) {
                            case query::VALUE:
                            {
                                query::ValueComparator<K, V> comp;
                                std::sort<typename std::vector<std::pair<K, V> >::iterator, query::ValueComparator<K, V> >(entries.begin(), entries.end(), comp);
                                break;
                            }
                            case query::KEY:
                            case query::ENTRY:
                            {
                                query::KeyComparator<K, V> comp;
                                std::sort<typename std::vector<std::pair<K, V> >::iterator, query::KeyComparator<K, V> >(entries.begin(), entries.end(), comp);
                                break;
                            }
                        }
                    }
                }

                template <typename K, typename V, typename Compare>
                static void setAnchor(std::vector<std::pair<K, V> > &entries, query::PagingPredicate<K, V, Compare> &predicate, int nearestPage) {
                    if (entries.empty()) {
                        return;
                    }

                    size_t size = entries.size();
                    size_t pageSize = (size_t)predicate.getPageSize();
                    for (size_t i = pageSize; i <= size; i += pageSize) {
                        const std::pair<K, V> &anchor = entries[i - 1];
                        nearestPage++;
                        predicate.setAnchor(nearestPage, anchor);
                    }
                }
            };
        }
    }
}

#endif /* HAZELCAST_IMAP_IMPL */

