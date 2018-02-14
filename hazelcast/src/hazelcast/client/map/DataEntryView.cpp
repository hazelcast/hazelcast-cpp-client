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

#include "hazelcast/client/map/DataEntryView.h"

namespace hazelcast {
    namespace client {
        namespace map{
            DataEntryView::DataEntryView(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, int64_t cost,
                          int64_t creationTime, int64_t expirationTime, int64_t hits, int64_t lastAccessTime,
                          int64_t lastStoredTime, int64_t lastUpdateTime, int64_t version, int64_t evictionCriteriaNumber,
                          int64_t ttl) : key(key), value(value), cost(cost), creationTime(creationTime),
                                      expirationTime(expirationTime), hits(hits), lastAccessTime(lastAccessTime),
                                      lastStoredTime(lastStoredTime), lastUpdateTime(lastUpdateTime),
                                      version(version), evictionCriteriaNumber(evictionCriteriaNumber), ttl(ttl) { }


            const serialization::pimpl::Data &DataEntryView::getKey() const {
                return key;
            }

            const serialization::pimpl::Data &DataEntryView::getValue() const {
                return value;
            }

            int64_t DataEntryView::getCost() const {
                return cost;
            }

            int64_t DataEntryView::getCreationTime() const {
                return creationTime;
            }

            int64_t DataEntryView::getExpirationTime() const {
                return expirationTime;
            }

            int64_t DataEntryView::getHits() const {
                return hits;
            }

            int64_t DataEntryView::getLastAccessTime() const {
                return lastAccessTime;
            }

            int64_t DataEntryView::getLastStoredTime() const {
                return lastStoredTime;
            }

            int64_t DataEntryView::getLastUpdateTime() const {
                return lastUpdateTime;
            }

            int64_t DataEntryView::getVersion() const {
                return version;
            }

            int64_t DataEntryView::getEvictionCriteriaNumber() const {
                return evictionCriteriaNumber;
            }

            int64_t DataEntryView::getTtl() const {
                return ttl;
            }

        }
    }
}

