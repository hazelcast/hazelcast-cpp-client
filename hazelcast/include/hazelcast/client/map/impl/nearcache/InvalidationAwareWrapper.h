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
#ifndef HAZELCAST_MAP_IMPL_NEARCACHE_INVALIDATIONAWAREWRAPPER_H_
#define HAZELCAST_MAP_IMPL_NEARCACHE_INVALIDATIONAWAREWRAPPER_H_

#include "hazelcast/client/internal/adapter/DataStructureAdapter.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/internal/nearcache/impl/KeyStateMarkerImpl.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            namespace impl {
                namespace nearcache {
                    /**
                     * Aware of invalidations and if an invalidation for a key comes before put to cache,
                     * cached value will be removed. See usages of this wrapper in proxies.
                     *
                     * @see KeyStateMarker
                     * @see com.hazelcast.map.impl.proxy.NearCachedMapProxyImpl#get
                     */
                    template<typename K, typename V>
                    class InvalidationAwareWrapper : public internal::nearcache::NearCache<K, V> {
                    public:
                        static boost::shared_ptr<internal::nearcache::NearCache<K, V> > asInvalidationAware(
                                boost::shared_ptr<internal::nearcache::NearCache<K, V> > nearCache,
                                int markerCount) {
                            return boost::shared_ptr<internal::nearcache::NearCache<K, V> >(
                                    new InvalidationAwareWrapper<K, V>(nearCache, markerCount));
                        }

                        InvalidationAwareWrapper(
                                boost::shared_ptr<internal::nearcache::NearCache<K, V> > cache,
                                int partitionCount)
                                : nearCache(cache),
                                  keyStateMarker(new internal::nearcache::impl::KeyStateMarkerImpl(partitionCount)) {
                        }

                        virtual ~InvalidationAwareWrapper() {
                        }

                        //@Override
                        void initialize() {
                            nearCache->initialize();
                        }

                        //@Override
                        const std::string &getName() const {
                            return nearCache->getName();
                        }

                        //@Override
                        boost::shared_ptr<V> get(const boost::shared_ptr<K> &key) {
                            return nearCache->get(key);
                        }

                        //@Override
                        void put(const boost::shared_ptr<K> &key, const boost::shared_ptr<V> &value) {
                            nearCache->put(key, value);
                        }

                        //@Override
                        void put(const boost::shared_ptr<K> &key,
                                 const boost::shared_ptr<serialization::pimpl::Data> &value) {
                            nearCache->put(key, value);
                        }

                        //@Override
                        bool remove(const boost::shared_ptr<K> &key) {
                            keyStateMarker->tryRemove(*key);
                            return nearCache->remove(key);
                        }

                        //@Override
                        bool isInvalidatedOnChange() const {
                            return nearCache->isInvalidatedOnChange();
                        }

                        //@Override
                        void clear() {
                            keyStateMarker->init();
                            nearCache->clear();
                        }

                        //@Override
                        void destroy() {
                            keyStateMarker->init();
                            nearCache->destroy();
                        }

                        //@Override
                        const config::InMemoryFormat getInMemoryFormat() const {
                            return nearCache->getInMemoryFormat();
                        }

                        //@Override
                        monitor::NearCacheStats &getNearCacheStats() {
                            return nearCache->getNearCacheStats();
                        }

                        //@Override
                        int size() const {
                            return nearCache->size();
                        }

                        KeyStateMarker *getKeyStateMarker() {
                            return keyStateMarker.get();
                        }
                    private:
                        boost::shared_ptr<internal::nearcache::NearCache<K, V> > nearCache;
                        std::auto_ptr<KeyStateMarker> keyStateMarker;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_MAP_IMPL_NEARCACHE_INVALIDATIONAWAREWRAPPER_H_ */

