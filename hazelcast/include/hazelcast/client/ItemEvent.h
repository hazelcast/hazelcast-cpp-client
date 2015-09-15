//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

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

        /**
         * ItemEvent
         *
         * @param E type of item.
         * @see Queue#addItemListener
         * @see List#addItemListener
         * @see Set#addItemListener
         */
        template <typename E>
        class ItemEvent {
        public:
            /**
             * constructor
             */
            ItemEvent(const std::string &name, ItemEventType eventType, const E &item, const Member &member)
            : name(name)
            , member(member)
            , eventType(eventType)
            , item(item) {

            };


            /**
             * @returns the item.
             */
            const E &geItem() const {
                return item;
            };

            /**
             * Returns the member fired this event.
             *
             * @return the member fired this event.
             */
            Member getMember() const {
                return member;
            };

            /**
             * Return the event type
             *
             * @return event type ItemEventType
             */
            ItemEventType getEventType() const {
                return eventType;
            };

            /**
             * Returns the name of the collection for this event.
             *
             * @return name of the collection.
             */
            std::string getName() const {
                return name;
            };

        private:
            std::string name;
            Member member;
            ItemEventType eventType;
            E item;

        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ITEM_EVENT

