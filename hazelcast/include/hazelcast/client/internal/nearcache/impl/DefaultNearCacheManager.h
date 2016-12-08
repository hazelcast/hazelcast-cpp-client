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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_DEFAULTNEARCACHEMANAGER_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_DEFAULTNEARCACHEMANAGER_H_

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }
        namespace internal {
            namespace nearcache {
                namespace impl {
                    class HAZELCAST_API DefaultNearCacheManager : public NearCacheManager {
                    public:
                        DefaultNearCacheManager(const serialization::pimpl::SerializationService &ss)
                                : serializationService(ss) {
                        // TODO
                            serializationService.getVersion();
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
                     * Creates a new {@link NearCache} with given configurations
                     * or returns existing one.
                     *
                     * @param name            the name of the {@link NearCache} to be created or existing one
                     * @param nearCacheConfig the {@link NearCacheConfig} of the {@link NearCache} to be created
                     * @param <K>             the key type of the {@link NearCache}
                     * @param <V>             the value type of the {@link NearCache}
                     * @return the created or existing {@link NearCache} instance associated with given {@code name}
                     */
                    template <typename K, typename V>
                    boost::shared_ptr<NearCache<K, V> > getOrCreateNearCache(
                            const std::string & name, const config::NearCacheConfig &nearCacheConfig) {
                    //TODO
                            return boost::shared_ptr<NearCache<K, V> >();
                    }

                    /**
                     * Creates a new {@link NearCache} with given configurations or returns existing one.
                     *
                     * Triggers the pre-loading of the created {@link NearCache} via the supplied {@link DataStructureAdapter}.
                     *
                     * @param name                 the name of the {@link NearCache}
                     *                             to be created or existing one
                     * @param nearCacheConfig      the {@link NearCacheConfig} of the {@link NearCache} to be created
                     * @param dataStructureAdapter the {@link DataStructureAdapter} of the {@link NearCache} to be created
                     * @param <K>                  the key type of the {@link NearCache}
                     * @param <V>                  the value type of the {@link NearCache}
                     * @return the created or existing {@link NearCache} instance associated with given {@code name}
                     */
                    template <typename KEYDATA, typename K, typename V>
                    boost::shared_ptr<NearCache<KEYDATA, V> > getOrCreateNearCache(
                                const std::string & name, const config::NearCacheConfig &nearCacheConfig,
                                boost::shared_ptr<adapter::DataStructureAdapter<K, V> > dataStructureAdapter) {
                            //TODO
                            return boost::shared_ptr<NearCache<KEYDATA, V> >();
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

                    private:
                        const serialization::pimpl::SerializationService &serializationService;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_DEFAULTNEARCACHEMANAGER_H_ */

