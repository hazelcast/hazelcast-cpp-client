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
#include "hazelcast/util/export.h"
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
                        static std::shared_ptr<internal::nearcache::NearCache<K, V> > as_invalidation_aware(
                                std::shared_ptr<internal::nearcache::NearCache<K, V> > near_cache,
                                int marker_count) {
                            return std::shared_ptr<internal::nearcache::NearCache<K, V> >(
                                    new InvalidationAwareWrapper<K, V>(near_cache, marker_count));
                        }

                        InvalidationAwareWrapper(
                                std::shared_ptr<internal::nearcache::NearCache<K, V> > cache,
                                int partition_count)
                                : near_cache_(cache),
                                  key_state_marker_(new internal::nearcache::impl::KeyStateMarkerImpl(partition_count)) {
                        }

                        ~InvalidationAwareWrapper() override = default;

                        void initialize() override {
                            near_cache_->initialize();
                        }

                        const std::string &get_name() const override {
                            return near_cache_->get_name();
                        }

                        std::shared_ptr<V> get(const std::shared_ptr<K> &key) override {
                            return near_cache_->get(key);
                        }

                        void put(const std::shared_ptr<K> &key, const std::shared_ptr<V> &value) override {
                            near_cache_->put(key, value);
                        }

/*
                        //@Override
                        void put(const std::shared_ptr<K> &key,
                                 const std::shared_ptr<serialization::pimpl::data> &value) {
                            nearCache->put(key, value);
                        }
*/

                        bool invalidate(const std::shared_ptr<K> &key) override {
                            key_state_marker_->try_remove(*key);
                            return near_cache_->invalidate(key);
                        }

                        bool is_invalidated_on_change() const override {
                            return near_cache_->is_invalidated_on_change();
                        }

                        void clear() override {
                            key_state_marker_->init();
                            near_cache_->clear();
                        }

                        void destroy() override {
                            key_state_marker_->init();
                            near_cache_->destroy();
                        }

                        const config::in_memory_format get_in_memory_format() const override {
                            return near_cache_->get_in_memory_format();
                        }

                        std::shared_ptr<monitor::near_cache_stats> get_near_cache_stats() const override {
                            return near_cache_->get_near_cache_stats();
                        }

                        int size() const override {
                            return near_cache_->size();
                        }

                        KeyStateMarker *get_key_state_marker() {
                            return key_state_marker_.get();
                        }
                    private:
                        std::shared_ptr<internal::nearcache::NearCache<K, V> > near_cache_;
                        std::unique_ptr<KeyStateMarker> key_state_marker_;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



