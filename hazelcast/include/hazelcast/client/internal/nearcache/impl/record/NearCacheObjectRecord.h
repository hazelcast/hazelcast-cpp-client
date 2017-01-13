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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_RECORD_NEARCACHEOBJECTRECORD_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_RECORD_NEARCACHEOBJECTRECORD_H_

#include <stdint.h>
#include <boost/shared_ptr.hpp>

#include "hazelcast/client/internal/nearcache/impl/record/AbstractNearCacheRecord.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace record {
                        /**
                         * Implementation of {@link NearCacheRecord} to store
                         * any object type without {@link com.hazelcast.nio.serialization.Data} type.
                         *
                         * @param <V> type of object instances to store.
                         */
                        template <typename V>
                        class NearCacheObjectRecord
                                : public AbstractNearCacheRecord<V> {
                        public:
                            NearCacheObjectRecord(const boost::shared_ptr<V> &value,
                                                int64_t creationTime, int64_t expiryTime)
                                    : AbstractNearCacheRecord<V>(value, creationTime, expiryTime) {
                            }
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_RECORD_NEARCACHEOBJECTRECORD_H_ */

