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
    entry_view(K key, V value, map::data_entry_view rhs)
      : key(std::move(key))
      , value(std::move(value))
      , cost(rhs.get_cost())
      , creation_time(rhs.get_creation_time())
      , expiration_time(rhs.get_expiration_time())
      , hits(rhs.get_hits())
      , last_access_time(rhs.get_last_access_time())
      , last_stored_time(rhs.get_last_stored_time())
      , last_update_time(rhs.get_last_update_time())
      , version(rhs.get_version()){};

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
