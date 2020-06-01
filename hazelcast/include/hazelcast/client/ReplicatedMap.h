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
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/query/Predicates.h"

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
        class HAZELCAST_API ReplicatedMap : public proxy::ReplicatedMapImpl {
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
             */
            template<typename K, typename V>
            boost::future<boost::optional<V>> put(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                return toObject<V>(putData(toData(key), toData(value), ttl));
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
            boost::future<void> putAll(const std::unordered_map<K, V> &entries) {
                return putAllData(toDataEntries(entries));
            }

            /**
             * Adds an entry listener for this map. The listener will be notified
             * for all map add/remove/update/evict events.
             *
             * @param listener entry listener
             */
            template<typename Listener>
            boost::future<std::string> addEntryListener(Listener &&listener) {
                return proxy::ReplicatedMapImpl::addEntryListener(
                        std::unique_ptr<impl::BaseEventHandler>(
                                new EntryEventHandler<Listener>(getName(), getContext().getClientClusterService(),
                                        getContext().getSerializationService(), std::forward<Listener>(listener), getContext().getLogger())));
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
            template<typename Listener, typename K>
            boost::future<std::string> addEntryListenerForKey(Listener &&listener, const K &key) {
                return proxy::ReplicatedMapImpl::addEntryListenerToKey(
                        std::unique_ptr<impl::BaseEventHandler>(
                                new EntryEventHandler<Listener>(getName(), getContext().getClientClusterService(),
                                                                getContext().getSerializationService(), std::forward<Listener>(listener),
                                                                getContext().getLogger())), toData(key));
            }

            /**
             * Adds an continuous entry listener for this map. The listener will be notified
             * for map add/remove/update/evict events filtered by the given predicate.
             *
             * @param listener  the entry listener to add
             * @param predicate the predicate for filtering entries
             */
            template<typename Listener, typename P>
            boost::future<std::string> addEntryListener(Listener &&listener, const P &predicate) {
                return proxy::ReplicatedMapImpl::addEntryListener(
                        std::unique_ptr<impl::BaseEventHandler>(
                                new EntryEventHandler<Listener>(getName(), getContext().getClientClusterService(),
                                                                getContext().getSerializationService(), std::forward<Listener>(listener),
                                                                getContext().getLogger())), toData(predicate));
            }

            /**
             * Adds an continuous entry listener for this map. The listener will be notified
             * for map add/remove/update/evict events filtered by the given predicate.
             *
             * @param listener  the entry listener
             * @param predicate the predicate for filtering entries
             * @param key       the key to listen to
             */
            template<typename Listener, typename K, typename P>
            boost::future<std::string> addEntryListener(Listener &&listener, const P &predicate, const K &key) {
                return proxy::ReplicatedMapImpl::addEntryListener(
                        std::unique_ptr<impl::BaseEventHandler>(
                                new EntryEventHandler<Listener>(getName(), getContext().getClientClusterService(),
                                                                getContext().getSerializationService(), std::forward<Listener>(listener),
                                                                getContext().getLogger())), toData(key), toData(predicate));
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
                return toObjectVector<V>(valuesData());
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
            boost::future<std::vector<std::pair<K, V>>> entrySet() {
                return toEntryObjectVector<K,V>(entrySetData());
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
            boost::future<std::vector<K>> keySet() {
                return toObjectVector<K>(keySetData());
            }
            
            /**
             *
             * @param key The key to be checked for existence
             * @return true if the entry with the key exist in the replicated map.
             */
            template<typename K>
            boost::future<bool> containsKey(const K &key) {
                return containsKeyData(toData(key));
            }

            /**
             *
             * @param value The value to check in the replicated map for existence.
             * @return true if the value exist in the replicated map.
             */
            template<typename V>
            boost::future<bool> containsValue(const V &value) {
                return containsValueData(toData(value));
            }

            /**
             *
             * @param key The key to be used to query from replicated map.
             * @return The value of the key if the key exist, null pointer otherwise.
             */
            template<typename K, typename V>
            boost::future<boost::optional<V>> get(const K &key) {
                return toObject<V>(getData(toData(key)));
            }

            /**
             *
             * @param key The key to be put into the replicated map.
             * @param value The value of the key
             * @return The previous value if the key existed in the map or null pointer otherwise.
             */
            template<typename K, typename V>
            boost::future<boost::optional<V>> put(const K &key, const V &value) {
                return put(key, value, std::chrono::milliseconds(0));
            }

            /**
             *
             * @param key The key of the entry to be removed.
             * @return The value associated with the removed key.
             */
            template<typename K, typename V>
            boost::future<boost::optional<V>> remove(const K &key) {
                return toObject<V>(removeData(toData(key)));
            }
        private:
            ReplicatedMap(const std::string &objectName, spi::ClientContext *context) : proxy::ReplicatedMapImpl(
                    SERVICE_NAME, objectName, context) {
            }

            template<typename Listener>
            class EntryEventHandler : public protocol::codec::ReplicatedMapAddEntryListenerCodec::AbstractEventHandler {
            public:
                EntryEventHandler(const std::string &instanceName, spi::ClientClusterService &clusterService,
                                  serialization::pimpl::SerializationService &serializationService,
                                  Listener &&listener, util::ILogger &log)
                        : instanceName(instanceName), clusterService(clusterService), serializationService(serializationService)
                        , listener(listener), logger(log) {}

                virtual void handleEntryEventV10(std::unique_ptr<serialization::pimpl::Data> &key,
                                                 std::unique_ptr<serialization::pimpl::Data> &value,
                                                 std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                                 std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                                 const int32_t &eventType, const std::string &uuid,
                                                 const int32_t &numberOfAffectedEntries) {
                    if (eventType == static_cast<int32_t>(EntryEvent::type::CLEAR_ALL)) {
                        fireMapWideEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }

                    fireEntryEvent(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                }

            private:
                void fireMapWideEvent(std::unique_ptr<serialization::pimpl::Data> &key,
                                      std::unique_ptr<serialization::pimpl::Data> &value,
                                      std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                      std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                      const int32_t &eventType, const std::string &uuid,
                                      const int32_t &numberOfAffectedEntries) {
                    auto member = clusterService.getMember(uuid);
                    auto mapEventType = static_cast<EntryEvent::type>(eventType);
                    MapEvent mapEvent(std::move(member).value(), mapEventType, instanceName, numberOfAffectedEntries);
                    listener.mapCleared(mapEvent);
                }

                void fireEntryEvent(std::unique_ptr<serialization::pimpl::Data> &key,
                                    std::unique_ptr<serialization::pimpl::Data> &value,
                                    std::unique_ptr<serialization::pimpl::Data> &oldValue,
                                    std::unique_ptr<serialization::pimpl::Data> &mergingValue,
                                    const int32_t &eventType, const std::string &uuid,
                                    const int32_t &numberOfAffectedEntries) {
                    TypedData eventKey, val, oldVal, mergingVal;
                    if (value) {
                        val = TypedData(std::move(*value), serializationService);
                    }
                    if (oldValue) {
                        oldVal = TypedData(std::move(*oldValue), serializationService);
                    }
                    if (mergingValue) {
                        mergingVal = TypedData(std::move(*mergingValue), serializationService);
                    }
                    if (key) {
                        eventKey = TypedData(std::move(*key), serializationService);
                    }
                    auto member = clusterService.getMember(uuid);
                    if (!member.has_value()) {
                        member = Member(uuid);
                    }
                    auto type = static_cast<EntryEvent::type>(eventType);
                    EntryEvent entryEvent(instanceName, member.value(), type, std::move(eventKey), std::move(val),
                                          std::move(oldVal), std::move(mergingVal));
                    switch(type) {
                        case EntryEvent::type::ADDED:
                            listener.entryAdded(entryEvent);
                            break;
                        case EntryEvent::type::REMOVED:
                            listener.entryRemoved(entryEvent);
                            break;
                        case EntryEvent::type::UPDATED:
                            listener.entryUpdated(entryEvent);
                            break;
                        case EntryEvent::type::EVICTED:
                            listener.entryEvicted(entryEvent);
                            break;
                        default:
                            logger.warning("Received unrecognized event with type: ", static_cast<int>(type),
                                           " Dropping the event!!!");
                    }
                }
            private:
                const std::string& instanceName;
                spi::ClientClusterService &clusterService;
                serialization::pimpl::SerializationService& serializationService;
                Listener listener;
                util::ILogger &logger;
            };
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
