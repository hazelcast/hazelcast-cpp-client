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

#include "hazelcast/client/internal/nearcache/impl/KeyStateMarkerImpl.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/util/HashUtil.h"
#include "hazelcast/client/internal/eviction/EvictionChecker.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                template class NearCache<serialization::pimpl::Data, serialization::pimpl::Data>;
		
                NearCacheManager::NearCacheManager(const std::shared_ptr<spi::impl::ClientExecutionServiceImpl> &es,
                                                   serialization::pimpl::SerializationService &ss,
                                                   util::ILogger &logger)
                        : executionService(es), serializationService(ss), logger(logger) {
                }

                bool NearCacheManager::clearNearCache(const std::string &name) {
                    std::shared_ptr<BaseNearCache> nearCache = nearCacheMap.get(name);
                    if (nearCache.get() != NULL) {
                        nearCache->clear();
                    }
                    return nearCache.get() != NULL;
                }

                void NearCacheManager::clearAllNearCaches() {
                    std::vector<std::shared_ptr<BaseNearCache> > caches = nearCacheMap.values();
                    for (std::vector<std::shared_ptr<BaseNearCache> >::iterator it = caches.begin();
                         it != caches.end(); ++it) {
                        (*it)->clear();
                    }
                }

                bool NearCacheManager::destroyNearCache(const std::string &name) {
                    std::shared_ptr<BaseNearCache> nearCache = nearCacheMap.remove(name);
                    if (nearCache.get() != NULL) {
                        nearCache->destroy();
                    }
                    return nearCache.get() != NULL;
                }

                void NearCacheManager::destroyAllNearCaches() {
                    std::vector<std::shared_ptr<BaseNearCache> > caches = nearCacheMap.values();
                    for (std::vector<std::shared_ptr<BaseNearCache> >::iterator it = caches.begin();
                         it != caches.end(); ++it) {
                        (*it)->destroy();
                    }
                }

                std::vector<std::shared_ptr<BaseNearCache> > NearCacheManager::listAllNearCaches() {
                    return nearCacheMap.values();
                }

                namespace impl {
                    namespace record {
                        NearCacheDataRecord::NearCacheDataRecord(
                                const std::shared_ptr<serialization::pimpl::Data> &dataValue,
                                int64_t createTime, int64_t expiryTime)
                                : AbstractNearCacheRecord<serialization::pimpl::Data>(dataValue,
                                                                                      createTime,
                                                                                      expiryTime) {
                        }
                    }

                    KeyStateMarkerImpl::KeyStateMarkerImpl(int count) : markCount(count),
                                                                        marks(new std::atomic<int32_t>[count]) {
                        for (int i = 0; i < count; ++i) {
                            marks[i] = 0;
                        }
                    }

                    KeyStateMarkerImpl::~KeyStateMarkerImpl() {
                        delete[] marks;
                    }

                    bool KeyStateMarkerImpl::tryMark(const serialization::pimpl::Data &key) {
                        return casState(key, UNMARKED, MARKED);
                    }

                    bool KeyStateMarkerImpl::tryUnmark(const serialization::pimpl::Data &key) {
                        return casState(key, MARKED, UNMARKED);
                    }

                    bool KeyStateMarkerImpl::tryRemove(const serialization::pimpl::Data &key) {
                        return casState(key, MARKED, REMOVED);
                    }

                    void KeyStateMarkerImpl::forceUnmark(const serialization::pimpl::Data &key) {
                        int slot = getSlot(key);
                        marks[slot] = UNMARKED;
                    }

                    void KeyStateMarkerImpl::init() {
                        for (int i = 0; i < markCount; ++i) {
                            marks[i] = UNMARKED;
                        }
                    }

                    bool
                    KeyStateMarkerImpl::casState(const serialization::pimpl::Data &key, STATE expect, STATE update) {
                        int slot = getSlot(key);
                        int expected = expect;
                        return marks[slot].compare_exchange_strong(expected, update);
                    }

                    int KeyStateMarkerImpl::getSlot(const serialization::pimpl::Data &key) {
                        return util::HashUtil::hashToIndex(key.getPartitionHash(), markCount);
                    }

                }
            }

            namespace eviction {
                bool EvictAlways::isEvictionRequired() const {
                    // Evict always at any case
                    return true;
                }

                const std::unique_ptr<EvictionChecker> EvictionChecker::EVICT_ALWAYS = std::unique_ptr<EvictionChecker>(
                        new EvictAlways());
            }
        }

        namespace map {
            namespace impl {
                namespace nearcache {
                    bool TrueMarkerImpl::tryMark(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    bool TrueMarkerImpl::tryUnmark(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    bool TrueMarkerImpl::tryRemove(const serialization::pimpl::Data &key) {
                        return true;
                    }

                    void TrueMarkerImpl::forceUnmark(const serialization::pimpl::Data &key) {
                    }

                    void TrueMarkerImpl::init() {
                    }

                    const std::unique_ptr<KeyStateMarker> KeyStateMarker::TRUE_MARKER =
                            std::unique_ptr<KeyStateMarker>(new TrueMarkerImpl());
                }
            }
        }
    }
}

