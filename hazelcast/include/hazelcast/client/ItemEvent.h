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
// Created by sancar koyunlu on 6/21/13.


#ifndef HAZELCAST_ITEM_EVENT
#define HAZELCAST_ITEM_EVENT

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
        class HAZELCAST_API ItemEventType {
        public:
            /**
             * Type enum.
             */
            enum Type {
                ADDED = 1, REMOVED = 2
            };

            /**
             * type value.
             */
            Type value;

            /**
             * Constructor.
             */
            ItemEventType();

            /**
             * Constructor.
             */
            ItemEventType(Type value);

            /**
             * cast to int.
             */
            operator int() const;

            /**
             * copy function.
             */
            void operator = (int i);

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
            Member getMember() const;

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
            std::string getName() const;

        private:
            std::string name;
            Member member;
            ItemEventType eventType;
        };

        /**
         *
         *
         * @param E type of item.
         * @see Queue#addItemListener
         * @see List#addItemListener
         * @see Set#addItemListener
         */
        template <typename E>
        class ItemEvent : public ItemEventBase {
        public:
            /**
             * constructor
             */
            ItemEvent(const std::string &name, ItemEventType eventType, const E &item, const Member &member)
            : ItemEventBase(name, member, eventType)
            , item(item) {
            };

            /**
             * @returns the item.
             */
            const E &getItem() const {
                return item;
            };

        private:
            E item;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ITEM_EVENT

