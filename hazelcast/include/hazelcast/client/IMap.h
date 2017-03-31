/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_IMAP
#define HAZELCAST_IMAP

#include <string>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <climits>
#include "hazelcast/client/protocol/codec/MapAddEntryListenerWithPredicateCodec.h"
#include "hazelcast/client/impl/EntryArrayImpl.h"
#include "hazelcast/client/proxy/IMapImpl.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/EntryView.h"
#include "hazelcast/client/map/ClientMapProxy.h"
#include "hazelcast/client/Future.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace adaptor {
            template<typename K, typename V>
            class RawPointerMap;
        }

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
        *      IMap<int,std::string> imap = client.getMap<int,std::string>("aKey");
        *
        * @param <K> key
        * @param <V> value
        */
        template<typename K, typename V>
        class IMap {
            friend class spi::ProxyManager;
            friend class HazelcastClient;
            friend class adaptor::RawPointerMap<K, V>;

        public:
            static const std::string SERVICE_NAME;

            /**
            * check if this map contains key.
            * @param key
            * @return true if contains, false otherwise
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool containsKey(const K &key) {
                return mapImpl->containsKey(key);
            }

            /**
            * check if this map contains value.
            * @param value
            * @return true if contains, false otherwise
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
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
            boost::shared_ptr<V> get(const K &key) {
                return mapImpl->get(key);
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
            boost::shared_ptr<V> put(const K &key, const V &value, long ttlInMillis) {
                return mapImpl->put(key, value, ttlInMillis);
            }

            /**
            * remove entry form map
            * @param key
            * @return the previous value in shared_ptr, if there is no mapping for key
            * then returns NULL in shared_ptr.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            boost::shared_ptr<V> remove(const K &key) {
                return mapImpl->remove(key);
            }

            /**
            * removes entry from map if there is an entry with same key and value.
            * @param key
            * @param value
            * @return true if remove is successful false otherwise
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            bool remove(const K &key, const V &value) {
                return mapImpl->remove(key, value);
            }

            /**
            * removes entry from map.
            * Does not return anything.
            * @param key The key of the map entry to remove.
            * @throws IClassCastException if the type of the specified element is incompatible with the server side.
            */
            void deleteEntry(const K &key) {
                mapImpl->deleteEntry(key);
            }

            /**
            * If this map has a MapStore this method flushes
            * all the local dirty entries by calling MapStore.storeAll() and/or MapStore.deleteAll()
            */
            void flush() {
                mapImpl->flush();
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
            bool tryRemove(const K &key, long timeoutInMillis) {
                return mapImpl->tryRemove(key, timeoutInMillis);
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
            bool tryPut(const K &key, const V &value, long timeoutInMillis) {
                return mapImpl->tryPut(key, value, timeoutInMillis);
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
            void putTransient(const K &key, const V &value, long ttlInMillis) {
                mapImpl->putTransient(key, value, ttlInMillis);
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
                return mapImpl->putIfAbsent(key, value);
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
            boost::shared_ptr<V> putIfAbsent(const K &key, const V &value, long ttlInMillis) {
                return mapImpl->putIfAbsent(key, value, ttlInMillis);
            }

            /**
            * Replaces the entry for a key only if currently mapped to a given value.
            * @param key key with which the specified value is associated
            * @param oldValue value expected to be associated with the specified key
            * @param newValue
            * @return <tt>true</tt> if the value was replaced
            */
            bool replace(const K &key, const V &oldValue, const V &newValue) {
                return mapImpl->replace(key, oldValue, newValue);
            }

            /**
            * Replaces the entry for a key only if currently mapped to some value.
            * @param key key with which the specified value is associated
            * @param value
            * @return the previous value of the entry, if there is no mapping for key
            * then returns NULL in shared_ptr.
            */
            boost::shared_ptr<V> replace(const K &key, const V &value) {
                return mapImpl->replace(key, value);
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
                mapImpl->lock(key);
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
            void lock(const K &key, long leaseTime) {
                mapImpl->lock(key, leaseTime);
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
                return mapImpl->isLocked(key);
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
                return mapImpl->tryLock(key);
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
            bool tryLock(const K &key, long timeInMillis) {
                return mapImpl->tryLock(key, timeInMillis);
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
                mapImpl->unlock(key);
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
                mapImpl->forceUnlock(key);
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
                return mapImpl->template addInterceptor<MapInterceptor>(interceptor);
            }

            /**
            * Removes the given interceptor for this map. So it will not intercept operations anymore.
            *
            *
            * @param id registration id of map interceptor
            */
            void removeInterceptor(const std::string &id) {
                mapImpl->removeInterceptor(id);
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
                return mapImpl->addEntryListener(listener, includeValue);
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
                return mapImpl->addEntryListener(listener, predicate, includeValue);
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
                return mapImpl->removeEntryListener(registrationId);
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
                return mapImpl->addEntryListener(listener, key, includeValue);
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
                return mapImpl->getEntryView(key);
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
                return mapImpl->evict(key);
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
                mapImpl->evictAll();
            }

            /**
            * Returns the entries for the given keys.
            *
            * @param keys keys to get
            * @return map of entries
            */
            std::map<K, V> getAll(const std::set<K> &keys) {
                return mapImpl->getAll(keys);
            }

            /**
            * Returns a vector clone of the keys contained in this map.
            * The vector is <b>NOT</b> backed by the map,
            * so changes to the map are <b>NOT</b> reflected in the vector, and vice-versa.
            *
            * @return a vector clone of the keys contained in this map
            */
            std::vector<K> keySet() {
                return mapImpl->keySet();
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
                return mapImpl->keySet(predicate);
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
                return mapImpl->keySet(predicate);
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
                return mapImpl->keySet(predicate);
            }

            /**
            * Returns a vector clone of the values contained in this map.
            * The vector is <b>NOT</b> backed by the map,
            * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
            *
            * @return a vector clone of the values contained in this map
            */
            std::vector<V> values() {
                return mapImpl->values();
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
                return mapImpl->values(predicate);
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
                return mapImpl->values(predicate);
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
                return mapImpl->values(predicate);
            }

            /**
            * Returns a std::vector< std::pair<K, V> > clone of the mappings contained in this map.
            * The vector is <b>NOT</b> backed by the map,
            * so changes to the map are <b>NOT</b> reflected in the set, and vice-versa.
            *
            * @return a vector clone of the keys mappings in this map
            */
            std::vector<std::pair<K, V> > entrySet() {
                return mapImpl->entrySet();
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
                return mapImpl->entrySet(predicate);
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
                return mapImpl->entrySet(predicate);
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
                return mapImpl->entrySet(predicate);
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
                mapImpl->addIndex(attribute, ordered);
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
            boost::shared_ptr<ResultType> executeOnKey(const K &key, EntryProcessor &entryProcessor) {
                return mapImpl->template executeOnKey<ResultType, EntryProcessor>(key, entryProcessor);
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
            template<typename ResultType, typename EntryProcessor>
            std::map<K, boost::shared_ptr<ResultType> >
            executeOnKeys(const std::set<K> &keys, EntryProcessor &entryProcessor) {
                return mapImpl->template executeOnKeys<ResultType, EntryProcessor>(keys, entryProcessor);
            }

            /**
             * Applies the user defined EntryProcessor to the entry mapped by the key.
             * Returns immediately with a ICompletableFuture representing that task.
             * <p/>
             *
             * @param key            key to be processed
             * @param entryProcessor processor to process the key
             * @return Future from which the result of the operation can be retrieved.
             */
            template<typename ResultType, typename EntryProcessor>
            Future<ResultType> submitToKey(const K &key, EntryProcessor &entryProcessor) {
                return mapImpl->template submitToKey<ResultType, EntryProcessor>(key, entryProcessor);
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
            std::map<K, boost::shared_ptr<ResultType> > executeOnEntries(EntryProcessor &entryProcessor) {
                return mapImpl->template executeOnEntries<ResultType, EntryProcessor>(entryProcessor);
            }

            /**
            * @deprecated This API is deprecated in favor of
            * @sa{executeOnEntries(EntryProcessor &entryProcessor, const query::Predicate &predicate)}
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
            std::map<K, boost::shared_ptr<ResultType> > executeOnEntries(EntryProcessor &entryProcessor,
                                                                         const serialization::IdentifiedDataSerializable &predicate) {
                return mapImpl->template executeOnEntries<ResultType, EntryProcessor>(entryProcessor, predicate);
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
            executeOnEntries(EntryProcessor &entryProcessor, const query::Predicate &predicate) {
                return mapImpl->template executeOnEntries<ResultType, EntryProcessor>(entryProcessor, predicate);
            }

            /**
            * Puts an entry into this map.
            * Similar to put operation except that set
            * doesn't return the old value which is more efficient.
            * @param key
            * @param value
            */
            void set(const K &key, const V &value) {
                mapImpl->set(key, value);
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
            void set(const K &key, const V &value, long ttl) {
                mapImpl->set(key, value, ttl);
            }

            /**
            * Returns the number of key-value mappings in this map.  If the
            * map contains more than <tt>Integer.MAX_VALUE</tt> elements, returns
            * <tt>Integer.MAX_VALUE</tt>.
            *
            * @return the number of key-value mappings in this map
            */
            int size() {
                return mapImpl->size();
            }

            /**
            * Returns <tt>true</tt> if this map contains no key-value mappings.
            *
            * @return <tt>true</tt> if this map contains no key-value mappings
            */
            bool isEmpty() {
                return mapImpl->isEmpty();
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
                return mapImpl->putAll(entries);
            }

            /**
            * Removes all of the mappings from this map (optional operation).
            * The map will be empty after this call returns.
            */
            void clear() {
                mapImpl->clear();
            }

            /**
            * Destroys this object cluster-wide.
            * Clears and releases all resources for this object.
            */
            void destroy() {
                mapImpl->destroy();
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
                return mapImpl->getLocalMapStats();
            }
        private:
            IMap(boost::shared_ptr<spi::ClientProxy> clientProxy) {
                mapImpl = boost::static_pointer_cast<map::ClientMapProxy<K, V> >(clientProxy);
            }

            boost::shared_ptr<map::ClientMapProxy<K, V> > mapImpl;
        };

        template <typename K, typename V>
        const std::string IMap<K, V>::SERVICE_NAME = "hz:impl:mapService";
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_IMAP */

