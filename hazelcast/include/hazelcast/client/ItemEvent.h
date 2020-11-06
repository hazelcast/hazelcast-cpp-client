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

#include "hazelcast/client/Member.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
        * Type of item event.
        */
        enum struct HAZELCAST_API item_event_type {
            ADDED = 1, REMOVED = 2
        };

        class HAZELCAST_API ItemEventBase {
        public:
            ItemEventBase(const std::string &name, const Member &member, const item_event_type &event_type);

            virtual ~ItemEventBase();

            /**
             * Returns the member fired this event.
             *
             * @return the member fired this event.
             */
            const Member &get_member() const;

            /**
             * Return the event type
             *
             * @return event type ItemEventType
             */
            item_event_type get_event_type() const;

            /**
             * Returns the name of the collection for this event.
             *
             * @return name of the collection.
             */
            const std::string &get_name() const;

        private:
            std::string name_;
            Member member_;
            item_event_type event_type_;
        };

        /**
         *
         * @see Queue#add_item_listener
         * @see List#add_item_listener
         * @see Set#add_item_listener
         */
        class HAZELCAST_API ItemEvent : public ItemEventBase {
        public:
            ItemEvent(const std::string &name, item_event_type event_type, TypedData &&item, const Member &member)
            : ItemEventBase(name, member, event_type), item_(item) {}

            /**
             * @returns the item.
             */
            const TypedData &get_item() const {
                return item_;
            }

        private:
            TypedData item_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

