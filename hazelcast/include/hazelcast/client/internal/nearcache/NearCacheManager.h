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

#include <string>
#include <vector>

#include "hazelcast/util/export.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/nearcache/impl/DefaultNearCache.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/logger.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace nearcache {
/**
 * {@link NearCacheManager} is the contract point to manage all existing {@link
 * NearCache} instances.
 */
class HAZELCAST_API NearCacheManager
{
public:
    NearCacheManager(
      const std::shared_ptr<spi::impl::ClientExecutionServiceImpl>& es,
      serialization::pimpl::SerializationService& ss,
      logger& lg);

    /**
     * Gets the {@link NearCache} instance associated with given {@code name}.
     *
     * @param name the name of the {@link NearCache} instance will be got
     * @param <K>  the type of the key for Near Cache
     * @param <V>  the type of the value for Near Cache
     * @return the {@link NearCache} instance
     * associated with given {@code name}
     */
    template<typename K, typename V, typename KS>
    std::shared_ptr<NearCache<KS, V>> get_near_cache(const std::string& name)
    {
        return std::static_pointer_cast<NearCache<KS, V>>(
          near_cache_map_.get(name));
    }

    /**
     * Creates a new {@link NearCache} with given configurations or returns
     * existing one.
     *
     *
     * @param name                 the name of the {@link NearCache}
     *                             to be created or existing one
     * @param nearCacheConfig      the {@link NearCacheConfig} of the {@link
     * NearCache} to be created
     * @param <K>           the key type of the {@link NearCache}
     * @param <KS>          the key type of the underlying cache store (this is
     * usually Data)
     * @param <V>                  the value type of the {@link NearCache}
     * @return the created or existing {@link NearCache} instance associated
     * with given {@code name}
     */
    template<typename K, typename V, typename KS>
    std::shared_ptr<NearCache<KS, V>> get_or_create_near_cache(
      const std::string& name,
      const client::config::near_cache_config& near_cache_config)
    {
        std::shared_ptr<BaseNearCache> nearCache = near_cache_map_.get(name);
        if (nullptr == nearCache.get()) {
            {
                std::lock_guard<std::mutex> guard(mutex_);
                nearCache = near_cache_map_.get(name);
                if (nullptr == nearCache.get()) {
                    nearCache =
                      create_near_cache<K, V, KS>(name, near_cache_config);
                    nearCache->initialize();

                    near_cache_map_.put(name, nearCache);
                }
            }
        }
        return std::static_pointer_cast<NearCache<KS, V>>(nearCache);
    }

    /**
     * Clears {@link NearCache} instance associated with given {@code name} but
     * not removes it.
     *
     * @param name name of the {@link NearCache} to be cleared
     * @return {@code true} if {@link NearCache} was found and cleared, {@code
     * false} otherwise
     */
    bool clear_near_cache(const std::string& name);

    /**
     * Clears all defined {@link NearCache} instances.
     */
    void clear_all_near_caches();

    /**
     * Destroys {@link NearCache} instance associated with given {@code name}
     * and also removes it.
     *
     * @param name name of the {@link NearCache} to be destroyed
     * @return {@code true} if {@link NearCache} was found and destroyed, {@code
     * false} otherwise
     */
    bool destroy_near_cache(const std::string& name);

    /**
     * Destroys all defined {@link NearCache} instances.
     */
    void destroy_all_near_caches();

    /**
     * Lists all existing {@link NearCache} instances.
     *
     * @return all existing {@link NearCache} instances
     */
    std::vector<std::shared_ptr<BaseNearCache>> list_all_near_caches();

protected:
    template<typename K, typename V, typename KS>
    std::unique_ptr<NearCache<KS, V>> create_near_cache(
      const std::string& name,
      const client::config::near_cache_config& near_cache_config)
    {
        return std::unique_ptr<NearCache<KS, V>>(
          new impl::DefaultNearCache<K, V, KS>(name,
                                               near_cache_config,
                                               execution_service_,
                                               serialization_service_,
                                               logger_));
    }

private:
    std::shared_ptr<spi::impl::ClientExecutionServiceImpl> execution_service_;
    serialization::pimpl::SerializationService& serialization_service_;
    logger& logger_;
    util::SynchronizedMap<std::string, BaseNearCache> near_cache_map_;
    std::mutex mutex_;
};
} // namespace nearcache
} // namespace internal
} // namespace client
}; // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
