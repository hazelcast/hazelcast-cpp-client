//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_LIFECYCLE_LISTENER
#define HAZELCAST_LIFECYCLE_LISTENER

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            class LifecycleEvent;

            class HAZELCAST_API LifecycleListener{
            public:
                virtual void stateChanged(const LifecycleEvent& lifecycleEvent) = 0;

                virtual ~LifecycleListener() = 0;
            };

            inline LifecycleListener::~LifecycleListener() {

            };
        }

    }
}


#endif //HAZELCAST_LIFECYCLE_LISTENER
