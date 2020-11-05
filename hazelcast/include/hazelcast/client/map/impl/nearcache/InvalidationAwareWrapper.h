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
                                : nearCache_(cache),
                                  keyStateMarker_(new internal::nearcache::impl::KeyStateMarkerImpl(partitionCount)) {
                        }

                        ~InvalidationAwareWrapper() override = default;

                        void initialize() override {
                            nearCache_->initialize();
                        }

                        const std::string &getName() const override {
                            return nearCache_->getName();
                        }

                        std::shared_ptr<V> get(const std::shared_ptr<K> &key) override {
                            return nearCache_->get(key);
                        }

                        void put(const std::shared_ptr<K> &key, const std::shared_ptr<V> &value) override {
                            nearCache_->put(key, value);
                        }

/*
                        //@Override
                        void put(const std::shared_ptr<K> &key,
                                 const std::shared_ptr<serialization::pimpl::Data> &value) {
                            nearCache->put(key, value);
                        }
*/

                        bool invalidate(const std::shared_ptr<K> &key) override {
                            keyStateMarker_->tryRemove(*key);
                            return nearCache_->invalidate(key);
                        }

                        bool isInvalidatedOnChange() const override {
                            return nearCache_->isInvalidatedOnChange();
                        }

                        void clear() override {
                            keyStateMarker_->init();
                            nearCache_->clear();
                        }

                        void destroy() override {
                            keyStateMarker_->init();
                            nearCache_->destroy();
                        }

                        const config::InMemoryFormat getInMemoryFormat() const override {
                            return nearCache_->getInMemoryFormat();
                        }

                        std::shared_ptr<monitor::NearCacheStats> getNearCacheStats() const override {
                            return nearCache_->getNearCacheStats();
                        }

                        int size() const override {
                            return nearCache_->size();
                        }

                        KeyStateMarker *getKeyStateMarker() {
                            return keyStateMarker_.get();
                        }
                    private:
                        std::shared_ptr<internal::nearcache::NearCache<K, V> > nearCache_;
                        std::unique_ptr<KeyStateMarker> keyStateMarker_;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



