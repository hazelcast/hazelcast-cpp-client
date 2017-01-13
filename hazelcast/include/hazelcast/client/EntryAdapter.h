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
#ifndef HAZELCAST_EntryAdapter
#define HAZELCAST_EntryAdapter

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
        template<typename K, typename V>
        class EntryAdapter : public EntryListener<K, V> {
        public:
            virtual ~EntryAdapter() {

            }

            virtual void entryAdded(const EntryEvent<K, V>& event) {

            }

            virtual void entryRemoved(const EntryEvent<K, V>& event) {

            }

            virtual void entryUpdated(const EntryEvent<K, V>& event) {

            }

            virtual void entryEvicted(const EntryEvent<K, V>& event) {

            }


            virtual void entryExpired(const EntryEvent<K, V>& event){

            }

            virtual void entryMerged(const EntryEvent<K, V>& event){

            }

            virtual void mapEvicted(const MapEvent& event) {

            }

            virtual void mapCleared(const MapEvent& event) {

            }
        };
    }
}


#endif //HAZELCAST_EntryAdapter
