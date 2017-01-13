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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_NEARCACHERESCORDSTORE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_NEARCACHERESCORDSTORE_H_

#include <assert.h>
#include <boost/shared_ptr.hpp>

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
            namespace nearcache {
                namespace impl {
                    /**
                     * {@link NearCacheRecordStore} is the contract point to store keys and values as
                     * {@link NearCacheRecord} internally and to serve them.
                     *
                     * @param <K> the type of the key
                     * @param <V> the type of the value
                     */
                    template<typename K, typename V>
                    class NearCacheRecordStore : public spi::InitializingObject {
                    public:
                        virtual ~NearCacheRecordStore() {
                        }

                        /**
                         * Gets the value associated with the given {@code key}.
                         *
                         * @param key the key from which to get the associated value.
                         * @return the value associated with the given {@code key}.
                         */
                        virtual boost::shared_ptr<V> get(const boost::shared_ptr<K> &key) {
                            assert(0);
                            return boost::shared_ptr<V>();
                        }

                        /**
                         * Puts (associates) a value with the given {@code key}.
                         *
                         * @param key   the key to which the given value will be associated.
                         * @param value the value that will be associated with the key.
                         */
                        virtual void put(const boost::shared_ptr<K> &key, const boost::shared_ptr<V> &value) {
                            assert(0);
                        }

                        /**
                         * Puts (associates) a value with the given {@code key}.
                         *
                         * @param key   the key to which the given value will be associated.
                         * @param value the value that will be associated with the key.
                         */
                        virtual void put(const boost::shared_ptr<K> &key,
                                         const boost::shared_ptr<serialization::pimpl::Data> &value) {
                            assert(0);
                        }

                        /**
                         * Removes the value associated with the given {@code key}.
                         *
                         * @param key the key from which the value will be removed.
                         * @return {@code true} if the value was removed, otherwise {@code false}.
                         */
                        virtual bool remove(const boost::shared_ptr<K> &key) {
                            assert(0);
                            return false;
                        }

                        /**
                         * Removes all stored values.
                         */
                        virtual void clear() {
                            assert(0);
                        }

                        /**
                         * Clears the record store and destroys it.
                         */
                        virtual void destroy() {
                            assert(0);
                        }

                        /**
                         * Get the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this record store.
                         *
                         * @return the {@link com.hazelcast.monitor.NearCacheStats} instance to monitor this record store.
                         */
                        virtual monitor::NearCacheStats &getNearCacheStats() = 0;

                        /**
                         * Selects the best candidate object to store from the given {@code candidates}.
                         *
                         * @param candidates the candidates from which the best candidate object will be selected.
                         * @return the best candidate object to store, selected from the given {@code candidates}.
                         */
                        virtual const boost::shared_ptr<V> selectToSave(const boost::shared_ptr<V> &value,
                                                                 const boost::shared_ptr<serialization::pimpl::Data> &valueData) const {
                            assert(0);
                            return boost::shared_ptr<V>();
                        }

                        /**
                         * Gets the number of stored records.
                         *
                         * @return the number of stored records.
                         */
                        virtual int size() const {
                            assert(0);
                            return -1;
                        }

                        /**
                         * Performs expiration and evicts expired records.
                         */
                        virtual void doExpiration() {
                            assert(0);
                        }

                        /**
                         * Does eviction as specified configuration {@link com.hazelcast.config.EvictionConfig}
                         * in {@link com.hazelcast.config.NearCacheConfig}.
                         */
                        virtual void doEvictionIfRequired() {
                            assert(0);
                        }

                        /**
                         * Does eviction as specified configuration {@link com.hazelcast.config.EvictionConfig}
                         * in {@link com.hazelcast.config.NearCacheConfig} regardless from the max-size policy.
                         */
                        virtual void doEviction() {
                            assert(0);
                        }

                        /**
                         * Persists the key set of the Near Cache.
                         */
                        virtual void storeKeys() {
                            assert(0);
                        }
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_NEARCACHERESCORDSTORE_H_ */

