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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHE_H_

#include <stdexcept>
#include <climits>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "hazelcast/client/config/InMemoryFormat.h"
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarker.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/spi/InitializingObject.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            class NearCachePreloaderConfig;
            class InMemoryFormat;
        }

        namespace internal {
            namespace nearcache {
                /**
                 * {@link NearCache} is the contract point to store keys and values in underlying
                 * {@link com.hazelcast.cache.impl.nearcache.NearCacheRecordStore}.
                 *
                 * @param <K> the type of the key
                 * @param <V> the type of the value
                 */
                template <typename K, typename V, typename ADAPTER>
                class NearCache : public spi::InitializingObject {
                public:
                    /**
                     * NULL Object
                     */
                    static boost::shared_ptr<V> NULL_OBJECT;

                    NearCache(int partitionCount) : keyStateMarker(partitionCount) {
                    }

                    virtual ~NearCache() {
                    }

                    void initialize() {
                        //TODO
                    }

                    /**
                     * Default expiration task initial delay time as seconds
                     */
                    static const int DEFAULT_EXPIRATION_TASK_INITIAL_DELAY_IN_SECONDS = 5;

                    /**
                     * Default expiration task delay time as seconds
                     */
                    static const int DEFAULT_EXPIRATION_TASK_DELAY_IN_SECONDS = 5;

                    /**
                     * Gets the name of the <code>this</code> {@link com.hazelcast.cache.impl.nearcache.NearCache} instance.
                     *
                     * @return the name of the <code>this</code> {@link com.hazelcast.cache.impl.nearcache.NearCache} instance
                     */
                    const std::string &getName() const {
                        //TODO
                        return name;
                    }

                    /**
                     * Gets the value associated with the given <code>key</code>.
                     *
                     * @param key the key of the requested value
                     * @return the value associated with the given <code>key</code>
                     */
                    boost::shared_ptr<V> get(const K &key) {
                        //TODO
                        return boost::shared_ptr<V>();
                    }

                    /**
                     * Puts (associates) a value with the given <code>key</code>.
                     *
                     * @param key   the key of the value will be stored
                     * @param value the value will be stored
                     */
                    void put(const K &key, const boost::shared_ptr<V> &value) {
                        //TODO
                    }

                    /**
                     * Removes the value associated with the given <code>key</code>.
                     *
                     * @param key the key of the value will be removed
                     */
                    bool remove(const K &key) {
                        //TODO
                        keyStateMarker.tryRemove(key);
                        return false;
                    }

                    /**
                     * @return
                     */
                    bool isInvalidatedOnChange() const {
                        //TODO
                        return false;
                    }

                    /**
                     * Removes all stored values.
                     */
                    void clear() {
                    //TODO
                        keyStateMarker.init();
                    }

                    /**
                     * Clears the record store and destroys it.
                     */
                    void destroy() {
                        //TODO
                        keyStateMarker.init();
                    }

                    /**
                     * Gets the {@link com.hazelcast.config.InMemoryFormat} of the storage for internal records.
                     *
                     * @return the {@link com.hazelcast.config.InMemoryFormat} of the storage for internal records
                     */
                    const config::InMemoryFormat &getInMemoryFormat() const {
                        //TODO
                        return inMemoryFormat;
                    }

                    /**
                     * Gets the {@link com.hazelcast.config.NearCachePreloaderConfig} of this Near Cache.
                     *
                     * @return the {@link NearCachePreloaderConfig} of this Near Cache
                     */
                    const boost::shared_ptr<config::NearCachePreloaderConfig> getPreloaderConfig() const {
                        //TODO
                        return preloaderConfig;
                    }

                    /**
                     * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store.
                     *
                     * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store
                     */
/*
                        const NearCacheStats &getNearCacheStats() const;
*/

                    /**
                     * Selects the best candidate object to store from the given <code>candidates</code>.
                     *
                     * @param candidates the candidates from which the best candidate object will be selected.
                     * @return the best candidate object to store, selected from the given <code>candidates</code>.
                     */
/*
                    boost::shared_ptr<V> selectToSave(std::vector<boost::shared_ptr<V> > candidates) const;
*/

                    /**
                     * Gets the count of stored records.
                     *
                     * @return the count of stored records
                     */
                    int size() const {
                        //TODO
                        return -1;
                    }

                    /**
                     * Executes the Near Cache pre-loader on the given {@link DataStructureAdapter}.
                     */
                    void preload(const ADAPTER &adapter) {
                        //TODO
                    }

                    /**
                     * Stores the keys of the Near Cache.
                     */
                    void storeKeys() {
                        //TODO
                    }

                    /**
                     * Checks if the pre-loading of the Near Cache is done.
                     *
                     * @return {@code true} if the pre-loading is done, {@code false} otherwise.
                     */
                    bool isPreloadDone() {
                        //TODO
                        return false;
                    }

                    impl::KeyStateMarker<K> &getKeyStateMarker() {
                        return keyStateMarker;
                    }
                private:
                    impl::KeyStateMarker<K> keyStateMarker;
                    std::string name;
                    boost::shared_ptr<config::NearCachePreloaderConfig> preloaderConfig;
                    config::InMemoryFormat inMemoryFormat;
                };

                template <typename K, typename V, typename ADAPTER>
                boost::shared_ptr<V> NearCache<K, V, ADAPTER>::NULL_OBJECT = boost::shared_ptr<V>(new V());
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHE_H_ */

