/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHEMANAGER_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHEMANAGER_H_

#include <string>
#include <vector>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/nearcache/impl/DefaultNearCache.h"
#include "hazelcast/client/internal/adapter/DataStructureAdapter.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                /**
                 * {@link NearCacheManager} is the contract point to manage all existing {@link NearCache} instances.
                 */
                class HAZELCAST_API NearCacheManager {
                public:
                    NearCacheManager(serialization::pimpl::SerializationService &ss)
                            : serializationService(ss) {
                    }

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
                    hazelcast::util::SharedPtr<NearCache<KS, V> > getNearCache(const std::string &name) {
//                        return boost::static_pointer_cast<NearCache<KS, V> >(nearCacheMap.get(name));
                        throw 1;
                    };

                    /**
                     * Creates a new {@link NearCache} with given configurations or returns existing one.
                     *
                     * Triggers the pre-loading of the created {@link NearCache} via the supplied {@link DataStructureAdapter}.
                     *
                     * @param name                 the name of the {@link NearCache}
                     *                             to be created or existing one
                     * @param nearCacheConfig      the {@link NearCacheConfig} of the {@link NearCache} to be created
                     * @param dataStructureAdapter the {@link DataStructureAdapter} of the {@link NearCache} to be created
                     * @param <K>           the key type of the {@link NearCache}
                     * @param <KS>          the key type of the underlying cache store (this is usually Data)
                     * @param <V>                  the value type of the {@link NearCache}
                     * @return the created or existing {@link NearCache} instance associated with given {@code name}
                     */
                    template<typename K, typename V, typename KS>
                    hazelcast::util::SharedPtr<NearCache<KS, V> > getOrCreateNearCache(
                            const std::string &name, const config::NearCacheConfig<K, V> &nearCacheConfig,
                            std::auto_ptr<adapter::DataStructureAdapter<K, V> > &dataStructureAdapter) {
                        hazelcast::util::SharedPtr<BaseNearCache> nearCache = nearCacheMap.get(name);
                        if (NULL == nearCache.get()) {
                            {
                                util::LockGuard guard(mutex);
                                nearCache = nearCacheMap.get(name);
                                if (NULL == nearCache.get()) {
//                                    nearCache = createNearCache<K, V, KS>(name, nearCacheConfig);
                                    nearCache->initialize();

                                    nearCacheMap.put(name, nearCache);
                                }

                            }
                        }
//                        return boost::static_pointer_cast<NearCache<KS, V> >(nearCache);
                        throw 1;
                    }

                    /**
                     * Lists all existing {@link NearCache} instances.
                     *
                     * @return all existing {@link NearCache} instances
                     */
/*
                    virtual std::vector<hazelcast::util::SharedPtr<NearCache<K, V> > > listAllNearCaches() const = 0;
*/

                    /**
                     * Clears {@link NearCache} instance associated with given {@code name} but not removes it.
                     *
                     * @param name name of the {@link NearCache} to be cleared
                     * @return {@code true} if {@link NearCache} was found and cleared, {@code false} otherwise
                     */
                    bool clearNearCache(const std::string &name) {
                        hazelcast::util::SharedPtr<BaseNearCache> nearCache = nearCacheMap.get(name);
                        if (nearCache.get() != NULL) {
                            nearCache->clear();
                        }
                        return nearCache.get() != NULL;
                    }

                    /**
                     * Clears all defined {@link NearCache} instances.
                     */
                    void clearAllNearCaches() {
                        std::vector<hazelcast::util::SharedPtr<BaseNearCache> > caches = nearCacheMap.values();
                        for (std::vector<hazelcast::util::SharedPtr<BaseNearCache> >::iterator it = caches.begin();
                             it != caches.end(); ++it) {
                            (*it)->clear();
                        }
                    }

                    /**
                     * Destroys {@link NearCache} instance associated with given {@code name} and also removes it.
                     *
                     * @param name name of the {@link NearCache} to be destroyed
                     * @return {@code true} if {@link NearCache} was found and destroyed, {@code false} otherwise
                     */
                    bool destroyNearCache(const std::string &name) {
                        hazelcast::util::SharedPtr<BaseNearCache> nearCache = nearCacheMap.remove(name);
                        if (nearCache.get() != NULL) {
                            nearCache->destroy();
                        }
                        return nearCache.get() != NULL;
                    }

                    /**
                     * Destroys all defined {@link NearCache} instances.
                     */
                    void destroyAllNearCaches() {
                        std::vector<hazelcast::util::SharedPtr<BaseNearCache> > caches = nearCacheMap.values();
                        for (std::vector<hazelcast::util::SharedPtr<BaseNearCache> >::iterator it = caches.begin();
                             it != caches.end(); ++it) {
                            (*it)->destroy();
                        }
                    }
                protected:
                    template<typename K, typename V, typename KS>
                    std::auto_ptr<NearCache<KS, V> > createNearCache(
                            const std::string &name, const config::NearCacheConfig<K, V> &nearCacheConfig) {
                        return std::auto_ptr<NearCache<KS, V> >(
                                new impl::DefaultNearCache<K, V, KS>(
                                        name, nearCacheConfig, serializationService));
                    }
                private:
                    serialization::pimpl::SerializationService &serializationService;
                    util::SynchronizedMap<std::string, BaseNearCache> nearCacheMap;
                    util::Mutex mutex;
                };
            }
        }
    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHEMANAGER_H_ */
