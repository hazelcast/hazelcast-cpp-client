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

#include <stdint.h>

#include "hazelcast/client/map/DataEntryView.h"
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
        class EntryView{
        public:
            EntryView(K key, V value, map::DataEntryView rhs)
            : key(std::move(key))
            , value(std::move(value))
            , cost (rhs.getCost())
            , creationTime (rhs.getCreationTime())
            , expirationTime (rhs.getExpirationTime())
            , hits (rhs.getHits())
            , lastAccessTime (rhs.getLastAccessTime())
            , lastStoredTime (rhs.getLastStoredTime())
            , lastUpdateTime (rhs.getLastUpdateTime())
            , version (rhs.getVersion()) {};

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
            int64_t creationTime;
            /**
             * entry expiration time if ttl is defined.
             */
            int64_t expirationTime;
            /**
             * number of hits.
             */
            int64_t hits;
            /**
             * last access time
             */
            int64_t lastAccessTime;
            /**
             * last stored time.
             */
            int64_t lastStoredTime;
            /**
             * last update time.
             */
            int64_t lastUpdateTime;
            /**
             * version.
             */
            int64_t version;
        };
    }
}


