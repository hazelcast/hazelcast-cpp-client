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

#include <hazelcast/client/EntryEvent.h>
#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/query/Predicates.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/protocol/codec/codecs.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class ClientLockReferenceIdGenerator;
        }

        namespace config {
            struct index_config;
        }
        namespace proxy {
            class HAZELCAST_API IMapImpl : public ProxyImpl {
            public:
                /**
                * If this map has a MapStore this method flushes
                * all the local dirty entries by calling MapStore.storeAll() and/or MapStore.deleteAll()
                */
                boost::future<protocol::ClientMessage> flush();

                /**
                * Removes the given interceptor for this map. So it will not intercept operations anymore.
                *
                * @param id registration id of map interceptor
                */
                boost::future<protocol::ClientMessage> removeInterceptor(const std::string &id);

                /**
                * Evicts all keys from this map except locked ones.
                * <p/>
                * If a <tt>MapStore</tt> is defined for this map, deleteAll is <strong>not</strong> called by this method.
                * If you do want to deleteAll to be called use the #clear() method.
                * <p/>
                * The EVICT_ALL event is fired for any registered listeners.
                * See EntryListener#map_evicted(MapEvent)}.
                *
                * @see #clear()
                */
                boost::future<protocol::ClientMessage> evictAll();

                /**
                * Removes the specified entry listener
                * Returns silently if there is no such listener added before.
                *
                *
                * @param registrationId id of registered listener
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> removeEntryListener(boost::uuids::uuid registrationId);

                /**
                * Returns the number of key-value mappings in this map.  If the
                * map contains more than <tt>Integer.MAX_VALUE</tt> elements, returns
                * <tt>Integer.MAX_VALUE</tt>.
                *
                * @return the number of key-value mappings in this map
                */
                boost::future<int> size();

                /**
                * Returns <tt>true</tt> if this map contains no key-value mappings.
                *
                * @return <tt>true</tt> if this map contains no key-value mappings
                */
                boost::future<bool> isEmpty();

                /**
                * Removes all of the mappings from this map (optional operation).
                * The map will be empty after this call returns.
                */
                boost::future<protocol::ClientMessage> clearData();

            protected:
                IMapImpl(const std::string &instanceName, spi::ClientContext *context);

                boost::future<bool> containsKey(const serialization::pimpl::Data &key);

                boost::future<bool> containsValue(const serialization::pimpl::Data &value);

                boost::future<boost::optional<serialization::pimpl::Data>> getData(const serialization::pimpl::Data &key);

                boost::future<boost::optional<serialization::pimpl::Data>> removeData(const serialization::pimpl::Data &key);

                boost::future<bool> remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<protocol::ClientMessage> removeAll(const serialization::pimpl::Data &predicateData);

                boost::future<protocol::ClientMessage> deleteEntry(const serialization::pimpl::Data &key);

                boost::future<bool> tryRemove(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout);

                boost::future<bool> tryPut(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                            std::chrono::steady_clock::duration timeout);

                boost::future<boost::optional<serialization::pimpl::Data>>
                putData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                        std::chrono::steady_clock::duration ttl);

                boost::future<protocol::ClientMessage> putTransient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  std::chrono::steady_clock::duration ttl);

                boost::future<boost::optional<serialization::pimpl::Data>>
                putIfAbsentData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                std::chrono::steady_clock::duration ttl);

                boost::future<bool> replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                             const serialization::pimpl::Data &newValue);

                boost::future<boost::optional<serialization::pimpl::Data>>
                replaceData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<protocol::ClientMessage>
                set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                    std::chrono::steady_clock::duration ttl);

                boost::future<protocol::ClientMessage> lock(const serialization::pimpl::Data &key);

                boost::future<protocol::ClientMessage>
                lock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration leaseTime);

                boost::future<bool> isLocked(const serialization::pimpl::Data &key);

                boost::future<bool> tryLock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout);

                boost::future<bool>
                tryLock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout,
                        std::chrono::steady_clock::duration leaseTime);

                boost::future<protocol::ClientMessage> unlock(const serialization::pimpl::Data &key);

                boost::future<protocol::ClientMessage> forceUnlock(const serialization::pimpl::Data &key);

                boost::future<std::string> addInterceptor(const serialization::pimpl::Data &interceptor);

                boost::future<boost::uuids::uuid>
                addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler, bool includeValue, int32_t listener_flags);

                boost::future<boost::uuids::uuid>
                addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler, Data &&predicate,
                                 bool includeValue, int32_t listener_flags);

                boost::future<boost::uuids::uuid>
                addEntryListener(std::shared_ptr<impl::BaseEventHandler> entryEventHandler, bool includeValue,
                                 Data &&key, int32_t listener_flags);

                boost::future<boost::optional<map::DataEntryView>> getEntryViewData(const serialization::pimpl::Data &key);

                boost::future<bool> evict(const serialization::pimpl::Data &key);

                boost::future<EntryVector> getAllData(int partitionId, const std::vector<serialization::pimpl::Data> &keys);

                boost::future<std::vector<serialization::pimpl::Data>> keySetData();

                boost::future<std::vector<serialization::pimpl::Data>> keySetData(const serialization::pimpl::Data &predicate);

                boost::future<std::pair<std::vector<serialization::pimpl::Data>, query::anchor_data_list>>
                keySetForPagingPredicateData(protocol::codec::holder::paging_predicate_holder const & predicate);

                boost::future<EntryVector> entrySetData();

                boost::future<EntryVector> entrySetData(const serialization::pimpl::Data &predicate);

                boost::future<std::pair<EntryVector, query::anchor_data_list>>
                entrySetForPagingPredicateData(protocol::codec::holder::paging_predicate_holder const &predicate);

                boost::future<std::vector<serialization::pimpl::Data>> valuesData();

                boost::future<std::vector<serialization::pimpl::Data>> valuesData(const serialization::pimpl::Data &predicate);

                boost::future<std::pair<std::vector<serialization::pimpl::Data>, query::anchor_data_list>>
                valuesForPagingPredicateData(protocol::codec::holder::paging_predicate_holder const &predicate);

                boost::future<protocol::ClientMessage> addIndexData(const config::index_config &config);

                boost::future<protocol::ClientMessage> putAllData(int partitionId, const EntryVector &entries);

                boost::future<boost::optional<serialization::pimpl::Data>> executeOnKeyData(const serialization::pimpl::Data &key,
                                                                           const serialization::pimpl::Data &processor);

                boost::future<boost::optional<serialization::pimpl::Data>> submitToKeyData(const serialization::pimpl::Data &key,
                                                                           const serialization::pimpl::Data &processor);

                boost::future<EntryVector> executeOnKeysData(const std::vector<serialization::pimpl::Data> &keys,
                                              const serialization::pimpl::Data &processor);

                boost::future<EntryVector> executeOnEntriesData(const serialization::pimpl::Data &entryProcessor);

                boost::future<EntryVector>
                executeOnEntriesData(const serialization::pimpl::Data &entryProcessor,
                                     const serialization::pimpl::Data &predicate);

                template<typename K, typename V>
                std::pair<size_t, size_t> updateAnchor(std::vector<std::pair<K, boost::optional<V>>> &entries,
                                                       query::PagingPredicate<K, V> &predicate,
                                                       query::IterationType iterationType) {
                    if (0 == entries.size()) {
                        return std::pair<size_t, size_t>(0, 0);
                    }

                    const std::pair<size_t, std::pair<K *, V *> > *nearestAnchorEntry = predicate.getNearestAnchorEntry();
                    int nearestPage = (NULL == nearestAnchorEntry ? -1 : (int) nearestAnchorEntry->first);
                    size_t page = predicate.getPage();
                    size_t pageSize = predicate.getPageSize();
                    size_t begin = pageSize * (page - nearestPage - 1);
                    size_t size = entries.size();
                    if (begin > size) {
                        return std::pair<size_t, size_t>(0, 0);
                    }
                    size_t end = begin + pageSize;
                    if (end > size) {
                        end = size;
                    }

                    setAnchor(entries, predicate, nearestPage);

                    return std::make_pair(begin, end);
                }

                template<typename K, typename V>
                static void
                setAnchor(std::vector<std::pair<K, boost::optional<V>>> &entries, query::PagingPredicate<K, V> &predicate, int nearestPage) {
                    if (0 == entries.size()) {
                        return;
                    }

                    size_t size = entries.size();
                    size_t pageSize = (size_t) predicate.getPageSize();
                    int page = (int) predicate.getPage();
                    for (size_t i = pageSize; i <= size && nearestPage < page; i += pageSize) {
                        V *value = entries[i-1].second ? new V(entries[i-1].second.value()) : nullptr;
                        std::pair<K *, V *> anchor(new K(entries[i-1].first), value);
                        nearestPage++;
                        predicate.setAnchor((size_t) nearestPage, anchor);
                    }
                }

                void onInitialize() override;

            private:
                std::shared_ptr<impl::ClientLockReferenceIdGenerator> lock_reference_id_generator_;

                class MapEntryListenerWithPredicateMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerWithPredicateMessageCodec(std::string name, bool includeValue,
                                                              int32_t listenerFlags,
                                                              serialization::pimpl::Data &&predicate);

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override;

                    protocol::ClientMessage encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override;
                private:
                    std::string name_;
                    bool include_value_;
                    int32_t listener_flags_;
                    serialization::pimpl::Data predicate_;
                };

                class MapEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerMessageCodec(std::string name, bool includeValue, int32_t listenerFlags);

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override;

                    protocol::ClientMessage
                    encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override;

                private:
                    std::string name_;
                    bool include_value_;
                    int32_t listener_flags_;
                };

                class MapEntryListenerToKeyCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerToKeyCodec(std::string name, bool includeValue, int32_t listenerFlags,
                                               serialization::pimpl::Data key);

                    protocol::ClientMessage encodeAddRequest(bool localOnly) const override;

                    protocol::ClientMessage encodeRemoveRequest(boost::uuids::uuid realRegistrationId) const override;

                private:
                    std::string name_;
                    bool include_value_;
                    int32_t listener_flags_;
                    serialization::pimpl::Data key_;
                };

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags);

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &&predicate,
                                            int32_t listenerFlags);

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                createMapEntryListenerCodec(bool includeValue, int32_t listenerFlags,
                                            serialization::pimpl::Data &&key);

                template<typename ResultVector>
                std::pair<ResultVector, query::anchor_data_list>
                get_paging_predicate_response(boost::future<protocol::ClientMessage> f) const {
                    auto msg = f.get();

                    auto entries_data = msg.get_first_var_sized_field<ResultVector>();
                    auto anchor_list = msg.get<query::anchor_data_list>();
                    return std::make_pair(std::move(entries_data.value()), std::move(anchor_list));
                }
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

