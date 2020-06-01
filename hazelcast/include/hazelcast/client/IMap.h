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
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <climits>
#include <assert.h>

#include "hazelcast/client/monitor/LocalMapStats.h"
#include "hazelcast/client/monitor/impl/NearCacheStatsImpl.h"
#include "hazelcast/client/monitor/impl/LocalMapStatsImpl.h"
#include "hazelcast/client/proxy/IMapImpl.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/EntryView.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/ExceptionUtil.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/spi/ClientContext.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace spi {
            class ProxyManager;
        }

        /**
        * Concurrent, distributed, observable and queryable map client.
        *
        * Notice that this class have a private constructor.
        * You can access get an IMap in the following way
        *
        *      ClientConfig clientConfig;
        *      HazelcastClient client(clientConfig);
        *      IMap imap = client.getMap("aKey");
        *
        */
        class HAZELCAST_API IMap : public proxy::IMapImpl {
            friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:mapService";

            /**
            * check if this map contains key.
            * @param key
            * @return true if contains, false otherwise
            */
            template<typename K>
            boost::future<bool> containsKey(const K &key) {
                return containsKeyInternal(toData(key));
            }

            /**
            * check if this map contains value.
            * @param value
            * @return true if contains, false otherwise
            */
            template<typename V>
            boost::future<bool> containsValue(const V &value) {
                return proxy::IMapImpl::containsValue(toData(value));
            }

            /**
            * get the value.
            * @param key
            * @return value value, if there is no mapping for key then returns boost::none.
            */
            template<typename K, typename V>
            boost::future<boost::optional<V>> get(const K &key) {
                return toObject<V>(getInternal(toData(key)));
            }

            /**
            * put new entry into map.
            * @param key
            * @param value
            * @return the previous value. if there is no mapping for key, then returns boost::none.
            */
            template<typename K, typename V, typename R=V>
            boost::future<boost::optional<R>> put(const K &key, const V &value) {
                return put(key, value, UNSET);
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
            boost::future<boost::optional<R>> put(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                return toObject<R>(putInternal(toData(key), toData(value), ttl));
            }

            /**
            * remove entry form map
            * @param key
            * @return the previous value. if there is no mapping for key then returns boost::none.
            */
            template<typename K, typename V>
            boost::future<boost::optional<V>> remove(const K &key) {
                return toObject<V>(removeInternal(toData(key)));
            }

            /**
            * removes entry from map if there is an entry with same key and value.
            * @param key key of the entry to be removed
             * @param value value of the entry to be removed
            * @return true if the entry with key and value exists and removed, false otherwise
            */
            template<typename K, typename V>
            boost::future<bool> remove(const K &key, const V &value) {
                return removeInternal(toData(key), toData(value));
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
            boost::future<void> removeAll(const P &predicate) {
                return toVoidFuture(removeAllInternal(toData<P>(predicate)));
            }

            /**
            * removes entry from map.
            * Does not return anything.
            * @param key The key of the map entry to remove.
            */
            template <typename K>
            boost::future<void> deleteEntry(const K &key) {
                return toVoidFuture(deleteInternal(toData(key)));
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
            boost::future<bool> tryRemove(const K &key, std::chrono::steady_clock::duration timeout) {
                return tryRemoveInternal(toData(key), timeout);
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
            boost::future<bool> tryPut(const K &key, const V &value, std::chrono::steady_clock::duration timeout) {
                return tryPutInternal(toData(key), toData(value), timeout);
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
            boost::future<void> putTransient(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                return toVoidFuture(tryPutTransientInternal(toData(key), toData(value), ttl));
            }

            /**
            * Puts an entry into this map, if the specified key is not already associated with a value.
            *
            * @param key key with which the specified value is to be associated
            * @param value
            * @return the previous value, if there is no mapping for key
            * then returns boost::none.
            */
            template<typename K, typename V>
            boost::future<boost::optional<V>> putIfAbsent(const K &key, const V &value) {
                return putIfAbsent(key, value, UNSET);
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
            template<typename K, typename V>
            boost::future<boost::optional<V>> putIfAbsent(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                return toObject<V>(putIfAbsentInternal(toData(key), toData(value), ttl));
            }

            /**
            * Replaces the entry for a key only if currently mapped to a given value.
            * @param key key with which the specified value is associated
            * @param oldValue value expected to be associated with the specified key
            * @param newValue
            * @return <tt>true</tt> if the value was replaced
            */
            template<typename K, typename V, typename N>
            boost::future<bool> replace(const K &key, const V &oldValue, const N &newValue) {
                return replaceIfSameInternal(toData(key), toData(oldValue), toData(newValue));
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
                return toObject<R>(replaceInternal(toData(key), toData(value)));
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
                return toVoidFuture(set(key, value, UNSET));
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
            boost::future<void> set(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                return toVoidFuture(setInternal(toData(key), toData(value), ttl));
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
                return toVoidFuture(lock(key, UNSET));
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
            boost::future<void> lock(const K &key, std::chrono::steady_clock::duration leaseTime) {
                return toVoidFuture(proxy::IMapImpl::lock(toData(key), leaseTime));
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
            boost::future<bool> isLocked(const K &key) {
                return proxy::IMapImpl::isLocked(toData(key));
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
            boost::future<bool> tryLock(const K &key) {
                return tryLock(key, std::chrono::milliseconds(0));
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
            boost::future<bool> tryLock(const K &key, std::chrono::steady_clock::duration timeout) {
                return proxy::IMapImpl::tryLock(toData(key), timeout);
            }

            /**
            * Releases the lock for the specified key. It never blocks and
            * returns immediately.
            *
            * <p>If the current thread is the holder of this lock then the hold
            * count is decremented.  If the hold count is now zero then the lock
            * is released.  If the current thread is not the holder of this
            * lock then IllegalMonitorStateException is thrown.
            *
            *
            * @param key key to lock.
            * @throws IllegalMonitorStateException if the current thread does not hold this lock MTODO
            */
            template<typename K>
            boost::future<void> unlock(const K &key) {
                return toVoidFuture(proxy::IMapImpl::unlock(toData(key)));
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
            boost::future<void> forceUnlock(const K &key) {
                return toVoidFuture(proxy::IMapImpl::forceUnlock(toData(key)));
            }

            /**
            * Adds an interceptor for this map. Added interceptor will intercept operations
            * and execute user defined methods and will cancel operations if user defined method throw exception.
            *
            *
            * Interceptor should extend either Portable or IdentifiedSerializable.
            * Notice that map interceptor runs on the nodes. Because of that same class should be implemented in java side
            * with same classId and factoryId.
            * @param interceptor map interceptor
            * @return id of registered interceptor
            */
            template<typename MapInterceptor>
            boost::future<std::string> addInterceptor(MapInterceptor &interceptor) {
                return proxy::IMapImpl::addInterceptor(toData(interceptor));
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
            template<typename Listener>
            boost::future<std::string> addEntryListener(Listener &&listener, bool includeValue) {
                return proxy::IMapImpl::addEntryListener(
                        std::unique_ptr<impl::BaseEventHandler>(
                                new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler>(
                                        getName(), getContext().getClientClusterService(),
                                        getContext().getSerializationService(),
                                        std::forward<Listener>(listener),
                                        includeValue, getContext().getLogger())), includeValue);
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
            template<typename Listener, typename P>
            boost::future<std::string>
            addEntryListener(Listener &&listener, const P &predicate, bool includeValue) {
                return proxy::IMapImpl::addEntryListener(
                        std::unique_ptr<impl::BaseEventHandler>(
                                new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler>(
                                        getName(), getContext().getClientClusterService(),
                                        getContext().getSerializationService(),
                                        std::forward<Listener>(listener),
                                        includeValue, getContext().getLogger())), toData<P>(predicate), includeValue);
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
            template<typename Listener, typename K>
            boost::future<std::string> addEntryListener(Listener &&listener, bool includeValue, const K &key) {
                return proxy::IMapImpl::addEntryListener(
                        std::unique_ptr<impl::BaseEventHandler>(
                                new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerToKeyCodec::AbstractEventHandler>(
                                        getName(), getContext().getClientClusterService(),
                                        getContext().getSerializationService(),
                                        std::forward<Listener>(listener),
                                        includeValue, getContext().getLogger())), includeValue, toData<K>(key));
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
            boost::future<boost::optional<EntryView<K, V>>> getEntryView(const K &key) {
                return proxy::IMapImpl::getEntryViewData(toData(key)).then([=] (boost::future<std::unique_ptr<map::DataEntryView>> f) {
                    auto dataView = f.get();
                    if (!dataView) {
                        return boost::optional<EntryView<K, V>>();
                    }
                    auto v = toObject<V>(dataView->getValue());
                    return boost::make_optional(EntryView<K, V>(key, std::move(v).value(), *std::move(dataView)));
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
                return evictInternal(toData(key));
            }

            /**
            * Returns the entries for the given keys.
            *
            * @param keys keys to get
            * @return map of entries
            */
            template<typename K, typename V>
            boost::future<std::unordered_map<K, V>> getAll(const std::unordered_set<K> &keys) {
                if (keys.empty()) {
                    return boost::make_ready_future(std::unordered_map<K, V>());
                }

                std::unordered_map<int, std::vector<serialization::pimpl::Data>> partitionToKeyData;
                // group the request per parition id
                for (auto &key : keys) {
                    auto keyData = toData<K>(key);
                    auto partitionId = getPartitionId(keyData);
                    partitionToKeyData[partitionId].push_back(std::move(keyData));
                }

                std::vector<boost::future<EntryVector>> futures;
                futures.reserve(partitionToKeyData.size());
                for (auto &entry : partitionToKeyData) {
                    futures.push_back(getAllInternal(entry.first, std::move(entry.second)));
                }

                auto val = boost::when_all(futures.begin(), futures.end());
                return boost::when_all(futures.begin(), futures.end()).then(boost::launch::deferred,
                                                                            [=](boost::future<std::vector<boost::future<EntryVector>>> resultsData) {
                                                                                std::unordered_map<K, V> result;
                                                                                for (auto &entryVectorFuture : resultsData.get()) {
                                                                                    for(auto &entry : entryVectorFuture.get()) {
                                                                                        auto val = toObject<V>(entry.second);
                                                                                        // it is guaranteed that all values are non-null
                                                                                        assert(val.has_value());
                                                                                        result[toObject<K>(entry.first).value()] = std::move(val.value());
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
            boost::future<std::vector<K>> keySet() {
                return toObjectVector<K>(proxy::IMapImpl::keySetData());
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
            template<typename K, typename P>
            boost::future<std::vector<K>> keySet(const P &predicate) {
                return toObjectVector<K>(proxy::IMapImpl::keySetData(toData(predicate)));
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
            boost::future<std::vector<K>> keySet(query::PagingPredicate<K, V> &predicate) {
                predicate.setIterationType(query::IterationType::KEY);
                return toObjectVector<K>(keySetForPagingPredicateData(toData(predicate))).then(boost::launch::deferred, [&](boost::future<std::vector<K>> f) {
                    auto keys = f.get();
                    std::vector<std::pair<K, boost::optional<V>>> entries;
                    for (auto &key : keys) {
                        entries.push_back(std::make_pair(std::move(key), boost::none));
                    }
                    auto resultEntries = sortAndGet(predicate, query::IterationType::KEY, entries);
                    std::vector<K> result;
                    result.reserve(resultEntries.size());
                    for (auto &entry : resultEntries) {
                        result.push_back(std::move(entry.first));
                    }
                    return result;
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
                return toObjectVector<V>(proxy::IMapImpl::valuesData());
            }

            /**
            * Returns a vector clone of the values contained in this map.
            * The vector is <b>NOT</b> backed by the map,
            * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
            *
            * @param predicate the criteria for values to match
            * @return a vector clone of the values contained in this map
            */
            template<typename V, typename P>
            boost::future<std::vector<V>> values(const P &predicate) {
                return toObjectVector<V>(proxy::IMapImpl::valuesData(toData(predicate)));
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
            boost::future<std::vector<V>> values(query::PagingPredicate<K, V> &predicate) {
                predicate.setIterationType(query::IterationType::VALUE);
                return toEntryObjectVector<K, V>(valuesForPagingPredicateData(toData(predicate))).then(boost::launch::deferred, [&](boost::future<std::vector<std::pair<K, V>>> f) {
                    auto entries = f.get();
                    auto resultEntries = sortAndGet(predicate, query::IterationType::VALUE, entries);
                    std::vector<V> result;
                    result.reserve(resultEntries.size());
                    for (auto &entry : resultEntries) {
                        result.push_back(std::move(entry.second.value()));
                    }
                    return result;
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
            boost::future<std::vector<std::pair<K, V>>> entrySet() {
                return toEntryObjectVector<K,V>(proxy::IMapImpl::entrySetData());
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
            template<typename K, typename V, typename P>
            boost::future<std::vector<std::pair<K, V>>> entrySet(const P &predicate) {
                return toEntryObjectVector<K,V>(proxy::IMapImpl::entrySetData(toData(predicate)));
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
            boost::future<std::vector<std::pair<K, V>>> entrySet(query::PagingPredicate<K, V> &predicate) {
                return toSortedEntryObjectVector<K, V>(predicate, query::IterationType::ENTRY);
            }

            /**
            * Adds an index to this map for the specified entries so
            * that queries can run faster.
            *
            * Let's say your map values are Employee objects.
            *
            *   class Employee : public serialization::Portable {
            *       //...
            *       private:
            *          bool active;
            *          int age;
            *          boost::future<std::string> name;
            *
            *   }
            *
            *
            * If you are querying your values mostly based on age and active then
            * you should consider indexing these fields.
            *
            *   IMap<std::string, Employee > imap = hazelcastInstance.getMap<std::string, Employee >("employees");
            *   imap.addIndex("age", true);        // ordered, since we have ranged queries for this field
            *   imap.addIndex("active", false);    // not ordered, because boolean field cannot have range
            *
            *
            * In the server side, Index  should either have a getter method or be public.
            * You should also make sure to add the indexes before adding
            * entries to this map.
            *
            * @param attribute attribute of value
            * @param ordered   <tt>true</tt> if index should be ordered,
            *                  <tt>false</tt> otherwise.
            */
            boost::future<void> addIndex(const std::string &attribute, bool ordered) {
                return toVoidFuture(proxy::IMapImpl::addIndex(attribute, ordered));
            }

            boost::future<void> clear() {
                return toVoidFuture(proxy::IMapImpl::clear());
            }

            /**
            * Applies the user defined EntryProcessor to the entry mapped by the key.
            * Returns the the ResultType which is result of the process() method of EntryProcessor.
            *
            * EntryProcessor should extend either Portable or IdentifiedSerializable.
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
            boost::future<boost::optional<ResultType>> executeOnKey(const K &key, const EntryProcessor &entryProcessor) {
                return toObject<ResultType>(executeOnKeyInternal(toData(key), toData(entryProcessor)));
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
            submitToKey(const K &key, const EntryProcessor &entryProcessor) {
                return toObject<ResultType>(submitToKeyInternal(toData(key), toData(entryProcessor)));
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
            executeOnKeys(const std::unordered_set<K> &keys, const EntryProcessor &entryProcessor) {
                return toObjectMap<K, ResultType>(executeOnKeysInternal<K, EntryProcessor>(keys, entryProcessor));
            }

            /**
            * Applies the user defined EntryProcessor to the all entries in the map.
            * Returns the results mapped by each key in the map.
            *
            *
            * EntryProcessor should extend either Portable or IdentifiedSerializable.
            * Notice that map EntryProcessor runs on the nodes. Because of that, same class should be implemented in java side
            * with same classId and factoryId.
            *
            * @tparam ResultType that entry processor will return
            * @tparam EntryProcessor type of entry processor class
            * @param entryProcessor that will be applied
            */
            template<typename K, typename ResultType, typename EntryProcessor>
            boost::future<std::unordered_map<K, boost::optional<ResultType>>> executeOnEntries(const EntryProcessor &entryProcessor) {
                return toObjectMap<K, ResultType>(proxy::IMapImpl::executeOnEntriesData(toData(entryProcessor)));
            }

            /**
            * Applies the user defined EntryProcessor to the all entries in the map.
            * Returns the results mapped by each key in the map.
            *
            *
            * EntryProcessor should extend either Portable or IdentifiedSerializable.
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
            executeOnEntries(const EntryProcessor &entryProcessor, const P &predicate) {
                return toObjectMap<K, ResultType>(proxy::IMapImpl::executeOnEntriesData(toData(entryProcessor),
                                                                                 toData(predicate)));
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
                std::unordered_map<int, EntryVector> entryMap;
                for (auto &entry : entries) {
                    serialization::pimpl::Data keyData = toData(entry.first);
                    int partitionId = getPartitionId(keyData);
                    entryMap[partitionId].push_back(std::make_pair(keyData, toData(entry.second)));
                }

                std::vector<boost::future<protocol::ClientMessage>> resultFutures;
                for (auto &&partitionEntry : entryMap) {
                    auto partitionId = partitionEntry.first;
                    resultFutures.push_back(putAllInternal(partitionId, std::move(partitionEntry.second)));
                }
                return boost::when_all(resultFutures.begin(), resultFutures.end()).then(boost::launch::deferred,
                                                                                        [](boost::future<std::vector<boost::future<protocol::ClientMessage>>> futures) {
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
            monitor::LocalMapStats &getLocalMapStats() {
                return localMapStats;
            }
        protected:
            /**
             * Default TTL value of a record.
             */
            static constexpr std::chrono::milliseconds UNSET{-1};

            monitor::impl::LocalMapStatsImpl localMapStats;

            virtual boost::future<std::unique_ptr<serialization::pimpl::Data>> getInternal(serialization::pimpl::Data &&keyData) {
                return proxy::IMapImpl::getData(keyData);
            }

            virtual boost::future<bool> containsKeyInternal(serialization::pimpl::Data &&keyData) {
                return proxy::IMapImpl::containsKey(keyData);
            }

            virtual boost::future<std::unique_ptr<serialization::pimpl::Data>> removeInternal(
                    serialization::pimpl::Data &&keyData) {
                return proxy::IMapImpl::removeData(keyData);
            }

            virtual boost::future<bool> removeInternal(
                    serialization::pimpl::Data &&keyData, const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::remove(keyData, valueData);
            }

            virtual boost::future<protocol::ClientMessage> removeAllInternal(const serialization::pimpl::Data &predicateData) {
                return proxy::IMapImpl::removeAll(predicateData);
            }

            virtual boost::future<protocol::ClientMessage> deleteInternal(serialization::pimpl::Data &&keyData) {
                return proxy::IMapImpl::deleteEntry(keyData);
            }

            virtual boost::future<bool> tryRemoveInternal(serialization::pimpl::Data &&keyData, std::chrono::steady_clock::duration timeout) {
                return proxy::IMapImpl::tryRemove(keyData, timeout);
            }

            virtual boost::future<bool> tryPutInternal(serialization::pimpl::Data &&keyData,
                                                       const serialization::pimpl::Data &valueData, std::chrono::steady_clock::duration timeout) {
                return proxy::IMapImpl::tryPut(keyData, valueData, timeout);
            }

            virtual boost::future<std::unique_ptr<serialization::pimpl::Data>> putInternal(serialization::pimpl::Data &&keyData,
                                                                          const serialization::pimpl::Data &valueData,
                                                                          std::chrono::steady_clock::duration ttl) {
                return proxy::IMapImpl::putData(keyData, valueData, ttl);
            }

            virtual boost::future<protocol::ClientMessage> tryPutTransientInternal(serialization::pimpl::Data &&keyData,
                                                                                   const serialization::pimpl::Data &valueData, std::chrono::steady_clock::duration ttl) {
                return proxy::IMapImpl::putTransient(keyData, valueData, ttl);
            }

            virtual boost::future<std::unique_ptr<serialization::pimpl::Data>>
            putIfAbsentInternal(serialization::pimpl::Data &&keyData,
                                const serialization::pimpl::Data &valueData,
                                std::chrono::steady_clock::duration ttl) {
                return proxy::IMapImpl::putIfAbsentData(keyData, valueData, ttl);
            }

            virtual boost::future<bool> replaceIfSameInternal(serialization::pimpl::Data &&keyData,
                                                              const serialization::pimpl::Data &valueData,
                                                              const serialization::pimpl::Data &newValueData) {
                return proxy::IMapImpl::replace(keyData, valueData, newValueData);
            }

            virtual boost::future<std::unique_ptr<serialization::pimpl::Data>>
            replaceInternal(serialization::pimpl::Data &&keyData,
                            const serialization::pimpl::Data &valueData) {
                return proxy::IMapImpl::replaceData(keyData, valueData);

            }

            virtual boost::future<protocol::ClientMessage>
            setInternal(serialization::pimpl::Data &&keyData, const serialization::pimpl::Data &valueData,
                        std::chrono::steady_clock::duration ttl) {
                return proxy::IMapImpl::set(keyData, valueData, ttl);
            }

            virtual boost::future<bool> evictInternal(serialization::pimpl::Data &&keyData) {
                return proxy::IMapImpl::evict(keyData);
            }

            virtual boost::future<EntryVector>
            getAllInternal(int partitionId, std::vector<serialization::pimpl::Data> &&partitionKeys) {
                return proxy::IMapImpl::getAllData(partitionId,
                                                   std::forward<std::vector<serialization::pimpl::Data>>(partitionKeys));
            }

            virtual boost::future<std::unique_ptr<serialization::pimpl::Data>>
            executeOnKeyInternal(serialization::pimpl::Data &&keyData,
                                 const serialization::pimpl::Data &processor) {
                return proxy::IMapImpl::executeOnKeyData(keyData, processor);
            }

            boost::future<std::unique_ptr<serialization::pimpl::Data>>
            submitToKeyInternal(const serialization::pimpl::Data &keyData,
                                const serialization::pimpl::Data &processor) {
                return submitToKeyData(keyData, processor);
            }

            template<typename K, typename EntryProcessor>
            boost::future<EntryVector> executeOnKeysInternal(const std::unordered_set<K> &keys, const EntryProcessor &entryProcessor) {
                if (keys.empty()) {
                    return boost::make_ready_future(EntryVector());
                }
                std::vector<serialization::pimpl::Data> keysData;
                std::for_each(keys.begin(), keys.end(), [&](const K &key) { keysData.push_back(toData<K>(key)); });
                return proxy::IMapImpl::executeOnKeysData(keysData, toData<EntryProcessor>(entryProcessor));
            }

            virtual boost::future<protocol::ClientMessage>
            putAllInternal(int partitionId, EntryVector &&entries) {
                return proxy::IMapImpl::putAllData(partitionId, entries);
            }

        private:
            IMap(const std::string &instanceName, spi::ClientContext *context) : proxy::IMapImpl(instanceName,
                                                                                                 context) {}

            template<typename K, typename V>
            boost::future<std::vector<std::pair<K, V>>> toSortedEntryObjectVector(query::PagingPredicate<K, V> &predicate, query::IterationType iterationType) {
                return toEntryObjectVector<K, V>(proxy::IMapImpl::entrySetForPagingPredicateData(toData(predicate))).then(
                        boost::launch::deferred, [&](boost::future<std::vector<std::pair<K, V>>> f) {
                            auto entries = sortAndGet<K, V>(predicate, iterationType, f.get());
                            std::vector<std::pair<K, V>> result;
                            result.reserve(entries.size());
                            for_each(entries.begin(), entries.end(), [&](std::pair<K, boost::optional<V>> &entry) {
                                result.push_back(
                                        std::make_pair(std::move(entry.first), std::move(entry.second.value())));
                            });
                            return result;
                        });
            }

            template<typename K, typename V>
            std::vector<std::pair<K, boost::optional<V>>> sortAndGet(query::PagingPredicate<K, V> &predicate, query::IterationType iterationType, std::vector<std::pair<K, V>> entries) {
                std::vector<std::pair<K, boost::optional<V>>> optionalEntries;
                optionalEntries.reserve(entries.size());
                for(auto &&pair : entries) {
                    optionalEntries.emplace_back(pair.first, boost::make_optional(pair.second));
                }
                return sortAndGet(predicate, iterationType, optionalEntries);
            }

            template<typename K, typename V>
            std::vector<std::pair<K, boost::optional<V>>> sortAndGet(query::PagingPredicate<K, V> &predicate, query::IterationType iterationType, std::vector<std::pair<K, boost::optional<V>>> entries) {
                std::sort(entries.begin(), entries.end(), [&] (const std::pair<K, boost::optional<V>> &lhs, const std::pair<K, boost::optional<V>> &rhs) {
                    auto comparator = predicate.getComparator();
                    if (!comparator) {
                        switch(predicate.getIterationType()) {
                            case query::IterationType::VALUE:
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

                std::pair<size_t, size_t> range = updateAnchor<K, V>(entries, predicate, iterationType);

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

