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
//
// Created by sancar koyunlu on 5/23/13.

#pragma once
#include <stdint.h>

#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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

            /**
             * Constructor
             */
            EntryView(const K &key, const V &value, const map::DataEntryView& rhs)
            : key(key)
            , value(value)
            , cost (rhs.getCost())
            , creationTime (rhs.getCreationTime())
            , expirationTime (rhs.getExpirationTime())
            , hits (rhs.getHits())
            , lastAccessTime (rhs.getLastAccessTime())
            , lastStoredTime (rhs.getLastStoredTime())
            , lastUpdateTime (rhs.getLastUpdateTime())
            , version (rhs.getVersion()) {

            };
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


