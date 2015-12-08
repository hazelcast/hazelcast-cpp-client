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

#include "hazelcast/client/map/DataEntryView.h"

namespace hazelcast {
    namespace client {
        namespace map{
            DataEntryView::DataEntryView(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, long cost,
                          long creationTime, long expirationTime, long hits, long lastAccessTime,
                          long lastStoredTime, long lastUpdateTime, long version, long evictionCriteriaNumber,
                          long ttl) : key(key), value(value), cost(cost), creationTime(creationTime),
                                      expirationTime(expirationTime), hits(hits), lastAccessTime(lastAccessTime),
                                      lastStoredTime(lastStoredTime), lastUpdateTime(lastUpdateTime),
                                      version(version), evictionCriteriaNumber(evictionCriteriaNumber), ttl(ttl) { }


            const serialization::pimpl::Data &DataEntryView::getKey() const {
                return key;
            }

            const serialization::pimpl::Data &DataEntryView::getValue() const {
                return value;
            }

            long DataEntryView::getCost() const {
                return cost;
            }

            long DataEntryView::getCreationTime() const {
                return creationTime;
            }

            long DataEntryView::getExpirationTime() const {
                return expirationTime;
            }

            long DataEntryView::getHits() const {
                return hits;
            }

            long DataEntryView::getLastAccessTime() const {
                return lastAccessTime;
            }

            long DataEntryView::getLastStoredTime() const {
                return lastStoredTime;
            }

            long DataEntryView::getLastUpdateTime() const {
                return lastUpdateTime;
            }

            long DataEntryView::getVersion() const {
                return version;
            }

            long DataEntryView::getEvictionCriteriaNumber() const {
                return evictionCriteriaNumber;
            }

            long DataEntryView::getTtl() const {
                return ttl;
            }

        }
    }
}

