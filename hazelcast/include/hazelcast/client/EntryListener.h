/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 04/09/14.
//


#ifndef HAZELCAST_EntryListener
#define HAZELCAST_EntryListener

#include "hazelcast/client/EntryEvent.h"

namespace hazelcast {
    namespace client {
        class MapEvent;

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
        template<typename K, typename V>
        class EntryListener {
        public:
            virtual ~EntryListener() {

            }

            /**
            * Invoked when an entry is added.
            *
            * @param event entry event
            */
            virtual void entryAdded(const EntryEvent<K, V>& event) = 0;

            /**
            * Invoked when an entry is removed.
            *
            * @param event entry event
            */
            virtual void entryRemoved(const EntryEvent<K, V>& event) = 0;

            /**
            * Invoked when an entry is removed.
            *
            * @param event entry event
            */
            virtual void entryUpdated(const EntryEvent<K, V>& event) = 0;

            /**
            * Invoked when an entry is evicted.
            *
            * @param event entry event
            */
            virtual void entryEvicted(const EntryEvent<K, V>& event) = 0;

            /**
            * Invoked upon expiration of an entry.
            *
            * @param event the event invoked when an entry is expired.
            */
            virtual void entryExpired(const EntryEvent<K, V>& event) = 0;

            /**
            *  Invoked after WAN replicated entry is merged.
            *
            * @param event the event invoked when an entry is expired.
            */
            virtual void entryMerged(const EntryEvent<K, V>& event) = 0;

            /**
            * Invoked when all entries evicted by {@link IMap#evictAll()}.
            *
            * @param event map event
            */
            virtual void mapEvicted(const MapEvent& event) = 0;

            /**
            * Invoked when all entries are removed by {@link IMap#clear()}.}
            */
            virtual void mapCleared(const MapEvent& event) = 0;

        };
    }
}


#endif //HAZELCAST_EntryListener
