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

#include "hazelcast/client/config/InMemoryFormat.h"
#include "hazelcast/client/config/EvictionConfig.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            /**
             * Contains the configuration for a Near Cache.
             * @BinaryInterface
             */
            class HAZELCAST_API NearCacheConfig {
            public:
                /**
                 * Default value of the time to live in seconds.
                 */
                static constexpr int32_t DEFAULT_TTL_SECONDS = 0;

                /**
                 * Default value of the idle time for eviction in seconds.
                 */
                static constexpr int32_t DEFAULT_MAX_IDLE_SECONDS = 0;

                /**
                 * Default value for the in-memory format.
                 */
                static constexpr InMemoryFormat DEFAULT_MEMORY_FORMAT = InMemoryFormat::BINARY;

                /**
                 * Local Update Policy enum.
                 */
                enum LocalUpdatePolicy {
                    /**
                     * INVALIDATE POLICY
                     */
                            INVALIDATE,

                    /**
                     * CACHE ON UPDATE POLICY
                     */
                            CACHE
                };

                NearCacheConfig();

                NearCacheConfig(const std::string &cache_name);

                NearCacheConfig(const std::string &cache_name, InMemoryFormat memory_format);

                NearCacheConfig(int32_t time_to_live_seconds, int32_t max_idle_seconds, bool invalidate_on_change,
                                InMemoryFormat in_memory_format, const EvictionConfig &evict_config);

                virtual ~NearCacheConfig() = default;

                /**
                 * Gets the name of the Near Cache.
                 *
                 * @return The name of the Near Cache.
                 */
                const std::string &get_name() const;

                /**
                 * Sets the name of the Near Cache.
                 *
                 * @param name The name of the Near Cache.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &set_name(const std::string &name);

                /**
                 * Gets the maximum number of seconds for each entry to stay in the Near Cache. Entries that are
                 * older than time-to-live-seconds will get automatically evicted from the Near Cache.
                 *
                 * @return The maximum number of seconds for each entry to stay in the Near Cache.
                 */
                int32_t get_time_to_live_seconds() const;

                /**
                 * Sets the maximum number of seconds for each entry to stay in the Near Cache. Entries that are
                 * older than time-to-live-seconds will get automatically evicted from the Near Cache.
                 * Any integer between 0 and INT32_MAX. 0 means infinite. Default is 0.
                 *
                 * @param timeToLiveSeconds The maximum number of seconds for each entry to stay in the Near Cache.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &set_time_to_live_seconds(int32_t time_to_live_seconds);

                /**
                 * Maximum number of seconds each entry can stay in the Near Cache as untouched (not-read).
                 * Entries that are not read (touched) more than max-idle-seconds value will get removed
                 * from the Near Cache.
                 *
                 * @return Maximum number of seconds each entry can stay in the Near Cache as
                 * untouched (not-read).
                 */
                int32_t get_max_idle_seconds() const;

                /**
                 * Maximum number of seconds each entry can stay in the Near Cache as untouched (not-read).
                 * Entries that are not read (touched) more than max-idle-seconds value will get removed
                 * from the Near Cache.
                 * Any integer between 0 and Integer.MAX_VALUE. 0 means INT32_MAX. Default is 0.
                 *
                 * @param maxIdleSeconds Maximum number of seconds each entry can stay in the Near Cache as
                 *                       untouched (not-read).
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &set_max_idle_seconds(int32_t max_idle_seconds);

                /**
                 * True to evict the cached entries if the entries are changed (updated or removed).
                 *
                 * When true, the member listens for cluster-wide changes on the entries and invalidates
                 * them on change. Changes done on the local member always invalidate the cache.
                 *
                 * @return This Near Cache config instance.
                 */
                bool is_invalidate_on_change() const;

                /**
                 * True to evict the cached entries if the entries are changed (updated or removed).
                 *
                 * If set to true, the member will listen for cluster-wide changes on the entries and invalidate
                 * them on change. Changes done on the local member always invalidate the cache.
                 *
                 * @param invalidateOnChange True to evict the cached entries if the entries are
                 *                           changed (updated or removed), false otherwise.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &set_invalidate_on_change(bool invalidate_on_change);

                /**
                 * Gets the data type used to store entries.
                 * Possible values:
                 * BINARY (default): keys and values are stored as binary data.
                 * OBJECT: values are stored in their object forms.
                 *
                 * @return The data type used to store entries.
                 */
                const InMemoryFormat &get_in_memory_format() const;

                /**
                 * Sets the data type used to store entries.
                 * Possible values:
                 * BINARY (default): keys and values are stored as binary data.
                 * OBJECT: values are stored in their object forms.
                 *
                 * @param inMemoryFormat The data type used to store entries.
                 * @return This Near Cache config instance.
                 */
                virtual NearCacheConfig &set_in_memory_format(const InMemoryFormat &in_memory_format);

                /**
                 * If true, cache local entries also.
                 * This is useful when in-memory-format for Near Cache is different than the map's one.
                 *
                 * @return True if local entries are cached also.
                 */
                bool is_cache_local_entries() const;

                /**
                 * True to cache local entries also.
                 * This is useful when in-memory-format for Near Cache is different than the map's one.
                 *
                 * @param cacheLocalEntries True to cache local entries also.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &set_cache_local_entries(bool cache_local_entries);

                const LocalUpdatePolicy &get_local_update_policy() const;

                NearCacheConfig &set_local_update_policy(const LocalUpdatePolicy &local_update_policy);

                /**
                 * The eviction configuration.
                 *
                 * @return The eviction configuration.
                 */
                EvictionConfig &get_eviction_config();

                /**
                 * Sets the eviction configuration.
                 *
                 * @param evictionConfig The eviction configuration.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &set_eviction_config(const EvictionConfig &eviction_config);

                friend std::ostream HAZELCAST_API &operator<<(std::ostream &out, const NearCacheConfig &cache_config);
            private:
                std::string name_;

                int32_t time_to_live_seconds_;
                int32_t max_idle_seconds_;

                InMemoryFormat in_memory_format_;

                LocalUpdatePolicy local_update_policy_;

                bool invalidate_on_change_;
                bool cache_local_entries_;

                /**
                 * Default value of eviction config is
                 * <ul>
                 * <li>ENTRY_COUNT as max size policy</li>
                 * <li>INT32_MAX as maximum size</li>
                 * <li>LRU as eviction policy</li>
                 * </ul>
                 */
                EvictionConfig eviction_config_;

                int32_t calculate_max_size(int32_t max_size);
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


