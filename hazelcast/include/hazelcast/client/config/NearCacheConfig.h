/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_CONFIG_NEARCACHECONFIG_H_
#define HAZELCAST_CLIENT_CONFIG_NEARCACHECONFIG_H_

#include <string>
#include <sstream>
#include <stdint.h>

#include <boost/shared_ptr.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/config/InMemoryFormat.h"
#include "hazelcast/client/config/EvictionConfig.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            class EvictionConfig;
            class NearCachePreloaderConfig;

            /**
             * Contains the configuration for a Near Cache.
             * @BinaryInterface
             */
            class HAZELCAST_API NearCacheConfig {
            public:
                /**
                 * Default value of the time to live in seconds.
                 */
                static const int32_t DEFAULT_TTL_SECONDS;

                /**
                 * Default value of the idle time for eviction in seconds.
                 */
                static const int32_t DEFAULT_MAX_IDLE_SECONDS;

                /**
                 * Default value for the in-memory format.
                 */
                static const InMemoryFormat DEFAULT_MEMORY_FORMAT;

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

                NearCacheConfig(const char *cacheName);

/*
            NearCacheConfig(int timeToLiveSeconds, int maxIdleSeconds, bool invalidateOnChange,
                                   InMemoryFormat inMemoryFormat);
*/

                NearCacheConfig(int32_t timeToLiveSeconds, int32_t maxIdleSeconds, bool invalidateOnChange,
                                InMemoryFormat inMemoryFormat, boost::shared_ptr<EvictionConfig> evictionConfig);

                NearCacheConfig(const NearCacheConfig &config);

/*
            public NearCacheConfigReadOnly getAsReadOnly() {
                    if (readOnly == NULL) {
                        readOnly = new NearCacheConfigReadOnly(this);
                    }
                    return readOnly;
                }
*/

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
                 * Any integer between 0 and Integer.MAX_VALUE. 0 means infinite. Default is 0.
                 *
                 * @param timeToLiveSeconds The maximum number of seconds for each entry to stay in the Near Cache.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setTimeToLiveSeconds(int32_t timeToLiveSeconds);
                /**
                 * Sets the maximum size of the Near Cache. When max size is reached,
                 * cache is evicted based on the policy defined.
                 * Any integer between 0 and Integer.MAX_VALUE. 0 means
                 * INT32_MAX. Default is 0.
                 *
                 * @param maxSize The maximum number of seconds for each entry to stay in the Near Cache.
                 * @return This Near Cache config instance.
                 * @deprecated since 3.8, use {@link #setEvictionConfig(EvictionConfig)} and {@link EvictionConfig#setSize(int)} instead
                 */
                NearCacheConfig &setMaxSize(int32_t maxSize);

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
                 * Any integer between 0 and Integer.MAX_VALUE. 0 means Integer.MAX_VALUE. Default is 0.
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
                 * NATIVE: keys and values are stored in native memory.
                 *
                 * @return The data type used to store entries.
                 */
                const InMemoryFormat &getInMemoryFormat() const;
                /**
                 * Sets the data type used to store entries.
                 * Possible values:
                 * BINARY (default): keys and values are stored as binary data.
                 * OBJECT: values are stored in their object forms.
                 * NATIVE: keys and values are stored in native memory.
                 *
                 * @param inMemoryFormat The data type used to store entries.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setInMemoryFormat(const InMemoryFormat &inMemoryFormat);

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
                const boost::shared_ptr<EvictionConfig> &getEvictionConfig() const;

                /**
                 * Sets the eviction configuration.
                 *
                 * @param evictionConfig The eviction configuration.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setEvictionConfig(const boost::shared_ptr<EvictionConfig> &evictionConfig);

                const boost::shared_ptr<NearCachePreloaderConfig> &getPreloaderConfig() const;

                NearCacheConfig &setPreloaderConfig(const boost::shared_ptr<NearCachePreloaderConfig> &preloaderConfig);
            private:
                std::string name;

                int32_t timeToLiveSeconds;
                int32_t maxIdleSeconds;

                int32_t maxSize;
                EvictionPolicy evictionPolicy;

                InMemoryFormat inMemoryFormat;

                LocalUpdatePolicy localUpdatePolicy;

                bool invalidateOnChange;
                bool cacheLocalEntries;

/*
            NearCacheConfigReadOnly readOnly;
*/

                /**
                 * Default value of eviction config is
                 * <ul>
                 * <li>ENTRY_COUNT as max size policy</li>
                 * <li>10000 as maximum size</li>
                 * <li>LRU as eviction policy</li>
                 * </ul>
                 */
                boost::shared_ptr<EvictionConfig> evictionConfig;

                boost::shared_ptr<NearCachePreloaderConfig> preloaderConfig;

                int32_t calculateMaxSize(int32_t maxSize);
            };

            std::ostream HAZELCAST_API &operator<<(std::ostream &out, const NearCacheConfig &config);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CONFIG_NEARCACHECONFIG_H_ */
