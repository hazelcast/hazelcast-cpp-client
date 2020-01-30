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

#ifndef HAZELCAST_CLIENT_REPLICATED_MAP_H_
#define HAZELCAST_CLIENT_REPLICATED_MAP_H_

#include <stdint.h>
#include <string>

#include <memory>

#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/query/Predicate.h"
#include "hazelcast/client/DataArray.h"
#include "hazelcast/client/LazyEntryArray.h"

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
         * @param <K> the type of keys maintained by this map
         * @param <V> the type of mapped values
         */
        template <typename K, typename V>
        class ReplicatedMap : public virtual DistributedObject {
        public:
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
             * @param ttl      ttl in milliseconds to be associated with the specified key-value pair.
             */
            virtual std::shared_ptr<V> put(const K &key, const V &value, int64_t ttl) = 0;

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
            virtual void putAll(const std::map<K, V> &entries) = 0;

            /**
             * <p>The clear operation wipes data out of the replicated maps.
             * <p>If some node fails on executing the operation, it is retried for at most
             * 5 times (on the failing nodes only).
             */
            virtual void clear() = 0;

            /**
             * Removes the specified entry listener.
             * Returns silently if there was no such listener added before.
             *
             * @param registrationId ID of the registered entry listener.
             * @return true if registration is removed, false otherwise.
             */
            virtual bool removeEntryListener(const std::string &registrationId) = 0;

            /**
             * Adds an entry listener for this map. The listener will be notified
             * for all map add/remove/update/evict events.
             *
             * @param listener entry listener
             */
            virtual std::string addEntryListener(const std::shared_ptr<EntryListener<K, V> > &listener) = 0;

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
            virtual std::string addEntryListener(const std::shared_ptr<EntryListener<K, V> > &listener, const K &key) = 0;

            /**
             * Adds an continuous entry listener for this map. The listener will be notified
             * for map add/remove/update/evict events filtered by the given predicate.
             *
             * @param listener  the entry listener to add
             * @param predicate the predicate for filtering entries
             */
            virtual const std::string addEntryListener(const std::shared_ptr<EntryListener<K, V> > &listener,
                    const query::Predicate &predicate) = 0;

            /**
             * Adds an continuous entry listener for this map. The listener will be notified
             * for map add/remove/update/evict events filtered by the given predicate.
             *
             * @param listener  the entry listener
             * @param predicate the predicate for filtering entries
             * @param key       the key to listen to
             */
            virtual std::string addEntryListener(const std::shared_ptr<EntryListener<K, V> > &listener,
                    const query::Predicate &predicate, const K &key) = 0;

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
            virtual std::shared_ptr<DataArray<V> > values() = 0;

            /**
             * Returns a lazy {@link LazyEntryArray} view of the mappings contained in this map.<br/>
             * Due to the lazy nature of the returned array, changes to the map (addition,
             * removal, update) might be reflected on the array.<br/>
             * Changes on the map are <b>NOT</b> reflected on the set on the <b>CLIENT</b> or vice versa.
             * The order of the elements is not guaranteed due to the internal
             * asynchronous replication behavior.<br/>
             * Changes to any returned object are <b>NOT</b> replicated back to other
             * members.
             *
             * @return A lazy set view of the mappings contained in this map.
             */
            virtual std::shared_ptr<LazyEntryArray<K, V> > entrySet() = 0;

            /**
             * Returns a lazy {@link DataArray} view of the key contained in this map.<br/>
             * Due to the lazy nature of the returned array, changes to the map (addition,
             * removal, update) might be reflected on the array.<br/>
             * Changes on the map are <b>NOT</b> reflected on the set on the <b>CLIENT</b> or vice versa.
             * The order of the elements is not guaranteed due to the internal
             * asynchronous replication behavior.<br/>
             * Changes to any returned object are <b>NOT</b> replicated back to other
             * members.
             *
             * @return A lazy {@link Set} view of the keys contained in this map.
             */
            virtual std::shared_ptr<DataArray<K> > keySet() = 0;

            /**
             *
             * @return The number of the replicated map entries in the cluster.
             */
            virtual int32_t size() = 0;

            /**
             *
             * @return true if the replicated map is empty, false otherwise
             */
            virtual bool isEmpty() = 0;

            /**
             *
             * @param key The key to be checked for existence
             * @return true if the entry with the key exist in the replicated map.
             */
            virtual bool containsKey(const K &key) = 0;

            /**
             *
             * @param value The value to check in the replicated map for existence.
             * @return true if the value exist in the replicated map.
             */
            virtual bool containsValue(const V &value) = 0;

            /**
             *
             * @param key The key to be used to query from replicated map.
             * @return The value of the key if the key exist, null pointer otherwise.
             */
            virtual std::shared_ptr<V> get(const K &key) = 0;

            /**
             *
             * @param key The key to be put into the replicated map.
             * @param value The value of the key
             * @return The previous value if the key existed in the map or null pointer otherwise.
             */
            virtual std::shared_ptr<V> put(const K &key, const V &value) = 0;

            /**
             *
             * @param key The key of the entry to be removed.
             * @return The value associated with the removed key.
             */
            virtual std::shared_ptr<V> remove(const K &key) = 0;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_REPLICATED_MAP_H_

