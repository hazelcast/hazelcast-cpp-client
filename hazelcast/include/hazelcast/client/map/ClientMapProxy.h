/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_MAP_CLIENTMAPPROXY_H_
#define HAZELCAST_CLIENT_MAP_CLIENTMAPPROXY_H_

#include <string>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <climits>
#include <assert.h>

#include "hazelcast/client/monitor/LocalMapStats.h"
#include "hazelcast/client/monitor/impl/NearCacheStatsImpl.h"
#include "hazelcast/client/monitor/impl/LocalMapStatsImpl.h"
#include "hazelcast/client/impl/EntryArrayImpl.h"
#include "hazelcast/client/proxy/IMapImpl.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/EntryView.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/Future.h"
#include "hazelcast/client/impl/ClientMessageDecoder.h"
#include "hazelcast/client/internal/ClientDelegatingFuture.h"
#include "hazelcast/util/ExceptionUtil.h"

// Codecs
#include "hazelcast/client/protocol/codec/MapAddEntryListenerToKeyCodec.h"
#include "hazelcast/client/protocol/codec/MapAddEntryListenerWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapSubmitToKeyCodec.h"
#include "hazelcast/client/protocol/codec/MapSetCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveCodec.h"


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace adaptor {
            template<typename K, typename V>
            class RawPointerMap;
        }

        namespace map {
            /**
            * Concurrent, distributed, observable and queryable map client.
            *
            * Notice that this class have a private constructor.
            * You can access get an IMap in the following way
            *
            *      ClientConfig clientConfig;
            *      HazelcastClient client(clientConfig);
            *      IMap<int,std::string> imap = client.getMap<int,std::string>("aKey");
            *
            * @param <K> key
            * @param <V> value
            */
            template<typename K, typename V>
            class ClientMapProxy : public proxy::IMapImpl {
                friend class adaptor::RawPointerMap<K, V>;

            public:
                ClientMapProxy(const std::string &instanceName, spi::ClientContext *context)
                        : proxy::IMapImpl(instanceName, context) {
                }

                /**
                * check if this map contains key.
                * @param key
                * @return true if contains, false otherwise
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool containsKey(const K &key) {
                    serialization::pimpl::Data keyData = toData(key);
                    return containsKeyInternal(keyData);
                }

                /**
                * check if this map contains value.
                * @param value
                * @return true if contains, false otherwise
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool containsValue(const V &value) {
                    return proxy::IMapImpl::containsValue(toData(value));
                }

                /**
                * get the value.
                * @param key
                * @return value value in shared_ptr, if there is no mapping for key
                * then return NULL in shared_ptr.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                boost::shared_ptr<V> get(const K &key) {
                    serialization::pimpl::Data keyData = toData(key);
                    return getInternal(keyData);
                }

                /**
                * put new entry into map.
                * @param key
                * @param value
                * @return the previous value in shared_ptr, if there is no mapping for key
                * @throws IClassCastException if the type of the specified elements are incompatible with the server side.
                * then returns NULL in shared_ptr.
                */
                boost::shared_ptr<V> put(const K &key, const V &value) {
                    return put(key, value, -1);
                }

                /**
                * Puts an entry into this map with a given ttl (time to live) value.
                * Entry will expire and get evicted after the ttl. If ttl is 0, then
                * the entry lives forever.
                *
                * @param key              key of the entry
                * @param value            value of the entry
                * @param ttlInMillis      maximum time for this entry to stay in the map in milliseconds,0 means infinite.
                * @return the previous value in shared_ptr, if there is no mapping for key
                * then returns NULL in shared_ptr.
                */
                boost::shared_ptr<V> put(const K &key, const V &value, int64_t ttlInMillis) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data valueData = toData(value);

                    return boost::shared_ptr<V>(toObject<V>(putInternal(keyData, valueData, ttlInMillis)));
                }

                /**
                * remove entry form map
                * @param key
                * @return the previous value in shared_ptr, if there is no mapping for key
                * then returns NULL in shared_ptr.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                boost::shared_ptr<V> remove(const K &key) {
                    serialization::pimpl::Data keyData = toData(key);

                    std::auto_ptr<serialization::pimpl::Data> response = removeInternal(keyData);
                    return boost::shared_ptr<V>(toObject<V>(response));
                }

                /**
                * removes entry from map if there is an entry with same key and value.
                * @param key
                * @param value
                * @return true if remove is successful false otherwise
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                bool remove(const K &key, const V &value) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data valueData = toData(value);

                    return removeInternal(keyData, valueData);
                }

                /**
                 * Removes all entries which match with the supplied predicate.
                 * If this map has index, matching entries will be found via index search, otherwise they will be found by full-scan.
                 *
                 * @param predicate matching entries with this predicate will be removed from this map
                 */
                void removeAll(const query::Predicate &predicate) {
                    serialization::pimpl::Data predicateData = toData(predicate);

                    removeAllInternal(predicateData);
                }

                /**
                * removes entry from map.
                * Does not return anything.
                * @param key The key of the map entry to remove.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                void deleteEntry(const K &key) {
                    serialization::pimpl::Data keyData = toData(key);

                    deleteInternal(keyData);
                }

                /**
                * If this map has a MapStore this method flushes
                * all the local dirty entries by calling MapStore.storeAll() and/or MapStore.deleteAll()
                */
                void flush() {
                    proxy::IMapImpl::flush();
                }

                /**
                * Tries to remove the entry with the given key from this map
                * within specified timeout value. If the key is already locked by another
                * thread and/or member, then this operation will wait timeout
                * amount for acquiring the lock.
                *
                * @param key      key of the entry
                * @param timeoutInMillis  maximum time in milliseconds to wait for acquiring the lock
                *                 for the key
                */
                bool tryRemove(const K &key, int64_t timeoutInMillis) {
                    serialization::pimpl::Data keyData = toData(key);

                    return tryRemoveInternal(keyData, timeoutInMillis);
                }

                /**
                * Tries to put the given key, value into this map within specified
                * timeout value. If this method returns false, it means that
                * the caller thread couldn't acquire the lock for the key within
                * timeout duration, thus put operation is not successful.
                *
                * @param key      key of the entry
                * @param value    value of the entry
                * @param timeoutInMillis  maximum time to wait in milliseconds
                * @return <tt>true</tt> if the put is successful, <tt>false</tt>
                *         otherwise.
                */
                bool tryPut(const K &key, const V &value, int64_t timeoutInMillis) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data valueData = toData(value);

                    return tryPutInternal(keyData, valueData, timeoutInMillis);
                }

                /**
                * Same as put(K, V, int64_t, TimeUnit) but MapStore, if defined,
                * will not be called to store/persist the entry.  If ttl is 0, then
                * the entry lives forever.
                *
                * @param key          key of the entry
                * @param value        value of the entry
                * @param ttlInMillis  maximum time for this entry to stay in the map in milliseconds, 0 means infinite.
                */
                void putTransient(const K &key, const V &value, int64_t ttlInMillis) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data valueData = toData(value);

                    tryPutTransientInternal(keyData, valueData, ttlInMillis);
                }

                /**
                * Puts an entry into this map, if the specified key is not already associated with a value.
                *
                * @param key key with which the specified value is to be associated
                * @param value
                * @return the previous value in shared_ptr, if there is no mapping for key
                * then returns NULL in shared_ptr.
                */
                boost::shared_ptr<V> putIfAbsent(const K &key, const V &value) {
                    return putIfAbsent(key, value, -1);
                }

                /**
                * Puts an entry into this map with a given ttl (time to live) value
                * if the specified key is not already associated with a value.
                * Entry will expire and get evicted after the ttl.
                *
                * @param key            key of the entry
                * @param value          value of the entry
                * @param ttlInMillis    maximum time in milliseconds for this entry to stay in the map
                * @return the previous value of the entry, if there is no mapping for key
                * then returns NULL in shared_ptr.
                */
                boost::shared_ptr<V> putIfAbsent(const K &key, const V &value, int64_t ttlInMillis) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data valueData = toData(value);

                    std::auto_ptr<serialization::pimpl::Data> response = putIfAbsentInternal(keyData, valueData,
                                                                                             ttlInMillis);
                    return boost::shared_ptr<V>(toObject<V>(response));
                }

                /**
                * Replaces the entry for a key only if currently mapped to a given value.
                * @param key key with which the specified value is associated
                * @param oldValue value expected to be associated with the specified key
                * @param newValue
                * @return <tt>true</tt> if the value was replaced
                */
                bool replace(const K &key, const V &oldValue, const V &newValue) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data oldValueData = toData(oldValue);
                    serialization::pimpl::Data newValueData = toData(newValue);

                    return replaceIfSameInternal(keyData, oldValueData, newValueData);
                }

                /**
                * Replaces the entry for a key only if currently mapped to some value.
                * @param key key with which the specified value is associated
                * @param value
                * @return the previous value of the entry, if there is no mapping for key
                * then returns NULL in shared_ptr.
                */
                boost::shared_ptr<V> replace(const K &key, const V &value) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data valueData = toData(value);

                    return boost::shared_ptr<V>(toObject<V>(replaceInternal(keyData, valueData)));
                }

                /**
                * Puts an entry into this map.
                * Similar to put operation except that set
                * doesn't return the old value which is more efficient.
                * @param key
                * @param value
                */
                void set(const K &key, const V &value) {
                    set(key, value, -1);
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
                void set(const K &key, const V &value, int64_t ttl) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data valueData = toData(value);

                    setInternal(keyData, valueData, ttl);
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
                void lock(const K &key) {
                    lock(key, -1);
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
                * @param leaseTime time in milliseconds to wait before releasing the lock.
                */
                void lock(const K &key, int64_t leaseTime) {
                    serialization::pimpl::Data keyData = toData(key);

                    proxy::IMapImpl::lock(toData(key), leaseTime);
                }

                /**
                * Checks the lock for the specified key.
                * <p>If the lock is acquired then returns true, else false.
                *
                *
                * @param key key to lock to be checked.
                * @return <tt>true</tt> if lock is acquired, <tt>false</tt> otherwise.
                */
                bool isLocked(const K &key) {
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
                bool tryLock(const K &key) {
                    return tryLock(key, 0);
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
                * @param timeInMillis     maximum time in milliseconds to wait for the lock
                * @return <tt>true</tt> if the lock was acquired and <tt>false</tt>
                *         if the waiting time elapsed before the lock was acquired.
                */
                bool tryLock(const K &key, int64_t timeInMillis) {
                    return proxy::IMapImpl::tryLock(toData(key), timeInMillis);
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
                void unlock(const K &key) {
                    proxy::IMapImpl::unlock(toData(key));
                }

                /**
                * Releases the lock for the specified key regardless of the lock owner.
                * It always successfully unlocks the key, never blocks
                * and returns immediately.
                *
                *
                * @param key key to lock.
                */
                void forceUnlock(const K &key) {
                    proxy::IMapImpl::forceUnlock(toData(key));
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
                std::string addInterceptor(MapInterceptor &interceptor) {
                    return proxy::IMapImpl::addInterceptor(interceptor);
                }

                /**
                * Removes the given interceptor for this map. So it will not intercept operations anymore.
                *
                *
                * @param id registration id of map interceptor
                */
                void removeInterceptor(const std::string &id) {
                    proxy::IMapImpl::removeInterceptor(id);
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
                std::string addEntryListener(EntryListener<K, V> &listener, bool includeValue) {
                    client::impl::EntryEventHandler<K, V, protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                            new client::impl::EntryEventHandler<K, V, protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler>(
                                    getName(), getContext().getClientClusterService(),
                                    getContext().getSerializationService(),
                                    listener,
                                    includeValue);
                    return proxy::IMapImpl::addEntryListener(entryEventHandler, includeValue);
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
                std::string
                addEntryListener(EntryListener<K, V> &listener, const query::Predicate &predicate, bool includeValue) {
                    client::impl::EntryEventHandler<K, V, protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler> *entryEventHandler =
                            new client::impl::EntryEventHandler<K, V, protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler>(
                                    getName(), getContext().getClientClusterService(),
                                    getContext().getSerializationService(),
                                    listener,
                                    includeValue);
                    return proxy::IMapImpl::addEntryListener(entryEventHandler, predicate, includeValue);
                }

                /**
                * Removes the specified entry listener
                * Returns silently if there is no such listener added before.
                *
                *
                * @param registrationId id of registered listener
                *
                * @return true if registration is removed, false otherwise
                */
                bool removeEntryListener(const std::string &registrationId) {
                    return proxy::IMapImpl::removeEntryListener(registrationId);
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
                std::string addEntryListener(EntryListener<K, V> &listener, const K &key, bool includeValue) {
                    serialization::pimpl::Data keyData = toData(key);
                    client::impl::EntryEventHandler<K, V, protocol::codec::MapAddEntryListenerToKeyCodec::AbstractEventHandler> *entryEventHandler =
                            new client::impl::EntryEventHandler<K, V, protocol::codec::MapAddEntryListenerToKeyCodec::AbstractEventHandler>(
                                    getName(), getContext().getClientClusterService(),
                                    getContext().getSerializationService(),
                                    listener,
                                    includeValue);
                    return proxy::IMapImpl::addEntryListener(entryEventHandler, keyData, includeValue);
                }

                /**
                * Returns the <tt>EntryView</tt> for the specified key.
                *
                *
                * @param key key of the entry
                * @return <tt>EntryView</tt> of the specified key
                * @see EntryView
                */
                EntryView<K, V> getEntryView(const K &key) {
                    serialization::pimpl::Data keyData = toData(key);
                    std::auto_ptr<map::DataEntryView> dataEntryView = proxy::IMapImpl::getEntryViewData(keyData);
                    std::auto_ptr<V> v = toObject<V>(dataEntryView->getValue());
                    EntryView<K, V> view(key, *v, *dataEntryView);
                    return view;
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
                bool evict(const K &key) {
                    serialization::pimpl::Data keyData = toData(key);

                    return evictInternal(keyData);
                }

                /**
                * Evicts all keys from this map except locked ones.
                * <p/>
                * If a <tt>MapStore</tt> is defined for this map, deleteAll is <strong>not</strong> called by this method.
                * If you do want to deleteAll to be called use the #clear() method.
                * <p/>
                * The EVICT_ALL event is fired for any registered listeners.
                * See EntryListener#mapEvicted(MapEvent)}.
                *
                * @see #clear()
                */
                void evictAll() {
                    proxy::IMapImpl::evictAll();
                }

                /**
                * Returns the entries for the given keys.
                *
                * @param keys keys to get
                * @return map of entries
                */
                std::map<K, V> getAll(const std::set<K> &keys) {
                    if (keys.empty()) {
                        return std::map<K, V>();
                    }

                    std::map<int, std::vector<KEY_DATA_PAIR> > partitionToKeyData;
                    // group the request per parition id
                    for (typename std::set<K>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
                        serialization::pimpl::Data keyData = toData<K>(*it);

                        int partitionId = getPartitionId(keyData);

                        partitionToKeyData[partitionId].push_back(std::make_pair(&(*it), toShared(keyData)));
                    }

                    std::map<K, V> result;
                    getAllInternal(partitionToKeyData, result);
                    return result;
                }

                /**
                * Returns a vector clone of the keys contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the vector, and vice-versa.
                *
                * @return a vector clone of the keys contained in this map
                */
                std::vector<K> keySet() {
                    std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData();
                    size_t size = dataResult.size();
                    std::vector<K> keys(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::auto_ptr<K> key = toObject<K>(dataResult[i]);
                        keys[i] = *key;
                    }
                    return keys;
                }

                /**
                  * @deprecated This API is deprecated in favor of @sa{keySet(const query::Predicate &predicate)}
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
                std::vector<K> keySet(const serialization::IdentifiedDataSerializable &predicate) {
                    const query::Predicate *p = (const query::Predicate *) (&predicate);
                    return keySet(*p);
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
                std::vector<K> keySet(const query::Predicate &predicate) {
                    std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData(predicate);
                    size_t size = dataResult.size();
                    std::vector<K> keys(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::auto_ptr<K> key = toObject<K>(dataResult[i]);
                        keys[i] = *key;
                    }
                    return keys;
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
                std::vector<K> keySet(query::PagingPredicate<K, V> &predicate) {
                    predicate.setIterationType(query::KEY);

                    std::vector<serialization::pimpl::Data> dataResult = keySetForPagingPredicateData(predicate);

                    EntryVector entryResult;
                    for (std::vector<serialization::pimpl::Data>::iterator it = dataResult.begin();
                         it != dataResult.end(); ++it) {
                        entryResult.push_back(std::pair<serialization::pimpl::Data, serialization::pimpl::Data>(*it,
                                                                                                                serialization::pimpl::Data()));
                    }

                    client::impl::EntryArrayImpl<K, V> entries(entryResult, getContext().getSerializationService());
                    entries.sort(query::KEY, predicate.getComparator());

                    std::pair<size_t, size_t> range = updateAnchor<K, V>(entries, predicate, query::KEY);

                    std::vector<K> result;
                    for (size_t i = range.first; i < range.second; ++i) {
                        result.push_back(*entries.getKey(i));
                    }

                    return result;
                }

                /**
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                * @return a vector clone of the values contained in this map
                */
                std::vector<V> values() {
                    std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData();
                    size_t size = dataResult.size();
                    std::vector<V> values(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::auto_ptr<V> value = toObject<V>(dataResult[i]);
                        values[i] = *value;
                    }
                    return values;
                }

                /**
                * @deprecated This API is deprecated in favor of @sa{values(const query::Predicate &predicate)}
                *
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                * @param predicate the criteria for values to match
                * @return a vector clone of the values contained in this map
                */
                std::vector<V> values(const serialization::IdentifiedDataSerializable &predicate) {
                    const query::Predicate *p = (const query::Predicate *) (&predicate);
                    return values(*p);
                }

                /**
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                * @param predicate the criteria for values to match
                * @return a vector clone of the values contained in this map
                */
                std::vector<V> values(const query::Predicate &predicate) {
                    std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData(predicate);
                    size_t size = dataResult.size();
                    std::vector<V> values;
                    for (size_t i = 0; i < size; ++i) {
                        std::auto_ptr<V> value = toObject<V>(dataResult[i]);
                        values.push_back(*value);
                    }
                    return values;
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
                std::vector<V> values(query::PagingPredicate<K, V> &predicate) {
                    predicate.setIterationType(query::VALUE);

                    EntryVector dataResult = proxy::IMapImpl::valuesForPagingPredicateData(predicate);

                    client::impl::EntryArrayImpl<K, V> entries(dataResult, getContext().getSerializationService());

                    entries.sort(query::VALUE, predicate.getComparator());

                    std::pair<size_t, size_t> range = updateAnchor<K, V>(entries, predicate, query::VALUE);

                    std::vector<V> result;
                    for (size_t i = range.first; i < range.second; ++i) {
                        result.push_back(*entries.getValue(i));
                    }
                    return result;
                }

                /**
                * Returns a std::vector< std::pair<K, V> > clone of the mappings contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the set, and vice-versa.
                *
                * @return a vector clone of the keys mappings in this map
                */
                std::vector<std::pair<K, V> > entrySet() {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData();
                    size_t size = dataResult.size();
                    std::vector<std::pair<K, V> > entries(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::auto_ptr<K> key = toObject<K>(dataResult[i].first);
                        std::auto_ptr<V> value = toObject<V>(dataResult[i].second);
                        entries[i] = std::make_pair(*key, *value);
                    }
                    return entries;
                }

                /**
                * @deprecated This API is deprecated in favor of @sa{entrySet(const query::Predicate &predicate)}
                *
                * Queries the map based on the specified predicate and
                * returns the matching entries.
                *
                * Specified predicate runs on all members in parallel.
                *
                *
                * @param predicate query criteria
                * @return result entry vector of the query
                */
                std::vector<std::pair<K, V> > entrySet(const serialization::IdentifiedDataSerializable &predicate) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                            predicate);
                    size_t size = dataResult.size();
                    std::vector<std::pair<K, V> > entries(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::auto_ptr<K> key = toObject<K>(dataResult[i].first);
                        std::auto_ptr<V> value = toObject<V>(dataResult[i].second);
                        entries[i] = std::make_pair(*key, *value);
                    }
                    return entries;
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
                std::vector<std::pair<K, V> > entrySet(const query::Predicate &predicate) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                            predicate);
                    size_t size = dataResult.size();
                    std::vector<std::pair<K, V> > entries(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::auto_ptr<K> key = toObject<K>(dataResult[i].first);
                        std::auto_ptr<V> value = toObject<V>(dataResult[i].second);
                        entries[i] = std::make_pair(*key, *value);
                    }
                    return entries;
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
                std::vector<std::pair<K, V> > entrySet(query::PagingPredicate<K, V> &predicate) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetForPagingPredicateData(
                            predicate);

                    client::impl::EntryArrayImpl<K, V> entries(dataResult, getContext().getSerializationService());
                    entries.sort(query::ENTRY, predicate.getComparator());

                    std::pair<size_t, size_t> range = updateAnchor<K, V>(entries, predicate, query::ENTRY);

                    std::vector<std::pair<K, V> > result;
                    for (size_t i = range.first; i < range.second; ++i) {
                        std::pair<const K *, const V *> entry = entries[i];
                        result.push_back(std::pair<K, V>(*entry.first, *entry.second));
                    }
                    return result;
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
                *          std::string name;
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
                void addIndex(const std::string &attribute, bool ordered) {
                    proxy::IMapImpl::addIndex(attribute, ordered);
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
                template<typename ResultType, typename EntryProcessor>
                boost::shared_ptr<ResultType> executeOnKey(const K &key, const EntryProcessor &entryProcessor) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data processorData = toData(entryProcessor);

                    std::auto_ptr<serialization::pimpl::Data> response = executeOnKeyInternal(keyData, processorData);

                    return boost::shared_ptr<ResultType>(toObject<ResultType>(response));
                }

                template<typename ResultType, typename EntryProcessor>
                Future<ResultType> submitToKey(const K &key, const EntryProcessor &entryProcessor) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data processorData = toData(entryProcessor);

                    return submitToKeyInternal<ResultType>(keyData, processorData);
                }

                template<typename ResultType, typename EntryProcessor>
                std::map<K, boost::shared_ptr<ResultType> >
                executeOnKeys(const std::set<K> &keys, const EntryProcessor &entryProcessor) {
                    EntryVector entries = executeOnKeysInternal<EntryProcessor>(keys, entryProcessor);

                    std::map<K, boost::shared_ptr<ResultType> > result;
                    for (size_t i = 0; i < entries.size(); ++i) {
                        std::auto_ptr<K> keyObj = toObject<K>(entries[i].first);
                        std::auto_ptr<ResultType> resObj = toObject<ResultType>(entries[i].second);
                        result[*keyObj] = resObj;
                    }
                    return result;
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
                template<typename ResultType, typename EntryProcessor>
                std::map<K, boost::shared_ptr<ResultType> > executeOnEntries(const EntryProcessor &entryProcessor) {
                    EntryVector entries = proxy::IMapImpl::executeOnEntriesData<EntryProcessor>(entryProcessor);
                    std::map<K, boost::shared_ptr<ResultType> > result;
                    for (size_t i = 0; i < entries.size(); ++i) {
                        std::auto_ptr<K> keyObj = toObject<K>(entries[i].first);
                        std::auto_ptr<ResultType> resObj = toObject<ResultType>(entries[i].second);
                        result[*keyObj] = resObj;
                    }
                    return result;
                }

                /**
                * @deprecated This API is deprecated in favor of
                * @sa{executeOnEntries(const EntryProcessor &entryProcessor, const query::Predicate &predicate)}
                *
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
                template<typename ResultType, typename EntryProcessor>
                std::map<K, boost::shared_ptr<ResultType> > executeOnEntries(const EntryProcessor &entryProcessor,
                                                                             const serialization::IdentifiedDataSerializable &predicate) {
                    const query::Predicate *p = (const query::Predicate *) (&predicate);
                    return executeOnEntries<ResultType, EntryProcessor>(entryProcessor, *p);
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
                template<typename ResultType, typename EntryProcessor>
                std::map<K, boost::shared_ptr<ResultType> >
                executeOnEntries(const EntryProcessor &entryProcessor, const query::Predicate &predicate) {
                    EntryVector entries = proxy::IMapImpl::executeOnEntriesData<EntryProcessor>(entryProcessor,
                                                                                                predicate);
                    std::map<K, boost::shared_ptr<ResultType> > result;
                    for (size_t i = 0; i < entries.size(); ++i) {
                        std::auto_ptr<K> keyObj = toObject<K>(entries[i].first);
                        std::auto_ptr<ResultType> resObj = toObject<ResultType>(entries[i].second);
                        result[*keyObj] = resObj;
                    }
                    return result;
                }

                /**
                * Returns the number of key-value mappings in this map.  If the
                * map contains more than <tt>Integer.MAX_VALUE</tt> elements, returns
                * <tt>Integer.MAX_VALUE</tt>.
                *
                * @return the number of key-value mappings in this map
                */
                int size() {
                    return proxy::IMapImpl::size();
                }

                /**
                * Returns <tt>true</tt> if this map contains no key-value mappings.
                *
                * @return <tt>true</tt> if this map contains no key-value mappings
                */
                bool isEmpty() {
                    return proxy::IMapImpl::isEmpty();
                }


                /**
                * Copies all of the mappings from the specified map to this map
                * (optional operation).  The effect of this call is equivalent to that
                * of calling put(k, v) on this map once
                * for each mapping from key <tt>k</tt> to value <tt>v</tt> in the
                * specified map.  The behavior of this operation is undefined if the
                * specified map is modified while the operation is in progress.
                *
                * @param m mappings to be stored in this map
                */
                void putAll(const std::map<K, V> &entries) {
                    std::map<int, EntryVector> entryMap;

                    for (typename std::map<K, V>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
                        serialization::pimpl::Data keyData = toData(it->first);
                        serialization::pimpl::Data valueData = toData(it->second);

                        int partitionId = getPartitionId(keyData);

                        entryMap[partitionId].push_back(std::make_pair(keyData, valueData));
                    }

                    putAllInternal(entryMap);
                }

                /**
                * Removes all of the mappings from this map (optional operation).
                * The map will be empty after this call returns.
                */
                void clear() {
                    proxy::IMapImpl::clear();
                }

                serialization::pimpl::SerializationService &getSerializationService() {
                    return getContext().getSerializationService();
                }

                virtual monitor::LocalMapStats &getLocalMapStats() {
                    return stats;
                }

                virtual boost::shared_ptr<ICompletableFuture<V> > getAsync(const K &key) {
                    return boost::shared_ptr<ICompletableFuture<V> >(
                            new internal::ClientDelegatingFuture<V>(getAsyncInternal(key), getSerializationService(),
                                                                    PUT_ASYNC_RESPONSE_DECODER()));
                }

                boost::shared_ptr<ICompletableFuture<V> > putAsync(const K &key, const V &value) {
                    return putAsyncInternal(DEFAULT_TTL, util::concurrent::TimeUnit::MILLISECONDS(), NULL,
                                            util::concurrent::TimeUnit::MILLISECONDS(), toData<K>(key), value);
                }

                boost::shared_ptr<ICompletableFuture<V> >
                putAsync(const K &key, const V &value, int64_t ttl, const util::concurrent::TimeUnit &ttlUnit) {
                    return putAsyncInternal(ttl, ttlUnit, NULL, ttlUnit, toData<K>(key), value);
                }

                boost::shared_ptr<ICompletableFuture<V> >
                putAsync(const K &key, const V &value, int64_t ttl, const util::concurrent::TimeUnit &ttlUnit,
                         int64_t maxIdle, const util::concurrent::TimeUnit &maxIdleUnit) {
                    return putAsyncInternal(ttl, ttlUnit, &maxIdle, maxIdleUnit, toData<K>(key), value);
                }

                boost::shared_ptr<ICompletableFuture<void> > setAsync(const K &key, const V &value) {
                    return setAsync(key, value, DEFAULT_TTL, util::concurrent::TimeUnit::MILLISECONDS());
                }

                boost::shared_ptr<ICompletableFuture<void> >
                setAsync(const K &key, const V &value, int64_t ttl, const util::concurrent::TimeUnit &ttlUnit) {
                    return setAsyncInternal(ttl, ttlUnit, NULL, ttlUnit, toData<K>(key), value);
                }

                boost::shared_ptr<ICompletableFuture<void> >
                setAsync(const K &key, const V &value, int64_t ttl, const util::concurrent::TimeUnit &ttlUnit,
                         int64_t maxIdle, const util::concurrent::TimeUnit &maxIdleUnit) {
                    return setAsyncInternal(ttl, ttlUnit, &maxIdle, maxIdleUnit, toData<K>(key), value);
                }

                boost::shared_ptr<ICompletableFuture<V> > removeAsync(const K &key) {
                    return removeAsyncInternal(toData<K>(key));
                }

            protected:
                typedef std::pair<const K *, boost::shared_ptr<serialization::pimpl::Data> > KEY_DATA_PAIR;

                /**
                 * Default TTL value of a record.
                 */
                static int64_t DEFAULT_TTL;

                /**
                 * Default Max Idle value of a record.
                 */
                static int64_t DEFAULT_MAX_IDLE;

                static const boost::shared_ptr<client::impl::ClientMessageDecoder<V> > &GET_ASYNC_RESPONSE_DECODER() {
                    return client::impl::DataMessageDecoder<protocol::codec::MapGetCodec, V>::instance();
                }

                static const boost::shared_ptr<client::impl::ClientMessageDecoder<V> > &PUT_ASYNC_RESPONSE_DECODER() {
                    return client::impl::DataMessageDecoder<protocol::codec::MapPutCodec, V>::instance();
                }

                static const boost::shared_ptr<client::impl::ClientMessageDecoder<void> > &
                SET_ASYNC_RESPONSE_DECODER() {
                    return client::impl::VoidMessageDecoder::instance();
                }

                static const boost::shared_ptr<client::impl::ClientMessageDecoder<V> > &
                REMOVE_ASYNC_RESPONSE_DECODER() {
                    return client::impl::DataMessageDecoder<protocol::codec::MapRemoveCodec, V>::instance();
                }

                virtual boost::shared_ptr<V> getInternal(serialization::pimpl::Data &keyData) {
                    return boost::shared_ptr<V>(toObject<V>(proxy::IMapImpl::getData(keyData)));
                }

                virtual bool containsKeyInternal(const serialization::pimpl::Data &keyData) {
                    return proxy::IMapImpl::containsKey(keyData);
                }

                virtual std::auto_ptr<serialization::pimpl::Data> removeInternal(
                        const serialization::pimpl::Data &keyData) {
                    return proxy::IMapImpl::removeData(keyData);
                }

                virtual bool removeInternal(
                        const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData) {
                    return proxy::IMapImpl::remove(keyData, valueData);
                }

                virtual boost::shared_ptr<ICompletableFuture<V> >
                removeAsyncInternal(const serialization::pimpl::Data &keyData) {
                    try {
                        std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapRemoveCodec::encodeRequest(
                                name, keyData, util::getCurrentThreadId());
                        boost::shared_ptr<spi::impl::ClientInvocationFuture> future = invokeOnKeyOwner(request,
                                                                                                       keyData);
                        return boost::shared_ptr<ICompletableFuture<V> >(
                                new internal::ClientDelegatingFuture<V>(future, getSerializationService(),
                                                                        REMOVE_ASYNC_RESPONSE_DECODER()));
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return boost::shared_ptr<ICompletableFuture<V> >();
                }

                virtual void removeAllInternal(const serialization::pimpl::Data &predicateData) {
                    return proxy::IMapImpl::removeAll(predicateData);
                }

                virtual void deleteInternal(const serialization::pimpl::Data &keyData) {
                    proxy::IMapImpl::deleteEntry(keyData);
                }

                virtual bool tryRemoveInternal(const serialization::pimpl::Data &keyData, int64_t timeoutInMillis) {
                    return proxy::IMapImpl::tryRemove(keyData, timeoutInMillis);
                }

                virtual bool tryPutInternal(const serialization::pimpl::Data &keyData,
                                            const serialization::pimpl::Data &valueData, int64_t timeoutInMillis) {
                    return proxy::IMapImpl::tryPut(keyData, valueData, timeoutInMillis);
                }

                virtual std::auto_ptr<serialization::pimpl::Data> putInternal(const serialization::pimpl::Data &keyData,
                                                                              const serialization::pimpl::Data &valueData,
                                                                              int64_t timeoutInMillis) {
                    return proxy::IMapImpl::putData(keyData, valueData, timeoutInMillis);
                }

                virtual void tryPutTransientInternal(const serialization::pimpl::Data &keyData,
                                                     const serialization::pimpl::Data &valueData, int64_t ttlInMillis) {
                    proxy::IMapImpl::putTransient(keyData, valueData, ttlInMillis);
                }

                virtual std::auto_ptr<serialization::pimpl::Data>
                putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                    const serialization::pimpl::Data &valueData,
                                    int64_t ttlInMillis) {
                    return proxy::IMapImpl::putIfAbsentData(keyData, valueData, ttlInMillis);
                }

                virtual bool replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                   const serialization::pimpl::Data &valueData,
                                                   const serialization::pimpl::Data &newValueData) {
                    return proxy::IMapImpl::replace(keyData, valueData, newValueData);
                }

                virtual std::auto_ptr<serialization::pimpl::Data>
                replaceInternal(const serialization::pimpl::Data &keyData,
                                const serialization::pimpl::Data &valueData) {
                    return proxy::IMapImpl::replaceData(keyData, valueData);

                }

                virtual void
                setInternal(const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData,
                            int64_t ttlInMillis) {
                    proxy::IMapImpl::set(keyData, valueData, ttlInMillis);
                }

                virtual bool evictInternal(const serialization::pimpl::Data &keyData) {
                    return proxy::IMapImpl::evict(keyData);
                }

                virtual EntryVector getAllInternal(
                        const std::map<int, std::vector<KEY_DATA_PAIR> > &partitionToKeyData,
                        std::map<K, V> &result) {

                    /**
                     * This map is needed so that we do not deserialize the response data keys but just use
                     * the keys at the original request
                     */
                    std::map<boost::shared_ptr<serialization::pimpl::Data>, const K *> dataKeyPairMap;

                    std::map<int, std::vector<serialization::pimpl::Data> > partitionKeys;

                    for (typename std::map<int, std::vector<KEY_DATA_PAIR> >::const_iterator
                                 it = partitionToKeyData.begin(); it != partitionToKeyData.end(); ++it) {
                        for (typename std::vector<KEY_DATA_PAIR>::const_iterator
                                     keyIt = it->second.begin(); keyIt != it->second.end(); ++keyIt) {
                            partitionKeys[it->first].push_back(serialization::pimpl::Data(*(*keyIt).second));

                            dataKeyPairMap[(*keyIt).second] = (*keyIt).first;
                        }
                    }
                    EntryVector allData = proxy::IMapImpl::getAllData(partitionKeys);
                    EntryVector responseEntries;
                    for (EntryVector::iterator it = allData.begin(); it != allData.end(); ++it) {
                        std::auto_ptr<V> value = toObject<V>(it->second);
                        boost::shared_ptr<serialization::pimpl::Data> keyPtr = boost::shared_ptr<serialization::pimpl::Data>(
                                new serialization::pimpl::Data(it->first));
                        const K *&keyObject = dataKeyPairMap[keyPtr];
                        assert(keyObject != 0);
                        // Use insert method instead of '[]' operator to prevent the need for
                        // std::is_default_constructible requirement for key and value
                        result.insert(std::make_pair(*keyObject, *value));
                        responseEntries.push_back(std::pair<serialization::pimpl::Data, serialization::pimpl::Data>(
                                *keyPtr, it->second));
                    }

                    return responseEntries;
                }

                virtual std::auto_ptr<serialization::pimpl::Data>
                executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                     const serialization::pimpl::Data &processor) {
                    return proxy::IMapImpl::executeOnKeyData(keyData, processor);
                }

                template<typename ResultType>
                Future<ResultType>
                submitToKeyInternal(const serialization::pimpl::Data &keyData,
                                    const serialization::pimpl::Data &processor) {
                    int partitionId = getPartitionId(keyData);

                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::MapSubmitToKeyCodec::encodeRequest(getName(),
                                                                                processor,
                                                                                keyData,
                                                                                util::getCurrentThreadId());

                    boost::shared_ptr<spi::impl::ClientInvocationFuture> clientInvocationFuture = invokeAndGetFuture(
                            request, partitionId);

                    return client::Future<ResultType>(clientInvocationFuture, getSerializationService(),
                                                      submitToKeyDecoder);
                }

                static std::auto_ptr<serialization::pimpl::Data> submitToKeyDecoder(protocol::ClientMessage &response) {
                    return protocol::codec::MapExecuteOnKeyCodec::ResponseParameters::decode(response).response;
                }

                template<typename EntryProcessor>
                EntryVector executeOnKeysInternal(const std::set<K> &keys, const EntryProcessor &entryProcessor) {
                    if (keys.empty()) {
                        return EntryVector();
                    }

                    std::vector<serialization::pimpl::Data> keysData;
                    for (typename std::set<K>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
                        keysData.push_back(toData<K>(*it));
                    }

                    serialization::pimpl::Data entryProcessorData = toData<EntryProcessor>(entryProcessor);

                    return proxy::IMapImpl::executeOnKeysData(keysData, entryProcessorData);
                }

                virtual void
                putAllInternal(const std::map<int, EntryVector> &entries) {
                    proxy::IMapImpl::putAllData(entries);
                }

                boost::shared_ptr<serialization::pimpl::Data> toShared(const serialization::pimpl::Data &data) {
                    return boost::shared_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(data));
                }

                virtual boost::shared_ptr<spi::impl::ClientInvocationFuture> getAsyncInternal(const K &key) {
                    try {
                        serialization::pimpl::Data keyData = toData<K>(key);
                        return getAsyncInternal(keyData);
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return boost::shared_ptr<spi::impl::ClientInvocationFuture>();
                }

                virtual boost::shared_ptr<spi::impl::ClientInvocationFuture>
                getAsyncInternal(const serialization::pimpl::Data &keyData) {
                    try {
                        std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapGetCodec::encodeRequest(
                                name, keyData, util::getCurrentThreadId());
                        return invokeOnKeyOwner(request, keyData);
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return boost::shared_ptr<spi::impl::ClientInvocationFuture>();
                }

                virtual boost::shared_ptr<ICompletableFuture<V> >
                putAsyncInternal(int64_t ttl, const util::concurrent::TimeUnit &ttlUnit, int64_t *maxIdle,
                                 const util::concurrent::TimeUnit &maxIdleUnit,
                                 const serialization::pimpl::Data &keyData,
                                 const V &value) {
                    try {
                        serialization::pimpl::Data valueData = toData<V>(value);
                        boost::shared_ptr<spi::impl::ClientInvocationFuture> future = putAsyncInternalData(ttl, ttlUnit,
                                                                                                           maxIdle,
                                                                                                           maxIdleUnit,
                                                                                                           keyData,
                                                                                                           valueData);
                        return boost::shared_ptr<ICompletableFuture<V> >(
                                new internal::ClientDelegatingFuture<V>(future, getSerializationService(),
                                                                        PUT_ASYNC_RESPONSE_DECODER()));
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return boost::shared_ptr<ICompletableFuture<V> >();
                }

                virtual boost::shared_ptr<ICompletableFuture<void> >
                setAsyncInternal(int64_t ttl, const util::concurrent::TimeUnit &ttlUnit, int64_t *maxIdle,
                                 const util::concurrent::TimeUnit &maxIdleUnit,
                                 const serialization::pimpl::Data &keyData, const V &value) {
                    try {
                        serialization::pimpl::Data valueData = toData<V>(value);
                        boost::shared_ptr<spi::impl::ClientInvocationFuture> future = setAsyncInternalData(ttl, ttlUnit,
                                                                                                           maxIdle,
                                                                                                           maxIdleUnit,
                                                                                                           keyData,
                                                                                                           valueData);
                        return boost::shared_ptr<ICompletableFuture<void> >(
                                new internal::ClientDelegatingFuture<void>(future, getSerializationService(),
                                                                           SET_ASYNC_RESPONSE_DECODER()));
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(e);
                    }
                    return boost::shared_ptr<ICompletableFuture<void> >();
                }

            private:
                monitor::impl::LocalMapStatsImpl stats;
            };

            template<typename K, typename V>
            int64_t ClientMapProxy<K, V>::DEFAULT_TTL = -1L;

            template<typename K, typename V>
            int64_t ClientMapProxy<K, V>::DEFAULT_MAX_IDLE = -1L;

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MAP_CLIENTMAPPROXY_H_ */

