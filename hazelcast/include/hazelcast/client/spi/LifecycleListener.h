//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_LIFECYCLE_LISTENER
#define HAZELCAST_LIFECYCLE_LISTENER

#include "hazelcast/client/spi/EventListener.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            class LifecycleEvent;

            class LifecycleListener : public EventListener {
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
