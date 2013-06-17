//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LIFECYCLE_SERVICE
#define HAZELCAST_LIFECYCLE_SERVICE

#include "LifecycleListener.h"
#include "Lock.h"
#include "LifecycleEvent.h"
#include <set>

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace spi {
            class LifecycleService {
            public:
                LifecycleService(HazelcastClient& hazelcastClient);

                void addLifecycleListener(LifecycleListener *lifecycleListener);

                bool removeLifecycleListener(LifecycleListener *lifecycleListener);

                void setStarted();

                bool isRunning();

                void shutdown();

            private:
                HazelcastClient& hazelcastClient;
                std::set<LifecycleListener *> listeners;
                util::Lock listenerLock;
                util::Lock lifecycleLock;
                volatile bool active;

                void fireLifecycleEvent(LifecycleEvent lifecycleEvent);

            };

        }
    }
}


#endif //__LifecycleService_H_
