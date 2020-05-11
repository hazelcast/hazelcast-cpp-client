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
#ifndef HAZELCAST_MULTI_MAP
#define HAZELCAST_MULTI_MAP

#include <string>
#include <vector>

#include "hazelcast/client/proxy/MultiMapImpl.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            template <typename K, typename V>
            class RawPointerMultiMap;
        }

        /**
        * A specialized distributed map client whose keys can be associated with multiple values.
        *
        * @see IMap
        */
        template<typename K, typename V>
        class MultiMap : public proxy::MultiMapImpl {
            friend class impl::HazelcastClientInstanceImpl;
            friend class adaptor::RawPointerMultiMap<K, V>;

        public:
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
                return proxy::MultiMapImpl::put(toData(key), toData(value));
            }

            /**
            * Returns the multimap of values associated with the key.
            *
            * @param key the key whose associated values are to be returned
            * @return the multimap of the values associated with the key.
            */
            std::vector<V> get(const K &key) {
                return toObjectCollection<V>(proxy::MultiMapImpl::getData(toData(key)));
            }

            /**
            * Removes the given key value pair from the multimap.
            *
            * @param key
            * @param value
            * @return true if the size of the multimap changed after the remove operation, false otherwise.
            */
            bool remove(const K &key, const V &value) {
                return proxy::MultiMapImpl::remove(toData(key), toData(value));
            }

            /**
            * Removes all the entries with the given key.
            *
            * @param key
            * @return the multimap of removed values associated with the given key. Returned multimap
            *         might be modifiable but it has no effect on the multimap
            */
            std::vector<V> remove(const K &key) {
                return toObjectCollection<V>(proxy::MultiMapImpl::removeData(toData(key)));
            }

            /**
            * Returns the set of keys in the multimap.
            *
            * @return the set of keys in the multimap. Returned set might be modifiable
            *         but it has no effect on the multimap
            */
            std::vector<K> keySet() {
                return toObjectCollection<K>(proxy::MultiMapImpl::keySetData());
            }

            /**
            * Returns the multimap of values in the multimap.
            *
            * @return the multimap of values in the multimap. Returned multimap might be modifiable
            *         but it has no effect on the multimap
            */
            std::vector<V> values() {
                return toObjectCollection<V>(proxy::MultiMapImpl::valuesData());
            }

            /**
            * Returns the set of key-value pairs in the multimap.
            *
            * @return the set of key-value pairs in the multimap. Returned set might be modifiable
            *         but it has no effect on the multimap
            */
            std::vector<std::pair<K, V> > entrySet() {
                return toObjectEntrySet<K, V>(proxy::MultiMapImpl::entrySetData());
            }

            /**
            * Returns whether the multimap contains an entry with the key.
            *
            * @param key the key whose existence is checked.
            * @return true if the multimap contains an entry with the key, false otherwise.
            */
            bool containsKey(const K &key) {
                return proxy::MultiMapImpl::containsKey(toData(key));
            }

            /**
            * Returns whether the multimap contains an entry with the value.
            *
            * @param value the value whose existence is checked.
            * @return true if the multimap contains an entry with the value, false otherwise.
            */
            bool containsValue(const V &value) {
                return proxy::MultiMapImpl::containsValue(toData(value));
            }

            /**
            * Returns whether the multimap contains the given key-value pair.
            *
            * @param key   the key whose existence is checked.
            * @param value the value whose existence is checked.
            * @return true if the multimap contains the key-value pair, false otherwise.
            */
            bool containsEntry(const K &key, const V &value) {
                return proxy::MultiMapImpl::containsEntry(toData(key), toData(value));
            }

            /**
            * Returns the number of key-value pairs in the multimap.
            *
            * @return the number of key-value pairs in the multimap.
            */
            int size() {
                return proxy::MultiMapImpl::size();
            }

            /**
            * Clears the multimap. Removes all key-value pairs.
            */
            void clear() {
                proxy::MultiMapImpl::clear();
            }

            /**
            * Returns number of values matching to given key in the multimap.
            *
            *
            * @param key the key whose values count are to be returned
            * @return number of values matching to given key in the multimap.
            */
            int valueCount(const K &key) {
                return proxy::MultiMapImpl::valueCount(toData(key));
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
                spi::ClientClusterService &clusterService = getContext().getClientClusterService();
                serialization::pimpl::SerializationService &ss = getContext().getSerializationService();
                impl::EntryEventHandler<K, V, protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::EntryEventHandler<K, V, protocol::codec::MultiMapAddEntryListenerCodec::AbstractEventHandler>(
                                getName(), clusterService, ss, listener, includeValue);
                return proxy::MultiMapImpl::addEntryListener(entryEventHandler, includeValue);
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
                impl::EntryEventHandler<K, V, protocol::codec::MultiMapAddEntryListenerToKeyCodec::AbstractEventHandler> *entryEventHandler =
                        new impl::EntryEventHandler<K, V, protocol::codec::MultiMapAddEntryListenerToKeyCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(), getContext().getSerializationService(), listener,
                                includeValue);
                serialization::pimpl::Data keyData = toData(key);
                return proxy::MultiMapImpl::addEntryListener(entryEventHandler, keyData, includeValue);
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
                return proxy::MultiMapImpl::removeEntryListener(registrationId);
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
                proxy::MultiMapImpl::lock(toData(key));
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
                proxy::MultiMapImpl::lock(toData(key), leaseTimeInMillis);
            }

            /**
            * Checks the lock for the specified key.
            * <p>If the lock is acquired then returns true, else false.
            *
            * @param key key to lock to be checked.
            * @return <tt>true</tt> if lock is acquired, <tt>false</tt> otherwise.
            */
            bool isLocked(const K &key) {
                return proxy::MultiMapImpl::isLocked(toData(key));
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
                return proxy::MultiMapImpl::tryLock(toData(key));
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
                return proxy::MultiMapImpl::tryLock(toData(key), timeoutInMillis);
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
            * @param key to be locked.
            * @param timeoutInMillis     the maximum time to wait for the lock
            * @param leaseTimeInMillis time in milliseconds to wait before releasing the lock.
            * @return <tt>true</tt> if the lock was acquired and <tt>false</tt>
            *         if the waiting time elapsed before the lock was acquired.
            */
            bool tryLock(const K &key, long timeoutInMillis, long leaseTimeInMillis) {
                return proxy::MultiMapImpl::tryLock(toData(key), timeoutInMillis, leaseTimeInMillis);
            }

            /**
            * Releases the lock for the specified key. It never blocks and
            * returns immediately.
            *
            *
            * @param key key to lock.
            */
            void unlock(const K &key) {
                proxy::MultiMapImpl::unlock(toData(key));
            }

            /**
            * Releases the lock for the specified key regardless of the lock owner.
            * It always successfully unlocks the key, never blocks
            * and returns immediately.
            * @param key key to lock.
            */
            void forceUnlock(const K &key) {
                proxy::MultiMapImpl::forceUnlock(toData(key));
            }

        private:
            MultiMap(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::MultiMapImpl(instanceName, context) {

            }
        };
    }
}

#endif /* HAZELCAST_MULTI_MAP */

