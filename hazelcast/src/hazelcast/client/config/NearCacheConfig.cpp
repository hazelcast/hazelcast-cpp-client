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
//
// Created by ihsan demir on 22 Nov 2016.

#include "hazelcast/client/config/NearCacheConfig.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/config/InMemoryFormat.h"
#include "hazelcast/client/config/NearCachePreloaderConfig.h"

namespace hazelcast {
    namespace client {
        namespace config {
            const int32_t NearCacheConfig::DEFAULT_TTL_SECONDS = 0;

            const int32_t NearCacheConfig::DEFAULT_MAX_IDLE_SECONDS = 0;

            const InMemoryFormat NearCacheConfig::DEFAULT_MEMORY_FORMAT = BINARY;

/*
            const int32_t NearCacheConfig::DEFAULT_MAX_SIZE = EvictionConfig::DEFAULT_MAX_ENTRY_COUNT_FOR_ON_HEAP_MAP;
*/

/*
            const int32_t NearCacheConfig::DEFAULT_EVICTION_POLICY = EvictionConfig::DEFAULT_EVICTION_POLICY;
*/
            NearCacheConfig::NearCacheConfig() : name("default"), timeToLiveSeconds(DEFAULT_TTL_SECONDS),
                                maxIdleSeconds(DEFAULT_MAX_IDLE_SECONDS),
                                maxSize(EvictionConfig::DEFAULT_MAX_ENTRY_COUNT_FOR_ON_HEAP_MAP),
                                evictionPolicy(EvictionConfig::DEFAULT_EVICTION_POLICY),
                                inMemoryFormat(DEFAULT_MEMORY_FORMAT),
                                localUpdatePolicy(INVALIDATE), invalidateOnChange(true) {
                (void) localUpdatePolicy;
            }

            NearCacheConfig::NearCacheConfig(const char *cacheName) : name(cacheName), timeToLiveSeconds(DEFAULT_TTL_SECONDS),
                                                     maxIdleSeconds(DEFAULT_MAX_IDLE_SECONDS),
                                                     inMemoryFormat(DEFAULT_MEMORY_FORMAT),
                                                     localUpdatePolicy(INVALIDATE), invalidateOnChange(true) {
            }

/*
            NearCacheConfig::NearCacheConfig(int32_t timeToLiveSeconds, int32_t maxIdleSeconds, bool invalidateOnChange,
                                   InMemoryFormat inMemoryFormat) {
                    this(timeToLiveSeconds, maxIdleSeconds, invalidateOnChange, inMemoryFormat, NULL);
                }
*/

            NearCacheConfig::NearCacheConfig(int32_t timeToLiveSeconds, int32_t maxIdleSeconds, bool invalidateOnChange,
                            InMemoryFormat inMemoryFormat, boost::shared_ptr<EvictionConfig> evictionConfig) {
                this->timeToLiveSeconds = timeToLiveSeconds;
                this->maxSize = calculateMaxSize(maxSize);
                this->maxIdleSeconds = maxIdleSeconds;
                this->invalidateOnChange = invalidateOnChange;
                this->inMemoryFormat = inMemoryFormat;
                // EvictionConfig is not allowed to be NULL
                if (evictionConfig.get() != NULL) {
                    this->evictionConfig = evictionConfig;
                    this->evictionPolicy = evictionConfig->getEvictionPolicy();
                    this->maxSize = evictionConfig->getSize();
                }
            }

            NearCacheConfig::NearCacheConfig(const NearCacheConfig &config) {
                name = config.getName();
                inMemoryFormat = config.getInMemoryFormat();
                invalidateOnChange = config.isInvalidateOnChange();
                maxIdleSeconds = config.getMaxIdleSeconds();
                timeToLiveSeconds = config.getTimeToLiveSeconds();
                cacheLocalEntries = config.isCacheLocalEntries();
                localUpdatePolicy = config.localUpdatePolicy;
                // EvictionConfig is not allowed to be NULL
                if (config.evictionConfig.get() != NULL) {
                    this->evictionConfig = config.evictionConfig;
                    evictionPolicy = this->evictionConfig->getEvictionPolicy();
                }
                // NearCachePreloaderConfig is not allowed to be NULL
                if (config.preloaderConfig.get() != NULL) {
                    this->preloaderConfig = config.preloaderConfig;
                }
            }

/*
            NearCacheConfig::NearCacheConfigReadOnly getAsReadOnly() {
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
            const std::string &NearCacheConfig::getName() const {
                return name;
            }

            /**
             * Sets the name of the Near Cache.
             *
             * @param name The name of the Near Cache.
             * @return This Near Cache config instance.
             */
            NearCacheConfig &NearCacheConfig::setName(const std::string &name) {
                this->name = name;
                return *this;
            }

            /**
             * Gets the maximum number of seconds for each entry to stay in the Near Cache. Entries that are
             * older than time-to-live-seconds will get automatically evicted from the Near Cache.
             *
             * @return The maximum number of seconds for each entry to stay in the Near Cache.
             */
            int32_t NearCacheConfig::getTimeToLiveSeconds() const {
                return timeToLiveSeconds;
            }

            /**
             * Sets the maximum number of seconds for each entry to stay in the Near Cache. Entries that are
             * older than time-to-live-seconds will get automatically evicted from the Near Cache.
             * Any integer between 0 and Integer.MAX_VALUE. 0 means infinite. Default is 0.
             *
             * @param timeToLiveSeconds The maximum number of seconds for each entry to stay in the Near Cache.
             * @return This Near Cache config instance.
             */
            NearCacheConfig &NearCacheConfig::setTimeToLiveSeconds(int32_t timeToLiveSeconds) {
                this->timeToLiveSeconds = util::Preconditions::checkNotNegative(timeToLiveSeconds,
                                                                                "TTL seconds cannot be negative!");
                return *this;
            }

            /**
             * Sets the maximum size of the Near Cache. When max size is reached,
             * cache is evicted based on the policy defined.
             * Any integer between 0 and Integer.MAX_VALUE. 0 means
             * Integer.MAX_VALUE. Default is 0.
             *
             * @param maxSize The maximum number of seconds for each entry to stay in the Near Cache.
             * @return This Near Cache config instance.
             * @deprecated since 3.8, use {@link #setEvictionConfig(EvictionConfig)} and {@link EvictionConfig#setSize(int)} instead
             */
            NearCacheConfig &NearCacheConfig::setMaxSize(int32_t maxSize) {
                this->maxSize = calculateMaxSize(maxSize);
                this->evictionConfig->setSize(this->maxSize);
                this->evictionConfig->setMaximumSizePolicy(EvictionConfig::ENTRY_COUNT);
                return *this;
            }

            /**
             * Maximum number of seconds each entry can stay in the Near Cache as untouched (not-read).
             * Entries that are not read (touched) more than max-idle-seconds value will get removed
             * from the Near Cache.
             *
             * @return Maximum number of seconds each entry can stay in the Near Cache as
             * untouched (not-read).
             */
            int32_t NearCacheConfig::getMaxIdleSeconds() const {
                return maxIdleSeconds;
            }

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
            NearCacheConfig &NearCacheConfig::setMaxIdleSeconds(int32_t maxIdleSeconds) {
                this->maxIdleSeconds = util::Preconditions::checkNotNegative(maxIdleSeconds,
                                                                             "Max-Idle seconds cannot be negative!");
                return *this;
            }

            /**
             * True to evict the cached entries if the entries are changed (updated or removed).
             *
             * When true, the member listens for cluster-wide changes on the entries and invalidates
             * them on change. Changes done on the local member always invalidate the cache.
             *
             * @return This Near Cache config instance.
             */
            bool NearCacheConfig::isInvalidateOnChange() const {
                return invalidateOnChange;
            }

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
            NearCacheConfig &NearCacheConfig::setInvalidateOnChange(bool invalidateOnChange) {
                this->invalidateOnChange = invalidateOnChange;
                return *this;
            }

            /**
             * Gets the data type used to store entries.
             * Possible values:
             * BINARY (default): keys and values are stored as binary data.
             * OBJECT: values are stored in their object forms.
             * NATIVE: keys and values are stored in native memory.
             *
             * @return The data type used to store entries.
             */
            const InMemoryFormat &NearCacheConfig::getInMemoryFormat() const {
                return inMemoryFormat;
            }

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
            NearCacheConfig &NearCacheConfig::setInMemoryFormat(const InMemoryFormat &inMemoryFormat) {
                this->inMemoryFormat = inMemoryFormat;
                return *this;
            }

            /**
             * If true, cache local entries also.
             * This is useful when in-memory-format for Near Cache is different than the map's one.
             *
             * @return True if local entries are cached also.
             */
            bool NearCacheConfig::isCacheLocalEntries() const {
                return cacheLocalEntries;
            }

            /**
             * True to cache local entries also.
             * This is useful when in-memory-format for Near Cache is different than the map's one.
             *
             * @param cacheLocalEntries True to cache local entries also.
             * @return This Near Cache config instance.
             */
            NearCacheConfig &NearCacheConfig::setCacheLocalEntries(bool cacheLocalEntries) {
                this->cacheLocalEntries = cacheLocalEntries;
                return *this;
            }

            const NearCacheConfig::LocalUpdatePolicy &NearCacheConfig::getLocalUpdatePolicy() const {
                return localUpdatePolicy;
            }

            NearCacheConfig &NearCacheConfig::setLocalUpdatePolicy(const NearCacheConfig::LocalUpdatePolicy &localUpdatePolicy) {
                this->localUpdatePolicy = localUpdatePolicy;
                return *this;
            }

            /**
             * The eviction configuration.
             *
             * @return The eviction configuration.
             */
            const boost::shared_ptr<EvictionConfig> &NearCacheConfig::getEvictionConfig() const {
                return evictionConfig;
            }

            /**
             * Sets the eviction configuration.
             *
             * @param evictionConfig The eviction configuration.
             * @return This Near Cache config instance.
             */
            NearCacheConfig &NearCacheConfig::setEvictionConfig(const boost::shared_ptr<EvictionConfig> &evictionConfig) {
                this->evictionConfig = util::Preconditions::checkNotNull<EvictionConfig>(evictionConfig,
                                                                                         "EvictionConfig cannot be NULL!");
                return *this;
            }

            const boost::shared_ptr<NearCachePreloaderConfig> &NearCacheConfig::getPreloaderConfig() const {
                return preloaderConfig;
            }

            NearCacheConfig &NearCacheConfig::setPreloaderConfig(const boost::shared_ptr<NearCachePreloaderConfig> &preloaderConfig) {
                this->preloaderConfig = util::Preconditions::checkNotNull<NearCachePreloaderConfig>(preloaderConfig,
                                                                                                    "NearCachePreloaderConfig cannot be NULL!");
                return *this;
            }

            int32_t NearCacheConfig::calculateMaxSize(int32_t maxSize) {
                return (maxSize == 0) ? INT32_MAX : util::Preconditions::checkNotNegative(maxSize,
                                                                                          "Max-size cannot be negative!");
            }

            std::ostream &operator<<(std::ostream &out, const NearCacheConfig &config) {
                out << "NearCacheConfig{"
                    << "timeToLiveSeconds=" << config.getTimeToLiveSeconds();
                const boost::shared_ptr<EvictionConfig> &evictionConfig = config.getEvictionConfig();
                
                out << ", maxIdleSeconds=" << config.getMaxIdleSeconds()
                    << ", invalidateOnChange=" << config.isInvalidateOnChange()
                    << ", inMemoryFormat=" << config.getInMemoryFormat()
                    << ", cacheLocalEntries=" << config.isCacheLocalEntries()
                    << ", localUpdatePolicy=" << config.getLocalUpdatePolicy();
                if (NULL != evictionConfig.get()) {
                    out << *evictionConfig;
                }

                const boost::shared_ptr<NearCachePreloaderConfig> &preloaderConfig = config.getPreloaderConfig();
                if (NULL != preloaderConfig.get()) {
                    out << *preloaderConfig;
                }

                out << '}';

                return out;
            }
        }
    }
}
