//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LIFECYCLE_EVENT
#define HAZELCAST_LIFECYCLE_EVENT

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class HAZELCAST_API LifecycleEvent {
        public:
            enum LifeCycleState {
                STARTING,
                STARTED,
                SHUTTING_DOWN,
                SHUTDOWN,
                MERGING,
                MERGED,
                CLIENT_CONNECTED,
                CLIENT_DISCONNECTED
            };

            LifecycleEvent(LifeCycleState state);

            LifeCycleState getState() const;

        private:
            LifeCycleState state;
        };
    }
}


#endif //__LifecycleEvent_H_
