//
// Created by sancar koyunlu on 8/14/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "DistributedObjectEvent.h"

namespace hazelcast {
    namespace client {
        DistributedObjectEvent::DistributedObjectEvent(DistributedObjectEvent::EventType eventType, const std::string& objectId)
        : eventType (eventType)
        , objectId(objectId) {

        }

        DistributedObjectEvent::EventType DistributedObjectEvent::getEventType() const{
            return eventType;
        }

        std::string DistributedObjectEvent::getObjectId() const{
            return objectId;
        }


    }
}