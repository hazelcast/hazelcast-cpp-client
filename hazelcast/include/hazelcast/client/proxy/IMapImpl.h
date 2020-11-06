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
                boost::future<protocol::ClientMessage> remove_interceptor(const std::string &id);

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
                boost::future<protocol::ClientMessage> evict_all();

                /**
                * Removes the specified entry listener
                * Returns silently if there is no such listener added before.
                *
                *
                * @param registrationId id of registered listener
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> remove_entry_listener(boost::uuids::uuid registration_id);

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
                boost::future<bool> is_empty();

                /**
                * Removes all of the mappings from this map (optional operation).
                * The map will be empty after this call returns.
                */
                boost::future<protocol::ClientMessage> clear_data();

            protected:
                IMapImpl(const std::string &instance_name, spi::ClientContext *context);

                boost::future<bool> contains_key(const serialization::pimpl::Data &key);

                boost::future<bool> contains_value(const serialization::pimpl::Data &value);

                boost::future<boost::optional<serialization::pimpl::Data>> get_data(const serialization::pimpl::Data &key);

                boost::future<boost::optional<serialization::pimpl::Data>> remove_data(const serialization::pimpl::Data &key);

                boost::future<bool> remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<protocol::ClientMessage> remove_all(const serialization::pimpl::Data &predicate_data);

                boost::future<protocol::ClientMessage> delete_entry(const serialization::pimpl::Data &key);

                boost::future<bool> try_remove(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout);

                boost::future<bool> try_put(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                            std::chrono::milliseconds timeout);

                boost::future<boost::optional<serialization::pimpl::Data>>
                put_data(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                        std::chrono::milliseconds ttl);

                boost::future<protocol::ClientMessage> put_transient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  std::chrono::milliseconds ttl);

                boost::future<boost::optional<serialization::pimpl::Data>>
                put_if_absent_data(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                std::chrono::milliseconds ttl);

                boost::future<bool> replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &old_value,
                             const serialization::pimpl::Data &new_value);

                boost::future<boost::optional<serialization::pimpl::Data>>
                replace_data(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<protocol::ClientMessage>
                set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                    std::chrono::milliseconds ttl);

                boost::future<protocol::ClientMessage> lock(const serialization::pimpl::Data &key);

                boost::future<protocol::ClientMessage>
                lock(const serialization::pimpl::Data &key, std::chrono::milliseconds lease_time);

                boost::future<bool> is_locked(const serialization::pimpl::Data &key);

                boost::future<bool> try_lock(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout);

                boost::future<bool>
                try_lock(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout,
                        std::chrono::milliseconds lease_time);

                boost::future<protocol::ClientMessage> unlock(const serialization::pimpl::Data &key);

                boost::future<protocol::ClientMessage> force_unlock(const serialization::pimpl::Data &key);

                boost::future<std::string> add_interceptor(const serialization::pimpl::Data &interceptor);

                boost::future<boost::uuids::uuid>
                add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler, bool include_value, int32_t listener_flags);

                boost::future<boost::uuids::uuid>
                add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler, Data &&predicate,
                                 bool include_value, int32_t listener_flags);

                boost::future<boost::uuids::uuid>
                add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler, bool include_value,
                                 Data &&key, int32_t listener_flags);

                boost::future<boost::optional<map::DataEntryView>> get_entry_view_data(const serialization::pimpl::Data &key);

                boost::future<bool> evict(const serialization::pimpl::Data &key);

                boost::future<EntryVector> get_all_data(int partition_id, const std::vector<serialization::pimpl::Data> &keys);

                boost::future<std::vector<serialization::pimpl::Data>> key_set_data();

                boost::future<std::vector<serialization::pimpl::Data>> key_set_data(const serialization::pimpl::Data &predicate);

                boost::future<std::pair<std::vector<serialization::pimpl::Data>, query::anchor_data_list>>
                key_set_for_paging_predicate_data(protocol::codec::holder::paging_predicate_holder const & predicate);

                boost::future<EntryVector> entry_set_data();

                boost::future<EntryVector> entry_set_data(const serialization::pimpl::Data &predicate);

                boost::future<std::pair<EntryVector, query::anchor_data_list>>
                entry_set_for_paging_predicate_data(protocol::codec::holder::paging_predicate_holder const &predicate);

                boost::future<std::vector<serialization::pimpl::Data>> values_data();

                boost::future<std::vector<serialization::pimpl::Data>> values_data(const serialization::pimpl::Data &predicate);

                boost::future<std::pair<std::vector<serialization::pimpl::Data>, query::anchor_data_list>>
                values_for_paging_predicate_data(protocol::codec::holder::paging_predicate_holder const &predicate);

                boost::future<protocol::ClientMessage> add_index_data(const config::index_config &config);

                boost::future<protocol::ClientMessage> put_all_data(int partition_id, const EntryVector &entries);

                boost::future<boost::optional<serialization::pimpl::Data>> execute_on_key_data(const serialization::pimpl::Data &key,
                                                                           const serialization::pimpl::Data &processor);

                boost::future<boost::optional<serialization::pimpl::Data>> submit_to_key_data(const serialization::pimpl::Data &key,
                                                                           const serialization::pimpl::Data &processor);

                boost::future<EntryVector> execute_on_keys_data(const std::vector<serialization::pimpl::Data> &keys,
                                              const serialization::pimpl::Data &processor);

                boost::future<EntryVector> execute_on_entries_data(const serialization::pimpl::Data &entry_processor);

                boost::future<EntryVector>
                execute_on_entries_data(const serialization::pimpl::Data &entry_processor,
                                     const serialization::pimpl::Data &predicate);

                template<typename K, typename V>
                std::pair<size_t, size_t> update_anchor(std::vector<std::pair<K, boost::optional<V>>> &entries,
                                                       query::PagingPredicate<K, V> &predicate,
                                                       query::iteration_type iteration_type) {
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
                set_anchor(std::vector<std::pair<K, boost::optional<V>>> &entries, query::PagingPredicate<K, V> &predicate, int nearest_page) {
                    if (0 == entries.size()) {
                        return;
                    }

                    size_t size = entries.size();
                    size_t pageSize = (size_t) predicate.getPageSize();
                    int page = (int) predicate.getPage();
                    for (size_t i = pageSize; i <= size && nearest_page < page; i += pageSize) {
                        V *value = entries[i-1].second ? new V(entries[i-1].second.value()) : nullptr;
                        std::pair<K *, V *> anchor(new K(entries[i-1].first), value);
                        nearest_page++;
                        predicate.setAnchor((size_t) nearest_page, anchor);
                    }
                }

                void on_initialize() override;

            private:
                std::shared_ptr<impl::ClientLockReferenceIdGenerator> lock_reference_id_generator_;

                class MapEntryListenerWithPredicateMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerWithPredicateMessageCodec(std::string name, bool include_value,
                                                              int32_t listener_flags,
                                                              serialization::pimpl::Data &&predicate);

                    protocol::ClientMessage encode_add_request(bool local_only) const override;

                    protocol::ClientMessage encode_remove_request(boost::uuids::uuid real_registration_id) const override;
                private:
                    std::string name_;
                    bool include_value_;
                    int32_t listener_flags_;
                    serialization::pimpl::Data predicate_;
                };

                class MapEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerMessageCodec(std::string name, bool include_value, int32_t listener_flags);

                    protocol::ClientMessage encode_add_request(bool local_only) const override;

                    protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid real_registration_id) const override;

                private:
                    std::string name_;
                    bool include_value_;
                    int32_t listener_flags_;
                };

                class MapEntryListenerToKeyCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerToKeyCodec(std::string name, bool include_value, int32_t listener_flags,
                                               serialization::pimpl::Data key);

                    protocol::ClientMessage encode_add_request(bool local_only) const override;

                    protocol::ClientMessage encode_remove_request(boost::uuids::uuid real_registration_id) const override;

                private:
                    std::string name_;
                    bool include_value_;
                    int32_t listener_flags_;
                    serialization::pimpl::Data key_;
                };

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                create_map_entry_listener_codec(bool include_value, int32_t listener_flags);

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                create_map_entry_listener_codec(bool include_value, serialization::pimpl::Data &&predicate,
                                            int32_t listener_flags);

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                create_map_entry_listener_codec(bool include_value, int32_t listener_flags,
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

