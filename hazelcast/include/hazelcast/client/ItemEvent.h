//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_ITEM_EVENT
#define HAZELCAST_ITEM_EVENT

#include "hazelcast/client/Member.h"

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
            ItemEventType() {

            }

            /**
             * Constructor.
             */
            ItemEventType(Type value)
            :value(value) {

            }

            /**
             * cast to int.
             */
            operator int() const {
                return value;
            }

            /**
             * copy function.
             */
            void operator = (int i) {
                switch (i) {
                    case 1:
                        value = ADDED;
                        break;
                    case 2:
                        value = REMOVED;
                        break;
                }
            }

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
        class HAZELCAST_API ItemEvent {
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
             * @return event type
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
            E item;
            ItemEventType eventType;
            Member member;
            std::string name;

        };
    }
}

#endif //HAZELCAST_ITEM_EVENT
