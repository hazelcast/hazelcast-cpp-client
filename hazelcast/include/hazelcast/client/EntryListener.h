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

#include <functional>

namespace hazelcast {
    namespace client {
        class MapEvent;
        class EntryEvent;

        /**
        * Map Entry listener to get notified when a map entry
        * is added, removed, updated or evicted.  Events will fire as a result
        * of operations carried out via the IMap.
        *
        * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
        * otherwise it will slow down the system.
        *
        * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
        *
        * @param <K> key of the map entry
        * @param <V> value of the map entry.
        * @see IMap#addEntryListener(EntryListener, boolean)
        */
        class EntryListener final {
        public:
            template<typename EntryAddedHandlerT, 
                     typename EntryRemovedHandlerT,
                     typename EntryUpdatedHandlerT,
                     typename EntryEvictedHandlerT,
                     typename EntryExpiredHandlerT,
                     typename EntryMergedHandlerT,
                     typename MapEvictedHandlerT,
                     typename MapClearedHandlerT>
            explicit EntryListener(EntryAddedHandlerT entryAdded, 
                        EntryRemovedHandlerT entryRemoved,
                        EntryUpdatedHandlerT entryUpdated,
                        EntryEvictedHandlerT entryEvicted,
                        EntryExpiredHandlerT entryExpired,
                        EntryMergedHandlerT entryMerged,
                        MapEvictedHandlerT mapEvicted,
                        MapClearedHandlerT mapCleared)
                    : entryAdded_(entryAdded),
                      entryRemoved_(entryRemoved),
                      entryEvicted_(entryEvicted),
                      entryExpired_(entryExpired),
                      entryMerged_(entryMerged),
                      mapEvicted_(mapEvicted),
                      mapCleared_(mapCleared) {}

            /**
            * Invoked when an entry is added.
            *
            * @param event entry event
            */
            void entryAdded(const EntryEvent &event) {
                entryAdded_(event);
            }

            /**
            * Invoked when an entry is removed.
            *
            * @param event entry event
            */
            void entryRemoved(const EntryEvent &event) {
                entryRemoved_(event);
            }

            /**
            * Invoked when an entry is removed.
            *
            * @param event entry event
            */
            void entryUpdated(const EntryEvent &event) {
                entryUpdated_(event);
            }

            /**
            * Invoked when an entry is evicted.
            *
            * @param event entry event
            */
            void entryEvicted(const EntryEvent &event) {
                entryEvicted_(event);
            }

            /**
            * Invoked upon expiration of an entry.
            *
            * @param event the event invoked when an entry is expired.
            */
            void entryExpired(const EntryEvent &event) {
                entryExpired_(event);
            }

            /**
            *  Invoked after WAN replicated entry is merged.
            *
            * @param event the event invoked when an entry is expired.
            */
            void entryMerged(const EntryEvent &event) {
                entryMerged_(event);
            }

            /**
            * Invoked when all entries evicted by {@link IMap#evictAll()}.
            *
            * @param event map event
            */
            void mapEvicted(const MapEvent &event) {
                mapEvicted_(event);
            }

            /**
            * Invoked when all entries are removed by {@link IMap#clear()}.}
            */
            void mapCleared(const MapEvent &event) {
                mapCleared_(event);
            }
        private:
            using EntryHandlerType = std::function<void(const EntryEvent &)>;
            using MapHandlerType = std::function<void(const MapEvent &)>;

            EntryHandlerType entryAdded_, entryRemoved_, entryUpdated_, entryEvicted_, 
                            entryExpired_, entryMerged_;

            MapHandlerType mapEvicted_, mapCleared_;
        };
    }
}



