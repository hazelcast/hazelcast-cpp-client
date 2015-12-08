/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API DataEntryView {


            public:
                DataEntryView(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, long cost,
                              long creationTime, long expirationTime, long hits, long lastAccessTime,
                              long lastStoredTime, long lastUpdateTime, long version, long evictionCriteriaNumber,
                              long ttl);

                const serialization::pimpl::Data &getKey() const;

                const serialization::pimpl::Data &getValue() const;

                long getCost() const;

                long getCreationTime() const;

                long getExpirationTime() const;

                long getHits() const;

                long getLastAccessTime() const;

                long getLastStoredTime() const;

                long getLastUpdateTime() const;

                long getVersion() const;

                long getEvictionCriteriaNumber() const;

                long getTtl() const;

            private:
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
                long cost;
                long creationTime;
                long expirationTime;
                long hits;
                long lastAccessTime;
                long lastStoredTime;
                long lastUpdateTime;
                long version;
                long evictionCriteriaNumber;
                long ttl;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_DataEntryView

