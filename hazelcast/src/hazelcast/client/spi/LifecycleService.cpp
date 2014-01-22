//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/LifecycleEvent.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            LifecycleService::LifecycleService(ClientContext &clientContext)
            :clientContext(clientContext)
            , active(false) {
                std::set<LifecycleListener *> const &lifecycleListeners = clientContext.getClientConfig().getLifecycleListeners();
                listeners.insert(lifecycleListeners.begin(), lifecycleListeners.end());
                fireLifecycleEvent(LifecycleEvent::STARTING);
            };

            void LifecycleService::start() {
                active = true;
                fireLifecycleEvent(LifecycleEvent::STARTED);
            };

            void LifecycleService::addLifecycleListener(LifecycleListener *lifecycleListener) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                listeners.insert(lifecycleListener);
            };

            bool LifecycleService::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                return listeners.erase(lifecycleListener) == 1;
            };

            void LifecycleService::fireLifecycleEvent(const LifecycleEvent &lifecycleEvent) {
                boost::lock_guard<boost::mutex> lg(listenerLock);
                for (std::set<LifecycleListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    (*it)->stateChanged(lifecycleEvent);
                };
            };

            bool LifecycleService::isRunning() {
                return active;
            };

            void LifecycleService::shutdown() {
                active = false;
                boost::lock_guard<boost::mutex> lg(listenerLock);
                fireLifecycleEvent(LifecycleEvent::SHUTTING_DOWN);
                clientContext.getConnectionManager().shutdown();
                clientContext.getClusterService().stop();
                clientContext.getPartitionService().stop();
                fireLifecycleEvent(LifecycleEvent::SHUTDOWN);
            };
        }
    }
}