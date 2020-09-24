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

#include <string>
#include <sstream>
#include <stdint.h>
#include <memory>

#include "hazelcast/client/config/InMemoryFormat.h"
#include "hazelcast/client/config/EvictionConfig.h"
#include "hazelcast/client/config/NearCacheConfigBase.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

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
            template<typename K = serialization::pimpl::Data, typename V = serialization::pimpl::Data>
            class NearCacheConfig : public NearCacheConfigBase {
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

                NearCacheConfig() : name_("default"), time_to_live_seconds_(DEFAULT_TTL_SECONDS),
                                    max_idle_seconds_(DEFAULT_MAX_IDLE_SECONDS),
                                    in_memory_format_(DEFAULT_MEMORY_FORMAT),
                                    local_update_policy_(INVALIDATE), invalidate_on_change_(true), cache_local_entries_(false),
                                    eviction_config_(new EvictionConfig<K, V>()) {
                }

                NearCacheConfig(const std::string &cacheName) : name_(cacheName), time_to_live_seconds_(DEFAULT_TTL_SECONDS),
                                                         max_idle_seconds_(DEFAULT_MAX_IDLE_SECONDS),
                                                         in_memory_format_(DEFAULT_MEMORY_FORMAT),
                                                         local_update_policy_(INVALIDATE), invalidate_on_change_(true),
                                                         cache_local_entries_(false),
                                                         eviction_config_(new EvictionConfig<K, V>()) {
                }

                NearCacheConfig(const std::string &cacheName, InMemoryFormat memoryFormat) : name_(cacheName), time_to_live_seconds_(DEFAULT_TTL_SECONDS),
                                                         max_idle_seconds_(DEFAULT_MAX_IDLE_SECONDS),
                                                         in_memory_format_(memoryFormat),
                                                         local_update_policy_(INVALIDATE), invalidate_on_change_(true),
                                                         cache_local_entries_(false),
                                                         eviction_config_(new EvictionConfig<K, V>()) {
                }

                NearCacheConfig(int32_t timeToLiveSeconds, int32_t maxIdleSeconds, bool invalidateOnChange,
                                InMemoryFormat inMemoryFormat, std::shared_ptr<EvictionConfig<K, V> > evictConfig)
                        : eviction_config_(new EvictionConfig<K, V>()) {
                    this->timeToLiveSeconds = timeToLiveSeconds;
                    this->maxIdleSeconds = maxIdleSeconds;
                    this->invalidateOnChange = invalidateOnChange;
                    this->inMemoryFormat = inMemoryFormat;
                    local_update_policy_= INVALIDATE;
                    // EvictionConfig is not allowed to be NULL
                    if (evictConfig.get() != NULL) {
                        this->evictionConfig = evictConfig;
                    }
                    this->cacheLocalEntries = false;
                }

                NearCacheConfig(const NearCacheConfig<K, V> &config) {
                    name_ = config.getName();
                    in_memory_format_ = config.getInMemoryFormat();
                    invalidate_on_change_ = config.isInvalidateOnChange();
                    max_idle_seconds_ = config.getMaxIdleSeconds();
                    time_to_live_seconds_ = config.getTimeToLiveSeconds();
                    cache_local_entries_ = config.isCacheLocalEntries();
                    local_update_policy_ = config.local_update_policy_;
                    // EvictionConfig is not allowed to be NULL
                    if (config.eviction_config_.get() != NULL) {
                        this->eviction_config_ = config.eviction_config_;
                    }
                }

                virtual ~NearCacheConfig() = default;

                /**
                 * Gets the name of the Near Cache.
                 *
                 * @return The name of the Near Cache.
                 */
                const std::string &getName() const {
                    return name_;
                }

                /**
                 * Sets the name of the Near Cache.
                 *
                 * @param name The name of the Near Cache.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setName(const std::string &name) {
                    this->name_ = name;
                    return *this;
                }

                /**
                 * Gets the maximum number of seconds for each entry to stay in the Near Cache. Entries that are
                 * older than time-to-live-seconds will get automatically evicted from the Near Cache.
                 *
                 * @return The maximum number of seconds for each entry to stay in the Near Cache.
                 */
                int32_t getTimeToLiveSeconds() const {
                    return time_to_live_seconds_;
                }

                /**
                 * Sets the maximum number of seconds for each entry to stay in the Near Cache. Entries that are
                 * older than time-to-live-seconds will get automatically evicted from the Near Cache.
                 * Any integer between 0 and INT32_MAX. 0 means infinite. Default is 0.
                 *
                 * @param timeToLiveSeconds The maximum number of seconds for each entry to stay in the Near Cache.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setTimeToLiveSeconds(int32_t timeToLiveSeconds) {
                    this->time_to_live_seconds_ = util::Preconditions::checkNotNegative(timeToLiveSeconds,
                                                                                    "TTL seconds cannot be negative!");
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
                int32_t getMaxIdleSeconds() const {
                    return max_idle_seconds_;
                }

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
                NearCacheConfig &setMaxIdleSeconds(int32_t maxIdleSeconds) {
                    this->max_idle_seconds_ = util::Preconditions::checkNotNegative(maxIdleSeconds,
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
                bool isInvalidateOnChange() const {
                    return invalidate_on_change_;
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
                NearCacheConfig &setInvalidateOnChange(bool invalidateOnChange) {
                    this->invalidate_on_change_ = invalidateOnChange;
                    return *this;
                }

                /**
                 * Gets the data type used to store entries.
                 * Possible values:
                 * BINARY (default): keys and values are stored as binary data.
                 * OBJECT: values are stored in their object forms.
                 *
                 * @return The data type used to store entries.
                 */
                const InMemoryFormat &getInMemoryFormat() const {
                    return in_memory_format_;
                }

                /**
                 * Sets the data type used to store entries.
                 * Possible values:
                 * BINARY (default): keys and values are stored as binary data.
                 * OBJECT: values are stored in their object forms.
                 *
                 * @param inMemoryFormat The data type used to store entries.
                 * @return This Near Cache config instance.
                 */
                virtual NearCacheConfig &setInMemoryFormat(const InMemoryFormat &inMemoryFormat) {
                    this->in_memory_format_ = inMemoryFormat;
                    return *this;
                }

                /**
                 * If true, cache local entries also.
                 * This is useful when in-memory-format for Near Cache is different than the map's one.
                 *
                 * @return True if local entries are cached also.
                 */
                bool isCacheLocalEntries() const {
                    return cache_local_entries_;
                }

                /**
                 * True to cache local entries also.
                 * This is useful when in-memory-format for Near Cache is different than the map's one.
                 *
                 * @param cacheLocalEntries True to cache local entries also.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setCacheLocalEntries(bool cacheLocalEntries) {
                    this->cacheLocalEntries = cacheLocalEntries;
                    return *this;
                }

                const LocalUpdatePolicy &getLocalUpdatePolicy() const {
                    return local_update_policy_;
                }

                NearCacheConfig &setLocalUpdatePolicy(const LocalUpdatePolicy &localUpdatePolicy) {
                    this->localUpdatePolicy = localUpdatePolicy;
                    return *this;
                }

                /**
                 * The eviction configuration.
                 *
                 * @return The eviction configuration.
                 */
                const std::shared_ptr<EvictionConfig<K, V> > &getEvictionConfig() const {
                    return eviction_config_;
                }

                /**
                 * Sets the eviction configuration.
                 *
                 * @param evictionConfig The eviction configuration.
                 * @return This Near Cache config instance.
                 */
                NearCacheConfig &setEvictionConfig(const std::shared_ptr<EvictionConfig<K, V> > &evictionConfig) {
                    this->eviction_config_ = util::Preconditions::checkNotNull<EvictionConfig<K, V> >(evictionConfig,
                                                                                                    "EvictionConfig cannot be NULL!");
                    return *this;
                }

                std::ostream &operator<<(std::ostream &out) {
                    out << "NearCacheConfig{"
                        << "timeToLiveSeconds=" << time_to_live_seconds_
                        << ", maxIdleSeconds=" << max_idle_seconds_
                        << ", invalidateOnChange=" << invalidate_on_change_
                        << ", inMemoryFormat=" << in_memory_format_
                        << ", cacheLocalEntries=" << cache_local_entries_
                        << ", localUpdatePolicy=" << local_update_policy_
                        << *eviction_config_;
                    out << '}';

                    return out;
                }
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
                std::shared_ptr<EvictionConfig<K, V> > eviction_config_;

                int32_t calculateMaxSize(int32_t maxSize) {
                    return (maxSize == 0) ? INT32_MAX : util::Preconditions::checkNotNegative(maxSize,
                                                                                              "Max-size cannot be negative!");
                }
            };

            template<typename K, typename V>
            const int32_t NearCacheConfig<K, V>::DEFAULT_TTL_SECONDS = 0;

            template<typename K, typename V>
            const int32_t NearCacheConfig<K, V>::DEFAULT_MAX_IDLE_SECONDS = 0;

            template<typename K, typename V>
            const InMemoryFormat NearCacheConfig<K, V>::DEFAULT_MEMORY_FORMAT = BINARY;

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


