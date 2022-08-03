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
#include <vector>

#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/serialization/pimpl/data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                /**
                 * Makes an authentication request to the cluster.
                 */
                ClientMessage HAZELCAST_API
                client_authentication_encode(const std::string &cluster_name, const std::string *username,
                                             const std::string *password, boost::uuids::uuid uuid,
                                             const std::string &client_type, byte serialization_version,
                                             const std::string &client_hazelcast_version,
                                             const std::string &client_name, const std::vector<std::string> &labels);

                /**
                 * Makes an authentication request to the cluster using custom credentials.
                 */
                ClientMessage HAZELCAST_API client_authenticationcustom_encode(const std::string &cluster_name,
                                                                               const std::vector<byte> &credentials,
                                                                               boost::uuids::uuid uuid,
                                                                               const std::string &client_type,
                                                                               byte serialization_version,
                                                                               const std::string &client_hazelcast_version,
                                                                               const std::string &client_name,
                                                                               const std::vector<std::string> &labels);

                /**
                 * Adds a cluster view listener to a connection.
                 */
                ClientMessage HAZELCAST_API client_addclusterviewlistener_encode();

                struct HAZELCAST_API client_addclusterviewlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param version Incremental member list version
                     * @param memberInfos List of member infos  at the cluster associated with the given version
                     *                    params:
                    */
                    virtual void handle_membersview(int32_t version, std::vector<member> const &member_infos) = 0;

                    /**
                     * @param version Incremental state version of the partition table
                     * @param partitions The partition table. In each entry, it has uuid of the member and list of partitions belonging to that member
                    */
                    virtual void handle_partitionsview(int32_t version, std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> const & partitions) = 0;

                };

                /**
                 * Creates a cluster-wide proxy with the given name and service.
                 */
                ClientMessage HAZELCAST_API
                client_createproxy_encode(const std::string &name, const std::string &service_name);

                /**
                 * Destroys the proxy given by its name cluster-wide. Also, clears and releases all resources of this proxy.
                 */
                ClientMessage HAZELCAST_API
                client_destroyproxy_encode(const std::string &name, const std::string &service_name);

                /**
                 * Sends a ping to the given connection.
                 */
                ClientMessage HAZELCAST_API client_ping_encode();

                /**
                 * The statistics is composed of three parameters.
                 * 
                 * The first paramteter is the timestamp taken when the statistics collected.
                 * 
                 * The second parameter, the clientAttribute is a String that is composed of key=value pairs separated by ','. The
                 * following characters ('=' '.' ',' '\') should be escaped.
                 * 
                 * Please note that if any client implementation can not provide the value for a statistics, the corresponding key, value
                 * pair will not be presented in the statistics string. Only the ones, that the client can provide will be added.
                 * 
                 * The third parameter, metrics is a compressed byte array containing all metrics recorded by the client.
                 * 
                 * The metrics are composed of the following fields:
                 *   - string:                 prefix
                 *   - string:                 metric
                 *   - string:                 discriminator
                 *   - string:                 discriminatorValue
                 *   - enum:                   unit [BYTES,MS,PERCENT,COUNT,BOOLEAN,ENUM]
                 *   - set of enum:            excluded targets [MANAGEMENT_CENTER,JMX,DIAGNOSTICS]
                 *   - set of <string,string>: tags associated with the metric
                 * 
                 * The used compression algorithm is the same that is used inside the IMDG clients and members for storing the metrics blob
                 * in-memory. The algorithm uses a dictionary based delta compression further deflated by using ZLIB compression.
                 * 
                 * The byte array has the following layout:
                 * 
                 * +---------------------------------+--------------------+
                 * | Compressor version              |   2 bytes (short)  |
                 * +---------------------------------+--------------------+
                 * | Size of dictionary blob         |   4 bytes (int)    |
                 * +---------------------------------+--------------------+
                 * | ZLIB compressed dictionary blob |   variable size    |
                 * +---------------------------------+--------------------+
                 * | ZLIB compressed metrics blob    |   variable size    |
                 * +---------------------------------+--------------------+
                 * 
                 * ==========
                 * THE HEADER
                 * ==========
                 * 
                 * Compressor version:      the version currently in use is 1.
                 * Size of dictionary blob: the size of the ZLIB compressed blob as it is constructed as follows.
                 * 
                 * ===================
                 * THE DICTIONARY BLOB
                 * ===================
                 * 
                 * The dictionary is built from the string fields of the metric and assigns an int dictionary id to every string in the metrics
                 * in the blob. The dictionary is serialized to the dictionary blob sorted by the strings using the following layout.
                 * 
                 * +------------------------------------------------+--------------------+
                 * | Number of dictionary entries                   |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | Dictionary id                                  |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | Number of chars shared with previous entry     |   1 unsigned byte  |
                 * +------------------------------------------------+--------------------+
                 * | Number of chars not shared with previous entry |   1 unsigned byte  |
                 * +------------------------------------------------+--------------------+
                 * | The different characters                       |   variable size    |
                 * +------------------------------------------------+--------------------+
                 * | Dictionary id                                  |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | ...                                            |   ...              |
                 * +------------------------------------------------+--------------------+
                 * 
                 * Let's say we have the following dictionary:
                 *   - <42,"gc.minorCount">
                 *   - <43,"gc.minorTime">
                 * 
                 * It is then serialized as follows:
                 * +------------------------------------------------+--------------------+
                 * | 2 (size of the dictionary)                     |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | 42                                             |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | 0                                              |   1 unsigned byte  |
                 * +------------------------------------------------+--------------------+
                 * | 13                                             |   1 unsigned byte  |
                 * +------------------------------------------------+--------------------+
                 * | "gc.minorCount"                                |   13 bytes         |
                 * +------------------------------------------------+--------------------+
                 * | 43                                             |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | 8                                              |   1 unsigned byte  |
                 * +------------------------------------------------+--------------------+
                 * | 4                                              |   1 unsigned byte  |
                 * +------------------------------------------------+--------------------+
                 * | "Time"                                         |   13 bytes         |
                 * +------------------------------------------------+--------------------+
                 * 
                 * The dictionary blob constructed this way is then gets ZLIB compressed.
                 * 
                 * ===============
                 * THE METRIC BLOB
                 * ===============
                 * 
                 * The compressed dictionary blob is followed by the compressed metrics blob
                 * with the following layout:
                 * 
                 * +------------------------------------------------+--------------------+
                 * | Number of metrics                              |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | Metrics mask                                   |   1 byte           |
                 * +------------------------------------------------+--------------------+
                 * | (*) Dictionary id of prefix                    |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | (*) Dictionary id of metric                    |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | (*) Dictionary id of discriminator             |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | (*) Dictionary id of discriminatorValue        |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | (*) Enum ordinal of the unit                   |   1 byte           |
                 * +------------------------------------------------+--------------------+
                 * | (*) Excluded targets bitset                    |   1 byte           |
                 * +------------------------------------------------+--------------------+
                 * | (*) Number of tags                             |   1 unsigned byte  |
                 * +------------------------------------------------+--------------------+
                 * | (**) Dictionary id of the tag 1                |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | (**) Dictionary id of the value of tag 1       |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | (**) Dictionary id of the tag 2                |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | (**) Dictionary id of the value of tag 2       |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | ...                                            |   ...              |
                 * +------------------------------------------------+--------------------+
                 * | Metrics mask                                   |   1 byte           |
                 * +------------------------------------------------+--------------------+
                 * | (*) Dictionary id of prefix                    |   4 bytes (int)    |
                 * +------------------------------------------------+--------------------+
                 * | ...                                            |   ...              |
                 * +------------------------------------------------+--------------------+
                 * 
                 * The metrics mask shows which fields are the same in the current and the
                 * previous metric. The following masks are used to construct the metrics
                 * mask.
                 * 
                 * MASK_PREFIX              = 0b00000001;
                 * MASK_METRIC              = 0b00000010;
                 * MASK_DISCRIMINATOR       = 0b00000100;
                 * MASK_DISCRIMINATOR_VALUE = 0b00001000;
                 * MASK_UNIT                = 0b00010000;
                 * MASK_EXCLUDED_TARGETS    = 0b00100000;
                 * MASK_TAG_COUNT           = 0b01000000;
                 * 
                 * If a bit representing a field is set, the given field marked above with (*)
                 * is not written to blob and the last value for that field should be taken
                 * during deserialization.
                 * 
                 * Since the number of tags are not limited, all tags and their values
                 * marked with (**) are written even if the tag set is the same as in the
                 * previous metric.
                 * 
                 * The metrics blob constructed this way is then gets ZLIB compressed.
                 */
                ClientMessage HAZELCAST_API
                client_statistics_encode(int64_t timestamp, const std::string &client_attributes,
                                         const std::vector<byte> &metrics_blob);

                /**
                 * Adds listener for backup acks
                 */
                ClientMessage HAZELCAST_API client_localbackuplistener_encode();

                struct HAZELCAST_API client_localbackuplistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param sourceInvocationCorrelationId correlation id of the invocation that backup acks belong to
                    */
                    virtual void handle_backup(int64_t source_invocation_correlation_id) = 0;

                };

                /**
                 * Removes the specified migration listener.
                 */
                ClientMessage HAZELCAST_API client_removemigrationlistener_encode(boost::uuids::uuid registration_id);

                /**
                 * Puts an entry into this map with a given ttl (time to live) value.Entry will expire and get evicted after the ttl
                 * If ttl is 0, then the entry lives forever.This method returns a clone of the previous value, not the original
                 * (identically equal) value previously put into the map.Time resolution for TTL is seconds. The given TTL value is
                 * rounded to the next closest second value.
                 */
                ClientMessage HAZELCAST_API
                map_put_encode(const std::string &name, const serialization::pimpl::data &key,
                               const serialization::pimpl::data &value, int64_t thread_id, int64_t ttl);

                /**
                 * This method returns a clone of the original value, so modifying the returned value does not change the actual
                 * value in the map. You should put the modified value back to make changes visible to all nodes.
                 */
                ClientMessage HAZELCAST_API
                map_get_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id);

                /**
                 * Removes the mapping for a key from this map if it is present (optional operation).
                 * Returns the value to which this map previously associated the key, or null if the map contained no mapping for the key.
                 * If this map permits null values, then a return value of null does not necessarily indicate that the map contained no mapping for the key; it's also
                 * possible that the map explicitly mapped the key to null. The map will not contain a mapping for the specified key once the
                 * call returns.
                 */
                ClientMessage HAZELCAST_API
                map_remove_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id);

                /**
                 * Replaces the entry for a key only if currently mapped to a given value.
                 */
                ClientMessage HAZELCAST_API
                map_replace_encode(const std::string &name, const serialization::pimpl::data &key,
                                   const serialization::pimpl::data &value, int64_t thread_id);

                /**
                 * Replaces the the entry for a key only if existing values equal to the testValue
                 */
                ClientMessage HAZELCAST_API
                map_replaceifsame_encode(const std::string &name, const serialization::pimpl::data &key,
                                         const serialization::pimpl::data &test_value,
                                         const serialization::pimpl::data &value, int64_t thread_id);

                /**
                 * Returns true if this map contains a mapping for the specified key.
                 */
                ClientMessage HAZELCAST_API
                map_containskey_encode(const std::string &name, const serialization::pimpl::data &key,
                                       int64_t thread_id);

                /**
                 * Returns true if this map maps one or more keys to the specified value.This operation will probably require time
                 * linear in the map size for most implementations of the Map interface.
                 */
                ClientMessage HAZELCAST_API
                map_containsvalue_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Removes the mapping for a key from this map if existing value equal to the this value
                 */
                ClientMessage HAZELCAST_API
                map_removeifsame_encode(const std::string &name, const serialization::pimpl::data &key,
                                        const serialization::pimpl::data &value, int64_t thread_id);

                /**
                 * Removes the mapping for a key from this map if it is present.Unlike remove(Object), this operation does not return
                 * the removed value, which avoids the serialization cost of the returned value.If the removed value will not be used,
                 * a delete operation is preferred over a remove operation for better performance. The map will not contain a mapping
                 * for the specified key once the call returns.
                 * This method breaks the contract of EntryListener. When an entry is removed by delete(), it fires an EntryEvent
                 * with a null oldValue. Also, a listener with predicates will have null values, so only keys can be queried via predicates
                 */
                ClientMessage HAZELCAST_API
                map_delete_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id);

                /**
                 * If this map has a MapStore, this method flushes all the local dirty entries by calling MapStore.storeAll()
                 * and/or MapStore.deleteAll().
                 */
                ClientMessage HAZELCAST_API map_flush_encode(const std::string  & name);

                /**
                 * Tries to remove the entry with the given key from this map within the specified timeout value.
                 * If the key is already locked by another thread and/or member, then this operation will wait the timeout
                 * amount for acquiring the lock.
                 */
                ClientMessage HAZELCAST_API
                map_tryremove_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id,
                                     int64_t timeout);

                /**
                 * Tries to put the given key and value into this map within a specified timeout value. If this method returns false,
                 * it means that the caller thread could not acquire the lock for the key within the timeout duration,
                 * thus the put operation is not successful.
                 */
                ClientMessage HAZELCAST_API
                map_tryput_encode(const std::string &name, const serialization::pimpl::data &key,
                                  const serialization::pimpl::data &value, int64_t thread_id, int64_t timeout);

                /**
                 * Same as put except that MapStore, if defined, will not be called to store/persist the entry.
                 * If ttl is 0, then the entry lives forever.
                 */
                ClientMessage HAZELCAST_API
                map_puttransient_encode(const std::string &name, const serialization::pimpl::data &key,
                                        const serialization::pimpl::data &value, int64_t thread_id, int64_t ttl);

                /**
                 * Puts an entry into this map with a given ttl (time to live) value if the specified key is not already associated
                 * with a value. Entry will expire and get evicted after the ttl.
                 */
                ClientMessage HAZELCAST_API
                map_putifabsent_encode(const std::string &name, const serialization::pimpl::data &key,
                                       const serialization::pimpl::data &value, int64_t thread_id, int64_t ttl);

                /**
                 * Puts an entry into this map with a given ttl (time to live) value.Entry will expire and get evicted after the ttl
                 * If ttl is 0, then the entry lives forever. Similar to the put operation except that set doesn't
                 * return the old value, which is more efficient.
                 */
                ClientMessage HAZELCAST_API
                map_set_encode(const std::string &name, const serialization::pimpl::data &key,
                               const serialization::pimpl::data &value, int64_t thread_id, int64_t ttl);

                /**
                 * Acquires the lock for the specified lease time.After lease time, lock will be released.If the lock is not
                 * available then the current thread becomes disabled for thread scheduling purposes and lies dormant until the lock
                 * has been acquired.
                 * Scope of the lock is this map only. Acquired lock is only for the key in this map. Locks are re-entrant,
                 * so if the key is locked N times then it should be unlocked N times before another thread can acquire it.
                 */
                ClientMessage HAZELCAST_API
                map_lock_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id,
                                int64_t ttl, int64_t reference_id);

                /**
                 * Tries to acquire the lock for the specified key for the specified lease time.After lease time, the lock will be
                 * released.If the lock is not available, then the current thread becomes disabled for thread scheduling
                 * purposes and lies dormant until one of two things happens the lock is acquired by the current thread, or
                 * the specified waiting time elapses.
                 */
                ClientMessage HAZELCAST_API
                map_trylock_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id,
                                   int64_t lease, int64_t timeout, int64_t reference_id);

                /**
                 * Checks the lock for the specified key.If the lock is acquired then returns true, else returns false.
                 */
                ClientMessage HAZELCAST_API
                map_islocked_encode(const std::string &name, const serialization::pimpl::data &key);

                /**
                 * Releases the lock for the specified key. It never blocks and returns immediately.
                 * If the current thread is the holder of this lock, then the hold count is decremented.If the hold count is zero,
                 * then the lock is released.  If the current thread is not the holder of this lock,
                 * then ILLEGAL_MONITOR_STATE is thrown.
                 */
                ClientMessage HAZELCAST_API
                map_unlock_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id,
                                  int64_t reference_id);

                /**
                 * Adds an interceptor for this map. Added interceptor will intercept operations
                 * and execute user defined methods and will cancel operations if user defined method throw exception.
                 */
                ClientMessage HAZELCAST_API
                map_addinterceptor_encode(const std::string &name, const serialization::pimpl::data &interceptor);

                /**
                 * Removes the given interceptor for this map so it will not intercept operations anymore.
                 */
                ClientMessage HAZELCAST_API map_removeinterceptor_encode(const std::string  & name, const std::string  & id);

                /**
                 * Adds an continuous entry listener for this map. Listener will get notified for map add/remove/update/evict events
                 * filtered by the given predicate.
                 */
                ClientMessage HAZELCAST_API map_addentrylistenerwithpredicate_encode(const std::string &name,
                                                                                     const serialization::pimpl::data &predicate,
                                                                                     bool include_value,
                                                                                     int32_t listener_flags,
                                                                                     bool local_only);

                struct HAZELCAST_API map_addentrylistenerwithpredicate_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Adds a MapListener for this map. To receive an event, you should implement a corresponding MapListener
                 * sub-interface for that event.
                 */
                ClientMessage HAZELCAST_API
                map_addentrylistenertokey_encode(const std::string &name, const serialization::pimpl::data &key,
                                                 bool include_value, int32_t listener_flags, bool local_only);

                struct HAZELCAST_API map_addentrylistenertokey_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Adds a MapListener for this map. To receive an event, you should implement a corresponding MapListener
                 * sub-interface for that event.
                 */
                ClientMessage HAZELCAST_API
                map_addentrylistener_encode(const std::string &name, bool include_value, int32_t listener_flags,
                                            bool local_only);

                struct HAZELCAST_API map_addentrylistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Removes the specified entry listener. If there is no such listener added before, this call does no change in the
                 * cluster and returns false.
                 */
                ClientMessage HAZELCAST_API
                map_removeentrylistener_encode(const std::string &name, boost::uuids::uuid registration_id);

                /**
                 * Returns the EntryView for the specified key.
                 * This method returns a clone of original mapping, modifying the returned value does not change the actual value
                 * in the map. One should put modified value back to make changes visible to all nodes.
                 */
                ClientMessage HAZELCAST_API
                map_getentryview_encode(const std::string &name, const serialization::pimpl::data &key,
                                        int64_t thread_id);

                /**
                 * Evicts the specified key from this map. If a MapStore is defined for this map, then the entry is not deleted
                 * from the underlying MapStore, evict only removes the entry from the memory.
                 */
                ClientMessage HAZELCAST_API
                map_evict_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id);

                /**
                 * Evicts all keys from this map except the locked ones. If a MapStore is defined for this map, deleteAll is not
                 * called by this method. If you do want to deleteAll to be called use the clear method. The EVICT_ALL event is
                 * fired for any registered listeners.
                 */
                ClientMessage HAZELCAST_API map_evictall_encode(const std::string  & name);

                /**
                 * Returns a set clone of the keys contained in this map. The set is NOT backed by the map, so changes to the map
                 * are NOT reflected in the set, and vice-versa. This method is always executed by a distributed query, so it may
                 * throw a query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API map_keyset_encode(const std::string  & name);

                /**
                 * Returns the entries for the given keys. If any keys are not present in the Map, it will call loadAll The returned
                 * map is NOT backed by the original map, so changes to the original map are NOT reflected in the returned map, and vice-versa.
                 * Please note that all the keys in the request should belong to the partition id to which this request is being sent, all keys
                 * matching to a different partition id shall be ignored. The API implementation using this request may need to send multiple
                 * of these request messages for filling a request for a key set if the keys belong to different partitions.
                 */
                ClientMessage HAZELCAST_API
                map_getall_encode(const std::string &name, const std::vector<serialization::pimpl::data> &keys);

                /**
                 * Returns a collection clone of the values contained in this map.
                 * The collection is NOT backed by the map, so changes to the map are NOT reflected in the collection, and vice-versa.
                 * This method is always executed by a distributed query, so it may throw a query_result_size_exceeded
                 * if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API map_values_encode(const std::string  & name);

                /**
                 * Returns a Set clone of the mappings contained in this map.
                 * The collection is NOT backed by the map, so changes to the map are NOT reflected in the collection, and vice-versa.
                 * This method is always executed by a distributed query, so it may throw a query_result_size_exceeded
                 * if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API map_entryset_encode(const std::string  & name);

                /**
                 * Queries the map based on the specified predicate and returns the keys of matching entries. Specified predicate
                 * runs on all members in parallel.The set is NOT backed by the map, so changes to the map are NOT reflected in the
                 * set, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API
                map_keysetwithpredicate_encode(const std::string &name, const serialization::pimpl::data &predicate);

                /**
                 * Queries the map based on the specified predicate and returns the values of matching entries.Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API
                map_valueswithpredicate_encode(const std::string &name, const serialization::pimpl::data &predicate);

                /**
                 * Queries the map based on the specified predicate and returns the matching entries.Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API
                map_entrieswithpredicate_encode(const std::string &name, const serialization::pimpl::data &predicate);

                /**
                 * Adds an index to this map with specified configuration.
                 */
                ClientMessage HAZELCAST_API
                map_addindex_encode(const std::string &name, const config::index_config &index_config);

                /**
                 * Returns the number of key-value mappings in this map.  If the map contains more than Integer.MAX_VALUE elements,
                 * returns Integer.MAX_VALUE
                 */
                ClientMessage HAZELCAST_API map_size_encode(const std::string  & name);

                /**
                 * Returns true if this map contains no key-value mappings.
                 */
                ClientMessage HAZELCAST_API map_isempty_encode(const std::string  & name);

                /**
                 * Copies all of the mappings from the specified map to this map (optional operation).The effect of this call is
                 * equivalent to that of calling put(Object,Object) put(k, v) on this map once for each mapping from key k to value
                 * v in the specified map.The behavior of this operation is undefined if the specified map is modified while the
                 * operation is in progress.
                 * Please note that all the keys in the request should belong to the partition id to which this request is being sent, all keys
                 * matching to a different partition id shall be ignored. The API implementation using this request may need to send multiple
                 * of these request messages for filling a request for a key set if the keys belong to different partitions.
                 */
                ClientMessage HAZELCAST_API map_putall_encode(const std::string &name,
                                                              const std::vector<std::pair<serialization::pimpl::data, serialization::pimpl::data>> &entries,
                                                              bool trigger_map_loader);

                /**
                 * This method clears the map and invokes MapStore#deleteAll deleteAll on MapStore which, if connected to a database,
                 * will delete the records from that database. The MAP_CLEARED event is fired for any registered listeners.
                 * To clear a map without calling MapStore#deleteAll, use #evictAll.
                 */
                ClientMessage HAZELCAST_API map_clear_encode(const std::string  & name);

                /**
                 * Applies the user defined EntryProcessor to the entry mapped by the key. Returns the the object which is result of
                 * the process() method of EntryProcessor.
                 */
                ClientMessage HAZELCAST_API
                map_executeonkey_encode(const std::string &name, const serialization::pimpl::data &entry_processor,
                                        const serialization::pimpl::data &key, int64_t thread_id);

                /**
                 * Applies the user defined EntryProcessor to the entry mapped by the key. Returns immediately with a Future
                 * representing that task.EntryProcessor is not cancellable, so calling Future.cancel() method won't cancel the
                 * operation of EntryProcessor.
                 */
                ClientMessage HAZELCAST_API
                map_submittokey_encode(const std::string &name, const serialization::pimpl::data &entry_processor,
                                       const serialization::pimpl::data &key, int64_t thread_id);

                /**
                 * Applies the user defined EntryProcessor to the all entries in the map.Returns the results mapped by each key in the map.
                 */
                ClientMessage HAZELCAST_API
                map_executeonallkeys_encode(const std::string &name, const serialization::pimpl::data &entry_processor);

                /**
                 * Applies the user defined EntryProcessor to the entries in the map which satisfies provided predicate.
                 * Returns the results mapped by each key in the map.
                 */
                ClientMessage HAZELCAST_API map_executewithpredicate_encode(const std::string &name,
                                                                            const serialization::pimpl::data &entry_processor,
                                                                            const serialization::pimpl::data &predicate);

                /**
                 * Applies the user defined EntryProcessor to the entries mapped by the collection of keys.The results mapped by
                 * each key in the collection.
                 */
                ClientMessage HAZELCAST_API
                map_executeonkeys_encode(const std::string &name, const serialization::pimpl::data &entry_processor,
                                         const std::vector<serialization::pimpl::data> &keys);

                /**
                 * Releases the lock for the specified key regardless of the lock owner.It always successfully unlocks the key,
                 * never blocks,and returns immediately.
                 */
                ClientMessage HAZELCAST_API
                map_forceunlock_encode(const std::string &name, const serialization::pimpl::data &key,
                                       int64_t reference_id);

                /**
                 * Queries the map based on the specified predicate and returns the keys of matching entries. Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API map_keysetwithpagingpredicate_encode(const std::string  & name, const codec::holder::paging_predicate_holder  & predicate);

                /**
                 * Queries the map based on the specified predicate and returns the values of matching entries. Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API map_valueswithpagingpredicate_encode(const std::string  & name, const codec::holder::paging_predicate_holder  & predicate);

                /**
                 * Queries the map based on the specified predicate and returns the matching entries. Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API map_entrieswithpagingpredicate_encode(const std::string  & name, const codec::holder::paging_predicate_holder  & predicate);

                /**
                 * Removes all entries which match with the supplied predicate
                 */
                ClientMessage HAZELCAST_API
                map_removeall_encode(const std::string &name, const serialization::pimpl::data &predicate);

                /**
                 * Adds listener to map. This listener will be used to listen near cache invalidation events.
                 */
                ClientMessage HAZELCAST_API
                map_addnearcacheinvalidationlistener_encode(const std::string &name, int32_t listener_flags,
                                                            bool local_only);

                struct HAZELCAST_API map_addnearcacheinvalidationlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key The key of the invalidated entry.
                     * @param sourceUuid UUID of the member who fired this event.
                     * @param partitionUuid UUID of the source partition that invalidated entry belongs to.
                     * @param sequence Sequence number of the invalidation event.
                    */
                    virtual void handle_imapinvalidation(const boost::optional<serialization::pimpl::data> &key,
                                                         boost::uuids::uuid source_uuid,
                                                         boost::uuids::uuid partition_uuid, int64_t sequence) = 0;

                    /**
                     * @param keys List of the keys of the invalidated entries.
                     * @param sourceUuids List of UUIDs of the members who fired these events.
                     * @param partitionUuids List of UUIDs of the source partitions that invalidated entries belong to.
                     * @param sequences List of sequence numbers of the invalidation events.
                    */
                    virtual void handle_imapbatchinvalidation(std::vector<serialization::pimpl::data> const &keys,
                                                              std::vector<boost::uuids::uuid> const &source_uuids,
                                                              std::vector<boost::uuids::uuid> const &partition_uuids,
                                                              std::vector<int64_t> const &sequences) = 0;

                };

                /**
                 * Stores a key-value pair in the multimap.
                 */
                ClientMessage HAZELCAST_API
                multimap_put_encode(const std::string &name, const serialization::pimpl::data &key,
                                    const serialization::pimpl::data &value, int64_t thread_id);

                /**
                 * Returns the collection of values associated with the key. The collection is NOT backed by the map, so changes to
                 * the map are NOT reflected in the collection, and vice-versa.
                 */
                ClientMessage HAZELCAST_API
                multimap_get_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id);

                /**
                 * Removes the given key value pair from the multimap.
                 */
                ClientMessage HAZELCAST_API
                multimap_remove_encode(const std::string &name, const serialization::pimpl::data &key,
                                       int64_t thread_id);

                /**
                 * Returns the set of keys in the multimap.The collection is NOT backed by the map, so changes to the map are NOT
                 * reflected in the collection, and vice-versa.
                 */
                ClientMessage HAZELCAST_API multimap_keyset_encode(const std::string  & name);

                /**
                 * Returns the collection of values in the multimap.The collection is NOT backed by the map, so changes to the map
                 * are NOT reflected in the collection, and vice-versa.
                 */
                ClientMessage HAZELCAST_API multimap_values_encode(const std::string  & name);

                /**
                 * Returns the set of key-value pairs in the multimap.The collection is NOT backed by the map, so changes to the map
                 * are NOT reflected in the collection, and vice-versa
                 */
                ClientMessage HAZELCAST_API multimap_entryset_encode(const std::string  & name);

                /**
                 * Returns whether the multimap contains an entry with the key.
                 */
                ClientMessage HAZELCAST_API
                multimap_containskey_encode(const std::string &name, const serialization::pimpl::data &key,
                                            int64_t thread_id);

                /**
                 * Returns whether the multimap contains an entry with the value.
                 */
                ClientMessage HAZELCAST_API
                multimap_containsvalue_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Returns whether the multimap contains the given key-value pair.
                 */
                ClientMessage HAZELCAST_API
                multimap_containsentry_encode(const std::string &name, const serialization::pimpl::data &key,
                                              const serialization::pimpl::data &value, int64_t thread_id);

                /**
                 * Returns the number of key-value pairs in the multimap.
                 */
                ClientMessage HAZELCAST_API multimap_size_encode(const std::string  & name);

                /**
                 * Clears the multimap. Removes all key-value pairs.
                 */
                ClientMessage HAZELCAST_API multimap_clear_encode(const std::string  & name);

                /**
                 * Returns the number of values that match the given key in the multimap.
                 */
                ClientMessage HAZELCAST_API
                multimap_valuecount_encode(const std::string &name, const serialization::pimpl::data &key,
                                           int64_t thread_id);

                /**
                 * Adds the specified entry listener for the specified key.The listener will be notified for all
                 * add/remove/update/evict events for the specified key only.
                 */
                ClientMessage HAZELCAST_API
                multimap_addentrylistenertokey_encode(const std::string &name, const serialization::pimpl::data &key,
                                                      bool include_value, bool local_only);

                struct HAZELCAST_API multimap_addentrylistenertokey_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Adds an entry listener for this multimap. The listener will be notified for all multimap add/remove/update/evict events.
                 */
                ClientMessage HAZELCAST_API
                multimap_addentrylistener_encode(const std::string &name, bool include_value, bool local_only);

                struct HAZELCAST_API multimap_addentrylistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Removes the specified entry listener. If there is no such listener added before, this call does no change in the
                 * cluster and returns false.
                 */
                ClientMessage HAZELCAST_API
                multimap_removeentrylistener_encode(const std::string &name, boost::uuids::uuid registration_id);

                /**
                 * Acquires the lock for the specified key for the specified lease time. After the lease time, the lock will be
                 * released. If the lock is not available, then the current thread becomes disabled for thread scheduling
                 * purposes and lies dormant until the lock has been acquired. Scope of the lock is for this map only. The acquired
                 * lock is only for the key in this map.Locks are re-entrant, so if the key is locked N times, then it should be
                 * unlocked N times before another thread can acquire it.
                 */
                ClientMessage HAZELCAST_API
                multimap_lock_encode(const std::string &name, const serialization::pimpl::data &key, int64_t thread_id,
                                     int64_t ttl, int64_t reference_id);

                /**
                 * Tries to acquire the lock for the specified key for the specified lease time. After lease time, the lock will be
                 * released. If the lock is not available, then the current thread becomes disabled for thread scheduling purposes
                 * and lies dormant until one of two things happens:the lock is acquired by the current thread, or the specified
                 * waiting time elapses.
                 */
                ClientMessage HAZELCAST_API
                multimap_trylock_encode(const std::string &name, const serialization::pimpl::data &key,
                                        int64_t thread_id, int64_t lease, int64_t timeout, int64_t reference_id);

                /**
                 * Checks the lock for the specified key. If the lock is acquired, this method returns true, else it returns false.
                 */
                ClientMessage HAZELCAST_API
                multimap_islocked_encode(const std::string &name, const serialization::pimpl::data &key);

                /**
                 * Releases the lock for the specified key regardless of the lock owner. It always successfully unlocks the key,
                 * never blocks and returns immediately.
                 */
                ClientMessage HAZELCAST_API
                multimap_unlock_encode(const std::string &name, const serialization::pimpl::data &key,
                                       int64_t thread_id, int64_t reference_id);

                /**
                 * Releases the lock for the specified key regardless of the lock owner. It always successfully unlocks the key,
                 * never blocks and returns immediately.
                 */
                ClientMessage HAZELCAST_API
                multimap_forceunlock_encode(const std::string &name, const serialization::pimpl::data &key,
                                            int64_t reference_id);

                /**
                 * Removes all the entries with the given key. The collection is NOT backed by the map, so changes to the map are
                 * NOT reflected in the collection, and vice-versa.
                 */
                ClientMessage HAZELCAST_API
                multimap_removeentry_encode(const std::string &name, const serialization::pimpl::data &key,
                                            const serialization::pimpl::data &value, int64_t thread_id);

                /**
                 * Inserts the specified element into this queue, waiting up to the specified wait time if necessary for space to
                 * become available.
                 */
                ClientMessage HAZELCAST_API
                queue_offer_encode(const std::string &name, const serialization::pimpl::data &value,
                                   int64_t timeout_millis);

                /**
                 * Inserts the specified element into this queue, waiting if necessary for space to become available.
                 */
                ClientMessage HAZELCAST_API
                queue_put_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Returns the number of elements in this collection.  If this collection contains more than Integer.MAX_VALUE
                 * elements, returns Integer.MAX_VALUE
                 */
                ClientMessage HAZELCAST_API queue_size_encode(const std::string  & name);

                /**
                 * Retrieves and removes the head of this queue.  This method differs from poll only in that it throws an exception
                 * if this queue is empty.
                 */
                ClientMessage HAZELCAST_API
                queue_remove_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Retrieves and removes the head of this queue, waiting up to the specified wait time if necessary for an element
                 * to become available.
                 */
                ClientMessage HAZELCAST_API queue_poll_encode(const std::string &name, int64_t timeout_millis);

                /**
                 * Retrieves and removes the head of this queue, waiting if necessary until an element becomes available.
                 */
                ClientMessage HAZELCAST_API queue_take_encode(const std::string  & name);

                /**
                 * Retrieves, but does not remove, the head of this queue, or returns null if this queue is empty.
                 */
                ClientMessage HAZELCAST_API queue_peek_encode(const std::string  & name);

                /**
                 * Returns an iterator over the elements in this collection.  There are no guarantees concerning the order in which
                 * the elements are returned (unless this collection is an instance of some class that provides a guarantee).
                 */
                ClientMessage HAZELCAST_API queue_iterator_encode(const std::string  & name);

                /**
                 * Removes all available elements from this queue and adds them to the given collection.  This operation may be more
                 * efficient than repeatedly polling this queue.  A failure encountered while attempting to add elements to
                 * collection c may result in elements being in neither, either or both collections when the associated exception is
                 * thrown. Attempts to drain a queue to itself result in ILLEGAL_ARGUMENT. Further, the behavior of
                 * this operation is undefined if the specified collection is modified while the operation is in progress.
                 */
                ClientMessage HAZELCAST_API queue_drainto_encode(const std::string  & name);

                /**
                 * Removes at most the given number of available elements from this queue and adds them to the given collection.
                 * A failure encountered while attempting to add elements to collection may result in elements being in neither,
                 * either or both collections when the associated exception is thrown. Attempts to drain a queue to itself result in
                 * ILLEGAL_ARGUMENT. Further, the behavior of this operation is undefined if the specified collection is
                 * modified while the operation is in progress.
                 */
                ClientMessage HAZELCAST_API queue_draintomaxsize_encode(const std::string &name, int32_t max_size);

                /**
                 * Returns true if this queue contains the specified element. More formally, returns true if and only if this queue
                 * contains at least one element e such that value.equals(e)
                 */
                ClientMessage HAZELCAST_API
                queue_contains_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Return true if this collection contains all of the elements in the specified collection.
                 */
                ClientMessage HAZELCAST_API queue_containsall_encode(const std::string &name,
                                                                     const std::vector<serialization::pimpl::data> &data_list);

                /**
                 * Removes all of this collection's elements that are also contained in the specified collection (optional operation).
                 * After this call returns, this collection will contain no elements in common with the specified collection.
                 */
                ClientMessage HAZELCAST_API queue_compareandremoveall_encode(const std::string &name,
                                                                             const std::vector<serialization::pimpl::data> &data_list);

                /**
                 * Retains only the elements in this collection that are contained in the specified collection (optional operation).
                 * In other words, removes from this collection all of its elements that are not contained in the specified collection.
                 */
                ClientMessage HAZELCAST_API queue_compareandretainall_encode(const std::string &name,
                                                                             const std::vector<serialization::pimpl::data> &data_list);

                /**
                 * Removes all of the elements from this collection (optional operation). The collection will be empty after this
                 * method returns.
                 */
                ClientMessage HAZELCAST_API queue_clear_encode(const std::string  & name);

                /**
                 * Adds all of the elements in the specified collection to this collection (optional operation).The behavior of this
                 * operation is undefined if the specified collection is modified while the operation is in progress.
                 * (This implies that the behavior of this call is undefined if the specified collection is this collection,
                 * and this collection is nonempty.)
                 */
                ClientMessage HAZELCAST_API
                queue_addall_encode(const std::string &name, const std::vector<serialization::pimpl::data> &data_list);

                /**
                 * Adds an listener for this collection. Listener will be notified or all collection add/remove events.
                 */
                ClientMessage HAZELCAST_API
                queue_addlistener_encode(const std::string &name, bool include_value, bool local_only);

                struct HAZELCAST_API queue_addlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param item Item that the event is fired for.
                     * @param uuid UUID of the member that dispatches this event.
                     * @param eventType Type of the event. It is either ADDED(1) or REMOVED(2).
                    */
                    virtual void
                    handle_item(const boost::optional<serialization::pimpl::data> &item, boost::uuids::uuid uuid,
                                int32_t event_type) = 0;

                };

                /**
                 * Removes the specified item listener. If there is no such listener added before, this call does no change in the
                 * cluster and returns false.
                 */
                ClientMessage HAZELCAST_API
                queue_removelistener_encode(const std::string &name, boost::uuids::uuid registration_id);

                /**
                 * Returns the number of additional elements that this queue can ideally (in the absence of memory or resource
                 * constraints) accept without blocking, or Integer.MAX_VALUE if there is no intrinsic limit. Note that you cannot
                 * always tell if an attempt to insert an element will succeed by inspecting remainingCapacity because it may be
                 * the case that another thread is about to insert or remove an element.
                 */
                ClientMessage HAZELCAST_API queue_remainingcapacity_encode(const std::string  & name);

                /**
                 * Returns true if this collection contains no elements.
                 */
                ClientMessage HAZELCAST_API queue_isempty_encode(const std::string  & name);

                /**
                 * Publishes the message to all subscribers of this topic
                 */
                ClientMessage HAZELCAST_API
                topic_publish_encode(const std::string &name, const serialization::pimpl::data &message);

                /**
                 * Subscribes to this topic. When someone publishes a message on this topic. onMessage() function of the given
                 * MessageListener is called. More than one message listener can be added on one instance.
                 */
                ClientMessage HAZELCAST_API topic_addmessagelistener_encode(const std::string &name, bool local_only);

                struct HAZELCAST_API topic_addmessagelistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param item Item that the event is fired for.
                     * @param publishTime Time that the item is published to the topic.
                     * @param uuid UUID of the member that dispatches this event.
                    */
                    virtual void handle_topic(serialization::pimpl::data const &item, int64_t publish_time,
                                              boost::uuids::uuid uuid) = 0;

                };

                /**
                 * Stops receiving messages for the given message listener.If the given listener already removed, this method does nothing.
                 */
                ClientMessage HAZELCAST_API
                topic_removemessagelistener_encode(const std::string &name, boost::uuids::uuid registration_id);

                /**
                 * Returns the number of elements in this list.  If this list contains more than Integer.MAX_VALUE elements, returns
                 * Integer.MAX_VALUE.
                 */
                ClientMessage HAZELCAST_API list_size_encode(const std::string  & name);

                /**
                 * Returns true if this list contains the specified element.
                 */
                ClientMessage HAZELCAST_API
                list_contains_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Returns true if this list contains all of the elements of the specified collection.
                 */
                ClientMessage HAZELCAST_API
                list_containsall_encode(const std::string &name, const std::vector<serialization::pimpl::data> &values);

                /**
                 * Appends the specified element to the end of this list (optional operation). Lists that support this operation may
                 * place limitations on what elements may be added to this list.  In particular, some lists will refuse to add null
                 * elements, and others will impose restrictions on the type of elements that may be added. List classes should
                 * clearly specify in their documentation any restrictions on what elements may be added.
                 */
                ClientMessage HAZELCAST_API
                list_add_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Removes the first occurrence of the specified element from this list, if it is present (optional operation).
                 * If this list does not contain the element, it is unchanged.
                 * Returns true if this list contained the specified element (or equivalently, if this list changed as a result of the call).
                 */
                ClientMessage HAZELCAST_API
                list_remove_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Appends all of the elements in the specified collection to the end of this list, in the order that they are
                 * returned by the specified collection's iterator (optional operation).
                 * The behavior of this operation is undefined if the specified collection is modified while the operation is in progress.
                 * (Note that this will occur if the specified collection is this list, and it's nonempty.)
                 */
                ClientMessage HAZELCAST_API
                list_addall_encode(const std::string &name, const std::vector<serialization::pimpl::data> &value_list);

                /**
                 * Removes from this list all of its elements that are contained in the specified collection (optional operation).
                 */
                ClientMessage HAZELCAST_API list_compareandremoveall_encode(const std::string &name,
                                                                            const std::vector<serialization::pimpl::data> &values);

                /**
                 * Retains only the elements in this list that are contained in the specified collection (optional operation).
                 * In other words, removes from this list all of its elements that are not contained in the specified collection.
                 */
                ClientMessage HAZELCAST_API list_compareandretainall_encode(const std::string &name,
                                                                            const std::vector<serialization::pimpl::data> &values);

                /**
                 * Removes all of the elements from this list (optional operation). The list will be empty after this call returns.
                 */
                ClientMessage HAZELCAST_API list_clear_encode(const std::string  & name);

                /**
                 * Return the all elements of this collection
                 */
                ClientMessage HAZELCAST_API list_getall_encode(const std::string  & name);

                /**
                 * Adds an item listener for this collection. Listener will be notified for all collection add/remove events.
                 */
                ClientMessage HAZELCAST_API
                list_addlistener_encode(const std::string &name, bool include_value, bool local_only);

                struct HAZELCAST_API list_addlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param item Item that the event is fired for.
                     * @param uuid UUID of the member that dispatches this event.
                     * @param eventType Type of the event. It is either ADDED(1) or REMOVED(2).
                    */
                    virtual void
                    handle_item(const boost::optional<serialization::pimpl::data> &item, boost::uuids::uuid uuid,
                                int32_t event_type) = 0;

                };

                /**
                 * Removes the specified item listener. If there is no such listener added before, this call does no change in the
                 * cluster and returns false.
                 */
                ClientMessage HAZELCAST_API
                list_removelistener_encode(const std::string &name, boost::uuids::uuid registration_id);

                /**
                 * Returns true if this list contains no elements
                 */
                ClientMessage HAZELCAST_API list_isempty_encode(const std::string  & name);

                /**
                 * Inserts all of the elements in the specified collection into this list at the specified position (optional operation).
                 * Shifts the element currently at that position (if any) and any subsequent elements to the right (increases their indices).
                 * The new elements will appear in this list in the order that they are returned by the specified collection's iterator.
                 * The behavior of this operation is undefined if the specified collection is modified while the operation is in progress.
                 * (Note that this will occur if the specified collection is this list, and it's nonempty.)
                 */
                ClientMessage HAZELCAST_API list_addallwithindex_encode(const std::string &name, int32_t index,
                                                                        const std::vector<serialization::pimpl::data> &value_list);

                /**
                 * Returns the element at the specified position in this list
                 */
                ClientMessage HAZELCAST_API list_get_encode(const std::string  & name, int32_t index);

                /**
                 * The element previously at the specified position
                 */
                ClientMessage HAZELCAST_API
                list_set_encode(const std::string &name, int32_t index, const serialization::pimpl::data &value);

                /**
                 * Inserts the specified element at the specified position in this list (optional operation). Shifts the element
                 * currently at that position (if any) and any subsequent elements to the right (adds one to their indices).
                 */
                ClientMessage HAZELCAST_API list_addwithindex_encode(const std::string &name, int32_t index,
                                                                     const serialization::pimpl::data &value);

                /**
                 * Removes the element at the specified position in this list (optional operation). Shifts any subsequent elements
                 * to the left (subtracts one from their indices). Returns the element that was removed from the list.
                 */
                ClientMessage HAZELCAST_API list_removewithindex_encode(const std::string  & name, int32_t index);

                /**
                 * Returns the index of the last occurrence of the specified element in this list, or -1 if this list does not
                 * contain the element.
                 */
                ClientMessage HAZELCAST_API
                list_lastindexof_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Returns the index of the first occurrence of the specified element in this list, or -1 if this list does not
                 * contain the element.
                 */
                ClientMessage HAZELCAST_API
                list_indexof_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Returns a view of the portion of this list between the specified from, inclusive, and to, exclusive.(If from and
                 * to are equal, the returned list is empty.) The returned list is backed by this list, so non-structural changes in
                 * the returned list are reflected in this list, and vice-versa. The returned list supports all of the optional list
                 * operations supported by this list.
                 * This method eliminates the need for explicit range operations (of the sort that commonly exist for arrays).
                 * Any operation that expects a list can be used as a range operation by passing a subList view instead of a whole list.
                 * Similar idioms may be constructed for indexOf and lastIndexOf, and all of the algorithms in the Collections class
                 * can be applied to a subList.
                 * The semantics of the list returned by this method become undefined if the backing list (i.e., this list) is
                 * structurally modified in any way other than via the returned list.(Structural modifications are those that change
                 * the size of this list, or otherwise perturb it in such a fashion that iterations in progress may yield incorrect results.)
                 */
                ClientMessage HAZELCAST_API list_sub_encode(const std::string  & name, int32_t from, int32_t to);

                /**
                 * Returns the number of elements in this set (its cardinality). If this set contains more than Integer.MAX_VALUE
                 * elements, returns Integer.MAX_VALUE.
                 */
                ClientMessage HAZELCAST_API set_size_encode(const std::string  & name);

                /**
                 * Returns true if this set contains the specified element.
                 */
                ClientMessage HAZELCAST_API
                set_contains_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Returns true if this set contains all of the elements of the specified collection. If the specified collection is
                 * also a set, this method returns true if it is a subset of this set.
                 */
                ClientMessage HAZELCAST_API
                set_containsall_encode(const std::string &name, const std::vector<serialization::pimpl::data> &items);

                /**
                 * Adds the specified element to this set if it is not already present (optional operation).
                 * If this set already contains the element, the call leaves the set unchanged and returns false.In combination with
                 * the restriction on constructors, this ensures that sets never contain duplicate elements.
                 * The stipulation above does not imply that sets must accept all elements; sets may refuse to add any particular
                 * element, including null, and throw an exception, as described in the specification for Collection
                 * Individual set implementations should clearly document any restrictions on the elements that they may contain.
                 */
                ClientMessage HAZELCAST_API
                set_add_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Removes the specified element from this set if it is present (optional operation).
                 * Returns true if this set contained the element (or equivalently, if this set changed as a result of the call).
                 * (This set will not contain the element once the call returns.)
                 */
                ClientMessage HAZELCAST_API
                set_remove_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Adds all of the elements in the specified collection to this set if they're not already present
                 * (optional operation). If the specified collection is also a set, the addAll operation effectively modifies this
                 * set so that its value is the union of the two sets. The behavior of this operation is undefined if the specified
                 * collection is modified while the operation is in progress.
                 */
                ClientMessage HAZELCAST_API
                set_addall_encode(const std::string &name, const std::vector<serialization::pimpl::data> &value_list);

                /**
                 * Removes from this set all of its elements that are contained in the specified collection (optional operation).
                 * If the specified collection is also a set, this operation effectively modifies this set so that its value is the
                 * asymmetric set difference of the two sets.
                 */
                ClientMessage HAZELCAST_API set_compareandremoveall_encode(const std::string &name,
                                                                           const std::vector<serialization::pimpl::data> &values);

                /**
                 * Retains only the elements in this set that are contained in the specified collection (optional operation).
                 * In other words, removes from this set all of its elements that are not contained in the specified collection.
                 * If the specified collection is also a set, this operation effectively modifies this set so that its value is the
                 * intersection of the two sets.
                 */
                ClientMessage HAZELCAST_API set_compareandretainall_encode(const std::string &name,
                                                                           const std::vector<serialization::pimpl::data> &values);

                /**
                 * Removes all of the elements from this set (optional operation). The set will be empty after this call returns.
                 */
                ClientMessage HAZELCAST_API set_clear_encode(const std::string  & name);

                /**
                 * Return the all elements of this collection
                 */
                ClientMessage HAZELCAST_API set_getall_encode(const std::string  & name);

                /**
                 * Adds an item listener for this collection. Listener will be notified for all collection add/remove events.
                 */
                ClientMessage HAZELCAST_API
                set_addlistener_encode(const std::string &name, bool include_value, bool local_only);

                struct HAZELCAST_API set_addlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param item Item that the event is fired for.
                     * @param uuid UUID of the member that dispatches this event.
                     * @param eventType Type of the event. It is either ADDED(1) or REMOVED(2).
                    */
                    virtual void
                    handle_item(const boost::optional<serialization::pimpl::data> &item, boost::uuids::uuid uuid,
                                int32_t event_type) = 0;

                };

                /**
                 * Removes the specified item listener. If there is no such listener added before, this call does no change in the
                 * cluster and returns false.
                 */
                ClientMessage HAZELCAST_API
                set_removelistener_encode(const std::string &name, boost::uuids::uuid registration_id);

                /**
                 * Returns true if this set contains no elements.
                 */
                ClientMessage HAZELCAST_API set_isempty_encode(const std::string  & name);

                /**
                 * Acquires the given FencedLock on the given CP group. If the lock is
                 * acquired, a valid fencing token (positive number) is returned. If not
                 * acquired because of max reentrant entry limit, the call returns -1.
                 * If the lock is held by some other endpoint when this method is called,
                 * the caller thread is blocked until the lock is released. If the session
                 * is closed between reentrant acquires, the call fails with
                 * {@code LockOwnershipLostException}.
                 */
                ClientMessage HAZELCAST_API
                fencedlock_lock_encode(const cp::raft_group_id &group_id, const std::string &name, int64_t session_id,
                                       int64_t thread_id, boost::uuids::uuid invocation_uid);

                /**
                 * Attempts to acquire the given FencedLock on the given CP group.
                 * If the lock is acquired, a valid fencing token (positive number) is
                 * returned. If not acquired either because of max reentrant entry limit or
                 * the lock is not free during the timeout duration, the call returns -1.
                 * If the lock is held by some other endpoint when this method is called,
                 * the caller thread is blocked until the lock is released or the timeout
                 * duration passes. If the session is closed between reentrant acquires,
                 * the call fails with {@code LockOwnershipLostException}.
                 */
                ClientMessage HAZELCAST_API
                fencedlock_trylock_encode(const cp::raft_group_id &group_id, const std::string &name,
                                          int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid,
                                          int64_t timeout_ms);

                /**
                 * Unlocks the given FencedLock on the given CP group. If the lock is
                 * not acquired, the call fails with {@link IllegalMonitorStateException}.
                 * If the session is closed while holding the lock, the call fails with
                 * {@code LockOwnershipLostException}. Returns true if the lock is still
                 * held by the caller after a successful unlock() call, false otherwise.
                 */
                ClientMessage HAZELCAST_API
                fencedlock_unlock_encode(const cp::raft_group_id &group_id, const std::string &name, int64_t session_id,
                                         int64_t thread_id, boost::uuids::uuid invocation_uid);

                /**
                 * Returns current lock ownership status of the given FencedLock instance.
                 */
                ClientMessage HAZELCAST_API
                fencedlock_getlockownership_encode(const cp::raft_group_id &group_id, const std::string &name);

                /**
                 * Initiates an orderly shutdown in which previously submitted tasks are executed, but no new tasks will be accepted.
                 * Invocation has no additional effect if already shut down.
                 */
                ClientMessage HAZELCAST_API executorservice_shutdown_encode(const std::string  & name);

                /**
                 * Returns true if this executor has been shut down.
                 */
                ClientMessage HAZELCAST_API executorservice_isshutdown_encode(const std::string  & name);

                /**
                 * Cancels the task running on the member that owns the partition with the given id.
                 */
                ClientMessage HAZELCAST_API executorservice_cancelonpartition_encode(boost::uuids::uuid uuid, bool interrupt);

                /**
                 * Cancels the task running on the member with the given address.
                 */
                ClientMessage HAZELCAST_API
                executorservice_cancelonmember_encode(boost::uuids::uuid uuid, boost::uuids::uuid member_uuid,
                                                      bool interrupt);

                /**
                 * Submits the task to the member that owns the partition with the given id.
                 */
                ClientMessage HAZELCAST_API
                executorservice_submittopartition_encode(const std::string &name, boost::uuids::uuid uuid,
                                                         const serialization::pimpl::data &callable);

                /**
                 * Submits the task to member specified by the address.
                 */
                ClientMessage HAZELCAST_API
                executorservice_submittomember_encode(const std::string &name, boost::uuids::uuid uuid,
                                                      const serialization::pimpl::data &callable,
                                                      boost::uuids::uuid member_uuid);

                /**
                 * Applies a function on the value, the actual stored value will not change
                 */
                ClientMessage HAZELCAST_API
                atomiclong_apply_encode(const cp::raft_group_id &group_id, const std::string &name,
                                        const serialization::pimpl::data &function);

                /**
                 * Alters the currently stored value by applying a function on it.
                 */
                ClientMessage HAZELCAST_API
                atomiclong_alter_encode(const cp::raft_group_id &group_id, const std::string &name,
                                        const serialization::pimpl::data &function, int32_t return_value_type);

                /**
                 * Atomically adds the given value to the current value.
                 */
                ClientMessage HAZELCAST_API
                atomiclong_addandget_encode(const cp::raft_group_id &group_id, const std::string &name, int64_t delta);

                /**
                 * Atomically sets the value to the given updated value only if the current
                 * value the expected value.
                 */
                ClientMessage HAZELCAST_API
                atomiclong_compareandset_encode(const cp::raft_group_id &group_id, const std::string &name,
                                                int64_t expected, int64_t updated);

                /**
                 * Gets the current value.
                 */
                ClientMessage HAZELCAST_API
                atomiclong_get_encode(const cp::raft_group_id &group_id, const std::string &name);

                /**
                 * Atomically adds the given value to the current value.
                 */
                ClientMessage HAZELCAST_API
                atomiclong_getandadd_encode(const cp::raft_group_id &group_id, const std::string &name, int64_t delta);

                /**
                 * Atomically sets the given value and returns the old value.
                 */
                ClientMessage HAZELCAST_API
                atomiclong_getandset_encode(const cp::raft_group_id &group_id, const std::string &name,
                                            int64_t new_value);

                /**
                 * Applies a function on the value
                 */
                ClientMessage HAZELCAST_API
                atomicref_apply_encode(const cp::raft_group_id &group_id, const std::string &name,
                                       const serialization::pimpl::data &function, int32_t return_value_type,
                                       bool alter);

                /**
                 * Alters the currently stored value by applying a function on it.
                 */
                ClientMessage HAZELCAST_API
                atomicref_compareandset_encode(const cp::raft_group_id &group_id, const std::string &name,
                                               const serialization::pimpl::data *old_value,
                                               const serialization::pimpl::data *new_value);

                /**
                 * Checks if the reference contains the value.
                 */
                ClientMessage HAZELCAST_API
                atomicref_contains_encode(const cp::raft_group_id &group_id, const std::string &name,
                                          const serialization::pimpl::data *value);

                /**
                 * Gets the current value.
                 */
                ClientMessage HAZELCAST_API
                atomicref_get_encode(const cp::raft_group_id &group_id, const std::string &name);

                /**
                 * Atomically sets the given value
                 */
                ClientMessage HAZELCAST_API
                atomicref_set_encode(const cp::raft_group_id &group_id, const std::string &name,
                                     const serialization::pimpl::data *new_value, bool return_old_value);

                /**
                 * Sets the count to the given value if the current count is zero.
                 * If the count is not zero, then this method does nothing
                 * and returns false
                 */
                ClientMessage HAZELCAST_API
                countdownlatch_trysetcount_encode(const cp::raft_group_id &group_id, const std::string &name,
                                                  int32_t count);

                /**
                 * Causes the current thread to wait until the latch has counted down
                 * to zero, or an exception is thrown, or the specified waiting time
                 * elapses. If the current count is zero then this method returns
                 * immediately with the value true. If the current count is greater than
                 * zero, then the current thread becomes disabled for thread scheduling
                 * purposes and lies dormant until one of five things happen: the count
                 * reaches zero due to invocations of the {@code countDown} method, this
                 * ICountDownLatch instance is destroyed, the countdown owner becomes
                 * disconnected, some other thread Thread#interrupt interrupts the current
                 * thread, or the specified waiting time elapses. If the count reaches zero
                 * then the method returns with the value true. If the current thread has
                 * its interrupted status set on entry to this method, or is interrupted
                 * while waiting, then {@code InterruptedException} is thrown
                 * and the current thread's interrupted status is cleared. If the specified
                 * waiting time elapses then the value false is returned.  If the time is
                 * less than or equal to zero, the method will not wait at all.
                 */
                ClientMessage HAZELCAST_API
                countdownlatch_await_encode(const cp::raft_group_id &group_id, const std::string &name,
                                            boost::uuids::uuid invocation_uid, int64_t timeout_ms);

                /**
                 * Decrements the count of the latch, releasing all waiting threads if
                 * the count reaches zero. If the current count is greater than zero, then
                 * it is decremented. If the new count is zero: All waiting threads are
                 * re-enabled for thread scheduling purposes, and Countdown owner is set to
                 * null. If the current count equals zero, then nothing happens.
                 */
                ClientMessage HAZELCAST_API
                countdownlatch_countdown_encode(const cp::raft_group_id &group_id, const std::string &name,
                                                boost::uuids::uuid invocation_uid, int32_t expected_round);

                /**
                 * Returns the current count.
                 */
                ClientMessage HAZELCAST_API
                countdownlatch_getcount_encode(const cp::raft_group_id &group_id, const std::string &name);

                /**
                 * Returns the current round. A round completes when the count value
                 * reaches to 0 and a new round starts afterwards.
                 */
                ClientMessage HAZELCAST_API
                countdownlatch_getround_encode(const cp::raft_group_id &group_id, const std::string &name);

                /**
                 * Initializes the ISemaphore instance with the given permit number, if not
                 * initialized before.
                 */
                ClientMessage HAZELCAST_API
                semaphore_init_encode(const cp::raft_group_id &group_id, const std::string &name, int32_t permits);

                /**
                 * Acquires the requested amount of permits if available, reducing
                 * the number of available permits. If no enough permits are available,
                 * then the current thread becomes disabled for thread scheduling purposes
                 * and lies dormant until other threads release enough permits.
                 */
                ClientMessage HAZELCAST_API
                semaphore_acquire_encode(const cp::raft_group_id &group_id, const std::string &name, int64_t session_id,
                                         int64_t thread_id, boost::uuids::uuid invocation_uid, int32_t permits,
                                         int64_t timeout_ms);

                /**
                 * Releases the given number of permits and increases the number of
                 * available permits by that amount.
                 */
                ClientMessage HAZELCAST_API
                semaphore_release_encode(const cp::raft_group_id &group_id, const std::string &name, int64_t session_id,
                                         int64_t thread_id, boost::uuids::uuid invocation_uid, int32_t permits);

                /**
                 * Acquires all available permits at once and returns immediately.
                 */
                ClientMessage HAZELCAST_API
                semaphore_drain_encode(const cp::raft_group_id &group_id, const std::string &name, int64_t session_id,
                                       int64_t thread_id, boost::uuids::uuid invocation_uid);

                /**
                 * Increases or decreases the number of permits by the given value.
                 */
                ClientMessage HAZELCAST_API
                semaphore_change_encode(const cp::raft_group_id &group_id, const std::string &name, int64_t session_id,
                                        int64_t thread_id, boost::uuids::uuid invocation_uid, int32_t permits);

                /**
                 * Returns the number of available permits.
                 */
                ClientMessage HAZELCAST_API
                semaphore_availablepermits_encode(const cp::raft_group_id &group_id, const std::string &name);

                /**
                 * Returns true if the semaphore is JDK compatible
                 */
                ClientMessage HAZELCAST_API semaphore_getsemaphoretype_encode(const std::string &proxy_name);

                /**
                 * Associates a given value to the specified key and replicates it to the cluster. If there is an old value, it will
                 * be replaced by the specified one and returned from the call. In addition, you have to specify a ttl and its TimeUnit
                 * to define when the value is outdated and thus should be removed from the replicated map.
                 */
                ClientMessage HAZELCAST_API
                replicatedmap_put_encode(const std::string &name, const serialization::pimpl::data &key,
                                         const serialization::pimpl::data &value, int64_t ttl);

                /**
                 * Returns the number of key-value mappings in this map. If the map contains more than Integer.MAX_VALUE elements,
                 * returns Integer.MAX_VALUE.
                 */
                ClientMessage HAZELCAST_API replicatedmap_size_encode(const std::string  & name);

                /**
                 * Return true if this map contains no key-value mappings
                 */
                ClientMessage HAZELCAST_API replicatedmap_isempty_encode(const std::string  & name);

                /**
                 * Returns true if this map contains a mapping for the specified key.
                 */
                ClientMessage HAZELCAST_API
                replicatedmap_containskey_encode(const std::string &name, const serialization::pimpl::data &key);

                /**
                 * Returns true if this map maps one or more keys to the specified value.
                 * This operation will probably require time linear in the map size for most implementations of the Map interface.
                 */
                ClientMessage HAZELCAST_API
                replicatedmap_containsvalue_encode(const std::string &name, const serialization::pimpl::data &value);

                /**
                 * Returns the value to which the specified key is mapped, or null if this map contains no mapping for the key.
                 * If this map permits null values, then a return value of null does not
                 * necessarily indicate that the map contains no mapping for the key; it's also possible that the map
                 * explicitly maps the key to null.  The #containsKey operation may be used to distinguish these two cases.
                 */
                ClientMessage HAZELCAST_API
                replicatedmap_get_encode(const std::string &name, const serialization::pimpl::data &key);

                /**
                 * Removes the mapping for a key from this map if it is present (optional operation). Returns the value to which this map previously associated the key,
                 * or null if the map contained no mapping for the key. If this map permits null values, then a return value of
                 * null does not necessarily indicate that the map contained no mapping for the key; it's also possible that the map
                 * explicitly mapped the key to null. The map will not contain a mapping for the specified key once the call returns.
                 */
                ClientMessage HAZELCAST_API
                replicatedmap_remove_encode(const std::string &name, const serialization::pimpl::data &key);

                /**
                 * Copies all of the mappings from the specified map to this map (optional operation). The effect of this call is
                 * equivalent to that of calling put(Object,Object) put(k, v) on this map once for each mapping from key k to value
                 * v in the specified map. The behavior of this operation is undefined if the specified map is modified while the
                 * operation is in progress.
                 */
                ClientMessage HAZELCAST_API replicatedmap_putall_encode(const std::string &name,
                                                                        const std::vector<std::pair<serialization::pimpl::data, serialization::pimpl::data>> &entries);

                /**
                 * The clear operation wipes data out of the replicated maps.It is the only synchronous remote operation in this
                 * implementation, so be aware that this might be a slow operation. If some node fails on executing the operation,
                 * it is retried for at most 3 times (on the failing nodes only). If it does not work after the third time, this
                 * method throws a OPERATION_TIMEOUT back to the caller.
                 */
                ClientMessage HAZELCAST_API replicatedmap_clear_encode(const std::string  & name);

                /**
                 * Adds an continuous entry listener for this map. The listener will be notified for map add/remove/update/evict
                 * events filtered by the given predicate.
                 */
                ClientMessage HAZELCAST_API
                replicatedmap_addentrylistenertokeywithpredicate_encode(const std::string &name,
                                                                        const serialization::pimpl::data &key,
                                                                        const serialization::pimpl::data &predicate,
                                                                        bool local_only);

                struct HAZELCAST_API replicatedmap_addentrylistenertokeywithpredicate_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Adds an continuous entry listener for this map. The listener will be notified for map add/remove/update/evict
                 * events filtered by the given predicate.
                 */
                ClientMessage HAZELCAST_API replicatedmap_addentrylistenerwithpredicate_encode(const std::string &name,
                                                                                               const serialization::pimpl::data &predicate,
                                                                                               bool local_only);

                struct HAZELCAST_API replicatedmap_addentrylistenerwithpredicate_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Adds the specified entry listener for the specified key. The listener will be notified for all
                 * add/remove/update/evict events of the specified key only.
                 */
                ClientMessage HAZELCAST_API replicatedmap_addentrylistenertokey_encode(const std::string &name,
                                                                                       const serialization::pimpl::data &key,
                                                                                       bool local_only);

                struct HAZELCAST_API replicatedmap_addentrylistenertokey_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Adds an entry listener for this map. The listener will be notified for all map add/remove/update/evict events.
                 */
                ClientMessage HAZELCAST_API
                replicatedmap_addentrylistener_encode(const std::string &name, bool local_only);

                struct HAZELCAST_API replicatedmap_addentrylistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Removes the specified entry listener. If there is no such listener added before, this call does no change in the
                 * cluster and returns false.
                 */
                ClientMessage HAZELCAST_API
                replicatedmap_removeentrylistener_encode(const std::string &name, boost::uuids::uuid registration_id);

                /**
                 * Returns a lazy Set view of the key contained in this map. A LazySet is optimized for querying speed
                 * (preventing eager deserialization and hashing on HashSet insertion) and does NOT provide all operations.
                 * Any kind of mutating function will throw an UNSUPPORTED_OPERATION. Same is true for operations
                 * like java.util.Set#contains(Object) and java.util.Set#containsAll(java.util.Collection) which would result in
                 * very poor performance if called repeatedly (for example, in a loop). If the use case is different from querying
                 * the data, please copy the resulting set into a new java.util.HashSet.
                 */
                ClientMessage HAZELCAST_API replicatedmap_keyset_encode(const std::string  & name);

                /**
                 * Returns a lazy collection view of the values contained in this map.
                 */
                ClientMessage HAZELCAST_API replicatedmap_values_encode(const std::string  & name);

                /**
                 * Gets a lazy set view of the mappings contained in this map.
                 */
                ClientMessage HAZELCAST_API replicatedmap_entryset_encode(const std::string  & name);

                /**
                 * Adds a near cache entry listener for this map. This listener will be notified when an entry is added/removed/updated/evicted/expired etc. so that the near cache entries can be invalidated.
                 */
                ClientMessage HAZELCAST_API
                replicatedmap_addnearcacheentrylistener_encode(const std::string &name, bool include_value,
                                                               bool local_only);

                struct HAZELCAST_API replicatedmap_addnearcacheentrylistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);

                    /**
                     * @param key Key of the entry event.
                     * @param value Value of the entry event.
                     * @param oldValue Old value of the entry event.
                     * @param mergingValue Incoming merging value of the entry event.
                     * @param eventType Type of the entry event. Possible values are
                     *                  ADDED(1)
                     *                  REMOVED(2)
                     *                  UPDATED(4)
                     *                  EVICTED(8)
                     *                  EXPIRED(16)
                     *                  EVICT_ALL(32)
                     *                  CLEAR_ALL(64)
                     *                  MERGED(128)
                     *                  INVALIDATION(256)
                     *                  LOADED(512)
                     * @param uuid UUID of the member that dispatches the event.
                     * @param numberOfAffectedEntries Number of entries affected by this event.
                    */
                    virtual void handle_entry(const boost::optional<serialization::pimpl::data> &key,
                                              const boost::optional<serialization::pimpl::data> &value,
                                              const boost::optional<serialization::pimpl::data> &old_value,
                                              const boost::optional<serialization::pimpl::data> &merging_value,
                                              int32_t event_type, boost::uuids::uuid uuid,
                                              int32_t number_of_affected_entries) = 0;

                };

                /**
                 * Returns true if this map contains an entry for the specified key.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_containskey_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                    int64_t thread_id, const serialization::pimpl::data &key);

                /**
                 * Returns the value for the specified key, or null if this map does not contain this key.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_get_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                            const serialization::pimpl::data &key);

                /**
                 * Returns the number of entries in this map.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_size_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id);

                /**
                 * Returns true if this map contains no entries.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_isempty_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id);

                /**
                 * Associates the specified value with the specified key in this map. If the map previously contained a mapping for
                 * the key, the old value is replaced by the specified value. The object to be put will be accessible only in the
                 * current transaction context till transaction is committed.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_put_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                            const serialization::pimpl::data &key,
                                            const serialization::pimpl::data &value, int64_t ttl);

                /**
                 * Associates the specified value with the specified key in this map. If the map previously contained a mapping for
                 * the key, the old value is replaced by the specified value. This method is preferred to #put(Object, Object)
                 * if the old value is not needed.
                 * The object to be set will be accessible only in the current transaction context until the transaction is committed.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_set_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                            const serialization::pimpl::data &key,
                                            const serialization::pimpl::data &value);

                /**
                 * If the specified key is not already associated with a value, associate it with the given value.
                 * The object to be put will be accessible only in the current transaction context until the transaction is committed.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_putifabsent_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                    int64_t thread_id, const serialization::pimpl::data &key,
                                                    const serialization::pimpl::data &value);

                /**
                 * Replaces the entry for a key only if it is currently mapped to some value. The object to be replaced will be
                 * accessible only in the current transaction context until the transaction is committed.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_replace_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                                const serialization::pimpl::data &key,
                                                const serialization::pimpl::data &value);

                /**
                 * Replaces the entry for a key only if currently mapped to a given value. The object to be replaced will be
                 * accessible only in the current transaction context until the transaction is committed.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_replaceifsame_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                      int64_t thread_id, const serialization::pimpl::data &key,
                                                      const serialization::pimpl::data &old_value,
                                                      const serialization::pimpl::data &new_value);

                /**
                 * Removes the mapping for a key from this map if it is present. The map will not contain a mapping for the
                 * specified key once the call returns. The object to be removed will be accessible only in the current transaction
                 * context until the transaction is committed.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_remove_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                               const serialization::pimpl::data &key);

                /**
                 * Removes the mapping for a key from this map if it is present. The map will not contain a mapping for the specified
                 * key once the call returns. This method is preferred to #remove(Object) if the old value is not needed. The object
                 * to be deleted will be removed from only the current transaction context until the transaction is committed.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_delete_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                               const serialization::pimpl::data &key);

                /**
                 * Removes the entry for a key only if currently mapped to a given value. The object to be removed will be removed
                 * from only the current transaction context until the transaction is committed.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_removeifsame_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                     int64_t thread_id, const serialization::pimpl::data &key,
                                                     const serialization::pimpl::data &value);

                /**
                 * Returns a set clone of the keys contained in this map. The set is NOT backed by the map, so changes to the map
                 * are NOT reflected in the set, and vice-versa. This method is always executed by a distributed query, so it may throw
                 * a query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_keyset_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id);

                /**
                 * Queries the map based on the specified predicate and returns the keys of matching entries. Specified predicate
                 * runs on all members in parallel.The set is NOT backed by the map, so changes to the map are NOT reflected in the
                 * set, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_keysetwithpredicate_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                            int64_t thread_id,
                                                            const serialization::pimpl::data &predicate);

                /**
                 * Returns a collection clone of the values contained in this map. The collection is NOT backed by the map,
                 * so changes to the map are NOT reflected in the collection, and vice-versa. This method is always executed by a
                 * distributed query, so it may throw a query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_values_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id);

                /**
                 * Queries the map based on the specified predicate and returns the values of matching entries.Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw
                 * a query_result_size_exceeded if query result size limit is configured.
                 */
                ClientMessage HAZELCAST_API
                transactionalmap_valueswithpredicate_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                            int64_t thread_id,
                                                            const serialization::pimpl::data &predicate);

                /**
                 * Stores a key-value pair in the multimap.
                 */
                ClientMessage HAZELCAST_API
                transactionalmultimap_put_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                                 const serialization::pimpl::data &key,
                                                 const serialization::pimpl::data &value);

                /**
                 * Returns the collection of values associated with the key.
                 */
                ClientMessage HAZELCAST_API
                transactionalmultimap_get_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                                 const serialization::pimpl::data &key);

                /**
                 * Removes the given key value pair from the multimap.
                 */
                ClientMessage HAZELCAST_API
                transactionalmultimap_remove_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                    int64_t thread_id, const serialization::pimpl::data &key);

                /**
                 * Removes all the entries associated with the given key.
                 */
                ClientMessage HAZELCAST_API
                transactionalmultimap_removeentry_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                         int64_t thread_id, const serialization::pimpl::data &key,
                                                         const serialization::pimpl::data &value);

                /**
                 * Returns the number of values matching the given key in the multimap.
                 */
                ClientMessage HAZELCAST_API
                transactionalmultimap_valuecount_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                        int64_t thread_id, const serialization::pimpl::data &key);

                /**
                 * Returns the number of key-value pairs in the multimap.
                 */
                ClientMessage HAZELCAST_API
                transactionalmultimap_size_encode(const std::string &name, boost::uuids::uuid txn_id,
                                                  int64_t thread_id);

                /**
                 * Add new item to transactional set.
                 */
                ClientMessage HAZELCAST_API
                transactionalset_add_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                            const serialization::pimpl::data &item);

                /**
                 * Remove item from transactional set.
                 */
                ClientMessage HAZELCAST_API
                transactionalset_remove_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                               const serialization::pimpl::data &item);

                /**
                 * Returns the size of the set.
                 */
                ClientMessage HAZELCAST_API
                transactionalset_size_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id);

                /**
                 * Adds a new item to the transactional list.
                 */
                ClientMessage HAZELCAST_API
                transactionallist_add_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                             const serialization::pimpl::data &item);

                /**
                 * Remove item from the transactional list
                 */
                ClientMessage HAZELCAST_API
                transactionallist_remove_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                                const serialization::pimpl::data &item);

                /**
                 * Returns the size of the list
                 */
                ClientMessage HAZELCAST_API
                transactionallist_size_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id);

                /**
                 * Inserts the specified element into this queue, waiting up to the specified wait time if necessary for space to
                 * become available.
                 */
                ClientMessage HAZELCAST_API
                transactionalqueue_offer_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                                const serialization::pimpl::data &item, int64_t timeout);

                /**
                 * Retrieves and removes the head of this queue, waiting up to the specified wait time if necessary for an element
                 * to become available.
                 */
                ClientMessage HAZELCAST_API
                transactionalqueue_poll_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id,
                                               int64_t timeout);

                /**
                 * Returns the number of elements in this collection.If this collection contains more than Integer.MAX_VALUE
                 * elements, returns Integer.MAX_VALUE.
                 */
                ClientMessage HAZELCAST_API
                transactionalqueue_size_encode(const std::string &name, boost::uuids::uuid txn_id, int64_t thread_id);

                /**
                 * Commits the transaction with the given id.
                 */
                ClientMessage HAZELCAST_API
                transaction_commit_encode(boost::uuids::uuid transaction_id, int64_t thread_id);

                /**
                 * Creates a transaction with the given parameters.
                 */
                ClientMessage HAZELCAST_API
                transaction_create_encode(int64_t timeout, int32_t durability, int32_t transaction_type,
                                          int64_t thread_id);

                /**
                 * Rollbacks the transaction with the given id.
                 */
                ClientMessage HAZELCAST_API
                transaction_rollback_encode(boost::uuids::uuid transaction_id, int64_t thread_id);

                /**
                 * Returns number of items in the ringbuffer. If no ttl is set, the size will always be equal to capacity after the
                 * head completed the first looparound the ring. This is because no items are getting retired.
                 */
                ClientMessage HAZELCAST_API ringbuffer_size_encode(const std::string  & name);

                /**
                 * Returns the sequence of the tail. The tail is the side of the ringbuffer where the items are added to.
                 * The initial value of the tail is -1.
                 */
                ClientMessage HAZELCAST_API ringbuffer_tailsequence_encode(const std::string  & name);

                /**
                 * Returns the sequence of the head. The head is the side of the ringbuffer where the oldest items in the ringbuffer
                 * are found. If the RingBuffer is empty, the head will be one more than the tail.
                 * The initial value of the head is 0 (1 more than tail).
                 */
                ClientMessage HAZELCAST_API ringbuffer_headsequence_encode(const std::string  & name);

                /**
                 * Returns the capacity of this Ringbuffer.
                 */
                ClientMessage HAZELCAST_API ringbuffer_capacity_encode(const std::string  & name);

                /**
                 * Returns the remaining capacity of the ringbuffer. The returned value could be stale as soon as it is returned.
                 * If ttl is not set, the remaining capacity will always be the capacity.
                 */
                ClientMessage HAZELCAST_API ringbuffer_remainingcapacity_encode(const std::string  & name);

                /**
                 * Adds an item to the tail of the Ringbuffer. If there is space in the ringbuffer, the call
                 * will return the sequence of the written item. If there is no space, it depends on the overflow policy what happens:
                 * OverflowPolicy OVERWRITE we just overwrite the oldest item in the ringbuffer and we violate the ttl
                 * OverflowPolicy FAIL we return -1. The reason that FAIL exist is to give the opportunity to obey the ttl.
                 * <p/>
                 * This sequence will always be unique for this Ringbuffer instance so it can be used as a unique id generator if you are
                 * publishing items on this Ringbuffer. However you need to take care of correctly determining an initial id when any node
                 * uses the ringbuffer for the first time. The most reliable way to do that is to write a dummy item into the ringbuffer and
                 * use the returned sequence as initial  id. On the reading side, this dummy item should be discard. Please keep in mind that
                 * this id is not the sequence of the item you are about to publish but from a previously published item. So it can't be used
                 * to find that item.
                 */
                ClientMessage HAZELCAST_API ringbuffer_add_encode(const std::string &name, int32_t overflow_policy,
                                                                  const serialization::pimpl::data &value);

                /**
                 * Reads one item from the Ringbuffer. If the sequence is one beyond the current tail, this call blocks until an
                 * item is added. This method is not destructive unlike e.g. a queue.take. So the same item can be read by multiple
                 * readers or it can be read multiple times by the same reader. Currently it isn't possible to control how long this
                 * call is going to block. In the future we could add e.g. tryReadOne(long sequence, long timeout, TimeUnit unit).
                 */
                ClientMessage HAZELCAST_API ringbuffer_readone_encode(const std::string  & name, int64_t sequence);

                /**
                 * Adds all the items of a collection to the tail of the Ringbuffer. A addAll is likely to outperform multiple calls
                 * to add(Object) due to better io utilization and a reduced number of executed operations. If the batch is empty,
                 * the call is ignored. When the collection is not empty, the content is copied into a different data-structure.
                 * This means that: after this call completes, the collection can be re-used. the collection doesn't need to be serializable.
                 * If the collection is larger than the capacity of the ringbuffer, then the items that were written first will be
                 * overwritten. Therefor this call will not block. The items are inserted in the order of the Iterator of the collection.
                 * If an addAll is executed concurrently with an add or addAll, no guarantee is given that items are contiguous.
                 * The result of the future contains the sequenceId of the last written item
                 */
                ClientMessage HAZELCAST_API ringbuffer_addall_encode(const std::string &name,
                                                                     const std::vector<serialization::pimpl::data> &value_list,
                                                                     int32_t overflow_policy);

                /**
                 * Reads a batch of items from the Ringbuffer. If the number of available items after the first read item is smaller
                 * than the maxCount, these items are returned. So it could be the number of items read is smaller than the maxCount.
                 * If there are less items available than minCount, then this call blacks. Reading a batch of items is likely to
                 * perform better because less overhead is involved. A filter can be provided to only select items that need to be read.
                 * If the filter is null, all items are read. If the filter is not null, only items where the filter function returns
                 * true are returned. Using filters is a good way to prevent getting items that are of no value to the receiver.
                 * This reduces the amount of IO and the number of operations being executed, and can result in a significant performance improvement.
                 */
                ClientMessage HAZELCAST_API
                ringbuffer_readmany_encode(const std::string &name, int64_t start_sequence, int32_t min_count,
                                           int32_t max_count, const serialization::pimpl::data *filter);

                /**
                 * Fetches a new batch of ids for the given flake id generator.
                 */
                ClientMessage HAZELCAST_API
                flakeidgenerator_newidbatch_encode(const std::string &name, int32_t batch_size);

                /**
                 * Query operation to retrieve the current value of the PNCounter.
                 * <p>
                 * The invocation will return the replica timestamps (vector clock) which
                 * can then be sent with the next invocation to keep session consistency
                 * guarantees.
                 * The target replica is determined by the {@code targetReplica} parameter.
                 * If smart routing is disabled, the actual member processing the client
                 * message may act as a proxy.
                 */
                ClientMessage HAZELCAST_API pncounter_get_encode(const std::string &name,
                                                                 const std::vector<std::pair<boost::uuids::uuid, int64_t>> &replica_timestamps,
                                                                 boost::uuids::uuid target_replica_uuid);

                /**
                 * Adds a delta to the PNCounter value. The delta may be negative for a
                 * subtraction.
                 * <p>
                 * The invocation will return the replica timestamps (vector clock) which
                 * can then be sent with the next invocation to keep session consistency
                 * guarantees.
                 * The target replica is determined by the {@code targetReplica} parameter.
                 * If smart routing is disabled, the actual member processing the client
                 * message may act as a proxy.
                 */
                ClientMessage HAZELCAST_API
                pncounter_add_encode(const std::string &name, int64_t delta, bool get_before_update,
                                     const std::vector<std::pair<boost::uuids::uuid, int64_t>> &replica_timestamps,
                                     boost::uuids::uuid target_replica_uuid);

                /**
                 * Returns the configured number of CRDT replicas for the PN counter with
                 * the given {@code name}.
                 * The actual replica count may be less, depending on the number of data
                 * members in the cluster (members that own data).
                 */
                ClientMessage HAZELCAST_API pncounter_getconfiguredreplicacount_encode(const std::string  & name);

                /**
                 * Creates a new CP group with the given name
                 */
                ClientMessage HAZELCAST_API cpgroup_createcpgroup_encode(const std::string &proxy_name);

                /**
                 * Destroys the distributed object with the given name on the requested
                 * CP group
                 */
                ClientMessage HAZELCAST_API
                cpgroup_destroycpobject_encode(const cp::raft_group_id &group_id, const std::string &service_name,
                                               const std::string &object_name);

                /**
                 * Creates a session for the caller on the given CP group.
                 */
                ClientMessage HAZELCAST_API
                cpsession_createsession_encode(const cp::raft_group_id &group_id, const std::string &endpoint_name);

                /**
                 * Closes the given session on the given CP group
                 */
                ClientMessage HAZELCAST_API
                cpsession_closesession_encode(const cp::raft_group_id &group_id, int64_t session_id);

                /**
                 * Commits a heartbeat for the given session on the given cP group and
                 * extends its session expiration time.
                 */
                ClientMessage HAZELCAST_API
                cpsession_heartbeatsession_encode(const cp::raft_group_id &group_id, int64_t session_id);

                /**
                 * Generates a new ID for the caller thread. The ID is unique in the given
                 * CP group.
                 */
                ClientMessage HAZELCAST_API cpsession_generatethreadid_encode(const cp::raft_group_id &group_id);

            }
        }
    }
}

