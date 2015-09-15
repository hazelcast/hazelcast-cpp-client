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
         *
         * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
         * otherwise it will slow down the system.
         *
         * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
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

            virtual ~LifecycleListener();
        };
    }
}


#endif //HAZELCAST_LIFECYCLE_LISTENER

