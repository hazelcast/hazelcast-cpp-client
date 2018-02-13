/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include <string>
#include <assert.h>

#include <boost/shared_ptr.hpp>

#include "hazelcast/client/config/InMemoryFormat.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Clearable.h"
#include "hazelcast/util/Destroyable.h"
#include "hazelcast/client/spi/InitializingObject.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace monitor {
            class NearCacheStats;
        }

        namespace internal {
            namespace adapter {
                template<typename K, typename V>
                class DataStructureAdapter;
            }

            namespace nearcache {
                class HAZELCAST_API BaseNearCache
                        : public spi::InitializingObject, public util::Clearable, public util::Destroyable {
                };

                /**
                 * {@link NearCache} is the contract point to store keys and values in underlying
                 * {@link com.hazelcast.cache.impl.nearcache.NearCacheRecordStore}.
                 *
                 * @param <K> the type of the key
                 * @param <V> the type of the value
                 *
                 * This is a no-op interface class. See InvalidationAwareNearCache for the actual implementation
                 */
                template<typename K, typename V>
                class NearCache : public BaseNearCache {
                public:
                    /**
                     * NULL Object
                     */
                    static boost::shared_ptr<V> NULL_OBJECT;

                    virtual ~NearCache() {
                    }

                    void initialize() {
                        assert(0);
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
                    virtual const std::string &getName() const {
                        assert(0);
                        return *(new std::string(""));
                    }

                    /**
                     * Gets the value associated with the given <code>key</code>.
                     *
                     * @param key the key of the requested value
                     * @return the value associated with the given <code>key</code>
                     */
                    virtual boost::shared_ptr<V> get(const boost::shared_ptr<K> &key) {
                        assert(0);
                        return boost::shared_ptr<V>();
                    }

                    /**
                     * Puts (associates) a value with the given <code>key</code>.
                     *
                     * @param key   the key of the value will be stored
                     * @param value the value will be stored
                     */
                    virtual void put(const boost::shared_ptr<K> &key, const boost::shared_ptr<V> &value) {
                        assert(0);
                    }

                    /**
                     * Puts (associates) a value with the given <code>key</code>.
                     *
                     * @param key   the key of the value will be stored
                     * @param value the value as Data which will be stored
                     */
                    virtual void put(const boost::shared_ptr<K> &key,
                                     const boost::shared_ptr<serialization::pimpl::Data> &value) {
                        assert(0);
                    }

                    /**
                     * Removes the value associated with the given <code>key</code>.
                     *
                     * @param key the key of the value will be removed
                     */
                    virtual bool remove(const boost::shared_ptr<K> &key) {
                        assert(0);
                        return false;
                    }

                    /**
                     * @return
                     */
                    virtual bool isInvalidatedOnChange() const {
                        assert(0);
                        return false;
                    }

                    /**
                     * Gets the {@link com.hazelcast.config.InMemoryFormat} of the storage for internal records.
                     *
                     * @return the {@link com.hazelcast.config.InMemoryFormat} of the storage for internal records
                     */
                    virtual const config::InMemoryFormat getInMemoryFormat() const {
                        assert(0);
                        return config::BINARY;
                    }

                    /**
                     * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store.
                     *
                     * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this store
                     */
                    virtual monitor::NearCacheStats &getNearCacheStats() = 0;

                    /**
                     * Gets the count of stored records.
                     *
                     * @return the count of stored records
                     */
                    virtual int size() const {
                        assert(0);
                        return -1;
                    }
                };

                template<typename K, typename V>
                boost::shared_ptr<V> NearCache<K, V>::NULL_OBJECT = boost::shared_ptr<V>(new V());
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_NEARCACHE_H_ */

