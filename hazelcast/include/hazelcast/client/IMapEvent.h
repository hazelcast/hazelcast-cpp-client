//
// Created by sancar koyunlu on 04/09/14.
//


#ifndef HAZELCAST_MapEvent
#define HAZELCAST_MapEvent

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/EntryEvent.h"
#include <string>

namespace hazelcast {
    namespace client {
        /**
        * Map events common contract.
        */
        class HAZELCAST_API IMapEvent {
        public:
            /**
            * Constructor
            */
            IMapEvent(Member& member, EntryEventType eventType, const std::string& name, int numberOfEntriesAffected);

            /**
            * Returns the member fired this event.
            *
            * @return the member fired this event.
            */
            Member getMember() const;

            /**
            * Return the event type
            *
            * @return event type
            */
            EntryEventType getEventType() const;

            /**
            * Returns the name of the map for this event.
            *
            * @return name of the map.
            */
            const std::string& getName() const;

            /**
            * Returns the number of entries affected by this event.
            *
            * @return number of entries affected.
            */
            int getNumberOfEntriesAffected() const;

        private:
            Member member;
            EntryEventType eventType;
            std::string name;
            int numberOfEntriesAffected;
        };
    }
}

#endif //HAZELCAST_MapEvent
