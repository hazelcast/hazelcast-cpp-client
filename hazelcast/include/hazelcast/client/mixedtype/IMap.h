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
#include <map>
#include <vector>
#include "hazelcast/client/mixedtype/ClientMapProxy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace mixedtype {
            namespace impl {
               class HazelcastClientImpl;
            }

            /**
            * Concurrent, distributed, observable and queryable map client.
            *
            * Notice that this class have a private constructor.
            * You can access get an IMap in the following way
            *
            *      ClientConfig clientConfig;
            *      HazelcastClient client(clientConfig);
            *      MixedMap map = client.getMixedMap("aKey");
            *
            */
            class HAZELCAST_API IMap {
                friend class impl::HazelcastClientImpl;
            public:
                /**
                * check if this map contains key.
                * @param key
                * @return true if contains, false otherwise
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename K>
                bool containsKey(const K &key) {
                    return mapImpl->containsKey<K>(key);
                }

                /**
                * check if this map contains value.
                * @param value
                * @return true if contains, false otherwise
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename V>
                bool containsValue(const V &value) {
                    return mapImpl->containsValue(value);
                }

                /**
                * get the value.
                * @param key
                * @return value value in shared_ptr, if there is no mapping for key
                * then return NULL in shared_ptr.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename K>
                TypedData get(const K &key) {
                    return mapImpl->get<K>(key);
                }

                /**
                * put new entry into map.
                * @param key
                * @param value
                * @return the previous value in shared_ptr, if there is no mapping for key
                * @throws IClassCastException if the type of the specified elements are incompatible with the server side.
                * then returns NULL in shared_ptr.
                */
                template <typename K, typename V>
                TypedData put(const K &key, const V &value) {
                    return mapImpl->put(key, value);
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
                template <typename K, typename V>
                TypedData put(const K &key, const V &value, long ttlInMillis) {
                    return mapImpl->put<K, V>(key, value, ttlInMillis);
                }

                /**
                * remove entry form map
                * @param key
                * @return the previous value in shared_ptr, if there is no mapping for key
                * then returns NULL in shared_ptr.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename K>
                TypedData remove(const K &key) {
                    return mapImpl->remove<K>(key);
                }

                /**
                * removes entry from map if there is an entry with same key and value.
                * @param key
                * @param value
                * @return true if remove is successful false otherwise
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename K, typename V>
                bool remove(const K &key, const V &value) {
                    return mapImpl->remove<K, V>(key, value);
                }

                /**
                 * Removes all entries which match with the supplied predicate.
                 * If this map has index, matching entries will be found via index search, otherwise they will be found by full-scan.
                 *
                 * @param predicate matching entries with this predicate will be removed from this map
                 */
                void removeAll(const query::Predicate &predicate);

                /**
                * removes entry from map.
                * Does not return anything.
                * @param key The key of the map entry to remove.
                * @throws IClassCastException if the type of the specified element is incompatible with the server side.
                */
                template <typename K>
                void deleteEntry(const K &key) {
                    return mapImpl->deleteEntry<K>(key);
                }

                /**
                * If this map has a MapStore this method flushes
                * all the local dirty entries by calling MapStore.storeAll() and/or MapStore.deleteAll()
                */
                void flush();

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
                template <typename K>
                bool tryRemove(const K &key, long timeoutInMillis) {
                    return mapImpl->tryRemove<K>(key, timeoutInMillis);
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
                template <typename K, typename V>
                bool tryPut(const K &key, const V &value, long timeoutInMillis) {
                    return mapImpl->tryPut<K, V>(key, value, timeoutInMillis);
                }

                /**
                * Same as put(K, V, long, TimeUnit) but MapStore, if defined,
                * will not be called to store/persist the entry.  If ttl is 0, then
                * the entry lives forever.
                *
                * @param key          key of the entry
                * @param value        value of the entry
                * @param ttlInMillis  maximum time for this entry to stay in the map in milliseconds, 0 means infinite.
                */
                template <typename K, typename V>
                void putTransient(const K &key, const V &value, long ttlInMillis) {
                    return mapImpl->putTransient<K, V>(key, value, ttlInMillis);
                }

                /**
                * Puts an entry into this map, if the specified key is not already associated with a value.
                *
                * @param key key with which the specified value is to be associated
                * @param value
                * @return the previous value in shared_ptr, if there is no mapping for key
                * then returns NULL in shared_ptr.
                */
                template <typename K, typename V>
                TypedData putIfAbsent(const K &key, const V &value) {
                    return mapImpl->putIfAbsent<K, V>(key, value);
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
                template <typename K, typename V>
                TypedData putIfAbsent(const K &key, const V &value, long ttlInMillis) {
                    return mapImpl->putIfAbsent<K, V>(key, value, ttlInMillis);
                }

                /**
                * Replaces the entry for a key only if currently mapped to a given value.
                * @param key key with which the specified value is associated
                * @param oldValue value expected to be associated with the specified key
                * @param newValue The new value to be put in place of the old value.
                * @return <tt>true</tt> if the value was replaced
                */
                template <typename K, typename V, typename NEWTYPE>
                bool replace(const K &key, const V &oldValue, const NEWTYPE &newValue) {
                    return mapImpl->replace<K, V, NEWTYPE>(key, oldValue, newValue);
                }

                /**
                * Replaces the entry for a key only if currently mapped to some value.
                * @param key key with which the specified value is associated
                * @param value
                * @return the previous value of the entry, if there is no mapping for key
                * then returns NULL in shared_ptr.
                */
                template <typename K, typename V>
                TypedData replace(const K &key, const V &value) {
                    return mapImpl->replace<K, V>(key, value);
                }

                /**
                * Puts an entry into this map.
                * Similar to put operation except that set
                * doesn't return the old value which is more efficient.
                * @param key
                * @param value
                */
                template <typename K, typename V>
                void set(const K &key, const V &value) {
                    return mapImpl->set<K, V>(key, value);
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
                template <typename K, typename V>
                void set(const K &key, const V &value, long ttl) {
                    return mapImpl->set<K, V>(key, value, ttl);
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
                template <typename K>
                void lock(const K &key) {
                    return mapImpl->lock<K>(key);
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
                template <typename K>
                void lock(const K &key, long leaseTime) {
                    return mapImpl->lock<K>(key, leaseTime);
                }

                /**
                * Checks the lock for the specified key.
                * <p>If the lock is acquired then returns true, else false.
                *
                *
                * @param key key to lock to be checked.
                * @return <tt>true</tt> if lock is acquired, <tt>false</tt> otherwise.
                */
                template <typename K>
                bool isLocked(const K &key) {
                    return mapImpl->isLocked<K>(key);
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
                template <typename K>
                bool tryLock(const K &key) {
                    return mapImpl->tryLock<K>(key);
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
                template <typename K>
                bool tryLock(const K &key, long timeInMillis) {
                    return mapImpl->tryLock<K>(key, timeInMillis);
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
                template <typename K>
                void unlock(const K &key) {
                    mapImpl->unlock<K>(key);
                }

                /**
                * Releases the lock for the specified key regardless of the lock owner.
                * It always successfully unlocks the key, never blocks
                * and returns immediately.
                *
                *
                * @param key key to lock.
                */
                template <typename K>
                void forceUnlock(const K &key) {
                    mapImpl->forceUnlock<K>(key);
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
                    return mapImpl->addInterceptor<MapInterceptor>(interceptor);
                }

                /**
                * Removes the given interceptor for this map. So it will not intercept operations anymore.
                *
                *
                * @param id registration id of map interceptor
                */
                void removeInterceptor(const std::string &id);

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
                std::string addEntryListener(MixedEntryListener &listener, bool includeValue);

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
                addEntryListener(MixedEntryListener &listener, const query::Predicate &predicate, bool includeValue);

                /**
                * Removes the specified entry listener
                * Returns silently if there is no such listener added before.
                *
                *
                * @param registrationId id of registered listener
                *
                * @return true if registration is removed, false otherwise
                */
                bool removeEntryListener(const std::string &registrationId);

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
                template <typename K>
                std::string addEntryListener(const K &key, MixedEntryListener &listener, bool includeValue) {
                    return mapImpl->addEntryListener<K>(key, listener, includeValue);
                }

                /**
                * Returns the <tt>EntryView</tt> for the specified key.
                *
                *
                * @param key key of the entry
                * @return <tt>EntryView</tt> of the specified key
                * @see EntryView
                */
                template <typename K>
                std::unique_ptr<EntryView<TypedData, TypedData> > getEntryView(const K &key) {
                    return mapImpl->getEntryView<K>(key);
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
                template <typename K>
                bool evict(const K &key) {
                    return mapImpl->evict<K>(key);
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
                void evictAll();

                /**
                * Returns the entries for the given keys.
                *
                * @param keys keys to get
                * @return map of entries
                */
                template <typename K>
                std::vector<std::pair<TypedData, TypedData> > getAll(const std::set<K> &keys) {
                    return mapImpl->getAll<K>(keys);
                }

                /**
                * Returns a vector clone of the keys contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the vector, and vice-versa.
                *
                * @return a vector clone of the keys contained in this map
                */
                std::vector<TypedData> keySet();

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
                std::vector<TypedData> keySet(const serialization::IdentifiedDataSerializable &predicate);

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
                std::vector<TypedData> keySet(const query::Predicate &predicate);

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
                template <typename K, typename V>
                std::vector<K> keySet(query::PagingPredicate <K, V> &predicate) {
                    return mapImpl->keySet<K, V>(predicate);
                }

                /**
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                * @return a vector clone of the values contained in this map
                */
                std::vector<TypedData> values();

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
                std::vector<TypedData> values(const serialization::IdentifiedDataSerializable &predicate);

                /**
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                * @param predicate the criteria for values to match
                * @return a vector clone of the values contained in this map
                */
                std::vector<TypedData> values(const query::Predicate &predicate);

                /**
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                *
                * @param predicate the criteria for values to match
                * @return a vector clone of the values contained in this map
                */
                template <typename K, typename V>
                std::vector<V> values(query::PagingPredicate <K, V> &predicate) {
                    return mapImpl->values<K, V>(predicate);
                }

                /**
                * Returns a std::vector< std::pair<K, V> > clone of the mappings contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the set, and vice-versa.
                *
                * @return a vector clone of the keys mappings in this map
                */
                std::vector<std::pair<TypedData, TypedData> > entrySet();

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
                std::vector<std::pair<TypedData, TypedData> > entrySet(const serialization::IdentifiedDataSerializable &predicate);

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
                std::vector<std::pair<TypedData, TypedData> > entrySet(const query::Predicate &predicate);

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
                template <typename K, typename V>
                std::vector<std::pair<K, V> > entrySet(query::PagingPredicate <K, V> &predicate) {
                    return mapImpl->entrySet<K, V>(predicate);
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
                void addIndex(const std::string &attribute, bool ordered);

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
                template<typename K, typename EntryProcessor>
                TypedData executeOnKey(const K &key, const EntryProcessor &entryProcessor) {
                    return mapImpl->executeOnKey<K, EntryProcessor>(key, entryProcessor);
                }

                template<typename K, typename EntryProcessor>
                boost::future<TypedData> submitToKey(const K &key, const EntryProcessor &entryProcessor) {
                    return mapImpl->submitToKey<K, EntryProcessor>(key, entryProcessor);
                }

                template<typename K, typename EntryProcessor>
                std::map<K, TypedData> executeOnKeys(const std::set<K> &keys, const EntryProcessor &entryProcessor) {
                    return mapImpl->executeOnKeys<K>(keys, entryProcessor);
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
                * @tparam EntryProcessor type of entry processor class
                * @param entryProcessor that will be applied
                */
                template<typename EntryProcessor>
                std::map<TypedData, TypedData > executeOnEntries(const EntryProcessor &entryProcessor) {
                    return mapImpl->executeOnEntries<EntryProcessor>(entryProcessor);
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
                template<typename EntryProcessor>
                std::map<TypedData, TypedData>
                executeOnEntries(const EntryProcessor &entryProcessor, const query::Predicate &predicate) {
                    return mapImpl->executeOnEntries<EntryProcessor>(entryProcessor, predicate);
                }

                /**
                * Returns the number of key-value mappings in this map.  If the
                * map contains more than <tt>Integer.MAX_VALUE</tt> elements, returns
                * <tt>Integer.MAX_VALUE</tt>.
                *
                * @return the number of key-value mappings in this map
                */
                int size();

                /**
                * Returns <tt>true</tt> if this map contains no key-value mappings.
                *
                * @return <tt>true</tt> if this map contains no key-value mappings
                */
                bool isEmpty();


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
                template<typename K, typename V>
                void putAll(const std::map<K, V> &entries) {
                    return mapImpl->putAll<K, V>(entries);
                }

                /**
                * Removes all of the mappings from this map (optional operation).
                * The map will be empty after this call returns.
                */
                void clear();

                /**
                * Destroys this object cluster-wide.
                * Clears and releases all resources for this object.
                */
                void destroy();

                monitor::LocalMapStats &getLocalMapStats();

            private:
                IMap(std::shared_ptr<mixedtype::ClientMapProxy> proxy);

                std::shared_ptr<mixedtype::ClientMapProxy> mapImpl;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


