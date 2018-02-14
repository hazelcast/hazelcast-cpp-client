/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 20/02/14.
//

#ifndef HAZELCAST_DataEntryView
#define HAZELCAST_DataEntryView

#include "hazelcast/client/serialization/pimpl/Data.h"

#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API DataEntryView {


            public:
                DataEntryView(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, int64_t cost,
                              int64_t creationTime, int64_t expirationTime, int64_t hits, int64_t lastAccessTime,
                              int64_t lastStoredTime, int64_t lastUpdateTime, int64_t version, int64_t evictionCriteriaNumber,
                              int64_t ttl);

                const serialization::pimpl::Data &getKey() const;

                const serialization::pimpl::Data &getValue() const;

                int64_t getCost() const;

                int64_t getCreationTime() const;

                int64_t getExpirationTime() const;

                int64_t getHits() const;

                int64_t getLastAccessTime() const;

                int64_t getLastStoredTime() const;

                int64_t getLastUpdateTime() const;

                int64_t getVersion() const;

                int64_t getEvictionCriteriaNumber() const;

                int64_t getTtl() const;

            private:
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
                int64_t cost;
                int64_t creationTime;
                int64_t expirationTime;
                int64_t hits;
                int64_t lastAccessTime;
                int64_t lastStoredTime;
                int64_t lastUpdateTime;
                int64_t version;
                int64_t evictionCriteriaNumber;
                int64_t ttl;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DataEntryView

