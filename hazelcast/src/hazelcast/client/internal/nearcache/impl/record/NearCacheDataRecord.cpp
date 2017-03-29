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
#include "hazelcast/client/internal/nearcache/impl/record/NearCacheDataRecord.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace record {
                        NearCacheDataRecord::NearCacheDataRecord(
                                const hazelcast::util::SharedPtr<serialization::pimpl::Data> &dataValue,
                                int64_t createTime, int64_t expiryTime)
                                : AbstractNearCacheRecord<serialization::pimpl::Data>(dataValue,
                                                                                      createTime,
                                                                                      expiryTime) {
                        }
                    }
                }
            }
        }
    }
}
