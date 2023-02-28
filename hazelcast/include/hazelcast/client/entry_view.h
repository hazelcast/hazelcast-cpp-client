/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <stdint.h>

#include "hazelcast/client/map/data_entry_view.h"
#include "hazelcast/client/serialization/serialization.h"

namespace hazelcast {
namespace client {
/**
 * EntryView represents a readonly view of a map entry.
 *
 * @param <K> key
 * @param <V> value
 */
template<typename K, typename V>
class entry_view
{
public:
    entry_view(K key,
               V value,
               int64_t cost,
               int64_t creationTime,
               int64_t expirationTime,
               int64_t hits,
               int64_t lastAccessTime,
               int64_t lastStoredTime,
               int64_t lastUpdateTime,
               int64_t version)
      : key(std::move(key))
      , value(std::move(value))
      , cost(cost)
      , creation_time(creationTime)
      , expiration_time(expirationTime)
      , hits(hits)
      , last_access_time(lastAccessTime)
      , last_stored_time(lastStoredTime)
      , last_update_time(lastUpdateTime)
      , version(version)
    {}

    /**
     * key
     */
    K key;
    /**
     * value
     */
    V value;
    /**
     * memory cost of entry
     */
    int64_t cost;
    /**
     * entry creation time
     */
    int64_t creation_time;
    /**
     * entry expiration time if ttl is defined.
     */
    int64_t expiration_time;
    /**
     * number of hits.
     */
    int64_t hits;
    /**
     * last access time
     */
    int64_t last_access_time;
    /**
     * last stored time.
     */
    int64_t last_stored_time;
    /**
     * last update time.
     */
    int64_t last_update_time;
    /**
     * version.
     */
    int64_t version;
};
} // namespace client
} // namespace hazelcast
