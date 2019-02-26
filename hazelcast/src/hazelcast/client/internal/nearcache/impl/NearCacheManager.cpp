/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                NearCacheManager::NearCacheManager(serialization::pimpl::SerializationService &ss,
                                                   util::ILogger &logger)
                        : serializationService(ss), logger(logger) {
                }

                bool NearCacheManager::clearNearCache(const std::string &name) {
                    boost::shared_ptr<BaseNearCache> nearCache = nearCacheMap.get(name);
                    if (nearCache.get() != NULL) {
                        nearCache->clear();
                    }
                    return nearCache.get() != NULL;
                }

                void NearCacheManager::clearAllNearCaches() {
                    std::vector<boost::shared_ptr<BaseNearCache> > caches = nearCacheMap.values();
                    for (std::vector<boost::shared_ptr<BaseNearCache> >::iterator it = caches.begin();
                         it != caches.end(); ++it) {
                        (*it)->clear();
                    }
                }

                bool NearCacheManager::destroyNearCache(const std::string &name) {
                    boost::shared_ptr<BaseNearCache> nearCache = nearCacheMap.remove(name);
                    if (nearCache.get() != NULL) {
                        nearCache->destroy();
                    }
                    return nearCache.get() != NULL;
                }

                void NearCacheManager::destroyAllNearCaches() {
                    std::vector<boost::shared_ptr<BaseNearCache> > caches = nearCacheMap.values();
                    for (std::vector<boost::shared_ptr<BaseNearCache> >::iterator it = caches.begin();
                         it != caches.end(); ++it) {
                        (*it)->destroy();
                    }
                }

                std::vector<boost::shared_ptr<BaseNearCache> > NearCacheManager::listAllNearCaches() {
                    return nearCacheMap.values();
                }

            }
        }
    }
}

