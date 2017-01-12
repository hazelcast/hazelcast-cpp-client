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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_MAXSIZE_ENTRYCOUNTNEARCACHEMAXSIZECHECKER_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_MAXSIZE_ENTRYCOUNTNEARCACHEMAXSIZECHECKER_H_

#include <stdint.h>

#include "hazelcast/client/internal/nearcache/impl/NearCacheRecordMap.h"
#include "hazelcast/client/internal/eviction/MaxSizeChecker.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace maxsize {
                        /**
                         * Near Cache max-size policy implementation for {@link com.hazelcast.config.EvictionConfig.MaxSizePolicy#ENTRY_COUNT}.
                         * <p>
                         * Checks if the Near Cache size is reached to max-size or not.
                         *
                         * @see MaxSizeChecker
                         */
                        template<typename K, typename V, typename KS, typename R>
                        class EntryCountNearCacheMaxSizeChecker : public eviction::MaxSizeChecker {
                        public:
                            EntryCountNearCacheMaxSizeChecker(int size,
                                                              const store::HeapNearCacheRecordMap<K, V, KS, R> &recordMap) :
                                    nearCacheRecordMap(recordMap), maxSize(size) {
                            }

                            //@Override
                            bool isReachedToMaxSize() const {
                                return (int32_t) nearCacheRecordMap.size() >= maxSize;
                            }
                        private:
                            const store::HeapNearCacheRecordMap<K, V, KS, R> &nearCacheRecordMap;
                            int32_t maxSize;
                        };
                    }
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_MAXSIZE_ENTRYCOUNTNEARCACHEMAXSIZECHECKER_H_ */

