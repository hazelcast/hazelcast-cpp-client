//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LIFECYCLE_EVENT
#define HAZELCAST_LIFECYCLE_EVENT

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        /**
         * Event to be fired when lifecycle states are changed.
         *
         *      enum LifeCycleState {
         *          STARTING,
         *          STARTED,
         *          SHUTTING_DOWN,
         *          SHUTDOWN,
         *          CLIENT_CONNECTED,
         *          CLIENT_DISCONNECTED
         *      };
         *
         * @see HazelcastInstance#getLifecycleService()
         * @see LifecycleService#addLifecycleListener(LifecycleListener *lifecycleListener)
         */
        class HAZELCAST_API LifecycleEvent {
        public:
            /**
             * State enum.
             */
            enum LifeCycleState {
                STARTING,
                STARTED,
                SHUTTING_DOWN,
                SHUTDOWN,
                CLIENT_CONNECTED,
                CLIENT_DISCONNECTED
            };

            /**
             * Constructor
             */
            LifecycleEvent(LifeCycleState state);

            /**
             * @return State;
             */
            LifeCycleState getState() const;

        private:
            LifeCycleState state;
        };
    }
}


#endif //__LifecycleEvent_H_
