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

                NearCacheConfig(const std::string &cacheName);

                NearCacheConfig(const std::string &cacheName, InMemoryFormat memoryFormat);

                NearCacheConfig(int32_t timeToLiveSeconds, int32_t maxIdleSeconds, bool invalidateOnChange,
                                InMemoryFormat inMemoryFormat, const EvictionConfig &evictConfig);

                virtual ~NearCacheConfig() = default;

                /**
                 * Gets the name of the Near Cache.
                 *
                 * @return The name of the Near Cache.
                 */
                const std::string &getName() const;

                /**
                 * Sets the name of the Near Cache.
                 *
                 * @param name The name of the Near Cache.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setName(const std::string &name);

                /**
                 * Gets the maximum number of seconds for each entry to stay in the Near Cache. Entries that are
                 * older than time-to-live-seconds will get automatically evicted from the Near Cache.
                 *
                 * @return The maximum number of seconds for each entry to stay in the Near Cache.
                 */
                int32_t getTimeToLiveSeconds() const;

                /**
                 * Sets the maximum number of seconds for each entry to stay in the Near Cache. Entries that are
                 * older than time-to-live-seconds will get automatically evicted from the Near Cache.
                 * Any integer between 0 and INT32_MAX. 0 means infinite. Default is 0.
                 *
                 * @param timeToLiveSeconds The maximum number of seconds for each entry to stay in the Near Cache.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setTimeToLiveSeconds(int32_t timeToLiveSeconds);

                /**
                 * Maximum number of seconds each entry can stay in the Near Cache as untouched (not-read).
                 * Entries that are not read (touched) more than max-idle-seconds value will get removed
                 * from the Near Cache.
                 *
                 * @return Maximum number of seconds each entry can stay in the Near Cache as
                 * untouched (not-read).
                 */
                int32_t getMaxIdleSeconds() const;

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
                NearCacheConfig &setMaxIdleSeconds(int32_t maxIdleSeconds);

                /**
                 * True to evict the cached entries if the entries are changed (updated or removed).
                 *
                 * When true, the member listens for cluster-wide changes on the entries and invalidates
                 * them on change. Changes done on the local member always invalidate the cache.
                 *
                 * @return This Near Cache config instance.
                 */
                bool isInvalidateOnChange() const;

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
                NearCacheConfig &setInvalidateOnChange(bool invalidateOnChange);

                /**
                 * Gets the data type used to store entries.
                 * Possible values:
                 * BINARY (default): keys and values are stored as binary data.
                 * OBJECT: values are stored in their object forms.
                 *
                 * @return The data type used to store entries.
                 */
                const InMemoryFormat &getInMemoryFormat() const;

                /**
                 * Sets the data type used to store entries.
                 * Possible values:
                 * BINARY (default): keys and values are stored as binary data.
                 * OBJECT: values are stored in their object forms.
                 *
                 * @param inMemoryFormat The data type used to store entries.
                 * @return This Near Cache config instance.
                 */
                virtual NearCacheConfig &setInMemoryFormat(const InMemoryFormat &inMemoryFormat);

                /**
                 * If true, cache local entries also.
                 * This is useful when in-memory-format for Near Cache is different than the map's one.
                 *
                 * @return True if local entries are cached also.
                 */
                bool isCacheLocalEntries() const;

                /**
                 * True to cache local entries also.
                 * This is useful when in-memory-format for Near Cache is different than the map's one.
                 *
                 * @param cacheLocalEntries True to cache local entries also.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setCacheLocalEntries(bool cacheLocalEntries);

                const LocalUpdatePolicy &getLocalUpdatePolicy() const;

                NearCacheConfig &setLocalUpdatePolicy(const LocalUpdatePolicy &localUpdatePolicy);

                /**
                 * The eviction configuration.
                 *
                 * @return The eviction configuration.
                 */
                EvictionConfig &getEvictionConfig();

                /**
                 * Sets the eviction configuration.
                 *
                 * @param evictionConfig The eviction configuration.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setEvictionConfig(const EvictionConfig &evictionConfig);

                friend std::ostream HAZELCAST_API &operator<<(std::ostream &out, const NearCacheConfig &cacheConfig);
            private:
                std::string name_;

                int32_t timeToLiveSeconds_;
                int32_t maxIdleSeconds_;

                InMemoryFormat inMemoryFormat_;

                LocalUpdatePolicy localUpdatePolicy_;

                bool invalidateOnChange_;
                bool cacheLocalEntries_;

                /**
                 * Default value of eviction config is
                 * <ul>
                 * <li>ENTRY_COUNT as max size policy</li>
                 * <li>INT32_MAX as maximum size</li>
                 * <li>LRU as eviction policy</li>
                 * </ul>
                 */
                EvictionConfig evictionConfig_;

                int32_t calculateMaxSize(int32_t maxSize);
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


