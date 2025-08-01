/*
 * Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/internal/nearcache/impl/NearCacheRecordMap.h"
#include "hazelcast/client/internal/eviction/impl/strategy/sampling/SampleableEvictableStore.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace internal {
namespace nearcache {
namespace impl {
/**
 * Contract point for all record maps these supports entry sampling to be used
 * for storage in Near Cache.
 *
 * @param <K> type of the key
 * @param <V> type of the {@link NearCacheRecord} to be stored
 * @see NearCacheRecord
 * @see NearCacheRecordMap
 * @see
 * com.hazelcast.internal.eviction.impl.strategy.sampling.SampleableEvictableStore
 */
template<typename K, typename V, typename KS, typename VS>
class SampleableNearCacheRecordMap
  : public NearCacheRecordMap<K, V, KS, VS>
  , public eviction::impl::strategy::sampling::
      SampleableEvictableStore<K, V, KS, VS>
{};
} // namespace impl
} // namespace nearcache
} // namespace internal
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
