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
            enum Type {
                ADDED = 1, REMOVED = 2
            } value;

            ItemEventType() {

            }

            ItemEventType(Type value)
            :value(value) {

            }

            operator int() const {
                return value;
            }

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

        template <typename E>
        class HAZELCAST_API ItemEvent {
        public:

            ItemEvent(const std::string &name, ItemEventType eventType, const E &item, const Member &member)
            : name(name)
            , member(member)
            , eventType(eventType)
            , item(item) {

            };


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
             * Returns the name of the map for this event.
             *
             * @return name of the map.
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
