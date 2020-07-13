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

#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/EntryListener.h"

namespace hazelcast {
    namespace client {
        class MapEvent;

        /**
        * Adapter for EntryListener.
        *
        * @param <K> key of the map entry
        * @param <V> value of the map entry.
        * @see EntryListener
        */
        class HAZELCAST_API EntryAdapter : public EntryListener {
        public:
            ~EntryAdapter() override = default;

            void entryAdded(const EntryEvent& event) override {}

            void entryRemoved(const EntryEvent& event) override {}

            void entryUpdated(const EntryEvent& event) override {}

            void entryEvicted(const EntryEvent& event) override {}

            void entryExpired(const EntryEvent& event) override {}

            void entryMerged(const EntryEvent& event) override {}

            void mapEvicted(const MapEvent& event) override {}

            void mapCleared(const MapEvent& event) override {}
        };
    }
}
