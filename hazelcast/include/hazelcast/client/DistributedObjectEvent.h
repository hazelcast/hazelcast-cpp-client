//
// Created by sancar koyunlu on 8/14/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DistributedObjectEvent
#define HAZELCAST_DistributedObjectEvent

#include <string>

namespace hazelcast {
    namespace client {
        /**
         * DistributedObjectEvent is fired when a {@link DistributedObject}
         * is created or destroyed cluster-wide.
         *
         * @see DistributedObject
         * @see DistributedObjectListener
         */
        class DistributedObjectEvent {
        public:
            enum EventType {
                CREATED, DESTROYED
            };

            DistributedObjectEvent(EventType eventType, const std::string& objectId);

            /**
             * Returns type of this event; one of {@link EventType#CREATED} or {@link EventType#DESTROYED}
             *
             * @return eventType
             */
            EventType getEventType() const;

            /**
             * Returns identifier of related DistributedObject
             *
             * @return identifier of DistributedObject
             */
            std::string getObjectId() const;

            /**
             * Returns DistributedObject instance
             *
             * @return DistributedObject
             */
            template<typename DistributedObject>
            DistributedObject getDistributedObject() const {

            };
        private:
            EventType eventType;
            std::string objectId;

        };

    }
}


#endif //HAZELCAST_DistributedObjectEvent
