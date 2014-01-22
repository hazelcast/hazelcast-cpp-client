//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_LIFECYCLE_LISTENER
#define HAZELCAST_LIFECYCLE_LISTENER

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class LifecycleEvent;
        /**
         * Listener object for listening lifecycle events of hazelcast instance
         *
         * @see LifecycleEvent
         * @see HazelcastInstance#getLifecycleService()
         *
         */
        class HAZELCAST_API LifecycleListener {
        public:
            /**
             * Called when instance's state changes
             * @param event Lifecycle event
             *
             */
            virtual void stateChanged(const LifecycleEvent &lifecycleEvent) = 0;

            virtual ~LifecycleListener(){

            };
        };
    }
}


#endif //HAZELCAST_LIFECYCLE_LISTENER
