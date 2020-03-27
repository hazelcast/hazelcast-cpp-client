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
// Created by ihsan demir on 29 Feb 2016.

#ifndef HAZELCAST_CLIENT_ADAPTOR_ENTRYVIEW_H_
#define HAZELCAST_CLIENT_ADAPTOR_ENTRYVIEW_H_

#include <memory>

#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
         * MapEntryView represents a readonly view of a map entry.
         */
        template <typename K, typename V>
        class MapEntryView {
        public:
            /**
             * Constructor
             */
            MapEntryView(std::unique_ptr<map::DataEntryView> &v, serialization::pimpl::SerializationService &srv)
                    : dataView(std::move(v)), serializationService(srv) {
            }

            /**
             * @return Returns the deserialized key object for the entry, performs lazy deserialization.
             */
            std::unique_ptr<K> getKey() const {
                return serializationService.toObject<K>(dataView->getKey());
            }

            /**
             * @return Returns the deserialized value object for the entry, performs lazy deserialization.
             */
            std::unique_ptr<V> getValue() const {
                return serializationService.toObject<V>(dataView->getValue());
            }

            /**
             * @return Returns the cost for the map entry
             */
            long getCost() const {
                return dataView->getCost();
            }

            /**
             * @return Returns the map entry creation time
             */
            long getCreationTime() const {
                return dataView->getCreationTime();
            }

            /**
             * @return Returns the expiration time of the entry.
             */
            long getExpirationTime() const {
                return dataView->getExpirationTime();
            }

            /**
             * @return Returns the number of hits for the map entry
             */
            long getHits() const {
                return dataView->getHits();
            }

            /**
             * @return Returns the time that the map entry is last accessed
             */
            long getLastAccessTime() const {
                return dataView->getLastAccessTime();
            }

            /**
             * @return Returns the last that that the map entry is stored
             */
            long getLastStoredTime() const {
                return dataView->getLastStoredTime();
            }

            /**
             * @return Returns the time that the entry is last updated.
             */
            long getLastUpdateTime() const {
                return dataView->getLastAccessTime();
            }

            /**
             * @return Returns the version of the map entry.
             */
            long getVersion() const {
                return dataView->getVersion();
            }

        private:
            std::unique_ptr<map::DataEntryView> dataView;

            serialization::pimpl::SerializationService &serializationService;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_ADAPTOR_ENTRYVIEW_H_

