//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_LIFECYCLE_SERVICE
#define HAZELCAST_LIFECYCLE_SERVICE

#include "LifecycleListener.h"
#include "LifecycleEvent.h"
#include <set>
#include <boost/thread/mutex.hpp>

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        class ClientConfig;
        namespace spi {
            class LifecycleService {
            public:
                LifecycleService(HazelcastClient& hazelcastClient, ClientConfig& config);

                void addLifecycleListener(LifecycleListener *lifecycleListener);

                bool removeLifecycleListener(LifecycleListener *lifecycleListener);

                void setStarted();

                bool isRunning();

                void shutdown();

            private:
                HazelcastClient& hazelcastClient;
                std::set<LifecycleListener *> listeners;
                boost::mutex listenerLock;
                boost::mutex lifecycleLock;
                volatile bool active;

                void fireLifecycleEvent(LifecycleEvent lifecycleEvent);

            };

        }
    }
}


#endif //__LifecycleService_H_
