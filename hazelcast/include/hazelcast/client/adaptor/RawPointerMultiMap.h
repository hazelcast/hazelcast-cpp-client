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
#ifndef HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERMULTIMAP_H_
#define HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERMULTIMAP_H_

#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/impl/DataArrayImpl.h"
#include "hazelcast/client/impl/EntryArrayImpl.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            /**
            * A specialized distributed map client whose keys can be associated with multiple values.
            *
            * @see IMap
            */
            template<typename K, typename V>
            class RawPointerMultiMap {
            public:
                RawPointerMultiMap(MultiMap<K, V> &m) :map(m), serializationService(m.context->getSerializationService()) {
                }

                /**
                * Stores a key-value pair in the multimap.
                *
                *
                * @param key   the key to be stored
                * @param value the value to be stored
                * @return true if size of the multimap is increased, false if the multimap
                *         already contains the key-value pair.
                */
                bool put(const K &key, const V &value) {
                    return map.put(key, value);
                }

                /**
                * Returns the multimap of values associated with the key.
                *
                * @param key the key whose associated values are to be returned
                * @return the multimap of the values associated with the key.
                */
                std::auto_ptr<DataArray<V> > get(const K &key) {
                    return std::auto_ptr<DataArray<V> >(new impl::DataArrayImpl<V>(map.getData(serializationService.toData<K>(&key)), serializationService));
                }

                /**
                * Removes the given key value pair from the multimap.
                *
                * @param key
                * @param value
                * @return true if the size of the multimap changed after the remove operation, false otherwise.
                */
                bool remove(const K &key, const V &value) {
                    return map.remove(key, value);
                }

                /**
                * Removes all the entries with the given key.
                *
                * @param key
                * @return the multimap of removed values associated with the given key. Returned multimap
                *         might be modifiable but it has no effect on the multimap
                */
                std::auto_ptr<DataArray<V> > remove(const K &key) {
                    return std::auto_ptr<DataArray<V> >(new impl::DataArrayImpl<V>(map.removeData(serializationService.toData<K>(&key)), serializationService));
                }

                /**
                * Returns the set of keys in the multimap.
                *
                * @return the set of keys in the multimap. Returned set might be modifiable
                *         but it has no effect on the multimap
                */
                std::auto_ptr<DataArray<K> > keySet() {
                    return std::auto_ptr<DataArray<K> >(new impl::DataArrayImpl<K>(map.keySetData(), serializationService));
                }

                /**
                * Returns the multimap of values in the multimap.
                *
                * @return the multimap of values in the multimap. Returned multimap might be modifiable
                *         but it has no effect on the multimap
                */
                std::auto_ptr<DataArray<V> > values() {
                    return std::auto_ptr<DataArray<V> >(new impl::DataArrayImpl<V>(map.valuesData(), serializationService));
                }

                /**
                * Returns the set of key-value pairs in the multimap.
                *
                * @return the set of key-value pairs in the multimap. Returned set might be modifiable
                *         but it has no effect on the multimap
                */
                std::auto_ptr<EntryArray<K, V> > entrySet() {
                    return std::auto_ptr<EntryArray<K, V> >(new impl::EntryArrayImpl<K, V>(map.entrySetData(), serializationService)) ;
                }

                /**
                * Returns whether the multimap contains an entry with the key.
                *
                * @param key the key whose existence is checked.
                * @return true if the multimap contains an entry with the key, false otherwise.
                */
                bool containsKey(const K &key) {
                    return map.containsKey(key);
                }

                /**
                * Returns whether the multimap contains an entry with the value.
                *
                * @param value the value whose existence is checked.
                * @return true if the multimap contains an entry with the value, false otherwise.
                */
                bool containsValue(const V &value) {
                    return map.containsValue(value);
                }

                /**
                * Returns whether the multimap contains the given key-value pair.
                *
                * @param key   the key whose existence is checked.
                * @param value the value whose existence is checked.
                * @return true if the multimap contains the key-value pair, false otherwise.
                */
                bool containsEntry(const K &key, const V &value) {
                    return map.containsEntry(key, value);
                }

                /**
                * Returns the number of key-value pairs in the multimap.
                *
                * @return the number of key-value pairs in the multimap.
                */
                int size() {
                    return map.size();
                }

                /**
                * Clears the multimap. Removes all key-value pairs.
                */
                void clear() {
                    map.clear();
                }

                /**
                * Returns number of values matching to given key in the multimap.
                *
                *
                * @param key the key whose values count are to be returned
                * @return number of values matching to given key in the multimap.
                */
                int valueCount(const K &key) {
                    return map.valueCount(key);
                }

                /**
                * Adds an entry listener for this multimap. Listener will get notified
                * for all multimap add/remove/update/evict events.
                *
                * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
                * otherwise it will slow down the system.
                *
                * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
                *
                * @param listener     entry listener
                * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
                *                     contain the value.
                * @return returns registration id.
                */
                std::string addEntryListener(EntryListener<K, V> &listener, bool includeValue) {
                    return map.addEntryListener(listener, includeValue);
                }

                /**
                * Adds the specified entry listener for the specified key.
                * The listener will get notified for all
                * add/remove/update/evict events of the specified key only.
                *
                * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
                * otherwise it will slow down the system.
                *
                * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
                *
                * @param listener     entry listener
                * @param key          the key to listen
                * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
                *                     contain the value.
                * @return returns registration id.
                */
                std::string addEntryListener(EntryListener<K, V> &listener, const K &key, bool includeValue) {
                    return map.addEntryListener(listener, key, includeValue);
                }

                /**
                * Removes the specified entry listener
                * Returns silently if there is no such listener added before.
                *
                * @param registrationId Id of listener registration
                *
                * @return true if registration is removed, false otherwise
                */
                bool removeEntryListener(const std::string &registrationId) {
                    return map.removeEntryListener(registrationId);
                }

                /**
                * Acquires the lock for the specified key.
                * <p>If the lock is not available then
                * the current thread becomes disabled for thread scheduling
                * purposes and lies dormant until the lock has been acquired.
                *
                * Scope of the lock is this multimap only.
                * Acquired lock is only for the key in this multimap.
                *
                * Locks are re-entrant so if the key is locked N times then
                * it should be unlocked N times before another thread can acquire it.
                *
                *
                * @param key key to lock.
                */
                void lock(const K &key) {
                    map.lock(key);
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
                * @param key key to lock.
                * @param leaseTimeInMillis time in milliseconds to wait before releasing the lock.
                */
                void lock(const K &key, long leaseTimeInMillis) {
                    map.lock(key, leaseTimeInMillis);
                }

                /**
                * Checks the lock for the specified key.
                * <p>If the lock is acquired then returns true, else false.
                *
                * @param key key to lock to be checked.
                * @return <tt>true</tt> if lock is acquired, <tt>false</tt> otherwise.
                */
                bool isLocked(const K &key) {
                    return map.isLocked(key);
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
                    return map.tryLock(key);
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
                * @param key to be locked.
                * @param timeoutInMillis     the maximum time to wait for the lock
                * @return <tt>true</tt> if the lock was acquired and <tt>false</tt>
                *         if the waiting time elapsed before the lock was acquired.
                */
                bool tryLock(const K &key, long timeoutInMillis) {
                    return map.tryLock(key, timeoutInMillis);
                }

                /**
                * Releases the lock for the specified key. It never blocks and
                * returns immediately.
                *
                *
                * @param key key to lock.
                */
                void unlock(const K &key) {
                    map.unlock(key);
                }

                /**
                * Releases the lock for the specified key regardless of the lock owner.
                * It always successfully unlocks the key, never blocks
                * and returns immediately.
                * @param key key to lock.
                */
                void forceUnlock(const K &key) {
                    map.forceUnlock(key);
                }

            private:
                MultiMap<K, V> &map;
                serialization::pimpl::SerializationService &serializationService;
            };
        }
    }
}

#endif /* HAZELCAST_CLIENT_ADAPTOR_RAWPOINTERMULTIMAP_H_ */

