//
// Created by sancar koyunlu on 04/09/14.
//

#include "hazelcast/client/IMapEvent.h"

namespace hazelcast {
    namespace client {

        IMapEvent::IMapEvent(Member& member, EntryEventType eventType, const std::string& name, int numberOfEntriesAffected)
        : member(member)
        , eventType(eventType)
        , name(name)
        , numberOfEntriesAffected(numberOfEntriesAffected) {

        }

        Member IMapEvent::getMember() const {
            return member;
        }

        EntryEventType IMapEvent::getEventType() const {
            return eventType;
        }

        const std::string& IMapEvent::getName() const {
            return name;
        }

        int IMapEvent::getNumberOfEntriesAffected() const {
            return numberOfEntriesAffected;
        }
    }
}