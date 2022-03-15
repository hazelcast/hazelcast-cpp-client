/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <memory>
#include <ostream>
#include "hazelcast/client/member.h"
#include "hazelcast/client/serialization/serialization.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {

        /**
         * Map Entry event.
         *
         * @param <K> key of the map entry
         * @param <V> value of the map entry
         * @see EntryListener
         * @see IMap#add_entry_listener(Listener, boolean)
         */
        class HAZELCAST_API entry_event {
        public:
            enum struct type {
                ADDED =  1,
                REMOVED = 1 << 1,
                UPDATED = 1 << 2,
                EVICTED = 1 << 3 ,
                EXPIRED = 1 << 4,
                EVICT_ALL = 1 << 5 ,
                CLEAR_ALL = 1 << 6 ,
                MERGED = 1 << 7 ,
                INVALIDATION = 1 << 8,
                LOADED = 1 << 9,
                ALL = 0xFF
            };

            entry_event(const std::string &name, member &&member, type event_type,
                        typed_data &&key, typed_data &&value, typed_data &&old_value, typed_data &&merging_value);

            /**
             *
             * Returns the key of the entry event
             *
             * @return the key
             */
            const typed_data &get_key() const;

            /**
             * Returns the old value of the entry event
             *
             * @return The older value for the entry
             */
            const typed_data &get_old_value() const;

            /**
             *
             * Returns the value of the entry event
             *
             * @return The value of for the entry
             */
            const typed_data &get_value() const;

            /**
            *
            * Returns the incoming merging value of the entry event.
            *
            * @return merging value
            */
            const typed_data &get_merging_value() const;

            /**
             * Returns the member fired this event.
             *
             * @return the member fired this event.
             */
            const member &get_member() const;

            /**
             * Return the event type
             *
             * @return event type
             */
            type get_event_type() const;

            /**
             * Returns the name of the map for this event.
             *
             * @return name of the map.
             */
            const std::string &get_name() const;

            friend std::ostream HAZELCAST_API &operator<<(std::ostream &os, const entry_event &event);

        protected:
            std::string name_;
            member member_;
            type event_type_;
            typed_data key_;
            typed_data value_;
            typed_data old_value_;
            typed_data merging_value_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


