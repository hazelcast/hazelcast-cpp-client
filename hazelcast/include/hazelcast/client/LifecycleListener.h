//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_LIFECYCLE_LISTENER
#define HAZELCAST_LIFECYCLE_LISTENER

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/LifecycleEvent.h"

namespace hazelcast {
    namespace client {
        /**
         * Listener object for listening lifecycle events of hazelcast instance
         *
         * @see LifecycleEvent
         * @see HazelcastClient::addLifecycleListener(LifecycleListener *lifecycleListener)
         *
         */
        class HAZELCAST_API LifecycleListener {
        public:
            /**
             * Called when instance's state changes
             * @param lifecycleEvent LifecycleEvent
             *
             */
            virtual void stateChanged(const LifecycleEvent &lifecycleEvent) = 0;

            virtual ~LifecycleListener(){

            };
        };
    }
}


#endif //HAZELCAST_LIFECYCLE_LISTENER

