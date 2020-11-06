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

#include <string>
#include <memory>

#include "hazelcast/client/proxy/ReplicatedMapImpl.h"
#include "hazelcast/client/entry_listener.h"
#include "hazelcast/client/entry_event.h"
#include "hazelcast/client/map_event.h"
#include "hazelcast/client/query/Predicates.h"
#include "hazelcast/logger.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
         * <p>A ReplicatedMap is a map-like data structure with weak consistency
         * and values locally stored on every node of the cluster. </p>
         * <p>Whenever a value is written asynchronously, the new value will be internally
         * distributed to all existing cluster members, and eventually every node will have
         * the new value.</p>
         * <p>When a new node joins the cluster, the new node initially will request existing
         * values from older nodes and replicate them locally.</p>
         *
         *
         */
        class HAZELCAST_API replicated_map : public proxy::ReplicatedMapImpl {
            friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:replicatedMapService";
            /**
             * <p>Associates a given value to the specified key and replicates it to the
             * cluster. If there is an old value, it will be replaced by the specified
             * one and returned from the call.</p>
             * <p>In addition, you have to specify a ttl in milliseconds
             * to define when the value is outdated and thus should be removed from the
             * replicated map.</p>
             *
             * @param key      key with which the specified value is to be associated.
             * @param value    value to be associated with the specified key.
             * @param ttl      ttl to be associated with the specified key-value pair.
             * @return the previous value associated with <tt>key</tt>, or
             *         <tt>empty</tt> if there was no mapping for <tt>key</tt>.
             */
            template<typename K, typename V, typename R = V>
            boost::future<boost::optional<R>> put(const K &key, const V &value, std::chrono::milliseconds ttl) {
                return to_object<R>(put_data(to_data(key), to_data(value), ttl));
            }

            /**
            * Copies all of the mappings from the specified map to this map
            * (optional operation).  The effect of this call is equivalent to that
            * of calling put(k, v) on this map once
            * for each mapping from key <tt>k</tt> to value <tt>v</tt> in the
            * specified map.  The behavior of this operation is undefined if the
            * specified map is modified while the operation is in progress.
            *
            * @param entries mappings to be stored in this map
            */
            template<typename K, typename V>
            boost::future<void> put_all(const std::unordered_map<K, V> &entries) {
                return put_all_data(to_data_entries(entries));
            }

            /**
             * Adds an entry listener for this map. The listener will be notified
             * for all map add/remove/update/evict events.
             *
             * @param listener entry listener
             */
            boost::future<boost::uuids::uuid> add_entry_listener(entry_listener &&listener) {
                return proxy::ReplicatedMapImpl::add_entry_listener(
                        std::shared_ptr<impl::BaseEventHandler>(
                                new EntryEventHandler<protocol::codec::replicatedmap_addentrylistener_handler>(get_name(), get_context().get_client_cluster_service(),
                                        get_context().get_serialization_service(), std::move(listener), get_context().get_logger())));
            }

            /**
             * Adds the specified entry listener for the specified key.
             * The listener will be notified for all
             * add/remove/update/evict events of the specified key only.
             * <p/>
             * <p><b>Warning:</b></p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of the binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt>.
             *
             * @param listener the entry listener to add
             * @param key      the key to listen to
             */
            template<typename K>
            typename std::enable_if<!std::is_base_of<query::Predicate, K>::value, boost::future<boost::uuids::uuid>>::type
            add_entry_listener(entry_listener &&listener, const K &key) {
                return proxy::ReplicatedMapImpl::add_entry_listener_to_key(
                        std::shared_ptr<impl::BaseEventHandler>(
                                new EntryEventHandler<protocol::codec::replicatedmap_addentrylistenertokey_handler>(get_name(), get_context().get_client_cluster_service(),
                                                      get_context().get_serialization_service(), std::move(listener),
                                                      get_context().get_logger())), to_data(key));
            }

            /**
             * Adds an continuous entry listener for this map. The listener will be notified
             * for map add/remove/update/evict events filtered by the given predicate.
             *
             * @param listener  the entry listener to add
             * @param predicate the predicate for filtering entries
             */
            template<typename P>
            typename std::enable_if<std::is_base_of<query::Predicate, P>::value, boost::future<boost::uuids::uuid>>::type
            add_entry_listener(entry_listener &&listener, const P &predicate) {
                return proxy::ReplicatedMapImpl::add_entry_listener(
                        std::shared_ptr<impl::BaseEventHandler>(
                                new EntryEventHandler<protocol::codec::replicatedmap_addentrylistenerwithpredicate_handler>(get_name(), get_context().get_client_cluster_service(),
                                                      get_context().get_serialization_service(), std::move(listener),
                                                      get_context().get_logger())), to_data(predicate));
            }

            /**
             * Adds an continuous entry listener for this map. The listener will be notified
             * for map add/remove/update/evict events filtered by the given predicate.
             *
             * @param listener  the entry listener
             * @param predicate the predicate for filtering entries
             * @param key       the key to listen to
             */
            template<typename K, typename P>
            typename std::enable_if<std::is_base_of<query::Predicate, P>::value, boost::future<boost::uuids::uuid>>::type
            add_entry_listener(entry_listener &&listener, const P &predicate, const K &key) {
                return proxy::ReplicatedMapImpl::add_entry_listener(
                        std::shared_ptr<impl::BaseEventHandler>(
                                new EntryEventHandler<protocol::codec::replicatedmap_addentrylistenertokeywithpredicate_handler>(get_name(), get_context().get_client_cluster_service(),
                                                      get_context().get_serialization_service(), std::move(listener),
                                                      get_context().get_logger())), to_data(key), to_data(predicate));
            }

            /**
             * Due to the lazy nature of the returned array, changes to the map (addition,
             * removal, update) might be reflected on the collection.<br/>
             * Changes on the map are <b>NOT</b> reflected on the collection on the <b>CLIENT</b> or vice versa.
             * The order of the elements is not guaranteed due to the internal
             * asynchronous replication behavior. If a specific order is needed, use
             * {@link #values(Comparator)} to force reordering of the
             * elements before returning.<br/>
             * Changes to any returned object are <b>NOT</b> replicated back to other
             * members.
             *
             * @return A collection view of the values contained in this map.
             */
            template<typename V>
            boost::future<std::vector<V>> values() {
                return to_object_vector<V>(values_data());
            }

            /**
             * Returns a view of the mappings contained in this map.<br/>
             * Due to the lazy nature of the returned array, changes to the map (addition,
             * removal, update) might be reflected on the array.<br/>
             * Changes on the map are <b>NOT</b> reflected on the set on the <b>CLIENT</b> or vice versa.
             * The order of the elements is not guaranteed due to the internal
             * asynchronous replication behavior.<br/>
             * Changes to any returned object are <b>NOT</b> replicated back to other
             * members.
             *
             * @return view of the mappings contained in this map.
             */
            template<typename K, typename V>
            boost::future<std::vector<std::pair<K, V>>> entry_set() {
                return to_entry_object_vector<K,V>(entry_set_data());
            }

            /**
             * Returns a view of the keys contained in this map.<br/>
             * Due to the lazy nature of the returned array, changes to the map (addition,
             * removal, update) might be reflected on the array.<br/>
             * Changes on the map are <b>NOT</b> reflected on the set on the <b>CLIENT</b> or vice versa.
             * The order of the elements is not guaranteed due to the internal
             * asynchronous replication behavior.<br/>
             * Changes to any returned object are <b>NOT</b> replicated back to other
             * members.
             *
             * @return The keys contained in this map.
             */
            template<typename K>
            boost::future<std::vector<K>> key_set() {
                return to_object_vector<K>(key_set_data());
            }
            
            /**
             *
             * @param key The key to be checked for existence
             * @return true if the entry with the key exist in the replicated map.
             */
            template<typename K>
            boost::future<bool> contains_key(const K &key) {
                return contains_key_data(to_data(key));
            }

            /**
             *
             * @param value The value to check in the replicated map for existence.
             * @return true if the value exist in the replicated map.
             */
            template<typename V>
            boost::future<bool> contains_value(const V &value) {
                return contains_value_data(to_data(value));
            }

            /**
             *
             * @param key The key to be used to query from replicated map.
             * @return The value of the key if the key exist, null pointer otherwise.
             */
            template<typename K, typename V>
            boost::future<boost::optional<V>> get(const K &key) {
                return to_object<V>(get_data(to_data(key)));
            }

            /**
             *
             * @param key The key to be put into the replicated map.
             * @param value The value of the key
             * @return The previous value if the key existed in the map or null pointer otherwise.
             */
            template<typename K, typename V, typename R = V>
            boost::future<boost::optional<R>> put(const K &key, const V &value) {
                return put<K, V, R>(key, value, std::chrono::milliseconds(0));
            }

            /**
             *
             * @param key The key of the entry to be removed.
             * @return The value associated with the removed key.
             */
            template<typename K, typename V>
            boost::future<boost::optional<V>> remove(const K &key) {
                return to_object<V>(remove_data(to_data(key)));
            }
        private:
            replicated_map(const std::string &object_name, spi::ClientContext *context) : proxy::ReplicatedMapImpl(
                    SERVICE_NAME, object_name, context) {
            }

            template<typename HANDLER>
            class EntryEventHandler : public HANDLER {
            public:
                EntryEventHandler(const std::string &instance_name, spi::impl::ClientClusterServiceImpl &cluster_service,
                                  serialization::pimpl::SerializationService &serialization_service,
                                  entry_listener &&listener, logger &lg)
                        : instance_name_(instance_name), cluster_service_(cluster_service), serialization_service_(serialization_service)
                        , listener_(std::move(listener)), logger_(lg) {}

                void handle_entry(const boost::optional<data> &key, const boost::optional<data> &value,
                                  const boost::optional<data> &old_value, const boost::optional<data> &merging_value,
                                  int32_t event_type, boost::uuids::uuid uuid,
                                  int32_t number_of_affected_entries) override {
                    if (event_type == static_cast<int32_t>(entry_event::type::CLEAR_ALL)) {
                        fire_map_wide_event(key, value, old_value, merging_value, event_type, uuid, number_of_affected_entries);
                        return;
                    }

                    fire_entry_event(key, value, old_value, merging_value, event_type, uuid, number_of_affected_entries);
                }

            private:
                void fire_map_wide_event(const boost::optional<data> &key, const boost::optional<data> &value,
                                      const boost::optional<data> &old_value, const boost::optional<data> &merging_value,
                                      int32_t event_type, boost::uuids::uuid uuid,
                                      int32_t number_of_affected_entries) {
                    auto member = cluster_service_.get_member(uuid);
                    auto mapEventType = static_cast<entry_event::type>(event_type);
                    map_event mapEvent(std::move(member).value(), mapEventType, instance_name_, number_of_affected_entries);
                    listener_.map_cleared_(std::move(mapEvent));
                }

                void fire_entry_event(const boost::optional<data> &key, const boost::optional<data> &value,
                                    const boost::optional<data> &old_value, const boost::optional<data> &merging_value,
                                    int32_t event_type, boost::uuids::uuid uuid,
                                    int32_t number_of_affected_entries) {
                    typed_data eventKey, val, oldVal, mergingVal;
                    if (value) {
                        val = typed_data(std::move(*value), serialization_service_);
                    }
                    if (old_value) {
                        oldVal = typed_data(std::move(*old_value), serialization_service_);
                    }
                    if (merging_value) {
                        mergingVal = typed_data(std::move(*merging_value), serialization_service_);
                    }
                    if (key) {
                        eventKey = typed_data(std::move(*key), serialization_service_);
                    }
                    auto m = cluster_service_.get_member(uuid);
                    if (!m.has_value()) {
                        m = member(uuid);
                    }
                    auto type = static_cast<entry_event::type>(event_type);
                    entry_event entryEvent(instance_name_, std::move(m.value()), type, std::move(eventKey), std::move(val),
                                           std::move(oldVal), std::move(mergingVal));
                    switch(type) {
                        case entry_event::type::ADDED:
                            listener_.added_(std::move(entryEvent));
                            break;
                        case entry_event::type::REMOVED:
                            listener_.removed_(std::move(entryEvent));
                            break;
                        case entry_event::type::UPDATED:
                            listener_.updated_(std::move(entryEvent));
                            break;
                        case entry_event::type::EVICTED:
                            listener_.evicted_(std::move(entryEvent));
                            break;
                        default:
                            HZ_LOG(logger_, warning, 
                                boost::str(boost::format("Received unrecognized event with type: %1% "
                                                         "Dropping the event!!!")
                                                         % static_cast<int>(type))
                            );
                    }
                }
            private:
                const std::string& instance_name_;
                spi::impl::ClientClusterServiceImpl &cluster_service_;
                serialization::pimpl::SerializationService& serialization_service_;
                entry_listener listener_;
                logger &logger_;
            };
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
