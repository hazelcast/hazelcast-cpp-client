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
        enum struct HAZELCAST_API ItemEventType {
            ADDED = 1, REMOVED = 2
        };

        class HAZELCAST_API ItemEventBase {
        public:
            ItemEventBase(const std::string &name, const Member &member, const ItemEventType &eventType);

            virtual ~ItemEventBase();

            /**
             * Returns the member fired this event.
             *
             * @return the member fired this event.
             */
            const Member &getMember() const;

            /**
             * Return the event type
             *
             * @return event type ItemEventType
             */
            ItemEventType getEventType() const;

            /**
             * Returns the name of the collection for this event.
             *
             * @return name of the collection.
             */
            const std::string &getName() const;

        private:
            std::string name_;
            Member member_;
            ItemEventType eventType_;
        };

        /**
         *
         * @see Queue#addItemListener
         * @see List#addItemListener
         * @see Set#addItemListener
         */
        class HAZELCAST_API ItemEvent : public ItemEventBase {
        public:
            ItemEvent(const std::string &name, ItemEventType eventType, TypedData &&item, const Member &member)
            : ItemEventBase(name, member, eventType), item_(item) {}

            /**
             * @returns the item.
             */
            const TypedData &getItem() const {
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

