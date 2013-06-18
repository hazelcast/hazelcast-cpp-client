//
// Created by sancar koyunlu on 6/17/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "LifecycleService.h"
#include "../HazelcastClient.h"
#include "../ClientConfig.h"

namespace hazelcast {
    namespace client {

        namespace spi {

            LifecycleService::LifecycleService(HazelcastClient & hazelcastClient, ClientConfig& config)
            :hazelcastClient(hazelcastClient)
            , active(false) {
                std::set<spi::EventListener *> listeners = config.getListeners();
                if (!listeners.empty()) {
                    for (std::set<spi::EventListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                        LifecycleListener *listener = dynamic_cast<LifecycleListener *>(*it);
                        if (listener) {
                            addLifecycleListener(listener);
                        }
                    }
                }
                fireLifecycleEvent(LifecycleEvent::STARTING);
            };

            void LifecycleService::addLifecycleListener(LifecycleListener *lifecycleListener) {
                listenerLock.lock();
                listeners.insert(lifecycleListener);
                listenerLock.unlock();
            };

            bool LifecycleService::removeLifecycleListener(LifecycleListener *lifecycleListener) {
                listenerLock.lock();
                bool b = listeners.erase(lifecycleListener) == 1;
                listenerLock.unlock();
                return b;
            };

            void LifecycleService::fireLifecycleEvent(LifecycleEvent lifecycleEvent) {
                listenerLock.lock();
                for (std::set<LifecycleListener *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
                    (*it)->stateChanged(lifecycleEvent);
                };
                listenerLock.unlock();
            };

            void LifecycleService::setStarted() {
                active = true;
                fireLifecycleEvent(LifecycleEvent::STARTED);
            };

            bool LifecycleService::isRunning() {
                return active;
            };

            void LifecycleService::shutdown() {
                active = false;
                lifecycleLock.lock();
                fireLifecycleEvent(LifecycleEvent::SHUTTING_DOWN);
                hazelcastClient.shutdown();
                fireLifecycleEvent(LifecycleEvent::SHUTDOWN);
                lifecycleLock.unlock();
            };


        }
    }
}