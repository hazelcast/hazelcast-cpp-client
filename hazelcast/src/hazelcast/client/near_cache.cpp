/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

NearCacheManager::NearCacheManager(const std::shared_ptr<spi::impl::ClientExecutionServiceImpl>& es,
                                   serialization::pimpl::SerializationService& ss,
                                   logger& lg)
  : execution_service_(es)
  , serialization_service_(ss)
  , logger_(lg)
{}

bool
NearCacheManager::clear_near_cache(const std::string& name)
{
    std::shared_ptr<BaseNearCache> nearCache = near_cache_map_.get(name);
    if (nearCache.get() != NULL) {
        nearCache->clear();
    }
    return nearCache.get() != NULL;
}

void
NearCacheManager::clear_all_near_caches()
{
    std::vector<std::shared_ptr<BaseNearCache>> caches = near_cache_map_.values();
    for (std::vector<std::shared_ptr<BaseNearCache>>::iterator it = caches.begin();
         it != caches.end();
         ++it) {
        (*it)->clear();
    }
}

bool
NearCacheManager::destroy_near_cache(const std::string& name)
{
    std::shared_ptr<BaseNearCache> nearCache = near_cache_map_.remove(name);
    if (nearCache.get() != NULL) {
        nearCache->destroy();
    }
    return nearCache.get() != NULL;
}

void
NearCacheManager::destroy_all_near_caches()
{
    std::vector<std::shared_ptr<BaseNearCache>> caches = near_cache_map_.values();
    for (std::vector<std::shared_ptr<BaseNearCache>>::iterator it = caches.begin();
         it != caches.end();
         ++it) {
        (*it)->destroy();
    }
}

std::vector<std::shared_ptr<BaseNearCache>>
NearCacheManager::list_all_near_caches()
{
    return near_cache_map_.values();
}

namespace impl {
namespace record {
NearCacheDataRecord::NearCacheDataRecord(
  const std::shared_ptr<serialization::pimpl::data>& data_value,
  int64_t create_time,
  int64_t expiry_time)
  : AbstractNearCacheRecord<serialization::pimpl::data>(data_value, create_time, expiry_time)
{}
} // namespace record

KeyStateMarkerImpl::KeyStateMarkerImpl(int count)
  : mark_count_(count)
  , marks_(new std::atomic<int32_t>[count])
{
    for (int i = 0; i < count; ++i) {
        marks_[i] = 0;
    }
}

KeyStateMarkerImpl::~KeyStateMarkerImpl()
{
    delete[] marks_;
}

bool
KeyStateMarkerImpl::try_mark(const serialization::pimpl::data& key)
{
    return cas_state(key, UNMARKED, MARKED);
}

bool
KeyStateMarkerImpl::try_unmark(const serialization::pimpl::data& key)
{
    return cas_state(key, MARKED, UNMARKED);
}

bool
KeyStateMarkerImpl::try_remove(const serialization::pimpl::data& key)
{
    return cas_state(key, MARKED, REMOVED);
}

void
KeyStateMarkerImpl::force_unmark(const serialization::pimpl::data& key)
{
    int slot = get_slot(key);
    marks_[slot] = UNMARKED;
}

void
KeyStateMarkerImpl::init()
{
    for (int i = 0; i < mark_count_; ++i) {
        marks_[i] = UNMARKED;
    }
}

bool
KeyStateMarkerImpl::cas_state(const serialization::pimpl::data& key, state expect, state update)
{
    int slot = get_slot(key);
    int expected = expect;
    return marks_[slot].compare_exchange_strong(expected, update);
}

int
KeyStateMarkerImpl::get_slot(const serialization::pimpl::data& key)
{
    return util::HashUtil::hash_to_index(key.get_partition_hash(), mark_count_);
}

} // namespace impl
} // namespace nearcache

namespace eviction {
bool
EvictAlways::is_eviction_required() const
{
    // Evict always at any case
    return true;
}

const std::unique_ptr<EvictionChecker> EvictionChecker::EVICT_ALWAYS =
  std::unique_ptr<EvictionChecker>(new EvictAlways());
} // namespace eviction
} // namespace internal

namespace map {
namespace impl {
namespace nearcache {
bool
TrueMarkerImpl::try_mark(const serialization::pimpl::data& key)
{
    return true;
}

bool
TrueMarkerImpl::try_unmark(const serialization::pimpl::data& key)
{
    return true;
}

bool
TrueMarkerImpl::try_remove(const serialization::pimpl::data& key)
{
    return true;
}

void
TrueMarkerImpl::force_unmark(const serialization::pimpl::data& key)
{}

void
TrueMarkerImpl::init()
{}

const std::unique_ptr<KeyStateMarker> KeyStateMarker::TRUE_MARKER =
  std::unique_ptr<KeyStateMarker>(new TrueMarkerImpl());
} // namespace nearcache
} // namespace impl
} // namespace map
} // namespace client
} // namespace hazelcast
