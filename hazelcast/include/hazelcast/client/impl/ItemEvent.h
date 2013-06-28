//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_ITEM_EVENT
#define HAZELCAST_ITEM_EVENT

#include "EventObject.h"
#include "../connection/Member.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            template <typename E>
            class ItemEvent : public EventObject {
            public:
                enum ItemEventType {
                    ADDED, REMOVED
                };

                ItemEvent(const std::string& name, ItemEventType eventType, const E& item, const connection::Member& member)
                : EventObject(name)
                , member(member)
                , eventType(eventType)
                , item(item) {

                };


                const E& geItem() const {
                    return item;
                };

                /**
                 * Returns the member fired this event.
                 *
                 * @return the member fired this event.
                 */
                connection::Member getMember() const {
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
                    return getSource();
                };

            private:
                E item;
                ItemEventType eventType;
                connection::Member member;

            };
        }
    }
}

#endif //HAZELCAST_ITEM_EVENT
