//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/LifecycleEvent.h"


namespace hazelcast {
    namespace client {
        LifecycleEvent::LifecycleEvent(LifeCycleState state)
        : state(state) {

        };

        LifecycleEvent::LifeCycleState LifecycleEvent::getState() const {
            return state;
        };
    }
}
