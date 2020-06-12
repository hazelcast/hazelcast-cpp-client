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
                        static std::shared_ptr<internal::nearcache::NearCache<K, V> > asInvalidationAware(
                                std::shared_ptr<internal::nearcache::NearCache<K, V> > nearCache,
                                int markerCount) {
                            return std::shared_ptr<internal::nearcache::NearCache<K, V> >(
                                    new InvalidationAwareWrapper<K, V>(nearCache, markerCount));
                        }

                        InvalidationAwareWrapper(
                                std::shared_ptr<internal::nearcache::NearCache<K, V> > cache,
                                int partitionCount)
                                : nearCache(cache),
                                  keyStateMarker(new internal::nearcache::impl::KeyStateMarkerImpl(partitionCount)) {
                        }

                        virtual ~InvalidationAwareWrapper() {
                        }

                        //@Override
                        void initialize() override {
                            nearCache->initialize();
                        }

                        //@Override
                        const std::string &getName() const override {
                            return nearCache->getName();
                        }

                        //@Override
                        std::shared_ptr<V> get(const std::shared_ptr<K> &key) override {
                            return nearCache->get(key);
                        }

                        //@Override
                        void put(const std::shared_ptr<K> &key, const std::shared_ptr<V> &value) override {
                            nearCache->put(key, value);
                        }

/*
                        //@Override
                        void put(const std::shared_ptr<K> &key,
                                 const std::shared_ptr<serialization::pimpl::Data> &value) {
                            nearCache->put(key, value);
                        }
*/

                        //@Override
                        bool invalidate(const std::shared_ptr<K> &key) override {
                            keyStateMarker->tryRemove(*key);
                            return nearCache->invalidate(key);
                        }

                        //@Override
                        bool isInvalidatedOnChange() const override {
                            return nearCache->isInvalidatedOnChange();
                        }

                        //@Override
                        void clear() override {
                            keyStateMarker->init();
                            nearCache->clear();
                        }

                        //@Override
                        void destroy() override {
                            keyStateMarker->init();
                            nearCache->destroy();
                        }

                        //@Override
                        const config::InMemoryFormat getInMemoryFormat() const override {
                            return nearCache->getInMemoryFormat();
                        }

                        std::shared_ptr<monitor::NearCacheStats> getNearCacheStats() const override {
                            return nearCache->getNearCacheStats();
                        }

                        //@Override
                        int size() const override {
                            return nearCache->size();
                        }

                        KeyStateMarker *getKeyStateMarker() {
                            return keyStateMarker.get();
                        }
                    private:
                        std::shared_ptr<internal::nearcache::NearCache<K, V> > nearCache;
                        std::unique_ptr<KeyStateMarker> keyStateMarker;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


