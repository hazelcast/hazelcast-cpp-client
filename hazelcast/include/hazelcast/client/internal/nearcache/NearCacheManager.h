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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHEMANAGER_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHEMANAGER_H_

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/client/internal/nearcache/impl/DefaultNearCache.h"
#include "hazelcast/client/internal/adapter/DataStructureAdapter.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

#include <string>
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            class NearCacheConfig;
        }

        namespace internal {
            namespace nearcache {
                /**
                 * {@link NearCacheManager} is the contract point to manage all existing {@link NearCache} instances.
                 */
                class HAZELCAST_API NearCacheManager {
                public:
                    NearCacheManager(serialization::pimpl::SerializationService &ss)
                            : serializationService(ss) {
                        // TODO
                        (void) serializationService;
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
                    template <typename K, typename V>
                    boost::shared_ptr<NearCache<K, V> > getNearCache(const std::string &name) {
                        // TODO
                        return boost::shared_ptr<NearCache<K, V> >();
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
                     * @param <CACHEKEY>           the key type of the {@link NearCache}
                     * @param <K>                  the key type of the {@link IMap}
                     * @param <V>                  the value type of the {@link NearCache}
                     * @return the created or existing {@link NearCache} instance associated with given {@code name}
                     */
                    template <typename CACHEKEY, typename K, typename V>
                    boost::shared_ptr<NearCache<CACHEKEY, V> > getOrCreateNearCache(
                            const std::string &name, const boost::shared_ptr<config::NearCacheConfig> &nearCacheConfig,
                            boost::shared_ptr<adapter::DataStructureAdapter<K, V> > &dataStructureAdapter) {
                        boost::shared_ptr<spi::InitializingObject> nearCache = nearCacheMap.get(name);
                        if (NULL == nearCache.get()) {
                            {
                                util::LockGuard guard(mutex);
                                nearCache = nearCacheMap.get(name);
                                if (NULL == nearCache.get()) {
                                    nearCache = createNearCache<CACHEKEY, V>(name, nearCacheConfig);
                                    nearCache->initialize();

                                    nearCacheMap.put(name, nearCache);

/*TODO
                                    NearCache<CACHEKEY, V> *nearCachePtr =
                                            (NearCache<CACHEKEY, V> *) nearCache.get();


                                    if (nearCachePtr->getPreloaderConfig()->isEnabled()) {
                                        createAndSchedulePreloadTask(nearCache, dataStructureAdapter);
                                        createAndScheduleStorageTask();
                                    }
*/
                                }

                            }
                        }
                        return boost::static_pointer_cast<NearCache<CACHEKEY, V> >(nearCache);
                    }

                    /**
                     * Lists all existing {@link NearCache} instances.
                     *
                     * @return all existing {@link NearCache} instances
                     */
/*
                    virtual std::vector<boost::shared_ptr<NearCache<K, V> > > listAllNearCaches() const = 0;
*/

                    /**
                     * Clears {@link NearCache} instance associated with given {@code name} but not removes it.
                     *
                     * @param name name of the {@link NearCache} to be cleared
                     * @return {@code true} if {@link NearCache} was found and cleared, {@code false} otherwise
                     */
                    bool clearNearCache(const std::string & name) {
                        //TODO
                        return false;
                    }

                    /**
                     * Clears all defined {@link NearCache} instances.
                     */
                    void clearAllNearCaches() {
                        //TODO
                    }

                    /**
                     * Destroys {@link NearCache} instance associated with given {@code name} and also removes it.
                     *
                     * @param name name of the {@link NearCache} to be destroyed
                     * @return {@code true} if {@link NearCache} was found and destroyed, {@code false} otherwise
                     */
                    bool destroyNearCache(const std::string & name) {
                        //TODO
                        return false;
                    }

                    /**
                     * Destroys all defined {@link NearCache} instances.
                     */
                    void destroyAllNearCaches() {
                        // TODO
                    }

                protected:
                    template <typename K, typename V>
                    std::auto_ptr<NearCache<K, V> > createNearCache(
                            const std::string &name, const boost::shared_ptr<config::NearCacheConfig> &nearCacheConfig) {
                        return std::auto_ptr<NearCache<K, V> >(
                                new impl::DefaultNearCache<K, V>(
                                        name, nearCacheConfig, serializationService));
                    }
                private:
                    serialization::pimpl::SerializationService &serializationService;
                    util::SynchronizedMap<std::string, spi::InitializingObject> nearCacheMap;
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
