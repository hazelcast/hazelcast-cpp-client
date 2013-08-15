//
// Created by sancar koyunlu on 8/14/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_DistributedObjectListener
#define HAZELCAST_DistributedObjectListener

namespace hazelcast {
    namespace client {

        class DistributedObjectEvent;

        /**
         * DistributedObjectListener allows to get notified when a {@link DistributedObject}
         * is created or destroyed cluster-wide.
         *
         * @see DistributedObject
         * @see HazelcastInstance#addDistributedObjectListener(DistributedObjectListener)
         */
        class DistributedObjectListener {
        public:
            /**
             * Invoked when a DistributedObject is created.
             *
             * @param event event
             */
            virtual void distributedObjectCreated(const DistributedObjectEvent& event) = 0;

            /**
             * Invoked when a DistributedObject is destroyed.
             *
             * @param event event
             */
            virtual void distributedObjectDestroyed(const DistributedObjectEvent& event) = 0;
        };

    }
}


#endif //HAZELCAST_DistributedObjectListener
