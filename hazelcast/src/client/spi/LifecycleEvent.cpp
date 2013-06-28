//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "LifecycleEvent.h"


namespace hazelcast {
    namespace client {
        namespace spi {
            LifecycleEvent::LifecycleEvent(LifeCycleState state)
            : state(state) {

            };

            LifecycleEvent::LifeCycleState LifecycleEvent::getState() const {
                return state;
            };
        }
    }
}