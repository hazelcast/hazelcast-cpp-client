//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LIFECYCLE_EVENT
#define HAZELCAST_LIFECYCLE_EVENT

namespace hazelcast {
    namespace client {
        namespace spi {
            class LifecycleEvent {
            public:
                enum LifeCycleState {
                    STARTING,
                    STARTED,
                    SHUTTING_DOWN,
                    SHUTDOWN,
                    MERGING,
                    MERGED
                };

                LifecycleEvent(LifeCycleState state);

                LifeCycleState getState() const;

            private:
                LifeCycleState state;
            };
        }
    }
}


#endif //__LifecycleEvent_H_
