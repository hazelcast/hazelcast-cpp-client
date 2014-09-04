//
// Created by sancar koyunlu on 04/09/14.
//

#include "hazelcast/client/MapEvent.h"

namespace hazelcast {
    namespace client {

        MapEvent::MapEvent(Member& member, EntryEventType eventType, const std::string& name, int numberOfEntriesAffected)
        : member(member)
        , eventType(eventType)
        , name(name)
        , numberOfEntriesAffected(numberOfEntriesAffected) {

        }

        Member MapEvent::getMember() const {
            return member;
        }

        EntryEventType MapEvent::getEventType() const {
            return eventType;
        }

        const std::string& MapEvent::getName() const {
            return name;
        }

        int MapEvent::getNumberOfEntriesAffected() const {
            return numberOfEntriesAffected;
        }
    }
}