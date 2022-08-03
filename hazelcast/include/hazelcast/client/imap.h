/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <climits>
#include <assert.h>

#include <boost/container/vector.hpp>

#include "hazelcast/client/monitor/local_map_stats.h"
#include "hazelcast/client/monitor/impl/NearCacheStatsImpl.h"
#include "hazelcast/client/monitor/impl/LocalMapStatsImpl.h"
#include "hazelcast/client/proxy/IMapImpl.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/entry_listener.h"
#include "hazelcast/client/entry_view.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/exception_util.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/spi/ClientContext.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class hazelcast_client;

        namespace spi {
            class ProxyManager;
        }

        /**
        * Concurrent, distributed, observable and queryable map client.
        *
        * Notice that this class have a private constructor.
        * You can access get an IMap in the following way
        *
        *      auto client = new_client().get();
        *      auto imap = client.get_map("aMap").get();
        *
        */
        class HAZELCAST_API imap : public proxy::IMapImpl {
            friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:mapService";

            imap(const std::string &instance_name, spi::ClientContext *context) : proxy::IMapImpl(instance_name,
                                                                                                  context) {}

            /**
            * check if this map contains key.
            * @param key
            * @return true if contains, false otherwise
            */
            template<typename K>
            boost::future<bool> contains_key(const K &key) {
                return contains_key_internal(to_data(key));
            }

            /**
            * check if this map contains value.
            * @param value
            * @return true if contains, false otherwise
            */
            template<typename V>
            boost::future<bool> contains_value(const V &value) {
                return proxy::IMapImpl::contains_value(to_data(value));
            }

            /**
            * get the value.
            * @param key
            * @return value value, if there is no mapping for key then returns boost::none.
            */
            template<typename K, typename V>
            boost::future<boost::optional<V>> get(const K &key) {
                return to_object<V>(get_internal(to_data(key)));
            }

            /**
            * put new entry into map.
            * @param key
            * @param value
            * @return the previous value. if there is no mapping for key, then returns boost::none.
            */
            template<typename K, typename V, typename R=V>
            boost::future<boost::optional<R>> put(const K &key, const V &value) {
                return put<K, V, R>(key, value, UNSET);
            }

            /**
            * Puts an entry into this map with a given ttl (time to live) value.
            * Entry will expire and get evicted after the ttl. If ttl is 0, then
            * the entry lives forever.
            *
            * @param key              key of the entry
            * @param value            value of the entry
            * @param ttl      maximum time for this entry to stay in the map. 0 means infinite.
            * @return the previous value. if there is no mapping for key, then returns boost::none.
            */
            template<typename K, typename V, typename R=V>
            boost::future<boost::optional<R>> put(const K &key, const V &value, std::chrono::milliseconds ttl) {
                return to_object<R>(put_internal(to_data(key), to_data(value), ttl));
            }

            /**
            * remove entry form map
            * @param key
            * @return the previous value. if there is no mapping for key then returns boost::none.
            */
            template<typename K, typename V>
            boost::future<boost::optional<V>> remove(const K &key) {
                return to_object<V>(remove_internal(to_data(key)));
            }

            /**
            * removes entry from map if there is an entry with same key and value.
            * @param key key of the entry to be removed
             * @param value value of the entry to be removed
            * @return true if the entry with key and value exists and removed, false otherwise
            */
            template<typename K, typename V>
            boost::future<bool> remove(const K &key, const V &value) {
                return remove_internal(to_data(key), to_data(value));
            }

            /**
             * Removes all entries which match with the supplied predicate.
             * If this map has index, matching entries will be found via index search,
             * otherwise they will be found by full-scan.
             *
             * Note that calling this method also removes all entries from callers Near Cache.
             *
             * @param predicate matching entries with this predicate will be removed from this map
             */
            template <typename P>
            boost::future<void> remove_all(const P &predicate) {
                return to_void_future(remove_all_internal(to_data<P>(predicate)));
            }

            /**
            * removes entry from map.
            * Does not return anything.
            * @param key The key of the map entry to remove.
            */
            template <typename K>
            boost::future<void> delete_entry(const K &key) {
                return to_void_future(delete_internal(to_data(key)));
            }

            /**
            * Tries to remove the entry with the given key from this map
            * within specified timeout value. If the key is already locked by another
            * thread and/or member, then this operation will wait timeout
            * amount for acquiring the lock.
            *
            * @param key      key of the entry
            * @param timeout  maximum time to wait for acquiring the lock for the key
            */
            template<typename K>
            boost::future<bool> try_remove(const K &key, std::chrono::milliseconds timeout) {
                return try_remove_internal(to_data(key), timeout);
            }

            /**
            * Tries to put the given key, value into this map within specified
            * timeout value. If this method returns false, it means that
            * the caller thread couldn't acquire the lock for the key within
            * timeout duration, thus put operation is not successful.
            *
            * @param key      key of the entry
            * @param value    value of the entry
            * @param timeout  maximum time to wait
            * @return <tt>true</tt> if the put is successful, <tt>false</tt>
            *         otherwise.
            */
            template<typename K, typename V>
            boost::future<bool> try_put(const K &key, const V &value, std::chrono::milliseconds timeout) {
                return try_put_internal(to_data(key), to_data(value), timeout);
            }

            /**
            * Same as put(K, V, int64_t, TimeUnit) but MapStore, if defined,
            * will not be called to store/persist the entry.  If ttl is 0, then
            * the entry lives forever.
            *
            * @param key          key of the entry
            * @param value        value of the entry
            * @param ttl  maximum time for this entry to stay in the map in milliseconds, 0 means infinite.
            */
            template<typename K, typename V>
            boost::future<void> put_transient(const K &key, const V &value, std::chrono::milliseconds ttl) {
                return to_void_future(try_put_transient_internal(to_data(key), to_data(value), ttl));
            }

            /**
            * Puts an entry into this map, if the specified key is not already associated with a value.
            *
            * @param key key with which the specified value is to be associated
            * @param value
            * @return the previous value, if there is no mapping for key
            * then returns boost::none.
            */
            template<typename K, typename V, typename R=V>
            boost::future<boost::optional<V>> put_if_absent(const K &key, const V &value) {
                return put_if_absent(key, value, UNSET);
            }

            /**
            * Puts an entry into this map with a given ttl (time to live) value
            * if the specified key is not already associated with a value.
            * Entry will expire and get evicted after the ttl.
            *
            * @param key            key of the entry
            * @param value          value of the entry
            * @param ttl    maximum time for this entry to stay in the map
            * @return the previous value of the entry, if there is no mapping for key
            * then returns boost::none.
            */
            template<typename K, typename V, typename R=V>
            boost::future<boost::optional<V>> put_if_absent(const K &key, const V &value, std::chrono::milliseconds ttl) {
                return to_object<R>(put_if_absent_internal(to_data(key), to_data(value), ttl));
            }

            /**
            * Replaces the entry for a key only if currently mapped to a given value.
            * @param key key with which the specified value is associated
            * @param oldValue value expected to be associated with the specified key
            * @param newValue
            * @return <tt>true</tt> if the value was replaced
            */
            template<typename K, typename V, typename N = V>
            boost::future<bool> replace(const K &key, const V &old_value, const N &new_value) {
                return replace_if_same_internal(to_data(key), to_data(old_value), to_data(new_value));
            }

            /**
            * Replaces the entry for a key only if currently mapped to some value.
            * @param key key with which the specified value is associated
            * @param value
            * @return the previous value of the entry, if there is no mapping for key
            * then returns boost::none.
            */
            template<typename K, typename V, typename R=V>
            boost::future<boost::optional<R>> replace(const K &key, const V &value) {
                return to_object<R>(replace_internal(to_data(key), to_data(value)));
            }

            /**
            * Puts an entry into this map.
            * Similar to put operation except that set
            * doesn't return the old value which is more efficient.
            * @param key
            * @param value
            */
            template<typename K, typename V, typename R=V>
            boost::future<void> set(const K &key, const V &value) {
                return to_void_future(set(key, value, UNSET));
            }

            /**
            * Puts an entry into this map.
            * Similar to put operation except that set
            * doesn't return the old value which is more efficient.
            * @param key key with which the specified value is associated
            * @param value
            * @param ttl maximum time in milliseconds for this entry to stay in the map
            0 means infinite.
            */
            template<typename K, typename V>
            boost::future<void> set(const K &key, const V &value, std::chrono::milliseconds ttl) {
                return to_void_future(set_internal(to_data(key), to_data(value), ttl));
            }

            /**
            * Acquires the lock for the specified key.
            * <p>If the lock is not available then
            * the current thread becomes disabled for thread scheduling
            * purposes and lies dormant until the lock has been acquired.
            *
            * Scope of the lock is this map only.
            * Acquired lock is only for the key in this map.
            *
            * Locks are re-entrant so if the key is locked N times then
            * it should be unlocked N times before another thread can acquire it.
            *
            * @param key key to lock.
            */
            template<typename K>
            boost::future<void> lock(const K &key) {
                return to_void_future(lock(key, UNSET));
            }

            /**
            * Acquires the lock for the specified key for the specified lease time.
            * <p>After lease time, lock will be released..
            *
            * <p>If the lock is not available then
            * the current thread becomes disabled for thread scheduling
            * purposes and lies dormant until the lock has been acquired.
            *
            * Scope of the lock is this map only.
            * Acquired lock is only for the key in this map.
            *
            * Locks are re-entrant so if the key is locked N times then
            * it should be unlocked N times before another thread can acquire it.
            *
            *
            * @param key key to lock.
            * @param leaseTime time to wait before releasing the lock.
            */
            template<typename K>
            boost::future<void> lock(const K &key, std::chrono::milliseconds lease_time) {
                return to_void_future(proxy::IMapImpl::lock(to_data(key), lease_time));
            }

            /**
            * Checks the lock for the specified key.
            * <p>If the lock is acquired then returns true, else false.
            *
            *
            * @param key key to lock to be checked.
            * @return <tt>true</tt> if lock is acquired, <tt>false</tt> otherwise.
            */
            template<typename K>
            boost::future<bool> is_locked(const K &key) {
                return proxy::IMapImpl::is_locked(to_data(key));
            }

            /**
            * Tries to acquire the lock for the specified key.
            * <p>If the lock is not available then the current thread
            * doesn't wait and returns false immediately.
            *
            *
            * @param key key to lock.
            * @return <tt>true</tt> if lock is acquired, <tt>false</tt> otherwise.
            */
            template<typename K>
            boost::future<bool> try_lock(const K &key) {
                return try_lock(key, std::chrono::milliseconds(0));
            }

            /**
            * Tries to acquire the lock for the specified key.
            * <p>If the lock is not available then
            * the current thread becomes disabled for thread scheduling
            * purposes and lies dormant until one of two things happens:
            * <ul>
            * <li>The lock is acquired by the current thread; or
            * <li>The specified waiting time elapses
            * </ul>
            *
            *
            * @param key      key to lock in this map
            * @param timeout     maximum time in milliseconds to wait for the lock
            * @return <tt>true</tt> if the lock was acquired and <tt>false</tt>
            *         if the waiting time elapsed before the lock was acquired.
            */
            template<typename K>
            boost::future<bool> try_lock(const K &key, std::chrono::milliseconds timeout) {
                return proxy::IMapImpl::try_lock(to_data(key), timeout);
            }


            /**
            * Tries to acquire the lock for the specified key for the specified lease time.
            * <p>After lease time, the lock will be released.
            * <p>If the lock is not available then
            * the current thread becomes disabled for thread scheduling
            * purposes and lies dormant until one of two things happens:
            * <ul>
            * <li>The lock is acquired by the current thread; or
            * <li>The specified waiting time elapses
            * </ul>
            *
            *
            * @param key      key to lock in this map
            * @param timeout     maximum time to wait for the lock
            * @param leaseTime time to wait before releasing the lock.
            * @return <tt>true</tt> if the lock was acquired and <tt>false</tt>
            *         if the waiting time elapsed before the lock was acquired.
            */
            template <typename K>
            boost::future<bool> try_lock(const K &key, std::chrono::milliseconds timeout, std::chrono::milliseconds lease_time) {
                return proxy::IMapImpl::try_lock(to_data(key), timeout, lease_time);
            }

            /**
            * Releases the lock for the specified key. It never blocks and
            * returns immediately.
            *
            * <p>If the current thread is the holder of this lock then the hold
            * count is decremented.  If the hold count is now zero then the lock
            * is released.  If the current thread is not the holder of this
            * lock then illegal_monitor_state is thrown.
            *
            *
            * @param key key to lock.
            * @throws illegal_monitor_state if the current thread does not hold this lock MTODO
            */
            template<typename K>
            boost::future<void> unlock(const K &key) {
                return to_void_future(proxy::IMapImpl::unlock(to_data(key)));
            }

            /**
            * Releases the lock for the specified key regardless of the lock owner.
            * It always successfully unlocks the key, never blocks
            * and returns immediately.
            *
            *
            * @param key key to lock.
            */
            template<typename K>
            boost::future<void> force_unlock(const K &key) {
                return to_void_future(proxy::IMapImpl::force_unlock(to_data(key)));
            }

            /**
            * Adds an interceptor for this map. Added interceptor will intercept operations
            * and execute user defined methods and will cancel operations if user defined method throw exception.
            *
            *
            * Interceptor should be serializable.
            * Notice that map interceptor runs on the nodes. Because of that same class should be implemented in java side
            * with same classId and factoryId.
            * @param interceptor map interceptor
            * @return id of registered interceptor
            */
            template<typename MapInterceptor>
            boost::future<std::string> add_interceptor(const MapInterceptor &interceptor) {
                return proxy::IMapImpl::add_interceptor(to_data(interceptor));
            }

            /**
            * Adds an entry listener for this map.
            *
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * @param listener     entry listener
            * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
            *                     contain the value.
            *
            * @return registrationId of added listener that can be used to remove the entry listener.
            */
            boost::future<boost::uuids::uuid> add_entry_listener(entry_listener &&listener, bool include_value) {
                const auto listener_flags = listener.flags_;
                return proxy::IMapImpl::add_entry_listener(
                        std::unique_ptr<impl::BaseEventHandler>(
                                new impl::EntryEventHandler<protocol::codec::map_addentrylistener_handler>(
                                        get_name(), get_context().get_client_cluster_service(),
                                        get_context().get_serialization_service(),
                                        std::move(listener),
                                        include_value, get_context().get_logger())), include_value, listener_flags);
            }

            /**
            * Adds an entry listener for this map.
            *
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * @param listener     entry listener
            * @param predicate The query filter to use when returning the events to the user.
            * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
            *                     contain the value.
            *
            * @return registrationId of added listener that can be used to remove the entry listener.
            */
            template<typename P>
            boost::future<boost::uuids::uuid>
            add_entry_listener(entry_listener &&listener, const P &predicate, bool include_value) {
                const auto listener_flags = listener.flags_;
                return proxy::IMapImpl::add_entry_listener(
                        std::unique_ptr<impl::BaseEventHandler>(
                                new impl::EntryEventHandler<protocol::codec::map_addentrylistenerwithpredicate_handler>(
                                        get_name(), get_context().get_client_cluster_service(),
                                        get_context().get_serialization_service(),
                                        std::move(listener),
                                        include_value, get_context().get_logger())), to_data<P>(predicate), include_value, listener_flags);
            }

            /**
            * Adds the specified entry listener for the specified key.
            *
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * @param listener     entry listener
            * @param key          key to listen
            * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
            *                     contain the value.
            */
            template<typename K>
            boost::future<boost::uuids::uuid> add_entry_listener(entry_listener &&listener, bool include_value, const K &key) {
                const auto listener_flags = listener.flags_;
                return proxy::IMapImpl::add_entry_listener(
                        std::shared_ptr<impl::BaseEventHandler>(
                                new impl::EntryEventHandler<protocol::codec::map_addentrylistenertokey_handler>(
                                        get_name(), get_context().get_client_cluster_service(),
                                        get_context().get_serialization_service(),
                                        std::move(listener),
                                        include_value, get_context().get_logger())), include_value, to_data<K>(key), listener_flags);
            }

            /**
            * Returns the <tt>EntryView</tt> for the specified key.
            *
            *
            * @param key key of the entry
            * @return <tt>EntryView</tt> of the specified key
            * @see EntryView
            */
            template<typename K, typename V>
            boost::future<boost::optional<entry_view<K, V>>> get_entry_view(const K &key) {
                return proxy::IMapImpl::get_entry_view_data(to_data(key)).then([=] (boost::future<boost::optional<map::data_entry_view>> f) {
                    auto dataView = f.get();
                    if (!dataView) {
                        return boost::optional<entry_view<K, V>>();
                    }
                    auto v = to_object<V>(dataView->get_value());
                    return boost::make_optional(entry_view<K, V>(key, std::move(v).value(), *std::move(dataView)));
                });
            }

            /**
            * Evicts the specified key from this map. If
            * a <tt>MapStore</tt> defined for this map, then the entry is not
            * deleted from the underlying <tt>MapStore</tt>, evict only removes
            * the entry from the memory.
            *
            *
            * @param key key to evict
            * @return <tt>true</tt> if the key is evicted, <tt>false</tt> otherwise.
            */
            template<typename K>
            boost::future<bool> evict(const K &key) {
                return evict_internal(to_data(key));
            }

            /**
            * Returns the entries for the given keys.
            *
            * @param keys keys to get
            * @return map of entries
            */
            template<typename K, typename V>
            boost::future<std::unordered_map<K, V>> get_all(const std::unordered_set<K> &keys) {
                if (keys.empty()) {
                    return boost::make_ready_future(std::unordered_map<K, V>());
                }

                std::unordered_map<int, std::vector<serialization::pimpl::data>> partition_to_key_data;
                // group the request per server
                for (auto &key : keys) {
                    auto key_data = to_data<K>(key);

                    auto partitionId = get_partition_id(key_data);
                    partition_to_key_data[partitionId].push_back(std::move(key_data));
                }

                std::vector<boost::future<EntryVector>> futures;
                futures.reserve(partition_to_key_data.size());
                for (auto &entry : partition_to_key_data) {
                    futures.push_back(get_all_internal(entry.first, entry.second));
                }

                return boost::when_all(futures.begin(), futures.end()).then(boost::launch::sync,
                                                                            [=](boost::future<boost::csbl::vector<boost::future<EntryVector>>> results_data) {
                                                                                std::unordered_map<K, V> result;
                                                                                for (auto &entryVectorFuture : results_data.get()) {
                                                                                    for (auto &entry : entryVectorFuture.get()) {
                                                                                        auto val = to_object<V>(
                                                                                                entry.second);
                                                                                        // it is guaranteed that all values are non-null
                                                                                        assert(val.has_value());
                                                                                        result[to_object<K>(
                                                                                                entry.first).value()] = std::move(
                                                                                                val.value());
                                                                                    }
                                                                                }
                                                                                return result;
                                                                            });
            }

            /**
            * Returns a vector clone of the keys contained in this map.
            * The vector is <b>NOT</b> backed by the map,
            * so changes to the map are <b>NOT</b> reflected in the vector, and vice-versa.
            *
            * @return a vector clone of the keys contained in this map
            */
            template<typename K>
            boost::future<std::vector<K>> key_set() {
                return to_object_vector<K>(proxy::IMapImpl::key_set_data());
            }

            /**
              *
              * Queries the map based on the specified predicate and
              * returns the keys of matching entries.
              *
              * Specified predicate runs on all members in parallel.
              *
              *
              * @param predicate query criteria
              * @return result key set of the query
              */
            template<typename K, typename P, class = typename std::enable_if<!std::is_base_of<query::paging_predicate_marker, P>::value>::type>
            boost::future<std::vector<K>> key_set(const P &predicate) {
                return to_object_vector<K>(proxy::IMapImpl::key_set_data(to_data(predicate)));
            }

            /**
            *
            * Queries the map based on the specified predicate and
            * returns the keys of matching entries.
            *
            * Specified predicate runs on all members in parallel.
            *
            *
            * @param predicate query criteria
            * @return result key set of the query
            */
            template<typename K, typename V>
            boost::future<std::vector<K>> key_set(query::paging_predicate<K, V> &predicate) {
                predicate.set_iteration_type(query::iteration_type::KEY);
                return key_set_for_paging_predicate_data(
                        protocol::codec::holder::paging_predicate_holder::of(predicate, serialization_service_)).then(
                        [=, &predicate](
                                boost::future<std::pair<std::vector<serialization::pimpl::data>, query::anchor_data_list>> f) {
                    auto result = f.get();
                    predicate.set_anchor_data_list(std::move(result.second));
                    const auto &entries = result.first;
                    std::vector<K> values;
                    values.reserve(entries.size());
                    for(const auto &e : entries) {
                        values.emplace_back(*to_object<K>(e));
                    }
                    return values;
                });
            }

            /**
            * Returns a vector clone of the values contained in this map.
            * The vector is <b>NOT</b> backed by the map,
            * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
            *
            * @return a vector clone of the values contained in this map
            */
            template<typename V>
            boost::future<std::vector<V>> values() {
                return to_object_vector<V>(proxy::IMapImpl::values_data());
            }

            /**
            * Returns a vector clone of the values contained in this map.
            * The vector is <b>NOT</b> backed by the map,
            * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
            *
            * @param predicate the criteria for values to match
            * @return a vector clone of the values contained in this map
            */
            template<typename V, typename P, class = typename std::enable_if<!std::is_base_of<query::paging_predicate_marker, P>::value>::type>
            boost::future<std::vector<V>> values(const P &predicate) {
                return to_object_vector<V>(proxy::IMapImpl::values_data(to_data(predicate)));
            }

            /**
            * Returns a vector clone of the values contained in this map.
            * The vector is <b>NOT</b> backed by the map,
            * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
            *
            *
            * @param predicate the criteria for values to match
            * @return a vector clone of the values contained in this map
            */
            template<typename K, typename V>
            boost::future<std::vector<V>> values(query::paging_predicate<K, V> &predicate) {
                predicate.set_iteration_type(query::iteration_type::VALUE);
                return values_for_paging_predicate_data(
                        protocol::codec::holder::paging_predicate_holder::of(predicate, serialization_service_)).then(
                        [=, &predicate](boost::future<std::pair<std::vector<serialization::pimpl::data>, query::anchor_data_list>> f) {
                    auto result = f.get();
                    predicate.set_anchor_data_list(std::move(result.second));
                    const auto &entries = result.first;
                    std::vector<V> values;
                    values.reserve(entries.size());
                    for(const auto &e : entries) {
                        values.emplace_back(*to_object<V>(e));
                    }
                    return values;
                });
            }

            /**
            * Returns a std::vector< std::pair<K, V> > clone of the mappings contained in this map.
            * The vector is <b>NOT</b> backed by the map,
            * so changes to the map are <b>NOT</b> reflected in the set, and vice-versa.
            *
            * @return a vector clone of the keys mappings in this map
            */
            template<typename K, typename V>
            boost::future<std::vector<std::pair<K, V>>> entry_set() {
                return to_entry_object_vector<K,V>(proxy::IMapImpl::entry_set_data());
            }

            /**
            * Queries the map based on the specified predicate and
            * returns the matching entries.
            *
            * Specified predicate runs on all members in parallel.
            *
            *
            * @param predicate query criteria
            * @return result entry vector of the query
            */
            template<typename K, typename V, typename P, class = typename std::enable_if<!std::is_base_of<query::paging_predicate_marker, P>::value>::type>
            boost::future<std::vector<std::pair<K, V>>> entry_set(const P &predicate) {
                return to_entry_object_vector<K,V>(proxy::IMapImpl::entry_set_data(to_data(predicate)));
            }

            /**
            * Queries the map based on the specified predicate and
            * returns the matching entries.
            *
            * Specified predicate runs on all members in parallel.
            *
            *
            * @param predicate query criteria
            * @return result entry vector of the query
            */
            template<typename K, typename V>
            boost::future<std::vector<std::pair<K, V>>> entry_set(query::paging_predicate<K, V> &predicate) {
                predicate.set_iteration_type(query::iteration_type::ENTRY);
                return entry_set_for_paging_predicate_data(
                        protocol::codec::holder::paging_predicate_holder::of(predicate, serialization_service_)).then(
                        [=, &predicate](boost::future<std::pair<EntryVector, query::anchor_data_list>> f) {
                    auto result = f.get();
                    predicate.set_anchor_data_list(std::move(result.second));
                    const auto &entries_data = result.first;
                    std::vector<std::pair<K, V>> entries;
                    entries.reserve(entries_data.size());
                    for(const auto &e : entries_data) {
                        entries.emplace_back(*to_object<K>(e.first), *to_object<V>(e.second));
                    }
                    return entries;
                });
            }

            /**
             * Adds an index to this map for the specified entries so
             * that queries can run faster.
             * <p>
             * Let's say your map values are Employee objects.
             * <pre>
             *   struct Employee {
             *     bool active;
             *     int32_t age;
             *     std::string name;
             *     // other fields
             *
             *   }
             * </pre>
             * If you are querying your values mostly based on age and active then
             * you may consider indexing these fields.
             * <pre>
             *   auto imap = client.getMap("employees");
             *   imap.addIndex(config::index_config(config::index_config::index_type::SORTED, "age"));  // Sorted index for range queries
             *   imap.addIndex(config::index_config(config::index_config::index_type::HASH, "active"));  // Sorted index for range queries
             * </pre>
             * Index attribute should either have a getter method or be public.
             * You should also make sure to add the indexes before adding
             * entries to this map.
             * <p>
             * <b>Time to Index</b>
             * <p>
             * Indexing time is executed in parallel on each partition by operation threads. The Map
             * is not blocked during this operation.
             * <p>
             * The time taken in proportional to the size of the Map and the number Members.
             * <p>
             * <b>Searches while indexes are being built</b>
             * <p>
             * Until the index finishes being created, any searches for the attribute will use a full Map scan,
             * thus avoiding using a partially built index and returning incorrect results.
             *
             * @param config Index configuration.
             */
            boost::future<void> add_index(const config::index_config &config) {
                return to_void_future(proxy::IMapImpl::add_index_data(config));
            }

            /**
             * Convenient method to add an index to this map with the given type and attributes.
             * Attributes are indexed in ascending order.
             * <p>
             *
             * \param type       Index type.
             * \param attributes Attributes to be indexed.
             */
            template<typename ...T>
            boost::future<void> add_index(config::index_config::index_type type, T... attributes) {
                return add_index(config::index_config(type, std::forward<T>(attributes)...));
            }

            boost::future<void> clear() {
                return to_void_future(proxy::IMapImpl::clear_data());
            }

            /**
            * Applies the user defined EntryProcessor to the entry mapped by the key.
            * Returns the the ResultType which is result of the process() method of EntryProcessor.
            *
            * EntryProcessor should be serializable.
            * Notice that map EntryProcessor runs on the nodes. Because of that, same class should be implemented in java side
            * with same classId and factoryId.
            *
            * @tparam EntryProcessor type of entry processor class
            * @tparam ResultType that entry processor will return
            * @param entryProcessor that will be applied
            * @param key of entry that entryProcessor will be applied on
            * @return result of entry process.
            */
            template<typename K, typename ResultType, typename EntryProcessor>
            boost::future<boost::optional<ResultType>> execute_on_key(const K &key, const EntryProcessor &entry_processor) {
                return to_object<ResultType>(execute_on_key_internal(to_data(key), to_data(entry_processor)));
            }


            /**
             * Applies the user defined EntryProcessor to the entry mapped by the key.
             * Returns immediately with a Future representing that task.
             * <p/>
             *
             * @param key            key to be processed
             * @param entryProcessor processor to process the key
             * @return Future from which the result of the operation can be retrieved.
             */
            template<typename K, typename ResultType, typename EntryProcessor>
            boost::future<boost::optional<ResultType>>
            submit_to_key(const K &key, const EntryProcessor &entry_processor) {
                return to_object<ResultType>(submit_to_key_internal(to_data(key), to_data(entry_processor)));
            }

            /**
            * Applies the user defined EntryProcessor to the entries mapped by the collection of keys.
            * Returns the results mapped by each key in the map.
            *
            *
            * Notice that map EntryProcessor runs on the nodes. Because of that, same class should be implemented in java side
            *
            * @tparam ResultType that entry processor will return
            * @tparam EntryProcessor type of entry processor class
            * @tparam keys The keys for which the entry processor will be applied.
            * @param entryProcessor that will be applied
            */
            template<typename K, typename ResultType, typename EntryProcessor>
            boost::future<std::unordered_map<K, boost::optional<ResultType>>>
            execute_on_keys(const std::unordered_set<K> &keys, const EntryProcessor &entry_processor) {
                return to_object_map<K, ResultType>(execute_on_keys_internal<K, EntryProcessor>(keys, entry_processor));
            }

            /**
            * Applies the user defined EntryProcessor to the all entries in the map.
            * Returns the results mapped by each key in the map.
            *
            *
            * EntryProcessor should be serializable.
            * Notice that map EntryProcessor runs on the nodes. Because of that, same class should be implemented in java side
            * with same classId and factoryId.
            *
            * @tparam ResultType that entry processor will return
            * @tparam EntryProcessor type of entry processor class
            * @param entryProcessor that will be applied
            */
            template<typename K, typename ResultType, typename EntryProcessor>
            boost::future<std::unordered_map<K, boost::optional<ResultType>>> execute_on_entries(const EntryProcessor &entry_processor) {
                return to_object_map<K, ResultType>(proxy::IMapImpl::execute_on_entries_data(to_data(entry_processor)));
            }

            /**
            * Applies the user defined EntryProcessor to the all entries in the map.
            * Returns the results mapped by each key in the map.
            *
            *
            * EntryProcessor should be serializable.
            * Notice that map EntryProcessor runs on the nodes. Because of that, same class should be implemented in java side
            * with same classId and factoryId.
            *
            * @tparam ResultType that entry processor will return
            * @tparam EntryProcessor type of entry processor class
            * @tparam predicate The filter to apply for selecting the entries at the server side.
            * @param entryProcessor that will be applied
            */
            template<typename K, typename ResultType, typename EntryProcessor, typename P>
            boost::future<std::unordered_map<K, boost::optional<ResultType>>>
            execute_on_entries(const EntryProcessor &entry_processor, const P &predicate) {
                return to_object_map<K, ResultType>(proxy::IMapImpl::execute_on_entries_data(to_data(entry_processor),
                                                                                 to_data(predicate)));
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
                std::unordered_map<int, EntryVector> entryMap;
                for (auto &entry : entries) {
                    serialization::pimpl::data key_data = to_data(entry.first);
                    int partitionId = get_partition_id(key_data);
                    entryMap[partitionId].push_back(std::make_pair(key_data, to_data(entry.second)));
                }

                std::vector<boost::future<protocol::ClientMessage>> resultFutures;
                for (auto &&partitionEntry : entryMap) {
                    auto partitionId = partitionEntry.first;
                    resultFutures.push_back(put_all_internal(partitionId, std::move(partitionEntry.second)));
                }
                return boost::when_all(resultFutures.begin(), resultFutures.end()).then(boost::launch::sync,
                                                                                        [](boost::future<boost::csbl::vector<boost::future<protocol::ClientMessage>>> futures) {
                                                                                            for (auto &f : futures.get()) {
                                                                                                f.get();
                                                                                            }
                                                                                        });
            }

            /**
             * Returns LocalMapStats for this map.
             * LocalMapStats is the statistics for the local portion of this
             * distributed map and contains information such as ownedEntryCount
             * backupEntryCount, lastUpdateTime, lockedEntryCount.
             * <p/>
             * Since this stats are only for the local portion of this map, if you
             * need the cluster-wide MapStats then you need to get the LocalMapStats
             * from all members of the cluster and combine them.
             *
             * @return this map's local statistics.
             */
            monitor::local_map_stats &get_local_map_stats() {
                return local_map_stats_;
            }

            template<typename K, typename V>
            query::paging_predicate<K, V> new_paging_predicate(size_t predicate_page_size) {
                return query::paging_predicate<K, V>(get_serialization_service(), predicate_page_size);
            }

            template<typename K, typename V, typename INNER_PREDICATE>
            query::paging_predicate<K, V> new_paging_predicate(size_t predicate_page_size, const INNER_PREDICATE &predicate) {
                return query::paging_predicate<K, V>(get_serialization_service(), predicate_page_size, predicate);
            }

            template<typename K, typename V, typename COMPARATOR>
            query::paging_predicate<K, V> new_paging_predicate(COMPARATOR &&comparator, size_t predicate_page_size) {
                return query::paging_predicate<K, V>(get_serialization_service(), std::move(comparator), predicate_page_size);
            }

            template<typename K, typename V, typename INNER_PREDICATE, typename COMPARATOR>
            query::paging_predicate<K, V> new_paging_predicate(const INNER_PREDICATE &predicate, COMPARATOR &&comparator, size_t predicate_page_size) {
                return query::paging_predicate<K, V>(get_serialization_service(), predicate, std::move(comparator), predicate_page_size);
            }

        protected:
            /**
             * Default TTL value of a record.
             */
            static const std::chrono::milliseconds UNSET;

            monitor::impl::LocalMapStatsImpl local_map_stats_;

            virtual boost::future<boost::optional<serialization::pimpl::data>> get_internal(const serialization::pimpl::data &key_data) {
                return proxy::IMapImpl::get_data(key_data);
            }

            virtual boost::future<bool> contains_key_internal(const serialization::pimpl::data &key_data) {
                return proxy::IMapImpl::contains_key(key_data);
            }

            virtual boost::future<boost::optional<serialization::pimpl::data>> remove_internal(
                    const serialization::pimpl::data &key_data) {
                return proxy::IMapImpl::remove_data(key_data);
            }

            virtual boost::future<bool> remove_internal(
                    const serialization::pimpl::data &key_data, const serialization::pimpl::data &value_data) {
                return proxy::IMapImpl::remove(key_data, value_data);
            }

            virtual boost::future<protocol::ClientMessage> remove_all_internal(const serialization::pimpl::data &predicate_data) {
                return proxy::IMapImpl::remove_all(predicate_data);
            }

            virtual boost::future<protocol::ClientMessage> delete_internal(const serialization::pimpl::data &key_data) {
                return proxy::IMapImpl::delete_entry(key_data);
            }

            virtual boost::future<bool> try_remove_internal(const serialization::pimpl::data &key_data, std::chrono::milliseconds timeout) {
                return proxy::IMapImpl::try_remove(key_data, timeout);
            }

            virtual boost::future<bool> try_put_internal(const serialization::pimpl::data &key_data,
                                                       const serialization::pimpl::data &value_data, std::chrono::milliseconds timeout) {
                return proxy::IMapImpl::try_put(key_data, value_data, timeout);
            }

            virtual boost::future<boost::optional<serialization::pimpl::data>> put_internal(const serialization::pimpl::data &key_data,
                                                                                           const serialization::pimpl::data &value_data,
                                                                                           std::chrono::milliseconds ttl) {
                return proxy::IMapImpl::put_data(key_data, value_data, ttl);
            }

            virtual boost::future<protocol::ClientMessage> try_put_transient_internal(const serialization::pimpl::data &key_data,
                                                                                   const serialization::pimpl::data &value_data, std::chrono::milliseconds ttl) {
                return proxy::IMapImpl::put_transient(key_data, value_data, ttl);
            }

            virtual boost::future<boost::optional<serialization::pimpl::data>>
            put_if_absent_internal(const serialization::pimpl::data &key_data,
                                const serialization::pimpl::data &value_data,
                                std::chrono::milliseconds ttl) {
                return proxy::IMapImpl::put_if_absent_data(key_data, value_data, ttl);
            }

            virtual boost::future<bool> replace_if_same_internal(const serialization::pimpl::data &key_data,
                                                              const serialization::pimpl::data &value_data,
                                                              const serialization::pimpl::data &new_value_data) {
                return proxy::IMapImpl::replace(key_data, value_data, new_value_data);
            }

            virtual boost::future<boost::optional<serialization::pimpl::data>>
            replace_internal(const serialization::pimpl::data &key_data,
                            const serialization::pimpl::data &value_data) {
                return proxy::IMapImpl::replace_data(key_data, value_data);
            }

            virtual boost::future<protocol::ClientMessage>
            set_internal(const serialization::pimpl::data &key_data, const serialization::pimpl::data &value_data,
                        std::chrono::milliseconds ttl) {
                return proxy::IMapImpl::set(key_data, value_data, ttl);
            }

            virtual boost::future<bool> evict_internal(const serialization::pimpl::data &key_data) {
                return proxy::IMapImpl::evict(key_data);
            }

            virtual boost::future<EntryVector>
            get_all_internal(int partition_id, const std::vector<serialization::pimpl::data> &partition_keys) {
                return proxy::IMapImpl::get_all_data(partition_id, partition_keys);
            }

            virtual boost::future<boost::optional<serialization::pimpl::data>>
            execute_on_key_internal(const serialization::pimpl::data &key_data,
                                 const serialization::pimpl::data &processor) {
                return proxy::IMapImpl::execute_on_key_data(key_data, processor);
            }

            boost::future<boost::optional<serialization::pimpl::data>>
            submit_to_key_internal(const serialization::pimpl::data &key_data,
                                const serialization::pimpl::data &processor) {
                return submit_to_key_data(key_data, processor);
            }

            template<typename K, typename EntryProcessor>
            boost::future<EntryVector> execute_on_keys_internal(const std::unordered_set<K> &keys, const EntryProcessor &entry_processor) {
                if (keys.empty()) {
                    return boost::make_ready_future(EntryVector());
                }
                std::vector<serialization::pimpl::data> keysData;
                for (const auto &k : keys) {
                    keysData.push_back(to_data<K>(k));
                }
                return proxy::IMapImpl::execute_on_keys_data(keysData, to_data<EntryProcessor>(entry_processor));
            }

            virtual boost::future<protocol::ClientMessage>
            put_all_internal(int partition_id, const EntryVector &entries) {
                return proxy::IMapImpl::put_all_data(partition_id, entries);
            }

        private:

            template<typename K, typename V>
            std::vector<std::pair<K, boost::optional<V>>> sort_and_get(query::paging_predicate<K, V> &predicate, query::iteration_type iteration_type, std::vector<std::pair<K, V>> entries) {
                std::vector<std::pair<K, boost::optional<V>>> optionalEntries;
                optionalEntries.reserve(entries.size());
                for(auto &&pair : entries) {
                    optionalEntries.emplace_back(pair.first, boost::make_optional(pair.second));
                }
                return sortAndGet(predicate, iteration_type, optionalEntries);
            }

            template<typename K, typename V>
            std::vector<std::pair<K, boost::optional<V>>> sort_and_get(query::paging_predicate<K, V> &predicate, query::iteration_type iteration_type, std::vector<std::pair<K, boost::optional<V>>> entries) {
                std::sort(entries.begin(), entries.end(), [&] (const std::pair<K, boost::optional<V>> &lhs, const std::pair<K, boost::optional<V>> &rhs) {
                    auto comparator = predicate.getComparator();
                    if (!comparator) {
                        switch(predicate.getIterationType()) {
                            case query::iteration_type::VALUE:
                                return lhs.second < rhs.second;
                            default:
                                return lhs.first < rhs.first;
                        }
                    }

                    std::pair<const K *, const V *> leftVal(&lhs.first, lhs.second.get_ptr());
                    std::pair<const K *, const V *> rightVal(&rhs.first, rhs.second.get_ptr());
                    int result = comparator->compare(&leftVal, &rightVal);
                    if (0 != result) {
                        // std sort: comparison function object returns ​true if the first argument is less
                        // than (i.e. is ordered before) the second.
                        return result < 0;
                    }

                    return lhs.first < rhs.first;
                });

                std::pair<size_t, size_t> range = update_anchor<K, V>(entries, predicate, iteration_type);

                std::vector<std::pair<K, boost::optional<V>>> result;
                for (size_t i = range.first; i < range.second; ++i) {
                    auto entry = entries[i];
                    result.push_back(std::make_pair(entry.first, entry.second));
                }
                return result;
            }
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
