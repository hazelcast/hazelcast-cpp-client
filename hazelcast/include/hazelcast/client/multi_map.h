/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/proxy/MultiMapImpl.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/protocol/codec/codecs.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
namespace client {
/**
 * A specialized distributed map client whose keys can be associated with
 * multiple values.
 *
 * @see IMap
 */
class HAZELCAST_API multi_map : public proxy::MultiMapImpl
{
    friend class spi::ProxyManager;

public:
    static constexpr const char* SERVICE_NAME = "hz:impl:multiMapService";

    /**
     * Stores a key-value pair in the multimap.
     *
     *
     * @param key   the key to be stored
     * @param value the value to be stored
     * @return true if size of the multimap is increased, false if the multimap
     *         already contains the key-value pair.
     */
    template<typename K, typename V>
    boost::future<bool> put(const K& key, const V& value)
    {
        return proxy::MultiMapImpl::put(to_data(key), to_data(value));
    }

    /**
     * Returns the multimap of values associated with the key.
     *
     * @param key the key whose associated values are to be returned
     * @return the multimap of the values associated with the key.
     */
    template<typename K, typename V>
    boost::future<std::vector<V>> get(const K& key)
    {
        return to_object_vector<V>(proxy::MultiMapImpl::get_data(to_data(key)));
    }

    /**
     * Removes the given key value pair from the multimap.
     *
     * @param key
     * @param value
     * @return true if the size of the multimap changed after the remove
     * operation, false otherwise.
     */
    template<typename K, typename V>
    boost::future<bool> remove(const K& key, const V& value)
    {
        return proxy::MultiMapImpl::remove(to_data(key), to_data(value));
    }

    /**
     * Removes all the entries with the given key.
     *
     * @param key
     * @return the multimap of removed values associated with the given key.
     * Returned multimap might be modifiable but it has no effect on the
     * multimap
     */
    template<typename K, typename V>
    boost::future<std::vector<V>> remove(const K& key)
    {
        return to_object_vector<V>(
          proxy::MultiMapImpl::remove_data(to_data(key)));
    }

    /**
     * Returns the set of keys in the multimap.
     *
     * @return the set of keys in the multimap. Returned set might be modifiable
     *         but it has no effect on the multimap
     */
    template<typename K>
    boost::future<std::vector<K>> key_set()
    {
        return to_object_vector<K>(proxy::MultiMapImpl::key_set_data());
    }

    /**
     * Returns the multimap of values in the multimap.
     *
     * @return the multimap of values in the multimap. Returned multimap might
     * be modifiable but it has no effect on the multimap
     */
    template<typename V>
    boost::future<std::vector<V>> values()
    {
        return to_object_vector<V>(proxy::MultiMapImpl::values_data());
    }

    /**
     * Returns the set of key-value pairs in the multimap.
     *
     * @return the set of key-value pairs in the multimap. Returned set might be
     * modifiable but it has no effect on the multimap
     */
    template<typename K, typename V>
    boost::future<std::vector<std::pair<K, V>>> entry_set()
    {
        return to_entry_object_vector<K, V>(
          proxy::MultiMapImpl::entry_set_data());
    }

    /**
     * Returns whether the multimap contains an entry with the key.
     *
     * @param key the key whose existence is checked.
     * @return true if the multimap contains an entry with the key, false
     * otherwise.
     */
    template<typename K>
    boost::future<bool> contains_key(const K& key)
    {
        return proxy::MultiMapImpl::contains_key(to_data(key));
    }

    /**
     * Returns whether the multimap contains an entry with the value.
     *
     * @param value the value whose existence is checked.
     * @return true if the multimap contains an entry with the value, false
     * otherwise.
     */
    template<typename V>
    boost::future<bool> contains_value(const V& value)
    {
        return proxy::MultiMapImpl::contains_value(to_data(value));
    }

    /**
     * Returns whether the multimap contains the given key-value pair.
     *
     * @param key   the key whose existence is checked.
     * @param value the value whose existence is checked.
     * @return true if the multimap contains the key-value pair, false
     * otherwise.
     */
    template<typename K, typename V>
    boost::future<bool> contains_entry(const K& key, const V& value)
    {
        return proxy::MultiMapImpl::contains_entry(to_data(key),
                                                   to_data(value));
    }

    /**
     * Returns number of values matching to given key in the multimap.
     *
     *
     * @param key the key whose values count are to be returned
     * @return number of values matching to given key in the multimap.
     */
    template<typename K>
    boost::future<int> value_count(const K& key)
    {
        return proxy::MultiMapImpl::value_count(to_data(key));
    }

    /**
     * Adds an entry listener for this multimap. Listener will get notified
     * for all multimap add/remove/update/evict events.
     *
     * Warning 1: If listener should do a time consuming operation, off-load the
     * operation to another thread. otherwise it will slow down the system.
     *
     * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
     *
     * @param listener     entry listener
     * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
     *                     contain the value.
     * @return returns registration id.
     */
    boost::future<boost::uuids::uuid> add_entry_listener(
      entry_listener&& listener,
      bool include_value)
    {
        return proxy::MultiMapImpl::add_entry_listener(
          std::unique_ptr<impl::BaseEventHandler>(
            new impl::EntryEventHandler<
              protocol::codec::multimap_addentrylistener_handler>(
              get_name(),
              get_context().get_client_cluster_service(),
              get_context().get_serialization_service(),
              std::move(listener),
              include_value,
              get_context().get_logger())),
          include_value);
    }

    /**
     * Adds the specified entry listener for the specified key.
     * The listener will get notified for all
     * add/remove/update/evict events of the specified key only.
     *
     * Warning 1: If listener should do a time consuming operation, off-load the
     * operation to another thread. otherwise it will slow down the system.
     *
     * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
     *
     * @param listener     entry listener
     * @param key          the key to listen
     * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
     *                     contain the value.
     * @return returns registration id.
     */
    template<typename K>
    boost::future<boost::uuids::uuid> add_entry_listener(
      entry_listener&& listener,
      const K& key,
      bool include_value)
    {
        return proxy::MultiMapImpl::add_entry_listener(
          std::shared_ptr<impl::BaseEventHandler>(
            new impl::EntryEventHandler<
              protocol::codec::multimap_addentrylistenertokey_handler>(
              get_name(),
              get_context().get_client_cluster_service(),
              get_context().get_serialization_service(),
              std::move(listener),
              include_value,
              get_context().get_logger())),
          include_value,
          to_data(key));
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
    template<typename K>
    boost::future<void> lock(const K& key)
    {
        return proxy::MultiMapImpl::lock(to_data(key));
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
     * @param leaseTime time to wait before releasing the lock.
     */
    template<typename K>
    boost::future<void> lock(const K& key, std::chrono::milliseconds lease_time)
    {
        return proxy::MultiMapImpl::lock(to_data(key), lease_time);
    }

    /**
     * Checks the lock for the specified key.
     * <p>If the lock is acquired then returns true, else false.
     *
     * @param key key to lock to be checked.
     * @return <tt>true</tt> if lock is acquired, <tt>false</tt> otherwise.
     */
    template<typename K>
    boost::future<bool> is_locked(const K& key)
    {
        return proxy::MultiMapImpl::is_locked(to_data(key));
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
    boost::future<bool> try_lock(const K& key)
    {
        return proxy::MultiMapImpl::try_lock(to_data(key));
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
     * @param timeout     the maximum time to wait for the lock
     * @return <tt>true</tt> if the lock was acquired and <tt>false</tt>
     *         if the waiting time elapsed before the lock was acquired.
     */
    template<typename K>
    boost::future<bool> try_lock(const K& key,
                                 std::chrono::milliseconds timeout)
    {
        return proxy::MultiMapImpl::try_lock(to_data(key), timeout);
    }

    /**
     * Tries to acquire the lock for the specified key for the specified lease
     * time. <p>After lease time, the lock will be released. <p>If the lock is
     * not available then the current thread becomes disabled for thread
     * scheduling purposes and lies dormant until one of two things happens:
     * <ul>
     * <li>The lock is acquired by the current thread; or
     * <li>The specified waiting time elapses
     * </ul>
     *
     *
     * @param key to be locked.
     * @param timeout     the maximum time to wait for the lock
     * @param leaseTime time to wait before releasing the lock.
     * @return <tt>true</tt> if the lock was acquired and <tt>false</tt>
     *         if the waiting time elapsed before the lock was acquired.
     */
    template<typename K>
    boost::future<bool> try_lock(const K& key,
                                 std::chrono::milliseconds timeout,
                                 std::chrono::milliseconds lease_time)
    {
        return proxy::MultiMapImpl::try_lock(to_data(key), timeout, lease_time);
    }

    /**
     * Releases the lock for the specified key. It never blocks and
     * returns immediately.
     *
     *
     * @param key key to lock.
     */
    template<typename K>
    boost::future<void> unlock(const K& key)
    {
        return proxy::MultiMapImpl::unlock(to_data(key));
    }

    /**
     * Releases the lock for the specified key regardless of the lock owner.
     * It always successfully unlocks the key, never blocks
     * and returns immediately.
     * @param key key to lock.
     */
    template<typename K>
    boost::future<void> force_unlock(const K& key)
    {
        return proxy::MultiMapImpl::force_unlock(to_data(key));
    }

private:
    multi_map(const std::string& instance_name, spi::ClientContext* context)
      : proxy::MultiMapImpl(instance_name, context)
    {}
};
} // namespace client
} // namespace hazelcast
